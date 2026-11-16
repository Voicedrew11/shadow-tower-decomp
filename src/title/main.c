/*
 * TITLE.EXE: the title screen. main loads \ST\TITLE\TITLE.MIC (the title
 * pictures and the menu sound bank), streams \ST\TITLE\TITLE.XA, and runs
 * the title menu. It returns to PSX.EXE with the next program in the
 * next-program byte: OPTXT.EXE for a new game, ST.EXE with 2 in the
 * mailbox byte for Continue, VS.EXE for versus, and STR.EXE (the opening
 * movie) when the title times out.
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

/* PSX.EXE's next-program byte and the mailbox byte after it (see src/psx/main.c). */
#define NEXT_PROGRAM_BYTE ((u8 *)0x800103f0)
#define MAILBOX_BYTE_1 ((u8 *)0x800103f1)
/* PSX.EXE's program table indices. */
#define PROGRAM_STR 0
#define PROGRAM_OPTXT 2
#define PROGRAM_ST 3
#define PROGRAM_VS 5
/* Mailbox value TITLE.EXE leaves for ST.EXE when Continue is chosen. */
#define MAILBOX_CONTINUE 2

/* TIM file ID word and the flag word's CLUT bit. */
#define TIM_ID 0x10
#define TIM_CLUT 0x8

/*
 * Button bits of the raw PadInitDirect data word (receive buffer bytes 2-3,
 * active low): byte 2 holds SELECT, L3, R3, START, UP, RIGHT, DOWN, LEFT
 * and byte 3 L2, R2, L1, R1, TRIANGLE, CIRCLE, CROSS, SQUARE.
 */
#define PAD_START 0x0008
#define PAD_UP 0x0010
#define PAD_DOWN 0x0040
#define PAD_CROSS 0x4000

/* Title menu items. */
#define MENU_NEW_GAME 0
#define MENU_CONTINUE 1
#define MENU_VERSUS 2

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

/* The menu sound bank: its VAB id and the VAB header (VH) copy. */
typedef struct {
    short id;
    u_long head[0x2000 / 4];
} SoundBank;

/*
 * PadInitDirect receive buffers and the actuator data PadSetAct hands to
 * LIBPAD. The actuator bytes are volatile accesses in retail: pad_init's
 * stores to them keep their order against the argument setup around them.
 */
typedef struct {
    u_char rx[2][34];
    volatile u_char act[2][4];
} PadBuffers;

/*
 * Controller state derived from port 1. The first three bytes are volatile
 * accesses in retail: pad_wait_stable reloads state for each comparison,
 * pad_update_state keeps a zero-extension of its switch value, and no %lo
 * folds into a load or store of them.
 */
typedef struct {
    volatile u_char state;        /* PadGetState(0) */
    volatile u_char disconnected; /* PadStateDiscon seen */
    volatile u_char searching;    /* PadStateFindPad seen */
    u_char actuators;             /* PadInfoAct(0, -1, 0) */
    u_char no_input;              /* buttons ignored until the pad is back */
    int trigger;                  /* buttons newly pressed this frame */
    int previous;                 /* buttons held last frame */
    int held;                     /* buttons held this frame */
} PadState;

typedef struct {
    int timer;        /* frames left before the opening movie */
    int started;      /* START pressed on the title picture */
    int cursor;       /* MENU_* item */
    int chosen;       /* 0 until an item is chosen, then 1/2 alternately */
    u_int glow;       /* highlight level, 0-33 */
    int glow_step;    /* +1 or -1 */
    int xa_frames;    /* frames of TITLE.XA left; -1 once stopped */
} TitleState;

static u_long *copy_words(u_long *dst, u_long *src, int count);
static void clear_vram(void);
static void load_tims(u_long *addr, VramOffset *pixel_offset, VramOffset *clut_offset,
                      int absolute);
static void cd_read_sectors(CdlLOC *loc, u_long *buf, int sectors);
static int load_st_file(u_long *buf, char *name, int size);
static void play_confirm_sound(void);
static void play_cursor_sound(void);
static void play_title_xa(void);
static void stop_title_xa(void);
static void pad_wait_stable(void);
static void pad_set_act_align(void);
static void pad_setup_actuators(void);
static void pad_update_state(void);
static void pad_init(void);
static void pad_read(void);
static void load_title_data(void);
static void title_menu(void);

