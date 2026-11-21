/*
 * OPTXT.EXE: the opening text. play_opening_text loads the seven TIM groups
 * of \ST\OPTXT\OPTXT.MIC into VRAM (six text pages and the picture parts 2
 * and 3 copy from (640, 256)), starts \ST\OPTXT\OPTXT.XA (file 1, channel 0)
 * and scrolls the pages up the screen in three parts, fading each pair in and
 * out; START ends it early. main then returns to PSX.EXE with program index 3
 * (ST.EXE) in the next-program byte.
 *
 * clear_vram, cd_read_sectors, load_st_file and draw_fade are END.EXE's
 * functions; load_tims here also loads each TIM's CLUT. The pad functions
 * drive one DualShock-capable pad in port 1 through LIBPAD.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBETC.H>
#include <LIBCD.H>
#include <LIBSND.H>
#include <LIBPAD.H>
#include <psyq/libc.h>
/* opening_state: another object defines it, so the assembler does not know
 * it is small data here and every access is an absolute lui pair. */
#include <optxt/state.h>

/* PSX.EXE's next-program byte and the mailbox byte after it (see src/psx/main.c). */
#define NEXT_PROGRAM_BYTE ((u8 *)0x800103f0)
#define MAILBOX_BYTE_1 ((u8 *)0x800103f1)
/* PSX.EXE's program table index of ST.EXE. */
#define PROGRAM_ST 3

/* TIM file ID word and the flag bit of a TIM with a CLUT block. */
#define TIM_ID 0x10
#define TIM_CLUT 0x8

/* Raw (inverted) pad button bit of START. */
#define PAD_RAW_START 0x8

typedef struct {
    DISPENV disp[2];
    DRAWENV draw[2];
    int id;
} DisplayBuffers;

/* A TIM block header: its byte length (header included) and VRAM area. */
typedef struct {
    u_long bytes;
    RECT rect;
} TimBlock;

typedef struct {
    short x, y;
} VramOffset;

/*
 * PadInitDirect receive buffers and the actuator data PadSetAct reads; LIBPAD's
 * interrupt handler fills them, so every access is a real load or store.
 */
typedef struct {
    volatile u_char buf[2][34];
    volatile u_char act[2][4];
} PadBuffers;

/*
 * The first three flags are volatile: retail reloads each of them after its
 * own store and never moves their stores into a delay slot.
 */
typedef struct {
    volatile u_char state;        /* last PadGetState */
    volatile u_char disconnected; /* set on PadStateDiscon, cleared once found again */
    volatile u_char found;        /* set on PadStateFindPad */
    u_char actuators;             /* PadInfoAct(0, -1, 0) of a stable pad */
    u_char lost;                  /* no input is read while set */
    int trigger;                  /* buttons newly pressed this frame */
    int previous;                 /* buttons of the previous frame */
    int buttons;                  /* buttons held (raw bit order) */
} PadStatus;

static void load_tims(u_long *addr, VramOffset *pixel_offset, VramOffset *clut_offset,
                      int absolute);
static void clear_vram(void);
static void cd_read_sectors(CdlLOC *loc, u_long *buf, int sectors);
static int load_st_file(u_long *buf, char *name, int size);
static void play_opening_xa(void);
static void pad_wait_stable(void);
static void pad_set_act_align(void);
static void pad_check_actuators(void);
static void pad_update_state(void);
static void pad_init(void);
static void pad_read(void);
static void draw_fade(int level);
static void draw_text_page(int page, int y, int level);
static void scroll_pages_1(void);
static void scroll_pages_2(void);
static void scroll_pages_3(void);
static void play_opening_text(void);

RODATA(0x80011000, 0xe8)
JUMPTABLES(0x800110e8, 0x1c)
SDATA(0x8001f7a4, 0x1c)

/* VRAM x of each text page's texture page (the CLUTs sit at (0, 480)). */
DATA(0x8001df50, 0x18, ".data")
VramOffset page_tpage[6] = {
    {320, 0}, {384, 0}, {448, 0}, {512, 0}, {576, 0}, {640, 0},
};

DATA(0x8001f798, 0x4, ".sdata")
u8 *next_program = NEXT_PROGRAM_BYTE;
DATA(0x8001f79c, 0x4, ".sdata")
u8 *PTR_8001f79c = MAILBOX_BYTE_1;
DATA(0x8001f7a0, 0x4, ".sdata")
VramOffset clut_origin = {0, 480};

