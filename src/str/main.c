/*
 * STR.EXE: the boot movies, the first program PSX.EXE runs. On a first run
 * (mailbox byte 0x800103f2 clear) main plays \ST\STR\AGETEC.STR, which START
 * cannot skip, fades in the STR\FLOGO320.TIM logo, plays \ST\STR\OPEN.STR and
 * sets the byte; on the next run it plays \ST\STR\DEMO.STR with the
 * STR\D_TITLE.TIM title blended over it and STR\POISON.TIM shown for a few
 * frames, and clears the byte. Either way it returns to PSX.EXE with program
 * index 1 (TITLE.EXE) in the next-program byte.
 *
 * The str* functions follow the shape of Sony's movie playback sample, as in
 * END.EXE, fixed to 24-bit playback with a global end frame. The pad*
 * functions drive LIBPAD's direct controller interface.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBETC.H>
#include <LIBCD.H>
#include <psyq/cdstream.h>
#include <LIBPRESS.H>
#include <LIBSND.H>
#include <LIBPAD.H>
#include <psyq/libc.h>

/* PSX.EXE's next-program byte and the mailbox bytes after it (see src/psx/main.c). */
#define NEXT_PROGRAM_BYTE ((u8 *)0x800103f0)
#define MAILBOX_BYTE_1 ((u8 *)0x800103f1)
#define MAILBOX_BYTE_2 ((u8 *)0x800103f2)
/* PSX.EXE's program table index of TITLE.EXE. */
#define PROGRAM_TITLE 1

/* Polling bound of the decoder and stream waits. */
#define WAIT_TIME 0x800000

/* TIM file ID word, and the flag word's bit for a CLUT block. */
#define TIM_ID 0x10
#define TIM_HAS_CLUT 0x8

/* START in the inverted raw button word of a LIBPAD receive buffer. */
#define PAD_RAW_START 0x8

/* Stream frames of each movie. */
#define AGETEC_FRAMES 104
#define OPEN_FRAMES 2090
#define DEMO_FRAMES 2160

/* One step of the DEMO.STR title blend: averages a frame byte with a TIM byte. */
#define BLEND_BYTE(dst, src) (*(dst) = (*(dst) + *(src)++) >> 1, (dst)++)

typedef struct {
    u_long *vlcbuf[2]; /* VLC (run-level) buffers */
    int vlcid;
    u_long *imgbuf[2]; /* DecDCTout slice buffers */
    int imgid;
    RECT rect[2]; /* VRAM frame areas */
    int rectid;
    RECT slice; /* area one DecDCTout fills */
    int isdone;
} DECENV;

typedef struct {
    DISPENV disp[2];
    DRAWENV draw[2];
    int id;
} DisplayBuffers;

/* A TIM pixel or CLUT block header: its byte length (header included) and VRAM area. */
typedef struct {
    u_long bytes;
    RECT rect;
} TimBlock;

typedef struct {
    short x, y;
} VramOffset;

/*
 * LIBPAD direct-connection receive buffers and actuator data. LIBPAD's
 * interrupt handler fills the buffers, so pad_data is volatile.
 */
typedef struct {
    u_char buf[2][34];
    u_char act[2][4];
} PadData;

/*
 * Controller state kept by the pad* functions. Retail treats the first three
 * fields as volatile: every access reloads them, their extensions and
 * addresses are never folded into the access, and their stores are never
 * moved into delay slots, unlike the fields after them.
 */
typedef struct {
    volatile u_char state;     /* last PadGetState(0) */
    volatile u_char lost;      /* disconnected since the last stable state */
    volatile u_char searching; /* PadStateFindPad seen; actuator alignment pending */
    u_char actuators; /* PadInfoAct(0, -1, 0) */
    u_char hold;      /* button words frozen while disconnected */
    int trigger;      /* buttons newly pressed this frame */
    int prev;         /* previous button word */
    int cur;          /* current button word (inverted raw data) */
} PadInfo;

