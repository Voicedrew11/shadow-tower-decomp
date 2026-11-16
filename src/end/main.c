/*
 * END.EXE: the ending. main plays \ST\STR\END.STR as a 24-bit movie, then
 * \ST\STR\STAFF.STR as a 16-bit movie with the staff roll from STR\STAFF.MIC
 * drawn over it, and returns to PSX.EXE with program index 1 (TITLE.EXE) in
 * the next-program byte.
 *
 * The str* functions follow the shape of Sony's movie playback sample
 * (DECENV, strSetDefDecEnv, strInit, strCallback, strNextVlc, strNext,
 * strSync, strKickCD) with Shadow Tower's changes: a 24-bit/16-bit switch,
 * an end-frame argument and no timeout message.
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
#include <psyq/libc.h>

/* PSX.EXE's next-program byte and the mailbox byte after it (see src/psx/main.c). */
#define NEXT_PROGRAM_BYTE ((u8 *)0x800103f0)
#define MAILBOX_BYTE_1 ((u8 *)0x800103f1)
/*
 * play_staff_roll clears this word by its fixed address: the store is an
 * absolute lui $at pair with no $gp form, which a symbol of this unit's small
 * data would get. 0x8002fd7c is this unit's own .sbss static staff_roll_word,
 * which nothing names; OPTXT.EXE writes 0x8001f7ec, the word after its own
 * small data, the same way. How the original spelled the address is unknown.
 */
#define STAFF_ROLL_WORD ((int *)0x8002fd7c)
/* PSX.EXE's program table index of TITLE.EXE. */
#define PROGRAM_TITLE 1

/* Polling bound of the decoder and stream waits. */
#define WAIT_TIME 0x800000

/* TIM file ID word. */
#define TIM_ID 0x10

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

/* A TIM pixel block header: its byte length (header included) and VRAM area. */
typedef struct {
    u_long bytes;
    RECT rect;
} TimBlock;

typedef struct {
    short x, y;
} VramOffset;

static void clear_vram(void);
static void cd_read_sectors(CdlLOC *loc, u_long *buf, int sectors);
static int load_st_file(u_long *buf, char *name, int size);
static void load_tims(u_long *addr, VramOffset *pixel_offset, VramOffset *clut_offset,
                      int absolute);
static int play_end_movie(void);
static void draw_fade(int level);
static int play_staff_roll(void);
static void strSetDefDecEnv(DECENV *dec, int x0, int y0, int x1, int y1);
static void strInit(CdlLOC *loc, void (*callback)());
static void strCallback(void);
static void strNextVlc(DECENV *dec, u_long end_frame);
static u_long *strNext(DECENV *dec, u_long end_frame);
static void strSync(DECENV *dec, int mode);
static void strKickCD(CdlLOC *loc);

RODATA(0x80011000, 0xd0)
SDATA(0x8002fd18, 0x14)

/* The left and right halves of a 320x240 staff roll page in VRAM at (320, 0). */
DATA(0x8001d28c, 0x28, ".data")
POLY_FT4 staff_poly_left = {
    0, 0x7f, 0x7f, 0x7f, 0, 0, 0, 0, 0, 0, 160, 0, 160, 0, 0, 0, 240, 0, 240, 0, 160, 240, 160, 240, 0,
};
DATA(0x8001d2b4, 0x28, ".data")
POLY_FT4 staff_poly_right = {
    0, 0x7f, 0x7f, 0x7f, 0, 160, 0, 0, 0, 0, 320, 0, 160, 0, 0, 160, 240, 0, 240, 0, 320, 240, 160, 240, 0,
};
/* Frames after which each staff roll page fades in and out. */
DATA(0x8001d2dc, 0x38, ".data")
int staff_page_frames[7][2] = {
    {40, 163}, {198, 321}, {356, 479}, {523, 646}, {681, 804}, {830, 927}, {958, 1050},
};

DATA(0x8002fd04, 0x4, ".sdata")
u8 *next_program = NEXT_PROGRAM_BYTE;
DATA(0x8002fd08, 0x4, ".sdata")
u8 *PTR_8002fd08 = MAILBOX_BYTE_1;
DATA(0x8002fd0c, 0x4, ".sdata")
int movie_width = 0;
DATA(0x8002fd10, 0x4, ".sdata")
int movie_height = 0;
DATA(0x8002fd14, 0x4, ".sdata")
u_long *scratchpad = (u_long *)0x1f800000;