DATA(0x8001f7e0, 0x6, ".sbss")
static u_char act_align[6];
DATA(0x8001f7e8, 0x4, ".sbss")
static int frame;

/*
 * Exported COMMON: PSYLINK allocates these after the fixed .bss in symbol
 * hash-bucket order, so their spellings are WIP names that keep retail order.
 */
DATA(0x8001f800, 0x190000, ".bss")
u_long text_buffer[0x190000 / 4];
DATA(0x801af800, 0xe4, ".bss")
DisplayBuffers display;
DATA(0x801af8e8, 0x4c, ".bss")
PadBuffers pad_data;
DATA(0x801af938, 0x14, ".bss")
PadStatus pad;

/* Loads a run of TIM files (CLUT and pixel blocks) into VRAM. */
ADDRESS(0x80011f8c, 0x1dc)
static void load_tims(u_long *addr, VramOffset *pixel_offset, VramOffset *clut_offset,
                      int absolute)
{
    TimBlock block;
    RECT rect;
    VramOffset no_clut_offset;
    VramOffset no_pixel_offset;
    VramOffset *cofs;
    VramOffset *pofs;
    u_long *p;

    memset(&no_clut_offset, 0, sizeof(no_clut_offset));
    memset(&no_pixel_offset, 0, sizeof(no_pixel_offset));
    cofs = &no_clut_offset;
    pofs = &no_pixel_offset;
    if (clut_offset)
        cofs = clut_offset;
    if (pixel_offset)
        pofs = pixel_offset;
    p = addr;
    for (;;) {
        u_long bytes;

        if (*p != TIM_ID)
            break;
        p++;
        if (*p++ & TIM_CLUT) {
            /* The header is copied out of the file as three words. */
            ((u_long *)&block)[0] = p[0];
            ((u_long *)&block)[1] = p[1];
            ((u_long *)&block)[2] = p[2];
            p += 3;
            if (!absolute) {
                rect.x = block.rect.x + cofs->x;
                rect.y = block.rect.y + cofs->y;
            } else {
                rect.x = cofs->x;
                rect.y = cofs->y;
            }
            rect.w = block.rect.w;
            rect.h = block.rect.h;
            LoadImage(&rect, p);
            bytes = block.bytes - sizeof(TimBlock);
            p = (u_long *)((u_char *)p + bytes);
        }
        ((u_long *)&block)[0] = p[0];
        ((u_long *)&block)[1] = p[1];
        ((u_long *)&block)[2] = p[2];
        p += 3;
        if (!absolute) {
            rect.x = block.rect.x + pofs->x;
            rect.y = block.rect.y + pofs->y;
        } else {
            rect.x = pofs->x;
            rect.y = pofs->y;
        }
        rect.w = block.rect.w;
        rect.h = block.rect.h;
        LoadImage(&rect, p);
        bytes = block.bytes - sizeof(TimBlock);
        p = (u_long *)((u_char *)p + bytes);
    }
}

ADDRESS(0x80012168, 0x54)
static void clear_vram(void)
{
    RECT rect = {0, 0, 1024, 512};

    ClearImage(&rect, 0, 0, 0);
}

ADDRESS(0x800121bc, 0xd4)
static void cd_read_sectors(CdlLOC *loc, u_long *buf, int sectors)
{
    u_char param[8];
    int remaining;

    param[0] = CdlModeSpeed;
    CdControlB(CdlSetmode, param, 0);
    VSync(4);
    for (;;) {
        while (CdControl(CdlSetloc, (u_char *)loc, 0) == 0)
            ;
        if (CdRead(sectors, buf, CdlModeSpeed) == 0) {
            CdSync(0, param);
            continue;
        }
        while ((remaining = CdReadSync(1, param)) > 0)
            ;
        CdControl(CdlPause, 0, 0);
        if (remaining == 0)
            break;
    }
}

ADDRESS(0x80012290, 0xb8)
static int load_st_file(u_long *buf, char *name, int size)
{
    char path[64];
    CdlFILE file;
    u_char result[8]; /* unused */

    strcpy(path, "\\ST\\");
    strcat(path, name);
    strcat(path, ";1");
    while (CdSearchFile(&file, path) == 0)
        CdControl(CdlPause, 0, 0);
    if (size == 0)
        size = file.size;
    cd_read_sectors(&file.pos, buf, (unsigned)(size + 2047) >> 11);
    return 0;
}