static void pad_wait_stable(void);
static void pad_set_align(void);
static void pad_init_actuators(void);
static void pad_update_state(void);
static void pad_init(void);
static void pad_read(void);
static void cd_read_sectors(CdlLOC *loc, u_long *buf, int sectors);
static int load_st_file(u_long *buf, char *name, int size);
static int play_movie(void);
static void strSetDefDecEnv(DECENV *dec, int x0, int y0, int x1, int y1);
static void strInit(CdlLOC *loc, void (*callback)());
static void strCallback(void);
static void strNextVlc(DECENV *dec);
static u_long *strNext(DECENV *dec);
static void strSync(DECENV *dec, int mode);
static void strKickCD(CdlLOC *loc);
static void clear_vram(void);
static void load_tims(u_long *addr, VramOffset *pixel_offset, VramOffset *clut_offset,
                      int absolute);
static void show_logo(void);

RODATA(0x80011000, 0x110)
JUMPTABLES(0x80011110, 0x1c)
SDATA(0x800328a0, 0x34)

DATA(0x8003288c, 0x4, ".sdata")
u8 *next_program = NEXT_PROGRAM_BYTE;
DATA(0x80032890, 0x4, ".sdata")
u8 *PTR_80032890 = MAILBOX_BYTE_1;
DATA(0x80032894, 0x4, ".sdata")
u8 *demo_flag = MAILBOX_BYTE_2;
DATA(0x80032898, 0x4, ".sdata")
int movie_width = 0;
DATA(0x8003289c, 0x4, ".sdata")
int movie_height = 0;

DATA(0x800328f8, 0x6, ".sbss")
static u_char pad_align[6];
DATA(0x80032900, 0x4, ".sbss")
static int movie_ended;
DATA(0x80032904, 0x2, ".sbss")
static u_short agetec_mode;
DATA(0x80032906, 0x2, ".sbss")
static u_short end_frame;

DATA(0x80032918, 0x38, ".bss")
static DECENV dec;
/*
 * Exported COMMON: PSYLINK allocates these after the fixed .bss in symbol
 * hash-bucket order, so their spellings are WIP names that keep retail order.
 */
DATA(0x80032950, 0x50000, ".bss")
u_long vlc_buffer[2][320 / 2 * 256];
DATA(0x80082950, 0x20000, ".bss")
u_long ring_buffer[0x8000];
DATA(0x800a2950, 0x1e00, ".bss")
u_long blend_buffer[0x1e00 / 4];
DATA(0x800a4750, 0x20000, ".bss")
u_long tim_buf[0x8000];
DATA(0x800c4750, 0xe4, ".bss")
DisplayBuffers display;
DATA(0x800c4838, 0x2000, ".bss")
u_long poison_image[0x800];
DATA(0x800c6838, 0x4c, ".bss")
volatile PadData pad_data;
DATA(0x800c6888, 0x18, ".bss")
CdlFILE cdl_file;
DATA(0x800c68a0, 0x14, ".bss")
PadInfo pad_info;
DATA(0x800c68b8, 0xb400, ".bss")
u_long image_rgb[2][16 * 3 * 240 / 2];

/* Waits (up to 61 polls) for the controller in port 1 to settle. */
ADDRESS(0x80012024, 0x7c)
static void pad_wait_stable(void)
{
    int i;

    for (i = 60;; i--) {
        pad_info.state = PadGetState(0);
        if (pad_info.state == PadStateStable)
            break;
        if (pad_info.state == PadStateFindCTP1)
            break;
        if (i == 0)
            break;
    }
}