RODATA(0x80012000, 0x164)
SDATA(0x80028ac4, 0x28)

DATA(0x80028abc, 0x4, ".sdata")
u8 *next_program = NEXT_PROGRAM_BYTE;
DATA(0x80028ac0, 0x4, ".sdata")
u8 *PTR_80028ac0 = MAILBOX_BYTE_1;

DATA(0x80028b10, 0x6, ".sbss")
static u_char pad_align[6];

/*
 * Exported COMMON: PSYLINK allocates these after MALLOC.OBJ's .bss in symbol
 * hash-bucket order, so their spellings are WIP names that keep retail order.
 */
DATA(0x80028b28, 0x2004, ".bss")
SoundBank sound_bank;
DATA(0x8002ab30, 0x190000, ".bss")
u_long file_buffer[0x190000 / 4];
DATA(0x801bab30, 0xe4, ".bss")
DisplayBuffers display;
DATA(0x801bac18, 0x4c, ".bss")
PadBuffers pad_data;
DATA(0x801bac68, 0x1c, ".bss")
TitleState menu;
DATA(0x801bac88, 0x14, ".bss")
PadState pad_state;

ADDRESS(0x8001304c, 0x30)
static u_long *copy_words(u_long *dst, u_long *src, int count)
{
    int n;

    while ((n = count, count = count - 1, n))
        *dst++ = *src++;
    return src;
}

ADDRESS(0x8001307c, 0x54)
static void clear_vram(void)
{
    RECT rect = {0, 0, 1024, 512};

    ClearImage(&rect, 0, 0, 0);
}