/* Starts the opening's XA audio: file 1, channel 0 of OPTXT.XA. */
ADDRESS(0x80012348, 0x9c)
static void play_opening_xa(void)
{
    u_char param[8];
    CdlFILTER filter;
    CdlFILE file;

    CdSearchFile(&file, "\\ST\\OPTXT\\OPTXT.XA;1");
    param[0] = CdlModeSpeed | CdlModeRT | CdlModeSF;
    CdControlB(CdlSetmode, param, 0);
    VSync(4);
    while (CdControl(CdlSetloc, (u_char *)&file.pos, 0) == 0)
        ;
    filter.file = 1;
    filter.chan = 0;
    while (CdControl(CdlSetfilter, (u_char *)&filter, 0) == 0)
        ;
    while (CdControlB(CdlReadS, (u_char *)&file.pos, 0) == 0)
        ;
}

/* Polls the pad state up to 61 times until it is stable or a plain pad. */
ADDRESS(0x800123e4, 0x7c)
static void pad_wait_stable(void)
{
    int tries = 60;

    for (;;) {
        pad.state = PadGetState(0);
        if (pad.state == PadStateStable)
            break;
        if (pad.state == PadStateFindCTP1)
            break;
        if (tries == 0)
            break;
        tries--;
    }
}

ADDRESS(0x80012460, 0x68)
static void pad_set_act_align(void)
{
    if (pad.actuators) {
        act_align[0] = 0;
        act_align[1] = 0;
        if (pad.actuators == 2) {
            act_align[2] = 1;
            act_align[3] = 1;
        } else {
            act_align[2] = 0xff;
            act_align[3] = 0xff;
        }
        act_align[4] = act_align[5] = 0xff;
        while (PadSetActAlign(0, act_align) == 0)
            ;
    }
}

ADDRESS(0x800124c8, 0x60)
static void pad_check_actuators(void)
{
    if (pad.state == PadStateStable) {
        pad.actuators = PadInfoAct(0, -1, 0);
        pad_wait_stable();
        pad_set_act_align();
    } else {
        pad.actuators = 0;
    }
}

ADDRESS(0x80012528, 0xe0)
static void pad_update_state(void)
{
    switch (pad.state = PadGetState(0)) {
    case PadStateDiscon:
        pad.disconnected = 1;
        pad.lost = 1;
        break;
    case PadStateFindPad:
        pad.found = 1;
        break;
    case PadStateStable:
        if (pad.found && !pad.disconnected) {
            pad.found = 0;
            pad_set_act_align();
        }
        /* fall through */
    case PadStateFindCTP1:
        if (pad.disconnected) {
            pad.disconnected = 0;
            pad.lost = 0;
        }
        break;
    case PadStateFindCTP2:
    case PadStateReqInfo:
    case PadStateExecCmd:
        break;
    }
}

ADDRESS(0x80012608, 0x98)
static void pad_init(void)
{
    pad_data.act[0][0] = 0x40;
    pad_data.act[1][0] = 0x40;
    pad_data.act[0][1] = 0;
    pad_data.act[1][1] = 0;
    pad_data.act[0][2] = 0x40;
    pad_data.act[1][2] = 0x40;
    pad_data.act[0][3] = 0;
    pad_data.act[1][3] = 0;
    PadInitDirect((u_char *)pad_data.buf[0], (u_char *)pad_data.buf[1]);
    PadSetAct(0, (u_char *)pad_data.act[0], sizeof(pad_data.act[0]));
    PadStartCom();
    pad_wait_stable();
    pad_check_actuators();
    pad.lost = 0;
    pad.trigger = pad.previous = pad.buttons = 0;
}

ADDRESS(0x800126a0, 0x70)
static void pad_read(void)
{
    pad_update_state();
    if (!pad.lost && pad_data.buf[0][0] == 0) {
        pad.previous = pad.buttons;
        pad.buttons = ~*(volatile short *)&pad_data.buf[0][2];
        pad.trigger = pad.buttons & ~pad.previous;
    }
}