ADDRESS(0x80011fa8, 0x54)
static void clear_vram(void)
{
    RECT rect = {0, 0, 1024, 512};

    ClearImage(&rect, 0, 0, 0);
}

ADDRESS(0x80011ffc, 0xd4)
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

ADDRESS(0x800120d0, 0xb8)
static int load_st_file(u_long *buf, char *name, int size)
{
    char path[64];
    CdlFILE file;

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

DATA(0x8002fd2c, 0x4, ".sdata")
int staff_page = 0;
DATA(0x8002fd30, 0x4, ".sdata")
int staff_unused_1 = 0;
DATA(0x8002fd34, 0x4, ".sdata")
int staff_unused_2 = 0;
DATA(0x8002fd38, 0x4, ".sdata")
int staff_faded_in = 0;
DATA(0x8002fd3c, 0x4, ".sdata")
int staff_faded_out = 0;
DATA(0x8002fd40, 0x4, ".sdata")
int staff_frame = 0;
DATA(0x8002fd44, 0x4, ".sdata")
int staff_unused_3 = 0;

DATA(0x8002fd68, 0x4, ".sbss")
static int movie_ended;
DATA(0x8002fd6c, 0x4, ".sbss")
static int rgb24_mode;
DATA(0x8002fd70, 0x4, ".sbss")
static int movie_unused;
DATA(0x8002fd74, 0x4, ".sbss")
static int end_movie_flag;
DATA(0x8002fd78, 0x4, ".sbss")
static int staff_roll_flag;
DATA(0x8002fd7c, 0x4, ".sbss")
static int staff_roll_word;
DATA(0x8002fd80, 0x4, ".sbss")
static int rgb16_mode;

DATA(0x8002fd98, 0x38, ".bss")
static DECENV dec;
/*
 * Exported COMMON: PSYLINK allocates these after the fixed .bss in symbol
 * hash-bucket order, so their spellings are WIP names that keep retail order.
 */
DATA(0x8002fdd0, 0xb400, ".bss")
u_long image_rgb24[2][16 * 3 * 240 / 2];
DATA(0x8003b1d0, 0x7800, ".bss")
u_long image_rgb16[2][16 * 2 * 240 / 2];
DATA(0x800429d0, 0x50000, ".bss")
u_long vlc_buffer[2][320 / 2 * 256];
DATA(0x800929d0, 0x20000, ".bss")
u_long ring_buffer[0x8000];
DATA(0x800b29d0, 0x107ac0, ".bss")
u_long staff_buffer[0x107ac0 / 4];
DATA(0x801ba490, 0xe4, ".bss")
DisplayBuffers display;

/* Loads a run of TIM files (pixel data only, no CLUT) into VRAM. */
ADDRESS(0x80012188, 0x11c)
static void load_tims(u_long *addr, VramOffset *pixel_offset, VramOffset *clut_offset,
                      int absolute)
{
    TimBlock block;
    RECT rect;
    VramOffset no_clut_offset;
    VramOffset no_pixel_offset;
    VramOffset *ofs;
    u_long *p;

    memset(&no_clut_offset, 0, sizeof(no_clut_offset));
    memset(&no_pixel_offset, 0, sizeof(no_pixel_offset));
    ofs = &no_pixel_offset;
    if (pixel_offset)
        ofs = pixel_offset;
    p = addr;
    for (;;) {
        u_long bytes;

        if (*p != TIM_ID)
            break;
        p += 2;
        /* The header is copied out of the file as three words. */
        ((u_long *)&block)[0] = p[0];
        ((u_long *)&block)[1] = p[1];
        ((u_long *)&block)[2] = p[2];
        p += 3;
        if (!absolute) {
            rect.x = block.rect.x + ofs->x;
            rect.y = block.rect.y + ofs->y;
        } else {
            rect.x = ofs->x;
            rect.y = ofs->y;
        }
        rect.w = block.rect.w;
        rect.h = block.rect.h;
        LoadImage(&rect, p);
        bytes = block.bytes - sizeof(TimBlock);
        p = (u_long *)((u_char *)p + bytes);
    }
}

ADDRESS(0x800122a4, 0x21c)
static int play_end_movie(void)
{
    DISPENV disp;
    DRAWENV draw;
    CdlFILE file;
    int id;

    if (CdSearchFile(&file, "\\ST\\STR\\END.STR;1") == 0)
        return 0;
    strSetDefDecEnv(&dec, 0, 0, 0, 240);
    strInit(&file.pos, strCallback);
    strNextVlc(&dec, 2465);
    movie_ended = 0;
    end_movie_flag = 1;
    do {
        DecDCTin(dec.vlcbuf[dec.vlcid], 3);
        DrawSync(0);
        DecDCTout(dec.imgbuf[dec.imgid], dec.slice.w * dec.slice.h / 2);
        strNextVlc(&dec, 2465);
        strSync(&dec, 0);
        DrawSync(0);
        VSync(0);
        id = dec.rectid ? 0 : 1;
        SetDefDispEnv(&disp, 0, id * 240, 480, 240);
        disp.disp.w = disp.disp.w * 2 / 3;
        disp.isrgb24 = 1;
        PutDispEnv(&disp);
        SetDispMask(1);
    } while (movie_ended == 0);
    clear_vram();
    SetDefDispEnv(&disp, 0, 0, 320, 240);
    PutDispEnv(&disp);
    DrawSync(0);
    VSync(0);
    SetDefDispEnv(&disp, 0, 240, 320, 240);
    PutDispEnv(&disp);
    DrawSync(0);
    VSync(0);
    DecDCToutCallback(0);
    StUnSetRing();
    CdControlB(CdlPause, 0, 0);
    return movie_ended;
}

/* Darkens the screen by level (0-255) with a subtractive full-screen polygon. */
ADDRESS(0x800124c0, 0xd8)
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

ADDRESS(0x80012598, 0x3f0)
static int play_staff_roll(void)
{
    DISPENV disp;
    DRAWENV draw;
    CdlFILE file;
    u_char *p;
    int fade;
    int id;

    load_st_file(staff_buffer, "STR\\STAFF.MIC", 0);
    p = (u_char *)staff_buffer;
    load_tims((u_long *)(p + 4), 0, 0, 0);
    p += *(u_long *)p + 4;
    DrawSync(0);
    load_tims((u_long *)(p + 4), 0, 0, 0);
    p += *(u_long *)p + 4;
    if (CdSearchFile(&file, "\\ST\\STR\\STAFF.STR;1") == 0)
        return 0;
    strSetDefDecEnv(&dec, 0, 0, 0, 240);
    strInit(&file.pos, strCallback);
    strNextVlc(&dec, 1620);
    fade = 254;
    movie_ended = 0;
    *STAFF_ROLL_WORD = 0;
    staff_roll_flag = 0;
    SetPolyFT4(&staff_poly_left);
    SetPolyFT4(&staff_poly_right);
    for (;;) {
        DecDCTin(dec.vlcbuf[dec.vlcid], 2);
        DrawSync(0);
        DecDCTout(dec.imgbuf[dec.imgid], dec.slice.w * dec.slice.h / 2);
        strNextVlc(&dec, 1620);
        strSync(&dec, 0);
        if (staff_page < 7) {
            staff_poly_left.tpage = GetTPage(2, 0, 320, 0);
            DrawPrim(&staff_poly_left);
            staff_poly_right.tpage = GetTPage(2, 0, 320, 256);
            DrawPrim(&staff_poly_right);
        }
        draw_fade(fade);
        if (staff_page_frames[staff_page][0] < staff_frame && !staff_faded_in) {
            fade -= 16;
            if (fade <= 0) {
                staff_faded_in = 1;
                staff_faded_out = 0;
            }
        }
        if (staff_page_frames[staff_page][1] < staff_frame && !staff_faded_out) {
            fade += 16;
            if (fade >= 256) {
                staff_faded_out = 1;
                staff_faded_in = 0;
                staff_page++;
                load_tims((u_long *)(p + 4), 0, 0, 0);
                p += *(u_long *)p + 4;
                DrawSync(0);
                load_tims((u_long *)(p + 4), 0, 0, 0);
                p += *(u_long *)p + 4;
                DrawSync(0);
            }
        }
        staff_frame++;
        DrawSync(0);
        VSync(0);
        id = dec.rectid ? 0 : 1;
        SetDefDrawEnv(&draw, 0, (1 - id) * 240, 320, 240);
        SetDefDispEnv(&disp, 0, id * 240, 320, 240);
        PutDispEnv(&disp);
        PutDrawEnv(&draw);
        SetDispMask(1);
        if (movie_ended)
            break;
        if (PadRead(0) & PADstart) {
            movie_ended = 1;
            break;
        }
    }
    DecDCToutCallback(0);
    StUnSetRing();
    CdControlB(CdlPause, 0, 0);
    return movie_ended;
}

ADDRESS(0x80012988, 0x94)
static void strSetDefDecEnv(DECENV *dec, int x0, int y0, int x1, int y1)
{
    dec->vlcbuf[0] = vlc_buffer[0];
    dec->vlcbuf[1] = vlc_buffer[1];
    if (rgb24_mode) {
        dec->imgbuf[0] = image_rgb24[0];
        dec->imgbuf[1] = image_rgb24[1];
    } else {
        dec->imgbuf[0] = image_rgb16[0];
        dec->imgbuf[1] = image_rgb16[1];
    }
    dec->rect[0].x = x0;
    dec->rect[0].y = y0;
    dec->rect[1].x = x1;
    dec->rect[1].y = y1;
    dec->slice.x = x0;
    dec->slice.y = y0;
    if (rgb16_mode)
        dec->slice.w = 16;
    else
        dec->slice.w = 24;
    dec->isdone = 0;
    dec->rectid = 0;
    dec->imgid = 0;
    dec->vlcid = 0;
}

ADDRESS(0x80012a1c, 0x8c)
static void strInit(CdlLOC *loc, void (*callback)())
{
    DecDCTReset(0);
    DecDCToutCallback(callback);
    StSetRing(ring_buffer, 32);
    if (rgb24_mode)
        StSetStream(1, 1, 0xffffffff, 0, 0);
    else
        StSetStream(0, 1, 0xffffffff, 0, 0);
    strKickCD(loc);
}

ADDRESS(0x80012aa8, 0x140)
static void strCallback(void)
{
    RECT snap_rect;
    int id;

    if (rgb24_mode && StCdIntrFlag) {
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

ADDRESS(0x80012be8, 0x98)
static void strNextVlc(DECENV *dec, u_long end_frame)
{
    int cnt = WAIT_TIME;
    u_long *next;

    while ((next = strNext(dec, end_frame)) == 0) {
        if (--cnt == 0)
            return;
    }
    dec->vlcid = dec->vlcid ? 0 : 1;
    DecDCTvlc(next, dec->vlcbuf[dec->vlcid]);
    StFreeRing(next);
}

ADDRESS(0x80012c80, 0x170)
static u_long *strNext(DECENV *dec, u_long end_frame)
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

        if (rgb16_mode) {
            setRECT(&rect, 0, 0, 320, 480);
        } else {
            setRECT(&rect, 0, 0, 480, 480);
        }
        ClearImage(&rect, 0, 0, 0);
        movie_width = sector->width;
        movie_height = sector->height;
    }
    if (rgb16_mode) {
        dec->rect[0].w = dec->rect[1].w = movie_width;
    } else {
        dec->rect[0].w = dec->rect[1].w = movie_width * 3 / 2;
    }
    dec->rect[0].h = dec->rect[1].h = movie_height;
    dec->slice.h = movie_height;
    return addr;
}

ADDRESS(0x80012df0, 0x98)
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

ADDRESS(0x80012e88, 0x68)
static void strKickCD(CdlLOC *loc)
{
    u_char param;

    param = CdlModeSpeed | CdlModeRT;
    do {
        while (CdControl(CdlSetloc, (u_char *)loc, 0) == 0)
            ;
        CdControlB(CdlSetmode, &param, 0);
        VSync(3);
    } while (CdRead2(CdlModeStream | CdlModeSpeed | CdlModeRT) == 0);
}

ADDRESS(0x80012ef0, 0x21c)
int main(void)
{
    char *program_paths[] = {
        "cdrom:STR.EXE;1", "cdrom:TITLE.EXE;1", "cdrom:OPTXT.EXE;1",
        "cdrom:ST.EXE;1", "cdrom:END.EXE;1", "cdrom:VS.EXE;1",
    };
    char path[64]; /* unused */
    u_char result[8];

    ResetCallback();
    PadInit(0);
    ResetGraph(3);
    SetGraphDebug(0);
    SetDispMask(0);
    while (CdInit() == 0)
        CdSync(0, result);
    CdSetDebug(0);
    *next_program = PROGRAM_TITLE;
    *PTR_8002fd08 = 0;
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
    display.id = 0;
    PutDispEnv(&display.disp[0]);
    PutDrawEnv(&display.draw[display.id]);
    SetDispMask(1);
    rgb24_mode = 1;
    rgb16_mode = 0;
    play_end_movie();
    clear_vram();
    VSync(10);
    rgb24_mode = 0;
    rgb16_mode = 1;
    play_staff_roll();
    clear_vram();
    SsSetMVol(0, 0);
    PadStop();
    ResetGraph(3);
    StopCallback();
}