ADDRESS(0x800120a0, 0x68)
static void pad_set_align(void)
{
    if (pad_info.actuators) {
        pad_align[0] = 0;
        pad_align[1] = 0;
        if (pad_info.actuators == 2) {
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
}

ADDRESS(0x80012108, 0x60)
static void pad_init_actuators(void)
{
    if (pad_info.state == PadStateStable) {
        pad_info.actuators = PadInfoAct(0, -1, 0);
        pad_wait_stable();
        pad_set_align();
    } else {
        pad_info.actuators = 0;
    }
}

ADDRESS(0x80012168, 0xe0)
static void pad_update_state(void)
{
    pad_info.state = PadGetState(0);
    switch (pad_info.state) {
    case PadStateDiscon:
        pad_info.lost = 1;
        pad_info.hold = 1;
        break;
    case PadStateFindPad:
        pad_info.searching = 1;
        break;
    case PadStateStable:
        if (pad_info.searching && !pad_info.lost) {
            pad_info.searching = 0;
            pad_set_align();
        }
        /* fall through */
    case PadStateFindCTP1:
        if (pad_info.lost) {
            pad_info.lost = 0;
            pad_info.hold = 0;
        }
        break;
    case PadStateFindCTP2:
    case PadStateReqInfo:
    case PadStateExecCmd:
        break;
    }
}

ADDRESS(0x80012248, 0x98)
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
    PadSetAct(0, (u_char *)pad_data.act[0], 4);
    PadStartCom();
    pad_wait_stable();
    pad_init_actuators();
    pad_info.hold = 0;
    pad_info.cur = 0;
    pad_info.prev = 0;
    pad_info.trigger = 0;
}

ADDRESS(0x800122e0, 0x70)
static void pad_read(void)
{
    pad_update_state();
    if (pad_info.hold == 0 && pad_data.buf[0][0] == 0) {
        pad_info.prev = pad_info.cur;
        pad_info.cur = ~*(volatile short *)&pad_data.buf[0][2];
        pad_info.trigger = pad_info.cur & ~pad_info.prev;
    }
}

ADDRESS(0x80012350, 0xd4)
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

ADDRESS(0x80012424, 0xb8)
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

ADDRESS(0x800124dc, 0x640)
static int play_movie(void)
{
    DISPENV disp;
    DRAWENV draw;
    RECT title_rect = {362, 204, 90, 16};
    RECT poison_rect = {436, 144, 24, 16};
    RECT unused_rect = {436, 144, 1, 1};
    int frame;
    int id;

    if (*demo_flag == 0) {
        if (agetec_mode) {
            CdSearchFile(&cdl_file, "\\ST\\STR\\AGETEC.STR;1");
            end_frame = AGETEC_FRAMES;
        } else {
            CdSearchFile(&cdl_file, "\\ST\\STR\\OPEN.STR;1");
            end_frame = OPEN_FRAMES;
        }
    } else {
        CdSearchFile(&cdl_file, "\\ST\\STR\\DEMO.STR;1");
        end_frame = DEMO_FRAMES;
        load_st_file(tim_buf, "STR\\D_TITLE.TIM", 0);
        load_st_file(poison_image, "STR\\POISON.TIM", 0);
    }
    strSetDefDecEnv(&dec, 0, 0, 0, 240);
    strInit(&cdl_file.pos, strCallback);
    strNextVlc(&dec);
    frame = 0;
    movie_ended = 0;
    for (;;) {
        pad_read();
        frame++;
        DecDCTin(dec.vlcbuf[dec.vlcid], 3);
        DrawSync(0);
        DecDCTout(dec.imgbuf[dec.imgid], dec.slice.w * dec.slice.h / 2);
        strNextVlc(&dec);
        strSync(&dec, 0);
        DrawSync(0);
        VSync(0);
        id = dec.rectid ? 0 : 1;
        SetDefDispEnv(&disp, 0, id * 240, 480, 240);
        if (*demo_flag) {
            if (frame >= 952 && frame < 1019) {
                if (dec.rectid)
                    poison_rect.y = 144;
                else
                    poison_rect.y = 384;
                LoadImage2(&poison_rect, poison_image + 5);
            }
            if (frame >= 16 && frame < 2121) {
                u_char *dst;
                u_char *src;
                int i;

                if (dec.rectid)
                    title_rect.y = 204;
                else
                    title_rect.y = 444;
                StoreImage2(&title_rect, blend_buffer);
                src = (u_char *)(tim_buf + 5);
                dst = (u_char *)blend_buffer;
                /* 120 steps of 24 bytes: the 90x16 halfwords of title_rect. */
                for (i = 0; i < 120; i++) {
                    BLEND_BYTE(dst, src); BLEND_BYTE(dst, src); BLEND_BYTE(dst, src);
                    BLEND_BYTE(dst, src); BLEND_BYTE(dst, src); BLEND_BYTE(dst, src);
                    BLEND_BYTE(dst, src); BLEND_BYTE(dst, src); BLEND_BYTE(dst, src);
                    BLEND_BYTE(dst, src); BLEND_BYTE(dst, src); BLEND_BYTE(dst, src);
                    BLEND_BYTE(dst, src); BLEND_BYTE(dst, src); BLEND_BYTE(dst, src);
                    BLEND_BYTE(dst, src); BLEND_BYTE(dst, src); BLEND_BYTE(dst, src);
                    BLEND_BYTE(dst, src); BLEND_BYTE(dst, src); BLEND_BYTE(dst, src);
                    BLEND_BYTE(dst, src); BLEND_BYTE(dst, src); BLEND_BYTE(dst, src);
                }
                LoadImage2(&title_rect, blend_buffer);
            }
        }
        disp.disp.w = disp.disp.w * 2 / 3;
        disp.isrgb24 = 1;
        PutDispEnv(&disp);
        SetDispMask(1);
        if (movie_ended)
            break;
        if (agetec_mode)
            continue;
        if (pad_info.cur & PAD_RAW_START) {
            movie_ended = 1;
            break;
        }
    }
    DecDCToutCallback(0);
    StUnSetRing();
    CdControlB(CdlPause, 0, 0);
    return movie_ended;
}

ADDRESS(0x80012b1c, 0x64)
static void strSetDefDecEnv(DECENV *dec, int x0, int y0, int x1, int y1)
{
    dec->vlcbuf[0] = vlc_buffer[0];
    dec->vlcbuf[1] = vlc_buffer[1];
    dec->imgbuf[0] = image_rgb[0];
    dec->imgbuf[1] = image_rgb[1];
    dec->rect[0].x = x0;
    dec->rect[0].y = y0;
    dec->rect[1].x = x1;
    dec->rect[1].y = y1;
    dec->slice.x = x0;
    dec->slice.y = y0;
    dec->slice.w = 24;
    dec->isdone = 0;
    dec->rectid = 0;
    dec->imgid = 0;
    dec->vlcid = 0;
}

ADDRESS(0x80012b80, 0x70)
static void strInit(CdlLOC *loc, void (*callback)())
{
    DecDCTReset(0);
    DecDCToutCallback(callback);
    StSetRing(ring_buffer, 32);
    StSetStream(1, 1, 0xffffffff, 0, 0);
    strKickCD(loc);
}

ADDRESS(0x80012bf0, 0x130)
static void strCallback(void)
{
    RECT snap_rect;
    int id;

    if (StCdIntrFlag) {
        StCdInterrupt();
        StCdIntrFlag = 0;
    }
    id = dec.imgid;
    snap_rect = dec.slice;
    dec.imgid = dec.imgid ? 0 : 1;
    dec.slice.x += dec.slice.w;
    if (dec.slice.x < dec.rect[dec.rectid].x + dec.rect[dec.rectid].w) {
        DecDCTout(dec.imgbuf[dec.imgid], dec.slice.w * dec.slice.h / 2);
    } else {
        dec.isdone = 1;
        dec.rectid = dec.rectid ? 0 : 1;
        dec.slice.x = dec.rect[dec.rectid].x;
        dec.slice.y = dec.rect[dec.rectid].y;
    }
    LoadImage(&snap_rect, dec.imgbuf[id]);
}

ADDRESS(0x80012d20, 0x88)
static void strNextVlc(DECENV *dec)
{
    int cnt = WAIT_TIME;
    u_long *next;

    while ((next = strNext(dec)) == 0) {
        if (--cnt == 0)
            return;
    }
    dec->vlcid = dec->vlcid ? 0 : 1;
    DecDCTvlc(next, dec->vlcbuf[dec->vlcid]);
    StFreeRing(next);
}

ADDRESS(0x80012da8, 0x118)
static u_long *strNext(DECENV *dec)
{
    u_long *addr;
    StHEADER *sector;
    int cnt = WAIT_TIME;

    while (StGetNext(&addr, (u_long **)&sector)) {
        if (--cnt == 0)
            return 0;
    }
    if (++sector->frameCount >= end_frame)
        movie_ended = 1;
    if (movie_width != sector->width || movie_height != sector->height) {
        RECT rect;

        setRECT(&rect, 0, 0, 480, 480);
        ClearImage(&rect, 0, 0, 0);
        movie_width = sector->width;
        movie_height = sector->height;
    }
    dec->rect[0].w = dec->rect[1].w = movie_width * 3 / 2;
    dec->rect[0].h = dec->rect[1].h = movie_height;
    dec->slice.h = movie_height;
    return addr;
}

ADDRESS(0x80012ec0, 0x98)
static void strSync(DECENV *dec, int mode)
{
    volatile u_long cnt = WAIT_TIME;

    while (dec->isdone == 0) {
        if (--cnt == 0) {
            dec->isdone = 1;
            dec->rectid = dec->rectid ? 0 : 1;
            dec->slice.x = dec->rect[dec->rectid].x;
            dec->slice.y = dec->rect[dec->rectid].y;
        }
    }
    dec->isdone = 0;
}

ADDRESS(0x80012f58, 0x68)
static void strKickCD(CdlLOC *loc)
{
    u_char param;

    param = CdlModeSpeed | CdlModeRT;
    do {
        while (CdControl(CdlSetloc, (u_char *)loc, 0) == 0)
            ;
        CdControlB(CdlSetmode, &param, 0);
        VSync(4);
    } while (CdRead2(CdlModeStream | CdlModeSpeed | CdlModeRT) == 0);
}

ADDRESS(0x80012fc0, 0x54)
static void clear_vram(void)
{
    RECT rect = {0, 0, 1024, 512};

    ClearImage2(&rect, 0, 0, 0);
}

/* Loads a run of TIM files (CLUT and pixel blocks) into VRAM. */
ADDRESS(0x80013014, 0x1dc)
static void load_tims(u_long *addr, VramOffset *pixel_offset, VramOffset *clut_offset,
                      int absolute)
{
    TimBlock block;
    RECT rect;
    VramOffset no_clut_offset;
    VramOffset no_pixel_offset;
    VramOffset *pofs;
    VramOffset *cofs;
    u_long *p;

    memset(&no_clut_offset, 0, sizeof(no_clut_offset));
    memset(&no_pixel_offset, 0, sizeof(no_pixel_offset));
    cofs = &no_clut_offset;
    pofs = &no_pixel_offset;
    if (clut_offset)
        cofs = clut_offset;
    p = addr;
    if (pixel_offset)
        pofs = pixel_offset;
    for (;;) {
        u_long bytes;

        if (*p++ != TIM_ID)
            break;
        if (*p++ & TIM_HAS_CLUT) {
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

/*
 * Shows the STR\FLOGO320.TIM logo for 256 frames: each frame copies its
 * 320x32 VRAM area at (320, 0) into the draw buffer and fades it in or out
 * with a subtractive POLY_F4.
 */
ADDRESS(0x800131f0, 0x204)
static void show_logo(void)
{
    POLY_F4 poly = {0, 0x80, 0x80, 0x80, 0, 0, 0, 320, 0, 0, 240, 320, 240};
    DR_MODE mode;
    RECT rect = {320, 0, 320, 32};
    int fade;
    int i;

    load_st_file(tim_buf, "STR\\FLOGO320.TIM", 0);
    load_tims(tim_buf, 0, 0, 0);
    SetDrawMode(&mode, 0, 0, GetTPage(0, 2, 0, 0), 0);
    i = 255;
    do {
        display.id = 1 - display.id;
        DrawSync(0);
        VSync(0);
        PutDrawEnv(&display.draw[display.id]);
        PutDispEnv(&display.disp[display.id]);
        if (i > 224)
            fade = (i - 224) * 8;
        else if (i < 32)
            fade = (32 - i) * 8;
        else
            fade = 0;
        MoveImage(&rect, display.draw[display.id].clip.x, display.draw[display.id].clip.y + 104);
        if (fade) {
            DrawPrim(&mode);
            poly.r0 = poly.g0 = poly.b0 = fade;
            SetPolyF4(&poly);
            SetSemiTrans(&poly, 1);
            DrawPrim(&poly);
        }
    } while (i--);
}

ADDRESS(0x800133f4, 0x248)
int main(void)
{
    char *program_paths[] = {
        "cdrom:STR.EXE;1", "cdrom:TITLE.EXE;1", "cdrom:OPTXT.EXE;1",
        "cdrom:ST.EXE;1", "cdrom:END.EXE;1", "cdrom:VS.EXE;1",
    };
    char path[64]; /* unused */
    u_char result[8];

    ResetCallback();
    ResetGraph(3);
    SetGraphDebug(0);
    SetDispMask(0);
    clear_vram();
    VSync(60);
    while (CdInit() == 0)
        CdSync(0, result);
    CdSetDebug(0);
    pad_init();
    *next_program = PROGRAM_TITLE;
    *PTR_80032890 = 1;
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
    if (*demo_flag == 0) {
        agetec_mode = 1;
        play_movie();
        agetec_mode = 0;
        show_logo();
        play_movie();
        *demo_flag = 1;
    } else {
        play_movie();
        *demo_flag = 0;
    }
    clear_vram();
    SsSetMVol(0, 0);
    PadStopCom();
    ResetGraph(3);
    StopCallback();
}