/* Darkens the screen by level (0-255) with a subtractive full-screen polygon. */
ADDRESS(0x80012710, 0xd8)
static void draw_fade(int level)
{
    POLY_F4 poly = {0, 0, 0, 0, 0, 0, 0, 320, 0, 0, 240, 320, 240};
    DR_MODE mode;
    int c;

    c = 0;
    if (level >= 0) {
        c = level;
        if (c > 255)
            c = 255;
    }
    poly.r0 = poly.g0 = poly.b0 = c;
    SetDrawMode(&mode, 0, 0, GetTPage(0, 2, 0, 0), 0);
    DrawPrim(&mode);
    SetPolyF4(&poly);
    SetSemiTrans(&poly, 1);
    DrawPrim(&poly);
}

/* Draws text page `page` (256x240, 4-bit) with its top at screen y, at brightness level. */
ADDRESS(0x800127e8, 0x130)
static void draw_text_page(int page, int y, int level)
{
    POLY_FT4 poly = {
        0, 0x80, 0x80, 0x80, 0, 32, 0, 0, 0, 0, 288, 0, 255, 0, 0, 32, 240, 0, 240, 0, 288, 240, 255, 240, 0,
    };
    int top, bottom;
    int v_top, v_bottom;

    if (y < 0) {
        top = 0;
        bottom = y + 240;
        v_top = -y;
        v_bottom = 240;
    } else {
        top = y;
        bottom = 240;
        v_top = 0;
        v_bottom = 240 - y;
    }
    if (level < 0)
        level = 0;
    else if (level > 255)
        level = 255;
    poly.y0 = poly.y1 = top;
    poly.y2 = poly.y3 = bottom;
    poly.v0 = poly.v1 = v_top;
    poly.v2 = poly.v3 = v_bottom;
    poly.r0 = poly.g0 = poly.b0 = level;
    poly.tpage = GetTPage(0, 1, page_tpage[page].x, page_tpage[page].y);
    poly.clut = GetClut(0, 480);
    SetPolyFT4(&poly);
    SetSemiTrans(&poly, 1);
    DrawPrim(&poly);
}

/* Part 1: pages 0 and 1 fade in, scroll up and fade out over 720 frames. */
ADDRESS(0x80012918, 0xf0)
static void scroll_pages_1(void)
{
    int level = 128;
    int y0, y1;

    if (frame < 64)
        level = frame * 2;
    if (frame > 540)
        level = (604 - frame) * 2;
    if (frame < 120) {
        y0 = 0;
        y1 = 320;
    } else if (frame > 440) {
        y0 = -240;
        y1 = 0;
    } else {
        y0 = 120 - frame;
        y1 = 440 - frame;
    }
    if (y0 >= -240 && y0 <= 240)
        draw_text_page(0, y0, level);
    if (y1 >= -240 && y1 <= 240)
        draw_text_page(1, y1, level);
    if (frame == 720) {
        opening_state = 2;
        frame = 0;
    } else {
        frame++;
    }
}

/* Part 2: pages 2 and 3 over the picture copied from (640, 256), 920 frames. */
ADDRESS(0x80012a08, 0x170)
static void scroll_pages_2(void)
{
    RECT rect = {640, 256, 320, 240};
    int level;
    int y0, y1;

    MoveImage(&rect, display.draw[display.id].clip.x, display.draw[display.id].clip.y);
    level = 128;
    if (frame < 128)
        draw_fade((level - frame) * 2);
    if (frame < 256)
        level = (frame - 192) * 2;
    if (frame > 740)
        level = (804 - frame) * 2;
    if (frame < 320) {
        y0 = 0;
        y1 = 320;
    } else if (frame > 640) {
        y0 = -240;
        y1 = 0;
    } else {
        y0 = 320 - frame;
        y1 = 640 - frame;
    }
    if (y0 >= -240 && y0 <= 240)
        draw_text_page(2, y0, level);
    if (y1 >= -240 && y1 <= 240)
        draw_text_page(3, y1, level);
    if (frame == 920) {
        opening_state = 3;
        frame = 0;
    } else {
        frame++;
    }
}