/* Loads a run of TIM files (CLUT and pixel blocks) into VRAM. */
ADDRESS(0x800130d0, 0x1dc)
static void load_tims(u_long *addr, VramOffset *pixel_offset, VramOffset *clut_offset,
                      int absolute)
{
    TimBlock block;
    RECT rect;
    VramOffset no_clut_offset;
    VramOffset no_pixel_offset;
    VramOffset *clut_ofs;
    VramOffset *pixel_ofs;
    u_long *p;

    memset(&no_clut_offset, 0, sizeof(no_clut_offset));
    memset(&no_pixel_offset, 0, sizeof(no_pixel_offset));
    clut_ofs = &no_clut_offset;
    pixel_ofs = &no_pixel_offset;
    if (clut_offset)
        clut_ofs = clut_offset;
    if (pixel_offset)
        pixel_ofs = pixel_offset;
    p = addr;
    for (;;) {
        u_long bytes;

        if (*p != TIM_ID)
            break;
        p++;
        if (*p++ & TIM_CLUT) {
            ((u_long *)&block)[0] = p[0];
            ((u_long *)&block)[1] = p[1];
            ((u_long *)&block)[2] = p[2];
            p += 3;
            if (!absolute) {
                rect.x = block.rect.x + clut_ofs->x;
                rect.y = block.rect.y + clut_ofs->y;
            } else {
                rect.x = clut_ofs->x;
                rect.y = clut_ofs->y;
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
            rect.x = block.rect.x + pixel_ofs->x;
            rect.y = block.rect.y + pixel_ofs->y;
        } else {
            rect.x = pixel_ofs->x;
            rect.y = pixel_ofs->y;
        }
        rect.w = block.rect.w;
        rect.h = block.rect.h;
        LoadImage(&rect, p);
        bytes = block.bytes - sizeof(TimBlock);
        p = (u_long *)((u_char *)p + bytes);
    }
}

ADDRESS(0x800132ac, 0xd4)
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

ADDRESS(0x80013380, 0xb8)
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

ADDRESS(0x80013438, 0x54)
static void play_confirm_sound(void)
{
    SsUtSetVVol(SsUtKeyOn(sound_bank.id, 0, 12, 60, 0, 127, 127), 70, 70);
}

ADDRESS(0x8001348c, 0x54)
static void play_cursor_sound(void)
{
    SsUtSetVVol(SsUtKeyOn(sound_bank.id, 0, 11, 60, 0, 127, 127), 65, 65);
}

/* Streams \ST\TITLE\TITLE.XA, file 1 channel 0, at double speed. */
ADDRESS(0x800134e0, 0x9c)
static void play_title_xa(void)
{
    u_char param[8];
    CdlFILTER filter;
    CdlFILE file;

    CdSearchFile(&file, "\\ST\\TITLE\\TITLE.XA;1");
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

ADDRESS(0x8001357c, 0x50)
static void stop_title_xa(void)
{
    u_char param[8];

    param[0] = CdlModeSpeed | CdlModeSize1;
    CdControlB(CdlSetmode, param, 0);
    SsSetSerialVol(SS_SERIAL_A, 0, 0);
    CdControl(CdlPause, 0, 0);
}

/* Polls port 1 for up to 61 frames until it is stable or still searching. */
ADDRESS(0x800135cc, 0x7c)
static void pad_wait_stable(void)
{
    int count = 60;

    for (;;) {
        pad_state.state = PadGetState(0);
        if (pad_state.state == PadStateStable)
            break;
        if (pad_state.state == PadStateFindCTP1)
            break;
        if (count == 0)
            break;
        count--;
    }
}

ADDRESS(0x80013648, 0x68)
static void pad_set_act_align(void)
{
    if (pad_state.actuators == 0)
        return;
    pad_align[0] = 0;
    pad_align[1] = 0;
    if (pad_state.actuators == 2) {
        pad_align[2] = 1;
        pad_align[3] = 1;
    } else {
        pad_align[2] = 0xff;
        pad_align[3] = 0xff;
    }
    pad_align[4] = pad_align[5] = 0xff;
    while (PadSetActAlign(0, pad_align) == 0)
        ;
}

ADDRESS(0x800136b0, 0x60)
static void pad_setup_actuators(void)
{
    if (pad_state.state == PadStateStable) {
        pad_state.actuators = PadInfoAct(0, -1, 0);
        pad_wait_stable();
        pad_set_act_align();
    } else {
        pad_state.actuators = 0;
    }
}

ADDRESS(0x80013710, 0xe0)
static void pad_update_state(void)
{
    pad_state.state = PadGetState(0);
    switch (pad_state.state) {
    case PadStateDiscon:
        pad_state.disconnected = 1;
        pad_state.no_input = 1;
        break;
    case PadStateFindPad:
        pad_state.searching = 1;
        break;
    case PadStateStable:
        if (pad_state.searching && !pad_state.disconnected) {
            pad_state.searching = 0;
            pad_set_act_align();
        }
        /* fall through */
    case PadStateFindCTP1:
        if (pad_state.disconnected) {
            pad_state.disconnected = 0;
            pad_state.no_input = 0;
        }
        break;
    case PadStateFindCTP2: /* retail dispatches all seven states through one jump table */
    case PadStateReqInfo:
    case PadStateExecCmd:
        break;
    }
}

ADDRESS(0x800137f0, 0x98)
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
    PadInitDirect(pad_data.rx[0], pad_data.rx[1]);
    PadSetAct(0, (u_char *)pad_data.act[0], 4);
    PadStartCom();
    pad_wait_stable();
    pad_setup_actuators();
    pad_state.no_input = 0;
    pad_state.trigger = pad_state.previous = pad_state.held = 0;
}

ADDRESS(0x80013888, 0x70)
static void pad_read(void)
{
    pad_update_state();
    if (!pad_state.no_input && pad_data.rx[0][0] == 0) {
        u_short buttons = *(u_short *)&pad_data.rx[0][2];

        pad_state.previous = pad_state.held;
        pad_state.held = ~(short)buttons;
        pad_state.trigger = pad_state.held & ~pad_state.previous;
    }
}

/* Loads TITLE.MIC: three TIM runs, then the VAB header and body. */
ADDRESS(0x800138f8, 0x118)
static void load_title_data(void)
{
    VramOffset background_offset = {320, 0};
    VramOffset menu_offset = {320, 256};
    VramOffset start_offset = {640, 0};
    u_char *p;

    load_st_file(file_buffer, "TITLE\\TITLE.MIC", 0);
    p = (u_char *)file_buffer;
    load_tims((u_long *)(p + 4), &background_offset, 0, 1);
    p += *(u_long *)p + 4;
    load_tims((u_long *)(p + 4), &start_offset, 0, 1);
    p += *(u_long *)p + 4;
    load_tims((u_long *)(p + 4), &menu_offset, 0, 1);
    p += *(u_long *)p + 4;
    copy_words(sound_bank.head, (u_long *)(p + 4), *(u_long *)p >> 2);
    p += *(u_long *)p + 4;
    sound_bank.id = SsVabOpenHead((u_char *)sound_bank.head, -1);
    SsVabTransBody(p + 4, sound_bank.id);
    SsVabTransCompleted(SS_WAIT_COMPLETED);
}

ADDRESS(0x80013a10, 0x734)
static void title_menu(void)
{
    RECT background = {320, 0, 320, 240};
    RECT unused_rect = {128, 164, 64, 48};
    CVECTOR unused_color = {16, 32, 80, 0};
    POLY_FT4 press_start = {
        0, 128, 128, 128, 0, 116, 176, 0, 0, 0, 233, 176, 117, 0, 0,
        116, 198, 0, 22, 0, 233, 198, 117, 22, 0,
    };
    POLY_FT4 new_game = {
        0, 128, 128, 128, 0, 141, 157, 0, 0, 0, 185, 157, 44, 0, 0,
        141, 171, 0, 14, 0, 185, 171, 44, 14, 0,
    };
    POLY_FT4 continue_game = {
        0, 128, 128, 128, 0, 141, 173, 0, 16, 0, 185, 173, 44, 16, 0,
        141, 187, 0, 30, 0, 185, 187, 44, 30, 0,
    };
    POLY_FT4 versus = {
        0, 128, 128, 128, 0, 141, 189, 0, 32, 0, 185, 189, 44, 32, 0,
        141, 203, 0, 46, 0, 185, 203, 44, 46, 0,
    };
    POLY_F4 fade;
    u_char unused[48]; /* unused frame space between fade and mode */
    DR_MODE mode;
    POLY_FT4 *item;

    play_title_xa();
    SsSetSerialVol(SS_SERIAL_A, 64, 64);
    menu.timer = 1320;
    menu.xa_frames = 900;
    menu.glow = 0;
    menu.started = 0;
    menu.cursor = 0;
    menu.chosen = 0;
    menu.glow_step = 1;
    while (menu.timer-- != 0) {
        display.id = 1 - display.id;
        DrawSync(0);
        VSync(0);
        PutDrawEnv(&display.draw[display.id]);
        PutDispEnv(&display.disp[display.id]);
        pad_read();
        if (menu.glow > 32)
            menu.glow_step = -menu.glow_step;
        menu.glow += menu.glow_step;
        if (menu.chosen || menu.timer < 60)
            pad_state.trigger = pad_state.previous = pad_state.held = 0;
        if (!menu.started) {
            if (pad_state.trigger & PAD_START) {
                play_confirm_sound();
                menu.started = 1;
            }
        } else {
            if (!menu.chosen && (pad_state.trigger & (PAD_START | PAD_CROSS))) {
                play_confirm_sound();
                menu.chosen = 1;
                menu.timer = 64;
            }
            if (pad_state.trigger & PAD_UP) {
                play_cursor_sound();
                if (menu.cursor > 0)
                    menu.cursor--;
            }
            if (pad_state.trigger & PAD_DOWN) {
                play_cursor_sound();
                if (menu.cursor < 2)
                    menu.cursor++;
            }
        }
        if (pad_state.held && !menu.chosen && menu.timer > 64)
            menu.timer = 1200;
        MoveImage(&background, display.draw[display.id].clip.x, display.draw[display.id].clip.y);
        if (!menu.started) {
            press_start.tpage = GetTPage(2, 0, 640, 0);
            press_start.r0 = 128;
            press_start.g0 = 96;
            press_start.b0 = 80 - menu.glow;
            SetPolyFT4(&press_start);
            SetSemiTrans(&press_start, 0);
            DrawPrim(&press_start);
        } else if (menu.started == 1) {
            versus.r0 = versus.g0 = versus.b0 = 64;
            continue_game.r0 = continue_game.g0 = continue_game.b0 = 64;
            new_game.r0 = new_game.g0 = new_game.b0 = 64;
            if (menu.cursor == MENU_NEW_GAME)
                item = &new_game;
            else if (menu.cursor == MENU_CONTINUE)
                item = &continue_game;
            else
                item = &versus;
            if (!menu.chosen) {
                item->r0 = 128;
                item->g0 = 96;
                item->b0 = 80 - menu.glow;
            } else if (menu.timer > 40) {
                item->r0 = 128;
                item->g0 = 96 - (menu.timer & 1) * 32;
                item->b0 = 80 - (menu.timer & 1) * 32;
            } else {
                item->r0 = 128;
                item->g0 = 96;
                item->b0 = 80;
            }
            new_game.tpage = GetTPage(2, 0, 320, 256);
            SetPolyFT4(&new_game);
            SetSemiTrans(&new_game, 0);
            DrawPrim(&new_game);
            continue_game.tpage = GetTPage(2, 0, 320, 256);
            SetPolyFT4(&continue_game);
            SetSemiTrans(&continue_game, 0);
            DrawPrim(&continue_game);
            versus.tpage = GetTPage(2, 0, 320, 256);
            SetPolyFT4(&versus);
            SetSemiTrans(&versus, 0);
            DrawPrim(&versus);
        }
        if (menu.timer < 32) {
            SetDrawMode(&mode, 0, 0, GetTPage(0, 2, 0, 0), 0);
            DrawPrim(&mode);
            setXY4(&fade, 0, 0, 320, 0, 0, 240, 320, 240);
            fade.r0 = fade.g0 = fade.b0 = (32 - menu.timer) << 3;
            SetPolyF4(&fade);
            SetSemiTrans(&fade, 1);
            DrawPrim(&fade);
        }
        if (menu.chosen == 1)
            menu.chosen = 2;
        else if (menu.chosen == 2)
            menu.chosen = 1;
        if (menu.xa_frames == 0) {
            stop_title_xa();
            menu.xa_frames = -1;
        } else if (menu.xa_frames > 0) {
            menu.xa_frames--;
        }
    }
}

ADDRESS(0x80014144, 0x2e8)
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
    while (CdInit() == 0)
        CdSync(0, result);
    CdSetDebug(0);
    pad_init();
    clear_vram();
    SsInit();
    SsSetTickMode(SS_TICK60);
    SsSetSerialAttr(SS_SERIAL_A, SS_MIX, SS_SON);
    SsUtSetReverbType(SS_REV_TYPE_STUDIO_C);
    SsUtReverbOn();
    SsUtSetReverbDepth(20, 20);
    SsStart();
    SsSetMVol(127, 127);
    SetDispMask(0);
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
    display.id = 0;
    PutDispEnv(&display.disp[0]);
    PutDrawEnv(&display.draw[display.id]);
    SetDispMask(1);
    load_title_data();
    title_menu();
    if (menu.xa_frames != -1)
        stop_title_xa();
    if (menu.chosen) {
        switch (menu.cursor) {
        case MENU_NEW_GAME:
            *next_program = PROGRAM_OPTXT;
            break;
        case MENU_CONTINUE:
            *next_program = PROGRAM_ST;
            *PTR_80028ac0 = MAILBOX_CONTINUE;
            break;
        case MENU_VERSUS:
            *next_program = PROGRAM_VS;
            break;
        }
    } else {
        *next_program = PROGRAM_STR;
    }
    clear_vram();
    SsSetMVol(0, 0);
    SsVabClose(sound_bank.id);
    SsEnd();
    SsQuit();
    PadStopCom();
    ResetGraph(3);
    StopCallback();
}