/* Part 3: the picture fades out while pages 4 and 5 scroll, 900 frames. */
ADDRESS(0x80012b78, 0x18c)
static void scroll_pages_3(void)
{
    RECT rect = {640, 256, 320, 240};
    int level = 128;
    int y0, y1;

    if (frame < 368)
        MoveImage(&rect, display.draw[display.id].clip.x, display.draw[display.id].clip.y);
    if (frame > 240 && frame < 368)
        draw_fade((frame - 240) * 2);
    if (frame < 64)
        level = frame * 2;
    if (frame > 660)
        level = (724 - frame) * 2;
    if (frame < 128) {
        y0 = 0;
        y1 = 320;
    } else if (frame > 540) {
        y0 = -240;
        y1 = 0;
    } else {
        y0 = 128 - frame;
        y1 = 540 - frame;
    }
    if (y0 >= -240 && y0 <= 240)
        draw_text_page(4, y0, level);
    if (y1 >= -240 && y1 <= 240)
        draw_text_page(5, y1, level);
    if (frame == 900) {
        opening_state = 4;
        frame = 0;
    } else {
        frame++;
    }
}

/*
 * Not exact: the state address stays in a saved register across the frame
 * loop; retail rematerializes it at each access. This adds a saved register
 * (48-byte frame versus 40) and four code bytes (0x210 versus 0x20c).
 */
ADDRESS(0x80012d04, 0x20c)
static void play_opening_text(void)
{
    u_char param[8];
    u_char *p;
    int n;

    load_st_file(text_buffer, "OPTXT\\OPTXT.MIC", 0);
    p = (u_char *)text_buffer;
    n = 7;
    while (n--) {
        load_tims((u_long *)(p + 4), 0, 0, 0);
        p += *(u_long *)p + 4;
    }
    DrawSync(0);
    opening_state = 0;
    frame = 0;
    do {
        pad_read();
        display.id = 1 - display.id;
        DrawSync(0);
        VSync(0);
        PutDrawEnv(&display.draw[display.id]);
        PutDispEnv(&display.disp[display.id]);
        if (opening_state == 0) {
            play_opening_xa();
            SsSetSerialVol(SS_SERIAL_A, 64, 64);
            SsSetMVol(127, 127);
            opening_state = 1;
        } else if (opening_state == 1) {
            scroll_pages_1();
        } else if (opening_state == 2) {
            scroll_pages_2();
        } else if (opening_state == 3) {
            scroll_pages_3();
        } else if (opening_state == 4) {
            opening_state = 5;
        }
        /* The held buttons as the third word from pad.trigger: the address
         * is one constant, which the loop pass leaves in the loop (the
         * member form's base register would be hoisted). */
        if ((&pad.trigger)[2] & PAD_RAW_START)
            opening_state = 5;
    } while (opening_state != 5);
    param[0] = CdlModeSpeed | CdlModeSize1;
    CdControlB(CdlSetmode, param, 0);
    SsSetSerialVol(SS_SERIAL_A, 0, 0);
    CdControl(CdlPause, 0, 0);
}

ADDRESS(0x80012f10, 0x1e0)
int main(void)
{
    char *program_paths[] = {
        "cdrom:STR.EXE;1", "cdrom:TITLE.EXE;1", "cdrom:OPTXT.EXE;1",
        "cdrom:ST.EXE;1", "cdrom:END.EXE;1", "cdrom:VS.EXE;1",
    };
    u_char result[8];

    ResetCallback();
    ResetGraph(3);
    SetGraphDebug(0);
    SetDispMask(0);
    pad_init();
    clear_vram();
    SetDefDrawEnv(&display.draw[0], 0, 0, 320, 240);
    SetDefDispEnv(&display.disp[0], 0, 240, 320, 240);
    SetDefDrawEnv(&display.draw[1], 0, 240, 320, 240);
    SetDefDispEnv(&display.disp[1], 0, 0, 320, 240);
    display.draw[0].dtd = 1;
    display.draw[1].dtd = 1;
    display.draw[0].isbg = 1;
    display.draw[1].isbg = 1;
    setRGB0(&display.draw[0], 0, 0, 0);
    setRGB0(&display.draw[1], 0, 0, 0);
    while (CdInit() == 0)
        CdSync(0, result);
    CdSetDebug(0);
    display.id = 0;
    PutDispEnv(&display.disp[0]);
    PutDrawEnv(&display.draw[display.id]);
    SetDispMask(1);
    play_opening_text();
    *next_program = PROGRAM_ST;
    *PTR_8001f79c = 1;
    clear_vram();
    PadStopCom();
    ResetGraph(3);
    StopCallback();
}
