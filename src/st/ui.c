/*
 * ST.EXE: the menu screens' text and 2D primitives and the pause, status,
 * item, stored-item and id grid screens (WIP module). One translation unit:
 * GCC merged constants across it (the .sdata words at 0x80087f1c, 0x80087f2c
 * and 0x80087f5c, the .rdata tables at 0x800110f0, 0x80011100 and
 * 0x80011194) and its .sbss statics are read $gp-relative from both ends.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <psyq/libc.h>
#include <st/data.h>
#include <st/gfx.h>
#include <st/vis.h>
#include <st/text.h>
#include <st/menu.h>
#include <st/funcs.h>

RODATA(0x800110d0, 0x2e0)
JUMPTABLES(0x80011490, 0x18)
SDATA(0x80087f1c, 0x18)
SDATA(0x80087f38, 0x30)
SDATA(0x80087f70, 0x14)
SDATA(0x80087f88, 0x8c)

/* Unreferenced: the halfword before the text clip window. */
DATA(0x800881b8, 0x2, ".sbss")
static s16 DAT_800881b8;
/* The text clip window. */
DATA(0x800881ba, 0x2, ".sbss")
static s16 DAT_800881ba;
DATA(0x800881bc, 0x2, ".sbss")
static s16 DAT_800881bc;
DATA(0x800881c0, 0x4, ".sbss")
static s32 DAT_800881c0;
DATA(0x800881c4, 0x4, ".sbss")
static s32 DAT_800881c4;
DATA(0x800881c8, 0x4, ".sbss")
static s32 DAT_800881c8;

/*
 * Font glyphs, text strings, numbers and 2D menu primitives. Glyphs are
 * textured quads cut from the font pages in VRAM; the scratchpad's font mode
 * picks the 6x12 or 7x10 half-width cell layout.
 */

/*
 * Takes the next primitive from the current packet buffer, or returns.
 * A statement sequence, not a do-while block: the block changes retail's
 * code.
 */
#define NEW_PRIM(buf, p, type)            \
    (buf) = world_state.packet;           \
    (p) = (type *)(buf)->cur;             \
    if ((buf)->end < (u_long *)((p) + 1)) \
        return;                           \
    (buf)->cur = (u_long *)((p) + 1);     \
    world_state.unk8328++

/*
 * NEW_PRIM with the end of the new primitive kept in the caller's `next`:
 * a function that takes a primitive in two branches uses one variable for
 * both.
 */
#define NEW_PRIM_NEXT(buf, p, next, type) \
    (buf) = world_state.packet;           \
    (p) = (type *)(buf)->cur;             \
    (next) = (u_long *)((p) + 1);         \
    if ((buf)->end < (next))              \
        return;                           \
    (buf)->cur = (next);                  \
    world_state.unk8328++

/* Draws glyph `c` at (x, y): half-width codes below the split, full-width
 * codes after it. */
ADDRESS(0x80026ecc, 0x460)
void func_80026ecc(u16 c, u16 x, u16 y, CVECTOR *color)
{
    StPacketBuffer *buf;
    POLY_FT4 *p;
    u_long *next;
    CVECTOR rgb;
    u16 half, cols, qw, qh, fw, ch;
    u16 split;
    int u, v;
    u16 q, r;
    int n;

    if (MENU_SCRATCH->unk2E5 == 0) {
        ch = 12;
        fw = ch;
        qh = fw;
        qw = fw;
        half = 6;
        cols = 21;
        split = 64;
    } else {
        half = 7;
        cols = 18;
        split = 4;
        qw = 12;
        qh = 10;
        fw = qw;
        ch = qh;
    }
    if (c < split) {
        r = c % (cols * 2);
        q = c / (cols * 2);
        u = r * half;
        v = ch * q;
        NEW_PRIM_NEXT(buf, p, next, POLY_FT4);
        setUV4(p, u, v, half + u, v, u, ch + v, half + u, ch + v);
        if (DAT_800881c4) {
            if (DAT_800881ac == 1)
                x -= DAT_800881a0 * (half + 1);
            else
                x -= DAT_800881a2 * (half + 1);
            DAT_800881c0 += half + 1;
            if (DAT_800881bc < x) {
                DAT_800881c8++;
                return;
            }
            if (x < DAT_800881ba)
                return;
        }
        p->x1 = p->x3 = x + half;
        p->x0 = p->x2 = x;
        p->y0 = p->y1 = y;
        p->y2 = p->y3 = y + qh;
        rgb = *color;
        rgb.r += 8;
        rgb.g += 8;
        rgb.b += 8;
    } else {
        n = c - split / 2;
        r = n % cols;
        q = n / cols;
        u = fw * r;
        v = ch * q;
        NEW_PRIM_NEXT(buf, p, next, POLY_FT4);
        setUV4(p, u, v, fw + u, v, u, ch + v, fw + u, ch + v);
        if (DAT_800881c4) {
            if (DAT_800881ac == 1)
                x -= DAT_800881a0 * (half + 1);
            else
                x -= DAT_800881a2 * (half + 1);
            DAT_800881c0 += fw;
            if (DAT_800881bc < x) {
                DAT_800881c8++;
                return;
            }
            if (x < DAT_800881ba)
                return;
        }
        p->x1 = p->x3 = x + qw;
        p->x0 = p->x2 = x;
        p->y0 = p->y1 = y;
        p->y2 = p->y3 = y + qh;
        rgb = *color;
        rgb.r += 8;
        rgb.g += 8;
        rgb.b += 8;
    }
    setRGB0(p, rgb.r, rgb.g, rgb.b);
    if (color->cd & 0x80) {
        p->tpage = GetTPage(0, color->cd & 3, 896, 256);
        p->clut = GetClut(560, 497);
    } else {
        p->tpage = GetTPage(0, color->cd & 3, 960, 256);
        p->clut = GetClut(560, 495);
    }
    setlen(p, 9);
    setcode(p, 0x2e);
    addPrim(world_state.ot_cur, p);
}

/* Draws a 0xffff-terminated glyph string; code 0 starts a new line. */
ADDRESS(0x8002732c, 0x114)
void func_8002732c(u16 *s, u16 x, u16 y, CVECTOR *color)
{
    u16 left = x;
    u_int space = (color->cd & 0x7c) >> 2;
    int line;
    u_int split;
    u_int half;

    if (MENU_SCRATCH->unk2E5 == 0) {
        line = 14;
        split = 64;
    } else {
        space = 11;
        line = 10;
        split = 4;
    }
    half = space >> 1;
    while (*s != 0xffff) {
        if (*s == 0) {
            x = left;
            y += line;
            s++;
            s++;
        } else {
            func_80026ecc(*s, x, y, color);
            if (*s < split)
                x += half;
            else
                x += space;
            s++;
        }
    }
}

/*
 * Number words: the value in the low halfword (signed), the digit count in
 * bits 24-27 and, in bits 28-31, flags that ask for a plus sign. Leading
 * zeros are blank; a value too wide for the digit count shows as nines.
 */
#define NUMBER_DIGITS(num) (((num) >> 24) & 0xf)
#define NUMBER_SIGNED 0xf0000000

/* Draws a number with the proportional font: digit glyphs follow the
 * half-width codes. */
ADDRESS(0x80027440, 0x27c)
void func_80027440(u32 num, u16 x, u16 y, CVECTOR *color)
{
    u16 digit[8];
    int end;
    u8 leading = 1;
    u8 neg = 0;
    u16 width = color->cd >> 2;
    u16 base;
    u8 n;
    u32 sign;
    s16 value;
    int pow;
    u32 div;
    int i;

    if (DAT_800881ae)
        width -= 4;
    if (MENU_SCRATCH->unk2E5 == 0)
        base = 64;
    else
        base = 4;
    n = NUMBER_DIGITS(num);
    value = num;
    sign = num & NUMBER_SIGNED;
    end = -1;
    if ((s16)num < 0) {
        value = -num;
        neg = 1;
    }
    if (value == 0) {
        func_80026ecc(base, x + width * (n - 1), y, color);
        return;
    }
    pow = 1;
    i = n - 1;
    if (i != end) {
        do {
            pow *= 10;
        } while (--i != -1);
    }
    if (value < pow) {
        div = pow;
        i = n;
        while (i--) {
            div /= 10;
            digit[i] = value / div;
            value -= digit[i] * div;
        }
    } else {
        i = n;
        while (i--)
            digit[i] = 9;
    }
    if (neg)
        func_80026ecc(base + 13, x - width, y, color);
    else if (sign)
        func_80026ecc(base + 12, x - width, y, color);
    i = n;
    while (i--) {
        if (digit[i] != 0)
            leading = 0;
        if (!leading)
            func_80026ecc(base + digit[i], x + width * (n - i - 1), y, color);
    }
}

/* Draws a number with the fixed 7-pixel digit glyphs (codes 2-11). */
ADDRESS(0x800276bc, 0x244)
void func_800276bc(u32 num, u16 x, u16 y, CVECTOR *color)
{
    u16 digit[8];
    u8 leading = 1;
    u8 neg = 0;
    u8 n = NUMBER_DIGITS(num);
    u16 width = 7;
    int end;
    u32 sign;
    s16 value;
    u32 pow;
    u32 div;
    int i;

    value = num;
    sign = num & NUMBER_SIGNED;
    end = -1;
    if ((s16)num < 0) {
        value = -num;
        neg = 1;
    }
    if (value == 0) {
        func_80026ecc(2, x + width * (n - 1), y, color);
        return;
    }
    pow = 1;
    i = n - 1;
    if (i != end) {
        do {
            pow *= 10;
        } while (--i != -1);
    }
    if (value < pow) {
        div = pow;
        i = n;
        while (i--) {
            div /= 10;
            digit[i] = value / div;
            value -= digit[i] * div;
        }
    } else {
        i = n;
        while (i--)
            digit[i] = 9;
    }
    if (neg)
        func_80026ecc(13, x - width, y, color);
    else if (sign)
        func_80026ecc(12, x - width, y, color);
    i = n;
    while (i--) {
        if (digit[i] != 0)
            leading = 0;
        if (!leading)
            func_80026ecc(digit[i] + 2, x + width * (n - i - 1), y, color);
    }
}

/* Draws a small arrow pointing along `dir` (0, 1024, 2048 or 3072) at
 * `pos`, over a copy offset by one pixel. Nothing calls it. */
ADDRESS(0x80027900, 0x418)
void func_80027900(DVECTOR *pos, CVECTOR *color, u16 dir)
{
    POLY_F3 *p;
    StPacketBuffer *buf;
    DR_TPAGE *t;
    short x0 = 0, x1 = 0, x2 = 0, y0 = 0, y1 = 0, y2 = 0;

    switch (dir) {
    case 0:
        x0 = pos->vx - 4;
        x1 = pos->vx + 4;
        x2 = x0;
        y0 = pos->vy - 8;
        y1 = pos->vy;
        y2 = y1 + 8;
        break;
    case 1024:
        x0 = pos->vx - 8;
        x1 = pos->vx;
        x2 = x1 + 8;
        y0 = pos->vy - 4;
        y1 = pos->vy + 4;
        y2 = y0;
        break;
    case 2048:
        x0 = pos->vx + 4;
        x1 = pos->vx - 4;
        x2 = x0;
        y0 = pos->vy - 8;
        y1 = pos->vy;
        y2 = y1 + 8;
        break;
    case 3072:
        x0 = pos->vx - 8;
        x1 = pos->vx;
        x2 = x1 + 8;
        y0 = pos->vy + 4;
        y1 = pos->vy - 4;
        y2 = y0;
        break;
    }
    NEW_PRIM(buf, p, POLY_F3);
    setlen(p, 4);
    setcode(p, 0x22);
    setXY3(p, x0, y0, x1, y1, x2, y2);
    setRGB0(p, color->r, color->g, color->b);
    addPrim(&world_state.ot_cur[1], p);
    NEW_PRIM(buf, t, DR_TPAGE);
    SetDrawTPage(t, 0, 1, GetTPage(0, 3, 0, 0));
    addPrim(&world_state.ot_cur[1], t);
    NEW_PRIM(buf, p, POLY_F3);
    setlen(p, 4);
    setcode(p, 0x22);
    setXY3(p, x0 + 1, y0 + 1, x1 + 1, y1 + 1, x2 + 1, y2 + 1);
    setRGB0(p, color->r, color->g, color->b);
    addPrim(&world_state.ot_cur[1], p);
    NEW_PRIM(buf, t, DR_TPAGE);
    SetDrawTPage(t, 0, 1, GetTPage(0, 3, 0, 0));
    addPrim(&world_state.ot_cur[1], t);
}

/* The body of a flat rectangle: the code (semi-transparent when `semi` is
 * 1), the tag linked in front of the ordering table's head, the corners
 * and the colour. */
#define FILL_F4(p, semi, x0, y0, x1, y1, x2, y2, x3, y3, color)    \
    (p)->code = (semi) == 1 ? 0x2a : 0x28;                     \
    (p)->tag = (*world_state.ot_cur & 0xffffff) | 0x05000000;  \
    setXY4(p, x0, y0, x1, y1, x2, y2, x3, y3);                 \
    setRGB0(p, (color)->r, (color)->g, (color)->b);            \
    *world_state.ot_cur = (u_long)(p) & 0xffffff

/* Draws a rectangle outline: bars `border & 0xff` high at the top and
 * bottom and `border >> 8` wide at the sides. */
/* Not exact (size-exact): retail keeps `color` in $t4 and computes
 * `border >> 8` before the world-state address. The bottom/right bars also
 * load y before h and x before w into different operand registers. Explicit
 * coordinate order, separate packet phases and low-border reuse did not
 * resolve those differences. */
ADDRESS(0x80027d18, 0x508)
void func_80027d18(RECT *r, CVECTOR *color, u16 border, u16 semi)
{
    POLY_F4 *p;
    StPacketBuffer *buf;
    int w = border >> 8;
    int h = border & 0xff;

    NEW_PRIM(buf, p, POLY_F4);
    FILL_F4(p, semi, r->x, r->y, r->x + r->w, r->y, r->x, r->y + h, r->x + r->w, r->y + h,
            color);
    NEW_PRIM(buf, p, POLY_F4);
    FILL_F4(p, semi, r->x, r->y + r->h - h, r->x + r->w, r->y + r->h - h, r->x, r->h + r->y,
            r->x + r->w, r->h + r->y, color);
    NEW_PRIM(buf, p, POLY_F4);
    FILL_F4(p, semi, r->x, r->y + h, r->x + w, r->y + h, r->x, r->y + h + (r->h - h * 2),
            r->x + w, r->y + h + (r->h - h * 2), color);
    NEW_PRIM(buf, p, POLY_F4);
    FILL_F4(p, semi, r->x + r->w - w, r->y + h, r->w + r->x, r->y + h,
            r->x + r->w - w, r->y + h + (r->h - h * 2), r->w + r->x, r->y + h + (r->h - h * 2),
            color);
}

/* Fills a rectangle (semi-transparent when `semi` is 1). */
ADDRESS(0x80028220, 0x144)
void func_80028220(RECT *r, CVECTOR *color, u16 semi)
{
    POLY_F4 *p;
    StPacketBuffer *buf;

    NEW_PRIM(buf, p, POLY_F4);
    FILL_F4(p, semi, r->x, r->y, r->x + r->w, r->y, r->x, r->y + r->h, r->x + r->w, r->y + r->h,
            color);
}

/*
 * Draws the texture rectangle `tex` (VRAM page from its x/y, u/v from the
 * low bits) over `r` with CLUT `clut`, tinted by `color` (grey when NULL).
 * `mode`: bit 0 semi-transparent, bits 1-2 blend rate, bits 3-4 colour
 * depth, bits 5-6 flip (1 horizontal, 2 vertical).
 */
ADDRESS(0x80028364, 0x344)
void func_80028364(RECT *r, DVECTOR *tex, DVECTOR *clut, CVECTOR *color, int mode)
{
    POLY_FT4 *p;
    StPacketBuffer *buf;
    int semi = mode & 1;
    int abr = (mode >> 1) & 3;
    int tp = (mode >> 3) & 3;
    int flip = (mode >> 5) & 3;
    short tx, ty;
    u8 u0, v0, u1, v1;

    NEW_PRIM(buf, p, POLY_FT4);
    tx = tex->vx & 0xffc0;
    ty = tex->vy & 0xff00;
    u0 = tex->vx & 0x3f;
    v0 = tex->vy;
    if (tp == 0) {
        if (u0 < 64)
            u0 *= 4;
        else
            u0 = 255;
    } else if (tp == 1) {
        if (u0 < 128)
            u0 *= 2;
        else
            u0 = 255;
    }
    u1 = 255;
    if (u0 + r->w < 256)
        u1 = u0 + r->w;
    v1 = 255;
    if (v0 + r->h < 256)
        v1 = v0 + r->h;
    p->tpage = GetTPage(tp, abr, tx, ty);
    p->clut = GetClut(clut->vx, clut->vy);
    if (color == NULL) {
        setRGB0(p, 128, 128, 128);
    } else {
        setRGB0(p, color->r, color->g, color->b);
    }
    p->tag = (*world_state.ot_cur & 0xffffff) | 0x09000000;
    setXY4(p, r->x, r->y, r->x + r->w, r->y, r->x, r->y + r->h, r->x + r->w, r->y + r->h);
    if (flip == 1)
        setUV4(p, u1 - 1, v0, u0 - 1, v0, u1 - 1, v1, u0 - 1, v1);
    else if (flip == 2)
        setUV4(p, u0, v1 - 1, u1, v1 - 1, u0, v0 - 1, u1, v0 - 1);
    else
        setUV4(p, u0, v0, u1, v0, u0, v1, u1, v1);
    p->code = semi ? 0x2e : 0x2c;
    *world_state.ot_cur = (u_long)p & 0xffffff;
}

/* Adds a draw-mode primitive selecting the texture page at (x, y) with
 * blend rate `abr`. */
ADDRESS(0x800286a8, 0xdc)
void func_800286a8(int abr, int x, int y)
{
    DR_MODE *p;
    StPacketBuffer *buf;

    NEW_PRIM(buf, p, DR_MODE);
    SetDrawMode(p, 0, 0, GetTPage(0, abr, x, y), NULL);
    addPrim(world_state.ot_cur, p);
}

/* Card fade states in world_state.unk1EE16. */
#define CARD_FADE_IN 1
#define CARD_SHOW_IN 2
#define CARD_SHOW 3
#define CARD_SHOW_OUT 4
#define CARD_DONE 5
#define CARD_FADE_OUT 6

/*
 * Steps the title-card overlay one frame: fades the screen, fades the
 * card image in and out, and counts the frames in unk1EE19
 * (unk1EE17 is the card mode, unk1EE18 the fade length).
 */
ADDRESS(0x80028784, 0x4a8)
void func_80028784(void)
{
    RECT screen = {0, 0, 320, 240};
    RECT card = {32, 132, 255, 255};
    CVECTOR fill;
    CVECTOR tint;
    DVECTOR tex = {960, 256};
    DVECTOR clut = {560, 495};

    if (world_state.unk1EE17 == 1) {
        tex.vy = 0;
        clut.vx = 0;
        clut.vy = 511;
    }
    if (world_state.unk1EE18 == 0)
        world_state.unk1EE18 = 1;
    switch (world_state.unk1EE16) {
    case CARD_FADE_IN:
        if (world_state.unk1EE17 != 3)
            fill.r = fill.g = fill.b = (world_state.unk1EE19 << 4) / world_state.unk1EE18;
        else
            fill.r = fill.g = fill.b = 255;
        func_80028220(&screen, &fill, 1);
        func_800286a8(2, 0, 0);
        break;
    case CARD_SHOW_IN:
        if (DAT_801cd120.unk4 == 1 || world_state.unk1EE17 == 2) {
            tint.r = tint.g = tint.b = world_state.unk1EE19 << 3;
            func_80028364(&card, &tex, &clut, &tint, 3);
            if (world_state.unk1EE19 >= 16)
                world_state.unk1EE16 = CARD_SHOW;
        } else {
            world_state.unk1EE16 = CARD_SHOW;
        }
        break;
    case CARD_SHOW:
        if (DAT_801cd120.unk4 == 1 || world_state.unk1EE17 == 2) {
            tint.r = tint.g = tint.b = 128;
            if (DAT_80088190 == 25 || DAT_80088190 == 30 || DAT_80088190 == 5)
                func_80028364(&card, &tex, &clut, &tint, 2);
            else
                func_80028364(&card, &tex, &clut, &tint, 3);
        }
        break;
    case CARD_SHOW_OUT:
        if (DAT_801cd120.unk4 == 1 || world_state.unk1EE17 == 2) {
            tint.r = tint.g = tint.b = (16 - world_state.unk1EE19) << 3;
            func_80028364(&card, &tex, &clut, &tint, 3);
            if (world_state.unk1EE19 >= 16)
                world_state.unk1EE16 = CARD_DONE;
        } else {
            world_state.unk1EE16 = CARD_DONE;
        }
        break;
    case CARD_FADE_OUT:
        if (world_state.unk1EE17 != 3)
            fill.r = fill.g = fill.b =
                ((world_state.unk1EE18 - world_state.unk1EE19) << 4) / world_state.unk1EE18;
        else
            fill.r = fill.g = fill.b = 255;
        func_80028220(&screen, &fill, 1);
        func_800286a8(2, 0, 0);
        break;
    default:
        return;
    }
    if (world_state.unk1EE17 == 0 && world_state.unk1EE16 >= CARD_SHOW_IN &&
        world_state.unk1EE16 < CARD_FADE_OUT) {
        fill.r = fill.g = fill.b = 16;
        func_80028220(&screen, &fill, 1);
        func_800286a8(2, 0, 0);
    }
    if (world_state.unk1EE17 == 3) {
        fill.r = fill.g = fill.b = 255;
        func_80028220(&screen, &fill, 1);
        func_800286a8(2, 0, 0);
    }
    world_state.unk1EE19++;
}

/* Draws a Gouraud quad: corner positions and colours in setXY4 order. */
ADDRESS(0x80028c2c, 0x18c)
void func_80028c2c(DVECTOR *xy, CVECTOR *rgb, int semi)
{
    StPacketBuffer *buf;
    POLY_G4 *p;

    NEW_PRIM(buf, p, POLY_G4);
    p->tag = (*world_state.ot_cur & 0xffffff) | 0x08000000;
    setRGB0(p, rgb[0].r, rgb[0].g, rgb[0].b);
    setRGB1(p, rgb[1].r, rgb[1].g, rgb[1].b);
    setRGB2(p, rgb[2].r, rgb[2].g, rgb[2].b);
    setRGB3(p, rgb[3].r, rgb[3].g, rgb[3].b);
    setXY4(p, xy[0].vx, xy[0].vy, xy[1].vx, xy[1].vy, xy[2].vx, xy[2].vy, xy[3].vx, xy[3].vy);
    p->code = semi == 0 ? 0x38 : 0x3a;
    *world_state.ot_cur = (u_long)p & 0xffffff;
}

/* Fills `r` with a vertical gradient from `edge` at the top and bottom to
 * `mid` across the middle. */
ADDRESS(0x80028db8, 0x3b4)
void func_80028db8(RECT *r, CVECTOR *mid, CVECTOR *edge, int semi)
{
    StPacketBuffer *buf;
    StPacketBuffer *lower;
    POLY_G4 *p;
    RECT b;

    b = *r;
    b.h = b.h >> 1;
    NEW_PRIM(buf, p, POLY_G4);
    p->code = semi == 0 ? 0x38 : 0x3a;
    p->tag = (*world_state.ot_cur & 0xffffff) | 0x08000000;
    setRGB0(p, edge->r, edge->g, edge->b);
    setRGB1(p, edge->r, edge->g, edge->b);
    setRGB2(p, mid->r, mid->g, mid->b);
    setRGB3(p, mid->r, mid->g, mid->b);
    setXY4(p, b.x, b.y, b.x + b.w, b.y, b.x, b.y + b.h, b.x + b.w, b.y + b.h);
    *world_state.ot_cur = (u_long)p & 0xffffff;
    b = *r;
    b.h = b.h >> 1;
    b.y += b.h;
    NEW_PRIM(lower, p, POLY_G4);
    p->code = semi == 0 ? 0x38 : 0x3a;
    p->tag = (*world_state.ot_cur & 0xffffff) | 0x08000000;
    setRGB0(p, mid->r, mid->g, mid->b);
    setRGB1(p, mid->r, mid->g, mid->b);
    setRGB2(p, edge->r, edge->g, edge->b);
    setRGB3(p, edge->r, edge->g, edge->b);
    setXY4(p, b.x, b.y, b.x + b.w, b.y, b.x, b.y + b.h, b.x + b.w, b.y + b.h);
    *world_state.ot_cur = (u_long)p & 0xffffff;
}

/* Draws a triangle filling `r` that points along `dir` (0, 1024, 2048 or
 * 3072), over a copy offset by one pixel. */
ADDRESS(0x8002916c, 0x320)
void func_8002916c(RECT *r, CVECTOR *color, u16 dir)
{
    short x0 = 0, x1 = 0, x2 = 0, y0 = 0, y1 = 0, y2 = 0;
    StPacketBuffer *buf;
    POLY_F3 *p;

    switch (dir) {
    case 0:
        x0 = r->x - r->h / 2;
        x1 = r->x + r->h / 2;
        x2 = x0;
        y0 = r->y - r->w / 2;
        y1 = r->y;
        y2 = y1 + r->w / 2;
        break;
    case 1024:
        x0 = r->x - r->w / 2;
        x1 = r->x;
        x2 = x1 + r->w / 2;
        y0 = r->y - r->h / 2;
        y1 = r->y + r->h / 2;
        y2 = y0;
        break;
    case 2048:
        x0 = r->x + r->h / 2;
        x1 = r->x - r->h / 2;
        x2 = x0;
        y0 = r->y - r->w / 2;
        y1 = r->y;
        y2 = y1 + r->w / 2;
        break;
    case 3072:
        x0 = r->x - r->w / 2;
        x1 = r->x;
        x2 = x1 + r->w / 2;
        y0 = r->y + r->h / 2;
        y1 = r->y - r->h / 2;
        y2 = y0;
        break;
    }
    NEW_PRIM(buf, p, POLY_F3);
    setlen(p, 4);
    setcode(p, 0x22);
    setXY3(p, x0, y0, x1, y1, x2, y2);
    setRGB0(p, color->r, color->g, color->b);
    addPrim(world_state.ot_cur, p);
    NEW_PRIM(buf, p, POLY_F3);
    setlen(p, 4);
    setcode(p, 0x22);
    setXY3(p, x0 + 1, y0 + 1, x1 + 1, y1 + 1, x2 + 1, y2 + 1);
    setRGB0(p, color->r, color->g, color->b);
    addPrim(world_state.ot_cur, p);
}

/* Copies the screen into the spare buffer's image and darkens it. */
ADDRESS(0x8002948c, 0xbc)
void func_8002948c(void)
{
    RECT screen = {0, 0, 320, 240};
    CVECTOR shade = {32, 32, 32};

    LoadImage(&world_state.draw[world_state.frame].clip, world_state.packets[1].end);
    func_80028220(&screen, &shade, 1);
    func_800286a8(2, 0, 0);
}

/* Draws a menu panel: a dark fill over a lighter border, both blended. */
ADDRESS(0x80029548, 0xd0)
void func_80029548(int x, int y, int w, int h)
{
    MENU_SCRATCH->unk338.g = 80;
    MENU_SCRATCH->unk338.r = 80;
    MENU_SCRATCH->unk338.b = 96;
    MENU_SCRATCH->unk344.g = 40;
    MENU_SCRATCH->unk344.r = 40;
    MENU_SCRATCH->unk33C.x = x;
    MENU_SCRATCH->unk33C.y = y;
    MENU_SCRATCH->unk33C.w = w;
    MENU_SCRATCH->unk33C.h = h;
    MENU_SCRATCH->unk344.b = 48;
    MENU_SCRATCH->unk348.x = x + 2;
    MENU_SCRATCH->unk348.y = y + 1;
    MENU_SCRATCH->unk348.w = w - 4;
    MENU_SCRATCH->unk348.h = h - 2;
    func_80028220(&MENU_SCRATCH->unk348, &MENU_SCRATCH->unk344, 1);
    func_800286a8(2, 0, 0);
    func_80028220(&MENU_SCRATCH->unk33C, &MENU_SCRATCH->unk338, 1);
    func_800286a8(0, 0, 0);
}

/* Tile modes for func_80028364: semi-transparent, optionally flipped. */
#define TILE (1)
#define TILE_HFLIP (1 | 1 << 5)
#define TILE_VFLIP (1 | 2 << 5)

/*
 * Draws a window frame from 16x16 tiles of the menu texture page: corners,
 * edges, an inner bevel and the filled middle, then a grey blended wash.
 */
/* Not exact (4 bytes short): retail spills `right` and keeps w/h in s7/s6;
 * this build spills w. Distinct tile-phase counters improve the loop shape.
 * A word-sized right edge adds sign extensions absent from retail, while
 * halfword signedness does not change the code. */
ADDRESS(0x80029618, 0x55c)
void func_80029618(short x, short y, short w, short h)
{
    MenuScratch *sp = MENU_SCRATCH;
    RECT r;
    CVECTOR wash = {48, 48, 48};
    short right;
    short bottom;

    sp->unk33C.w = 16;
    sp->unk33C.h = 16;
    sp->unk35C.vx = 544;
    sp->unk35C.vy = 504;
    sp->unk358.vx = 612;
    sp->unk33C.x = x;
    sp->unk33C.y = y;
    sp->unk358.vy = 400;
    func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, TILE);
    sp->unk33C.y = y + h - 16;
    func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, TILE_VFLIP);
    right = x + w - 16;
    sp->unk33C.y = y;
    sp->unk358.vx = 620;
    sp->unk33C.x = right;
    func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, TILE);
    sp->unk33C.y = y + h - 16;
    func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, TILE_VFLIP);
    sp->unk33C.x = x;
    sp->unk33C.y = y + 16;
    sp->unk358.vx = 604;
    func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, TILE);
    sp->unk33C.y = y + h - 32;
    func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, TILE_VFLIP);
    sp->unk33C.y = y + 16;
    sp->unk358.vx = 596;
    sp->unk33C.x = right;
    func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, TILE);
    sp->unk33C.y = y + h - 32;
    func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, TILE_VFLIP);

    sp->unk33C.y = y + 32;
    sp->unk33C.x = x;
    sp->unk358.vx = 624;
    {
        int rows = h / 16 - 4;
        while (rows--) {
            sp->unk33C.x = x;
            func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, TILE_HFLIP);
            sp->unk33C.x = right;
            func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, TILE);
            sp->unk33C.y += 16;
        }
    }
    sp->unk33C.x = x + 16;
    sp->unk358.vx = 616;
    {
        int columns = w / 16 - 2;
        while (columns--) {
            sp->unk33C.y = y;
            func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, TILE);
            sp->unk33C.y = y + h - 16;
            func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, TILE_VFLIP);
            sp->unk33C.x += 16;
        }
    }
    sp->unk33C.x = x + 16;
    sp->unk358.vx = 608;
    {
        int columns = w / 16 - 2;
        while (columns--) {
            sp->unk33C.y = y + 16;
            func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, TILE);
            sp->unk33C.y = y + h - 32;
            func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, TILE_VFLIP);
            sp->unk33C.x += 16;
        }
    }
    sp->unk33C.y = y + 32;
    sp->unk358.vx = 600;
    {
        int rows = h / 16 - 4;
        while (rows--) {
            int columns;

            sp->unk33C.x = x + 16;
            columns = w / 16 - 2;
            while (columns--) {
                func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, TILE);
                sp->unk33C.x += 16;
            }
            sp->unk33C.y += 16;
        }
    }
    r.x = x;
    r.y = y;
    r.w = w;
    r.h = h;
    func_80028220(&r, &wash, 1);
    func_800286a8(2, 0, 0);
}

/*
 * The pause menu pages. Each page restarts the frame,
 * draws its text, numbers and icons with the text primitives above,
 * frames itself with func_80029618 and darkens the world behind it.
 */

/* Blink phase of the map page's arrows (0..31). */
DATA(0x80087f34, 0x4, ".sdata")
static s32 DAT_80087f34 = 0;

/* func_80027440/func_800276bc take the digit count in the top byte. */
#define DIGITS(n) ((n) << 24)

/* Closes a page: its frame, the end of the frame and the darkened world. */
#define PAGE_END()                     \
    func_80029618(16, 36, 288, 192);   \
    func_800175a4();                   \
    func_8002948c();                   \
    func_80017634()

/* The status page. */
ADDRESS(0x80029b74, 0x65c)
void func_80029b74(void)
{
    u16 label_x[8] = {37, 47, 20, 20, 30, 41, 34, 37};
    DVECTOR row[4] = {{196, 94}, {274, 94}, {172, 104}, {298, 104}};
    CVECTOR lit[4] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {128, 64, 0, 0}, {128, 64, 0, 0}};
    CVECTOR dim[4] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 64, 128, 0}, {0, 64, 128, 0}};
    MenuScratch *sp;
    int i;
    int kind;
    int n;

    func_800174c4();
    sp = MENU_SCRATCH;
    sp->style[0].r = 72;
    sp->style[0].g = 72;
    sp->style[0].b = 72;
    sp->style[2].r = 98;
    sp->style[2].g = 66;
    sp->style[2].b = 48;
    sp->style[2].cd = 57;
    sp->style[0].cd = 57;
    for (i = 0; i < 11; i++)
        func_8002732c(MESSAGE(i), 24, i * 16 + 48, &sp->style[0]);
    if (viewer.unk2C & 7)
        func_8002732c(MESSAGE(20), 106, 112, &sp->style[0]);
    else if (viewer.unk2C & 0x18)
        func_8002732c(MESSAGE(21), 93, 112, &sp->style[0]);
    else if (viewer.unk2C & 0x60)
        func_8002732c(MESSAGE(22), 119, 112, &sp->style[0]);
    else if (viewer.unk2C & 0x80)
        func_8002732c(MESSAGE(23), 119, 112, &sp->style[0]);
    else if (viewer.unk2C & 0x100)
        func_8002732c(MESSAGE(24), 106, 112, &sp->style[0]);
    else
        func_8002732c(MESSAGE(19), 106, 112, &sp->style[0]);
    if (viewer.unk38[16] != 0xffff && vis_table_c.unk0118[viewer.unk38[16]].unk5[0x1d] >= 9) {
        kind = vis_table_c.unk0118[viewer.unk38[16]].unk5[0x1d];
        n = 0;
        if (kind - 9 == 2)
            n = 14;
        func_8002732c(MESSAGE(17 + kind), 119 - n, 208, &sp->style[0]);
    } else {
        func_8002732c(MESSAGE(25), 119, 208, &sp->style[0]);
    }
    for (i = 0; i < 8; i++)
        func_8002732c(MESSAGE(11 + i), label_x[i] + 172, i * 16 + 90, &sp->style[2]);
    func_800276bc(viewer.unk2 | DIGITS(4), 78, 48, &sp->style[0]);
    i = 0;
    func_80026ecc(15, 106, 48, &sp->style[0]);
    func_800276bc(viewer.unk0 | DIGITS(4), 120, 48, &sp->style[0]);
    func_800276bc(viewer.unk6 | DIGITS(4), 78, 64, &sp->style[0]);
    func_80026ecc(15, 106, 64, &sp->style[0]);
    func_800276bc(viewer.unk4 | DIGITS(4), 120, 64, &sp->style[0]);
    func_800276bc(viewer.unk8 | DIGITS(4), 78, 80, &sp->style[0]);
    func_80026ecc(15, 106, 80, &sp->style[0]);
    func_800276bc(4921 | DIGITS(4), 120, 80, &sp->style[0]);
    func_800276bc(DAT_801c7590.counts[0x10a].count | DIGITS(3), 127, 96, &sp->style[0]);
    func_800276bc(viewer.unk10 | DIGITS(3), 127, 128, &sp->style[0]);
    func_800276bc(viewer.unkE | DIGITS(3), 127, 144, &sp->style[0]);
    func_800276bc(viewer.unkBA[0] | DIGITS(3), 127, 160, &sp->style[0]);
    func_800276bc(viewer.unkBA[1] | DIGITS(3), 127, 176, &sp->style[0]);
    func_800276bc(viewer.unkBA[2] | DIGITS(3), 127, 192, &sp->style[0]);
    sp->unk358.vx = 622;
    sp->unk358.vy = 416;
    sp->unk35C.vx = 544;
    sp->unk35C.vy = 511;
    sp->unk33C.x = 196;
    sp->unk33C.y = 54;
    sp->unk33C.w = 72;
    sp->unk33C.h = 24;
    func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, 1);
    for (; i < 8; i++) {
        if (i == (s16)sp->unk2DE)
            func_80028c2c(row, lit, 1);
        else
            func_80028c2c(row, dim, 1);
        row[0].vy += 16;
        row[1].vy += 16;
        row[2].vy += 16;
        row[3].vy += 16;
    }
    func_800286a8(0, 0, 0);
    PAGE_END();
}

/* A scrolling list page (five rows of icon, name and count). */
ADDRESS(0x8002a1d0, 0x3d4)
void func_8002a1d0(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    int id = sp->unk0.page_list.entries[sp->unk2E2 + sp->unk2E0];
    int entry;
    int rows;
    int i;
    int red, green;

    func_800174c4();
    sp->style[0].r = sp->style[4].r = 72;
    sp->style[0].g = sp->style[4].g = 72;
    sp->style[0].b = sp->style[4].b = 72;
    sp->style[2].r = 98;
    sp->style[2].g = 66;
    sp->style[2].b = 48;
    sp->style[2].cd = 57;
    sp->style[0].cd = 57;
    sp->style[4].cd = 41;
    if (sp->unk2E4 < 16)
        sp->unk360 = 8 - sp->unk2E4;
    else
        sp->unk360 = sp->unk2E4 - 24;
    sp->unk330.x = 23;
    sp->unk330.w = 276;
    red = (u8)sp->unk360 * 4 + 96;
    sp->unk32C.b = 0;
    green = (u8)sp->unk360 * 2 + 48;
    sp->unk330.h = 26;
    sp->unk32C.r = red;
    sp->unk32C.g = green;
    sp->unk330.y = sp->unk2E0 * 26 + 69;
    func_80027d18(&sp->unk330, &sp->unk32C, 257, 1);
    func_800286a8(1, 0, 0);
    sp->unk350.x = 24;
    sp->unk350.y = 70;
    sp->unk350.w = 24;
    sp->unk350.h = 24;
    rows = 5;
    if (sp->unk0.page_list.count < 6)
        rows = sp->unk0.page_list.count;
    sp->style[0].cd |= 0x80;
    if (sp->unk2E0 < rows)
        func_8002732c(MESSAGE_IN(sp->unk0.page_list.messages, 0x176 + id), 32, 208, &sp->style[0]);
    for (i = 0; i < rows; i++) {
        entry = sp->unk0.page_list.entries[sp->unk2E2 + i];
        func_8002732c(MESSAGE_IN(sp->unk0.page_list.messages, entry), 54, i * 26 + 76, &sp->style[0]);
        func_80027440(DAT_801c7590.counts[entry].count | DIGITS(2), 274, i * 26 + 76, &sp->style[4]);
        func_80028364(&sp->unk350, &sp->unk0.icons[(sp->unk2E2 + i) % 5 + 1], NULL, NULL, 16);
        sp->unk350.y += 26;
    }
    sp->unk35C.vx = 544;
    sp->unk35C.vy = 511;
    sp->unk33C.x = 70;
    sp->unk33C.y = 44;
    sp->unk33C.w = 35;
    sp->unk33C.h = 16;
    sp->unk358.vx = 624;
    sp->unk358.vy = 448;
    sp->style[0].cd &= 0x7f;
    func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, 3);
    sp->unk358.vx = 633;
    sp->unk33C.x = 32;
    sp->unk358.vy = 448;
    sp->unk33C.w = 30;
    func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, 3);
    PAGE_END();
}

/* The attribute page: seven rows of two columns. */
ADDRESS(0x8002a5a4, 0x3b0)
void func_8002a5a4(void)
{
    MenuScratch *sp;
    CVECTOR *left, *left_num, *right, *right_num;
    int i;

    func_800174c4();
    sp = MENU_SCRATCH;
    sp->style[0].r = sp->style[4].r = 42;
    sp->style[0].g = sp->style[4].g = 42;
    sp->style[0].b = sp->style[4].b = 42;
    sp->style[2].r = sp->style[6].r = 98;
    sp->style[2].g = sp->style[6].g = 66;
    sp->style[2].b = sp->style[6].b = 48;
    sp->style[2].cd = 57;
    sp->style[0].cd = 57;
    sp->style[6].cd = 41;
    sp->style[4].cd = 41;
    sp->unk33C.y = 70;
    sp->unk33C.w = 10;
    sp->unk33C.h = 10;
    sp->unk358.vx = 597;
    sp->unk358.vy = 433;
    sp->unk35C.vx = 544;
    sp->unk35C.vy = 502;
    for (i = 0; i < 7; i++) {
        if (sp->unk0.stats.selected == i) {
            left = &sp->style[2];
            left_num = &sp->style[6];
        } else {
            left = &sp->style[0];
            left_num = &sp->style[4];
        }
        if (sp->unk0.stats.selected == i + 7) {
            right = &sp->style[2];
            right_num = &sp->style[6];
        } else {
            right = &sp->style[0];
            right_num = &sp->style[4];
        }
        func_8002732c(MESSAGE(0x212 + i), 46, i * 20 + 68, left);
        func_8002732c(MESSAGE(0x219 + i), 170, i * 20 + 68, right);
        func_80027440((sp->unk0.stats.base[0][i] + sp->unk0.stats.bonus[0][i]) | DIGITS(3), 108, i * 20 + 68, left_num);
        func_80027440((sp->unk0.stats.base[1][i] + sp->unk0.stats.bonus[1][i]) | DIGITS(3), 232, i * 20 + 68, right_num);
        sp->unk33C.x = 141;
        func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, left, 3);
        sp->unk33C.x = 265;
        func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, right, 3);
        sp->unk33C.y += 20;
    }
    sp->style[0].r = sp->style[4].r = 72;
    sp->style[0].g = sp->style[4].g = 72;
    sp->style[0].b = sp->style[4].b = 72;
    func_8002732c(MESSAGE(0x220), 56, 208, &sp->style[0]);
    func_80027440(sp->unk0.stats.value | DIGITS(2), 242, 208, &sp->style[4]);
    sp->unk35C.vx = 544;
    sp->unk35C.vy = 511;
    sp->unk33C.x = 70;
    sp->unk33C.y = 44;
    sp->unk33C.w = 76;
    sp->unk33C.h = 16;
    sp->unk358.vx = 621;
    sp->unk358.vy = 480;
    func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, 3);
    sp->unk358.vx = 633;
    sp->unk358.vy = 448;
    sp->unk33C.x = 32;
    sp->unk33C.w = 30;
    func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, 3);
    PAGE_END();
}

/* The map page: visited areas and their completion, or the area summary. */
ADDRESS(0x8002a954, 0x994)
void func_8002a954(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    u8 total[48] = {
        255, 18, 25, 21, 20, 19, 20, 255, 25, 27, 29, 32, 28, 24, 26, 23,
        255, 19, 25, 255, 27, 13, 255, 22, 24, 23, 22, 32, 24, 255, 255, 255,
        32, 12, 255, 255, 255, 11, 4, 255, 255, 255, 255, 255, 255, 255, 255, 0,
    };
    CVECTOR arrow;
    RECT unused;
    RECT up = {159, 130, 16, 8};
    RECT next = {296, 130, 16, 8};
    RECT prev = {23, 130, 16, 8};
    RECT down = {160, 130, 16, 8};
    s16 name_x[42] = {
        0,  42, 35, 42, 35, 28, 21, 21, 42, 21, 35, 35, 14, 14, 28, 0,  35, 42, 0,  35, 28,
        0,  28, 42, 21, 42, 7,  42, 0,  0,  0,  35, 21, 0,  0,  0,  35, 42, 0,  42, 0,  0,
    };
    u32 time;
    int hours;
    int minutes;
    int i;
    int id;
    int level;

    func_800174c4();
    if (DAT_80087f34 < 16)
        sp->unk360 = 8 - DAT_80087f34;
    else
        sp->unk360 = DAT_80087f34 - 24;
    sp->style[0].r = sp->style[4].r = 72;
    sp->style[0].g = sp->style[4].g = 72;
    sp->style[0].b = sp->style[4].b = 72;
    sp->style[2].r = sp->style[6].r = 98;
    sp->style[2].g = sp->style[6].g = 66;
    sp->style[2].b = sp->style[6].b = 48;
    sp->style[2].cd = 57;
    sp->style[0].cd = 57;
    sp->style[6].cd = 41;
    sp->style[4].cd = 41;
    sp->unk32C.b = 0;
    sp->unk32C.r = (u8)sp->unk360 * 4 + 96;
    sp->unk32C.g = (u8)sp->unk360 * 4 + 48;
    if (DAT_800881a8 != 0) {
        for (i = 0; i < 10; i++) {
            id = sp->unk0.map.ids[i];
            if (id >= 90) {
                if (id == 99)
                    func_8002732c(MESSAGE(0x269), 53, i * 16 + 54, &sp->style[0]);
                else if (id == sp->unk0.map.current)
                    func_8002732c(MESSAGE(0x208 + id), 53, i * 16 + 54, &sp->style[2]);
                else
                    func_8002732c(MESSAGE(0x208 + id), 53, i * 16 + 54, &sp->style[0]);
            } else if (id == 89) {
                func_8002732c(MESSAGE(0x26a), 67, i * 16 + 54, &sp->style[0]);
            } else if (id == area_state.area) {
                func_8002732c(MESSAGE(0x2bc + id), 67, i * 16 + 54, &sp->style[2]);
                func_80027440(DAT_801cba08.unk16E4[sp->unk0.map.ids[i]] * 100 / total[sp->unk0.map.ids[i]] | DIGITS(3),
                              223, i * 16 + 54, &sp->style[6]);
                func_80026ecc(80, 256, i * 16 + 54, &sp->style[2]);
            } else {
                func_8002732c(MESSAGE(0x2bc + id), 67, i * 16 + 54, &sp->style[0]);
                func_80027440(DAT_801cba08.unk16E4[sp->unk0.map.ids[i]] * 100 / total[sp->unk0.map.ids[i]] | DIGITS(3),
                              223, i * 16 + 54, &sp->style[4]);
                func_80026ecc(80, 256, i * 16 + 54, &sp->style[0]);
            }
        }
    } else {
        func_8002732c(MESSAGE(0x258), 72, 53, &sp->style[2]);
        for (i = 1; i < 5; i++)
            func_8002732c(MESSAGE(0x258 + i), 72, i * 30 + 63, &sp->style[2]);
        if (sp->unk0.map.current != -1) {
            func_8002732c(MESSAGE(0x208 + sp->unk0.map.current), 86, 66, &sp->style[0]);
            func_8002732c(MESSAGE(0x2bc + area_state.area), name_x[area_state.area] + 100, 79,
                          &sp->style[0]);
        } else {
            func_8002732c(MESSAGE(0x2bc + area_state.area), 100, 73, &sp->style[0]);
        }
        func_80027440(viewer.unk27 | DIGITS(3), 179, 107, &sp->style[4]);
        func_80026ecc(15, 213, 107, &sp->style[0]);
        func_80027440(99 | DIGITS(3), 221, 107, &sp->style[4]);
        func_80027440(viewer.unk28 | DIGITS(3), 179, 138, &sp->style[4]);
        func_80026ecc(15, 213, 138, &sp->style[0]);
        func_80027440(265 | DIGITS(3), 221, 138, &sp->style[4]);
        func_80027440(viewer.unk30 | DIGITS(3), 179, 168, &sp->style[4]);
        func_80026ecc(15, 213, 168, &sp->style[0]);
        func_80027440(630 | DIGITS(3), 221, 168, &sp->style[4]);
        time = (u32)viewer.unk34 / 60;
        minutes = time / 60;
        hours = minutes;
        minutes = time - hours * 60;
        func_80027440((u16)hours | DIGITS(2), 199, 198, &sp->style[4]);
        func_80026ecc(27, 224, 198, &sp->style[0]);
        func_80027440((u16)(minutes / 10) | DIGITS(1), 231, 198, &sp->style[4]);
        func_80027440((u16)(minutes % 10) | DIGITS(1), 241, 198, &sp->style[4]);
    }
    DAT_80087f34 = (DAT_80087f34 + 1) % 32;
    if (DAT_80087f34 < 16)
        level = 8 - (u16)DAT_80087f34;
    else
        level = (u16)DAT_80087f34 - 24;
    arrow.b = arrow.g = arrow.r = level * 4 + 96;
    if (DAT_800881a8 != 0) {
        if (DAT_8008818c != 0) {
            sp->unk33C.x = 160;
            sp->unk33C.y = 43;
            sp->unk33C.w = 16;
            sp->unk33C.h = 8;
            func_8002916c(&sp->unk33C, &sp->unk32C, 3072);
        }
        if (DAT_8008818c != 25) {
            sp->unk33C.x = 160;
            sp->unk33C.y = 220;
            sp->unk33C.w = 16;
            sp->unk33C.h = 8;
            func_8002916c(&sp->unk33C, &sp->unk32C, 1024);
        }
    }
    sp->unk33C.x = 28;
    sp->unk33C.y = 48;
    sp->unk33C.w = 264;
    sp->unk33C.h = 168;
    sp->unk338.r = 80;
    sp->unk338.g = 88;
    sp->unk338.b = 96;
    if (DAT_800881a8 != 0)
        func_8002916c(&prev, &sp->unk32C, 2048);
    else
        func_8002916c(&next, &sp->unk32C, 0);
    func_800286a8(1, 0, 0);
    func_80027d18(&sp->unk33C, &sp->unk338, 257, 1);
    PAGE_END();
}

/*
 * The option page. `wash` is func_8002948c's {32, 32, 32}: GCC merged the
 * two constants.
 */
ADDRESS(0x8002b2e8, 0x564)
void func_8002b2e8(void)
{
    CVECTOR black = {0};
    CVECTOR shadow = {24, 24, 24};
    CVECTOR light = {96, 96, 96};
    CVECTOR wash = {32, 32, 32};
    DVECTOR row[4] = {{48, 84}, {114, 84}, {32, 94}, {130, 94}};
    CVECTOR lit[4] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {128, 64, 0, 0}, {128, 64, 0, 0}};
    CVECTOR dim[4] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 64, 128, 0}, {0, 64, 128, 0}};
    u16 label_x[6] = {15, 12, 12, 19, 19, 5};
    MenuScratch *sp;
    u8 *on;
    int i;
    int y;

    func_800174c4();
    y = 80;
    sp = MENU_SCRATCH;
    sp->style[0].r = 72;
    sp->style[0].g = 72;
    sp->style[0].b = 72;
    sp->style[2].r = 98;
    sp->style[2].g = 66;
    sp->style[2].b = 48;
    sp->style[3].r = 24;
    sp->style[3].g = 24;
    sp->style[3].b = 24;
    sp->style[3].cd = 57;
    sp->style[2].cd = 57;
    sp->style[0].cd = 57;
    sp->unk358.vx = 600;
    sp->unk358.vy = 416;
    sp->unk35C.vx = 544;
    sp->unk35C.vy = 511;
    sp->unk33C.y = 83;
    sp->unk33C.w = 5;
    sp->unk33C.h = 5;
    sp->unk33C.x = DAT_801cd120.unk1 * 83 / 63 + 180;
    func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, 0);
    sp->unk358.vx = 576;
    sp->unk33C.x = 176;
    sp->unk33C.y = 78;
    sp->unk33C.w = 96;
    sp->unk33C.h = 14;
    sp->unk358.vy = 416;
    func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, 1);
    for (i = 0; i < 6; y += 20, i++)
        func_8002732c(MESSAGE(0xc9 + i), label_x[i] + 32, y, &sp->style[2]);
    for (on = &DAT_801cd120.unk2, i = 1; i < 5; on++, i++) {
        if (*on == 1) {
            func_8002732c(MESSAGE(0xcf), 193, i * 20 + 80, &sp->style[2]);
            func_8002732c(MESSAGE(0xd0), 241, i * 20 + 80, &sp->style[3]);
        } else {
            func_8002732c(MESSAGE(0xcf), 193, i * 20 + 80, &sp->style[3]);
            func_8002732c(MESSAGE(0xd0), 241, i * 20 + 80, &sp->style[2]);
        }
    }
    func_800276bc(DAT_801cd120.unk1 | DIGITS(2), 280, 80, &sp->style[0]);
    func_8002732c(MESSAGE(0xe0 + sp->unk2E0), 64, 208, &sp->style[0]);
    sp->unk358.vx = 596;
    sp->unk358.vy = 448;
    sp->unk35C.vx = 544;
    sp->unk35C.vy = 511;
    sp->unk33C.x = 32;
    sp->unk33C.y = 44;
    sp->unk33C.w = 70;
    sp->unk33C.h = 16;
    func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, 3);
    for (i = 0; i < 6; i++) {
        if (i == sp->unk2E0)
            func_80028c2c(row, lit, 1);
        else
            func_80028c2c(row, dim, 1);
        row[0].vy += 20;
        row[1].vy += 20;
        row[2].vy += 20;
        row[3].vy += 20;
    }
    func_800286a8(0, 0, 0);
    PAGE_END();
}

/*
 * A settings page: two columns of six labels (lit while their DAT_801cd180.unk22
 * bit is set in the page flags), twelve level gauges and two buttons
 * below them.
 * The masks are indexed through &DAT_801cd180.unk22[0] so the base register
 * holds the array address (retail's lui/addiu of DAT_801cd180+0x22).
 *
 * Not exact (12 bytes long): conditional draw paths now pick each label
 * colour before its message and x. The y normalization is duplicated and
 * label positions are indexed rather than walked as in retail. The level
 * byte's address still follows the row divide.
 */
ADDRESS(0x8002b84c, 0x804)
void func_8002b84c(void)
{
    DVECTOR row[4] = {{48, 84}, {114, 84}, {32, 94}, {130, 94}};
    CVECTOR lit[4] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {128, 64, 0, 0}, {128, 64, 0, 0}};
    CVECTOR dim[4] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 64, 128, 0}, {0, 64, 128, 0}};
    u16 label_x[14] = {26, 36, 20, 16, 20, 16, 27, 20, 17, 20, 34, 16, 10, 37};
    MenuScratch *sp;
    int i;

    func_800174c4();
    sp = MENU_SCRATCH;
    sp->style[0].b = sp->style[0].g = sp->style[0].r = 72;
    sp->style[2].r = 98;
    sp->style[2].g = 66;
    sp->style[2].b = 48;
    sp->style[3].r = 96;
    sp->style[3].b = sp->style[3].g = 32;
    sp->style[0].cd = sp->style[2].cd = sp->style[3].cd = 57;
    if (sp->unk2E4 < 16)
        sp->unk360 = 8 - sp->unk2E4;
    else
        sp->unk360 = sp->unk2E4 - 24;
    if (sp->unk2E2 == 1) {
        sp->unk32C.b = 0;
        lit[2].r += (u8)sp->unk360 * 8;
        lit[2].g += (u8)sp->unk360 * 4;
        lit[3].r += (u8)sp->unk360 * 8;
        lit[3].g += (u8)sp->unk360 * 4;
        sp->unk32C.r = (u8)sp->unk360 * 4 + 96;
        sp->unk32C.g = (u8)sp->unk360 * 2 + 48;
    } else {
        sp->unk32C = sp->style[2];
    }
    for (i = 0; i < 6; i++) {
        if ((&DAT_801cd180.unk22[0])[i] & sp->unk0.levels.flags)
            func_8002732c(MESSAGE(209 + i), label_x[i] + 32, i * 18 + 80, &sp->style[3]);
        else
            func_8002732c(MESSAGE(209 + i), label_x[i] + 32, i * 18 + 80, &sp->style[2]);
    }
    for (i = 0; i < 6; i++) {
        if ((&DAT_801cd180.unk22[0])[i + 6] & sp->unk0.levels.flags)
            func_8002732c(MESSAGE(215 + i), label_x[i + 6] + 168, i * 18 + 80, &sp->style[3]);
        else
            func_8002732c(MESSAGE(215 + i), label_x[i + 6] + 168, i * 18 + 80, &sp->style[2]);
    }
    sp->unk358.vy = 384;
    sp->unk35C.vx = 544;
    for (i = 0; i < 12; i++) {
        sp->unk33C.x = i < 6 ? 138 : 274;
        sp->unk33C.y = i % 6 * 18 + 79;
        if (sp->unk0.levels.level[i] >= 8) {
            sp->unk33C.w = 32;
            sp->unk35C.vy = 506;
            sp->unk358.vx = sp->unk0.levels.level[i] * 8 + 544;
            sp->unk33C.x -= 8;
        } else {
            sp->unk33C.w = 16;
            sp->unk358.vx = sp->unk0.levels.level[i] * 4 + 576;
            if (sp->unk0.levels.level[i] < 4)
                sp->unk35C.vy = 510 - sp->unk0.levels.level[i];
            else
                sp->unk35C.vy = 506;
        }
        sp->unk338.r = sp->unk338.g = sp->unk338.b = (u8)sp->unk360 * 4 - 128;
        if (i == sp->unk2E0 && sp->unk2E2 == 1)
            func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, &sp->unk338, 0);
        else
            func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, 0);
    }
    sp->unk358.vx = 576;
    sp->unk358.vy = 432;
    sp->unk33C.x = 32;
    sp->unk35C.vx = 544;
    sp->unk35C.vy = 511;
    sp->unk33C.y = 44;
    sp->unk33C.w = 60;
    sp->unk33C.h = 16;
    func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, 3);
    sp->unk358.vx = 590;
    sp->unk358.vy = 480;
    sp->unk33C.x = 96;
    sp->unk35C.vx = 544;
    sp->unk35C.vy = 511;
    sp->unk33C.y = 44;
    sp->unk33C.w = 120;
    sp->unk33C.h = 16;
    func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, 3);
    i = 0;
    func_8002732c(MESSAGE(221), label_x[12] + 48, 204, &sp->style[2]);
    func_8002732c(MESSAGE(222), label_x[13] + 152, 204, &sp->style[2]);
    for (; i < 12; i++) {
        if (i == 6) {
            row[0].vx = 184;
            row[1].vx = 250;
            row[2].vx = 168;
            row[0].vy = row[1].vy = 84;
            row[3].vx = 266;
            row[2].vy = row[3].vy = 94;
        }
        if (i == sp->unk2E0)
            func_80028c2c(row, lit, 1);
        else
            func_80028c2c(row, dim, 1);
        row[0].vy += 18;
        row[1].vy += 18;
        row[2].vy += 18;
        row[3].vy += 18;
    }
    row[0].vx = 64;
    row[1].vx = 130;
    row[0].vy = row[1].vy = 208;
    row[2].vx = 48;
    row[3].vx = 146;
    row[2].vy = row[3].vy = 218;
    if (sp->unk2E0 == 12)
        func_80028c2c(row, lit, 1);
    else
        func_80028c2c(row, dim, 1);
    func_800286a8(0, 0, 0);
    row[0].vx = 168;
    row[1].vx = 234;
    row[0].vy = row[1].vy = 208;
    row[2].vx = 152;
    row[3].vx = 250;
    row[2].vy = row[3].vy = 218;
    if (sp->unk2E0 == 13)
        func_80028c2c(row, lit, 1);
    else
        func_80028c2c(row, dim, 1);
    func_800286a8(0, 0, 0);
    PAGE_END();
}

/*
 * The first menu page bodies; func_8002ca84 calls one
 * per page bit.
 */

ADDRESS(0x8002c050, 0x40)
void func_8002c050(void)
{
    MenuScratch *sp = MENU_SCRATCH;

    func_8002732c(MESSAGE(310), 32, 74, &sp->style[0]);
}

ADDRESS(0x8002c090, 0x114)
void func_8002c090(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    short i;

    sp->unk330.x = 94;
    sp->unk330.w = 138;
    sp->unk330.h = 17;
    if (sp->unk1F8 == 0)
        sp->unk330.y = 108;
    else
        sp->unk330.y = 132;
    func_80027d18(&sp->unk330, &sp->unk32C, 257, 1);
    func_800286a8(1, 0, 0);
    for (i = 0; i < 7; i++) {
        DAT_8017beb0[i] = 0;
        DAT_800881f0[i] = 0;
    }
    func_8002732c(MESSAGE(320), 102, 110, &sp->style[0]);
    func_8002732c(MESSAGE(321), 102, 134, &sp->style[0]);
}

ADDRESS(0x8002c1a4, 0x114)
void func_8002c1a4(void)
{
    MenuScratch *sp = MENU_SCRATCH;

    if (sp->unk0.half[1] == 0) {
        func_8002732c(MESSAGE(311), 32, 74, &sp->style[0]);
        if (sp->unk202) {
            func_8002732c(MESSAGE(325), 121, 160, &sp->style[2]);
            func_8002732c(MESSAGE(326), 191, 160, &sp->style[0]);
        } else {
            func_8002732c(MESSAGE(325), 121, 160, &sp->style[0]);
            func_8002732c(MESSAGE(326), 191, 160, &sp->style[2]);
        }
    } else {
        func_8002732c(MESSAGE(312), 32, 74, &sp->style[0]);
        func_8002732c(MESSAGE(332), 40, 100, &sp->style[0]);
    }
}

/*
 * The record list page: a two-choice header while the first state half is
 * 1, scroll arrows, the cursor bar and three rows of record number, title,
 * kind name and figure. Each row's glyph overflow count goes to
 * DAT_8017beb0.
 */
ADDRESS(0x8002c2b8, 0x604)
void func_8002c2b8(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    int titles[48] = {
        -1, 0, 5, 2, 0, 0, 0, -1, 4, 4, 3, 1, 1, 4, 6, 1,
        -1, 5, 2, -1, 2, 3, -1, 6, 2, 6, 6, 1, 1, -1, -1, -1,
        3, 1, -1, -1, -1, 3, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    };
    RECT unused;
    int id;
    int n, i;

    sp->unk338.r = sp->unk338.g = sp->unk338.b = 64;
    if (sp->unk0.half[1] == 1) {
        id = (sp->unk2DC & 0xff) ? 323 : 324;
        func_8002732c(MESSAGE(id), 68, 128, &sp->style[0]);
        sp->style[0].b = sp->style[0].g = sp->style[0].r = 24;
        if (sp->unk202) {
            func_8002732c(MESSAGE(325), 120, 150, &sp->style[2]);
            func_8002732c(MESSAGE(326), 190, 150, &sp->style[0]);
        } else {
            func_8002732c(MESSAGE(325), 120, 150, &sp->style[0]);
            func_8002732c(MESSAGE(326), 190, 150, &sp->style[2]);
        }
        sp->style[0].b = sp->style[0].g = sp->style[0].r = 72;
        sp->unk33C.x = 60;
        sp->unk33C.y = 120;
        sp->unk33C.w = 200;
        sp->unk33C.h = 50;
        sp->unk348.x = 61;
        sp->unk348.y = 121;
        sp->unk348.w = 198;
        sp->unk348.h = 48;
        sp->unk344.r = sp->unk344.g = sp->unk344.b = 32;
        func_80028220(&sp->unk348, &sp->unk344, 1);
        func_800286a8(2, 0, 0);
        func_80028220(&sp->unk33C, &sp->unk338, 0);
    }
    if (sp->unk1FA >= 4) {
        if (sp->unk200 > 0) {
            sp->unk33C.x = 156;
            sp->unk33C.y = 68;
            sp->unk33C.w = 16;
            sp->unk33C.h = 8;
            func_8002916c(&sp->unk33C, &sp->unk32C, 3072);
            func_800286a8(1, 0, 0);
        }
        if (sp->unk200 < sp->unk1FA - 3) {
            sp->unk33C.x = 156;
            sp->unk33C.y = 215;
            sp->unk33C.w = 16;
            sp->unk33C.h = 8;
            func_8002916c(&sp->unk33C, &sp->unk32C, 1024);
            func_800286a8(1, 0, 0);
        }
    }
    sp->unk330.x = 32;
    sp->unk330.w = 256;
    sp->unk330.h = 40;
    sp->unk330.y = sp->unk1FE * 48 + 74;
    func_80027d18(&sp->unk330, &sp->unk32C, 257, 1);
    n = 3;
    func_800286a8(1, 0, 0);
    sp->unk330.y = 74;
    if (sp->unk1FA < 3)
        n = sp->unk1FA;
    for (i = 0; i < n; i++) {
        if (sp->unk1FC == 1 && i + sp->unk200 == sp->unk1FA - 1) {
            func_8002732c(MESSAGE(322), 64, sp->unk330.y + 6, &sp->style[0]);
        } else {
            func_800276bc((u16)(sp->unk200 + i + 1) | DIGITS(2), 38, sp->unk330.y + 6,
                          &sp->style[0]);
            func_8002732c(MESSAGE(328), 218, sp->unk330.y + 6, &sp->style[0]);
            DAT_800881ac = 1;
            DAT_800881bc = 280;
            DAT_800881c4 = 0;
            DAT_800881ba = 0;
            if (titles[sp->unk1F4->files[i + sp->unk200].kind] != -1)
                func_8002732c(MESSAGE(610 + titles[sp->unk1F4->files[i + sp->unk200].kind]), 64,
                              sp->unk330.y + 6, &sp->style[0]);
            if (titles[sp->unk1F4->files[i + sp->unk200].kind] != -1)
                func_8002732c(MESSAGE(700 + sp->unk1F4->files[i + sp->unk200].kind), 64,
                              sp->unk330.y + 22, &sp->style[0]);
            else
                func_8002732c(MESSAGE(700 + sp->unk1F4->files[i + sp->unk200].kind), 64,
                              sp->unk330.y + 6, &sp->style[0]);
            func_800276bc(sp->unk1F4->files[i + sp->unk200].unk1A | DIGITS(4), 248, sp->unk330.y + 6,
                          &sp->style[0]);
        }
        DAT_8017beb0[i + sp->unk200] = DAT_800881c8;
        func_80027d18(&sp->unk330, &sp->unk338, 257, 1);
        sp->unk330.y += 48;
    }
    func_800286a8(1, 0, 0);
}

/*
 * The remaining menu page bodies, the page frame that dispatches
 * them by page bit, and three two-choice dialogs.
 */

ADDRESS(0x8002c8bc, 0x9c)
void func_8002c8bc(void)
{
    MenuScratch *sp = MENU_SCRATCH;

    if (sp->unk2DC & 0xff)
        func_8002732c(MESSAGE(330), 32, 74, &sp->style[0]);
    else
        func_8002732c(MESSAGE(331), 32, 74, &sp->style[0]);
    func_8002732c(MESSAGE(332), 40, 100, &sp->style[0]);
}

ADDRESS(0x8002c958, 0x40)
void func_8002c958(void)
{
    MenuScratch *sp = MENU_SCRATCH;

    func_8002732c(MESSAGE(340), 32, 74, &sp->style[0]);
}

ADDRESS(0x8002c998, 0x50)
void func_8002c998(void)
{
    MenuScratch *sp = MENU_SCRATCH;

    func_8002732c(MESSAGE(sp->unk0.half[0]), 60, 114, &sp->style[0]);
}

ADDRESS(0x8002c9e8, 0x9c)
void func_8002c9e8(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    int n;

    sp->unk350.w = 128;
    sp->unk350.h = 240;
    sp->unk358.vx = 768;
    sp->unk350.y = 0;
    sp->unk350.x = 0;
    sp->unk358.vy = 256;
    n = 5;
    while (n--) {
        func_80028364(&sp->unk350, &sp->unk358, 0, 0, 16);
        sp->unk350.x += 128;
    }
}

ADDRESS(0x8002ca84, 0x314)
void func_8002ca84(void)
{
    MenuScratch *sp = MENU_SCRATCH;

    func_800174c4();
    sp->style[0].b = sp->style[0].g = sp->style[0].r = 72;
    sp->style[2].r = 98;
    sp->style[2].g = 66;
    sp->style[2].b = 48;
    sp->style[3].r = 96;
    sp->style[3].b = sp->style[3].g = 32;
    sp->style[0].cd = sp->style[2].cd = sp->style[3].cd = 57;
    if (sp->unk2E4 < 16)
        sp->unk360 = 8 - sp->unk2E4;
    else
        sp->unk360 = sp->unk2E4 - 24;
    sp->unk358.vx = 576;
    sp->unk358.vy = 448;
    sp->unk35C.vx = 544;
    sp->unk35C.vy = 511;
    sp->unk33C.x = 81;
    sp->unk33C.y = 45;
    sp->unk33C.w = 40;
    sp->unk33C.h = 16;
    sp->unk32C.b = 0;
    sp->unk32C.r = (u8)sp->unk360 * 4 + 96;
    sp->unk32C.g = (u8)sp->unk360 * 2 + 48;
    func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, 0, 3);
    sp->unk33C.y = 44;
    if (sp->unk2DC & 0xff)
        sp->unk358.vx = 586;
    else
        sp->unk358.vx = 613;
    sp->unk33C.x = 32;
    sp->unk33C.w = 40;
    func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, 0, 3);
    switch (sp->unk2DC) {
    case 0x2:
    case 0x200:
        func_8002c090();
        break;
    case 0x4:
    case 0x400:
        func_8002c050();
        break;
    case 0x8:
    case 0x800:
        func_8002c1a4();
        break;
    case 0x10:
    case 0x1000:
        func_8002c2b8();
        break;
    case 0x20:
    case 0x2000:
        func_8002c8bc();
        break;
    case 0x40:
    case 0x4000:
        func_8002c958();
        break;
    case 0x80:
    case 0x8000:
        func_8002c998();
        break;
    }
    func_80029548(16, 36, 288, 192);
    func_800175a4();
    if (sp->unk206 == 1)
        func_8002948c();
    func_80017634();
}

ADDRESS(0x8002cd98, 0xec)
void func_8002cd98(void)
{
    MenuScratch *sp = MENU_SCRATCH;

    func_8002732c(MESSAGE(400), 76, 92, &sp->style[0]);
    sp->style[0].b = sp->style[0].g = sp->style[0].r = 24;
    if (sp->unk2E0 == 0) {
        func_8002732c(MESSAGE(401), 121, 136, &sp->style[2]);
        func_8002732c(MESSAGE(402), 189, 136, &sp->style[0]);
    } else {
        func_8002732c(MESSAGE(401), 121, 136, &sp->style[0]);
        func_8002732c(MESSAGE(402), 189, 136, &sp->style[2]);
    }
    func_80029618(40, 80, 240, 80);
}

ADDRESS(0x8002ce84, 0x74)
void func_8002ce84(void)
{
    MenuScratch *sp = MENU_SCRATCH;

    func_800174c4();
    sp->style[0].b = sp->style[0].g = sp->style[0].r = 96;
    sp->style[2].r = 98;
    sp->style[2].g = 66;
    sp->style[2].b = 48;
    sp->style[0].cd = sp->style[2].cd = 57;
    func_8002cd98();
    func_800175a4();
    func_8002948c();
    func_80017634();
}

ADDRESS(0x8002cef8, 0x1b8)
void func_8002cef8(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    short x[2] = {16, 10};

    func_800174c4();
    sp->style[0].b = sp->style[0].g = sp->style[0].r = 96;
    sp->style[0].cd = 49;
    if (sp->unk2E4 < 16)
        sp->unk360 = 8 - sp->unk2E4;
    else
        sp->unk360 = sp->unk2E4 - 24;
    sp->unk32C.r = (u8)sp->unk360 * 4 + 96;
    sp->unk32C.g = (u8)sp->unk360 * 2 + 48;
    sp->unk32C.b = 0;
    sp->unk330.x = 93;
    sp->unk330.y = sp->unk0.word * 24 + 118;
    sp->unk330.w = 134;
    sp->unk330.h = 16;
    func_80027d18(&sp->unk330, &sp->unk32C, 257, 1);
    func_800286a8(1, 0, 0);
    func_8002732c(MESSAGE(800), x[0] + 97, 120, &sp->style[0]);
    func_8002732c(MESSAGE(801), x[1] + 97, 144, &sp->style[0]);
    func_80029618(72, 100, 176, 80);
    func_800175a4();
    ClearImage(&world_state.draw[world_state.frame].clip, 0, 0, 0);
    func_80017634();
}

ADDRESS(0x8002d0b0, 0x12c)
void func_8002d0b0(void)
{
    MenuScratch *sp = MENU_SCRATCH;

    func_800174c4();
    sp->style[0].b = sp->style[0].g = sp->style[0].r = 96;
    sp->style[2].r = 98;
    sp->style[2].g = 66;
    sp->style[2].b = 48;
    sp->style[0].cd = sp->style[2].cd = 57;
    func_8002732c(MESSAGE(850), 92, 120, &sp->style[0]);
    if (sp->unk2DE == 0) {
        func_8002732c(MESSAGE(851), 127, 152, &sp->style[2]);
        func_8002732c(MESSAGE(852), 183, 152, &sp->style[0]);
    } else {
        func_8002732c(MESSAGE(851), 127, 152, &sp->style[0]);
        func_8002732c(MESSAGE(852), 183, 152, &sp->style[2]);
    }
    func_80029618(72, 100, 176, 80);
    func_800175a4();
    func_80017634();
}

/*
 * The header tabs of the item list screens for page bits 0x1, 0x2, 0x4 and
 * 0x8, the window, and the darkened screen while bit 0x1 is set.
 */
ADDRESS(0x8002d1dc, 0x398)
void func_8002d1dc(void)
{
    MenuScratch *sp = MENU_SCRATCH;

    sp->unk33C.x = 34;
    sp->unk33C.y = 24;
    sp->unk33C.h = 16;
    sp->unk35C.vx = 544;
    sp->unk35C.vy = 511;
    if (sp->unk2DC & 0x1) {
        sp->unk33C.w = 30;
        sp->unk358.vx = 631;
        sp->unk358.vy = 464;
        func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, 3);
        sp->unk33C.w = 35;
        sp->unk358.vx = 624;
        sp->unk358.vy = 448;
        sp->unk33C.x += 37;
        func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, 3);
    } else if (sp->unk2DC & 0x2) {
        sp->unk33C.w = 90;
        sp->unk358.vx = 588;
        sp->unk358.vy = 464;
        func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, 3);
        sp->unk33C.x = 153;
        sp->unk33C.w = 54;
        sp->unk358.vx = 617;
        sp->unk358.vy = 464;
        func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, 3);
        sp->unk33C.x = 131;
        sp->unk33C.w = 20;
        sp->unk358.vx = 612;
        sp->unk358.vy = 464;
        func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, 3);
    } else if (sp->unk2DC & 0x4) {
        sp->unk33C.w = 90;
        sp->unk358.vx = 588;
        sp->unk358.vy = 464;
        func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, 3);
        sp->unk33C.x = 153;
        sp->unk33C.w = 54;
        sp->unk358.vx = 617;
        sp->unk358.vy = 464;
        func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, 3);
        sp->unk33C.x = 130;
        sp->unk33C.w = 20;
        sp->unk358.vx = 591;
        sp->unk358.vy = 432;
        func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, 3);
    } else if (sp->unk2DC & 0x8) {
        sp->unk33C.w = 60;
        sp->unk358.vx = 576;
        sp->unk358.vy = 480;
        func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, 3);
        sp->unk33C.w = 36;
        sp->unk358.vx = 624;
        sp->unk358.vy = 448;
        sp->unk33C.x += 63;
        func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, 3);
    }
    func_80029618(16, 16, 288, 208);
    if (sp->unk2DC & 0x1) {
        RECT screen = {0, 0, 320, 240};
        /* Four initializers: GCC keeps it apart from func_8002948c's {32, 32, 32}. */
        CVECTOR shade = {32, 32, 32, 0};

        func_80028220(&screen, &shade, 1);
        func_800286a8(2, 0, 0);
    }
}

/*
 * The item list screens: a cursor bar, three list
 * layouts chosen by the page bits (with carried counts, with prices, plain),
 * the selected item's detail panel and the screen frame that picks them.
 */

/* The list rows shown at once. */
#define LIST_ROWS 5

#define LIST_TEXT(sp, id) \
    ((u16 *)((sp)->unk0.list.messages->text + (sp)->unk0.list.messages->offsets[id]))

ADDRESS(0x8002d574, 0x98)
void func_8002d574(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    int x, w;

    if (sp->unk2DC & 0x6) {
        x = 54;
        sp->unk330.w = 218;
    } else {
        x = 28;
        sp->unk330.w = 272;
    }
    sp->unk330.x = x - 4;
    sp->unk330.h = 34;
    sp->unk330.y = sp->unk2E0 * 36 + 44;
    func_80027d18(&sp->unk330, &sp->unk32C, 257, 1);
    func_800286a8(1, 0, 0);
}

ADDRESS(0x8002d60c, 0x2dc)
void func_8002d60c(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    int rows;
    int i;
    int id;
    CVECTOR *text, *figure;

    rows = LIST_ROWS;
    if (sp->unk0.list.count < LIST_ROWS + 1)
        rows = sp->unk0.list.count;
    func_80027440(DAT_801c7590.counts[266].count | 0x3000000, 244, 27, &sp->style[6]);
    func_8002732c(MESSAGE(100), 160, 27, &sp->style[2]);
    for (i = 0; i < rows; i++) {
        id = DAT_801c7590.unk13A[sp->unk0.list.ids[sp->unk2E2 + i]];
        if (DAT_801c7590.counts[266].count >= vis_table_c.unk0118[id].unk2) {
            text = &sp->style[0];
            figure = &sp->style[4];
        } else {
            text = &sp->style[3];
            figure = &sp->style[7];
        }
        sp->style[0].cd |= 0x80;
        sp->style[3].cd |= 0x80;
        func_8002732c(LIST_TEXT(sp, id), 28, i * 36 + 48, text);
        sp->style[0].cd &= 0x7f;
        sp->style[3].cd &= 0x7f;
        if (vis_table_c.unk0118[id].unk27) {
            func_8002732c(MESSAGE(109), 72, i * 36 + 62, text);
            func_80027440(vis_table_c.unk0118[id].unk28 | 0x3000000, 128, i * 36 + 62, figure);
            func_80026ecc(74, 160, i * 36 + 62, text);
            func_80027440(vis_table_c.unk0118[id].unk27 | 0x3000000, 172, i * 36 + 62, figure);
        }
        func_80027440(vis_table_c.unk0118[id].unk2 | 0x3000000, 244, i * 36 + 62, figure);
        func_8002732c(MESSAGE(102), 216, i * 36 + 62, text);
    }
}

ADDRESS(0x8002d8e8, 0x1b8)
void func_8002d8e8(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    int rows;
    int i;
    int id;

    rows = LIST_ROWS;
    if (sp->unk0.list.count < LIST_ROWS + 1)
        rows = sp->unk0.list.count;
    for (i = 0; i < rows; i++) {
        id = sp->unk0.list.ids[sp->unk2E2 + i];
        sp->style[0].cd |= 0x80;
        func_8002732c(LIST_TEXT(sp, id), 54, i * 36 + 48, &sp->style[0]);
        sp->style[0].cd &= 0x7f;
        if (vis_table_c.unk0118[id].unk27) {
            func_8002732c(MESSAGE(109), 131, i * 36 + 62, &sp->style[0]);
            func_80027440(vis_table_c.unk0118[id].unk28 | 0x3000000, 187, i * 36 + 62,
                          &sp->style[4]);
            func_80026ecc(74, 219, i * 36 + 62, &sp->style[0]);
            func_80027440(vis_table_c.unk0118[id].unk27 | 0x3000000, 231, i * 36 + 62,
                          &sp->style[4]);
        }
    }
}

ADDRESS(0x8002daa0, 0x2d0)
void func_8002daa0(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    int rows;
    int i;
    int id;
    int price;
    CVECTOR *text, *figure;

    func_80027440(viewer.unk2 | 0x4000000, 244, 27, &sp->style[6]);
    func_8002732c(MESSAGE(101), 176, 27, &sp->style[2]);
    rows = LIST_ROWS;
    if (sp->unk0.list.count < LIST_ROWS + 1)
        rows = sp->unk0.list.count;
    for (i = 0; i < rows; i++) {
        id = sp->unk0.list.ids[sp->unk2E2 + i];
        price = vis_table_c.unk0118[id].unk3 * 2 / 3 *
                (vis_table_c.unk0118[id].unk27 - vis_table_c.unk0118[id].unk28);
        if (vis_table_c.unk0118[id].unk28 == 0)
            price *= 2;
        if (price < viewer.unk2) {
            text = &sp->style[0];
            figure = &sp->style[4];
        } else {
            text = &sp->style[3];
            figure = &sp->style[7];
        }
        sp->style[0].cd |= 0x80;
        sp->style[3].cd |= 0x80;
        func_8002732c(LIST_TEXT(sp, id), 28, i * 36 + 48, text);
        sp->style[0].cd &= 0x7f;
        sp->style[3].cd &= 0x7f;
        if (vis_table_c.unk0118[id].unk27) {
            func_8002732c(MESSAGE(109), 77, i * 36 + 62, text);
            func_80027440(vis_table_c.unk0118[id].unk28 | 0x3000000, 133, i * 36 + 62, figure);
            func_80026ecc(74, 165, i * 36 + 62, text);
            func_80027440(vis_table_c.unk0118[id].unk27 | 0x3000000, 177, i * 36 + 62, figure);
        }
        func_80027440((u16)price | 0x4000000, 244, i * 36 + 62, figure);
    }
}

/*
 * Not exact (20 bytes long): retail keeps the scratchpad base in s4 from the first
 * load; this build rematerializes lui 0x1f80 for the first load and the
 * first three argument addresses. Direct rectangle-coordinate selection,
 * a named MenuList owner and a separate RECT owner did not resolve it.
 */
ADDRESS(0x8002dd70, 0x5a0)
void func_8002dd70(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    short id;
    int price;
    int i;
    int y;
    int x;

    x = 105;
    if (sp->unk0.list.unkA == 0)
        x = 167;
    sp->unk33C.x = x;
    sp->unk33C.y = 206;
    sp->unk33C.w = 50;
    sp->unk33C.h = 16;
    func_80027d18(&sp->unk33C, &sp->unk32C, 257, 1);
    func_800286a8(1, 0, 0);
    if (sp->unk2DC & 0x1) {
        id = DAT_801c7590.unk13A[sp->unk0.list.ids[sp->unk2E2 + sp->unk2E0]];
        func_8002732c(MESSAGE(105), 120, 208, &sp->style[0]);
    } else if (sp->unk2DC & 0x8) {
        id = sp->unk0.list.ids[sp->unk2E2 + sp->unk2E0];
        func_8002732c(MESSAGE(106), 110, 208, &sp->style[0]);
    } else {
        id = sp->unk0.list.ids[sp->unk2E2 + sp->unk2E0];
        func_8002732c(MESSAGE(107), 113, 208, &sp->style[0]);
    }
    func_8002732c(MESSAGE(108), 172, 208, &sp->style[0]);
    sp->style[0].cd |= 0x80;
    func_8002732c(LIST_TEXT(sp, id), 24, 176, &sp->style[0]);
    sp->style[0].cd &= 0x7f;
    if (vis_table_c.unk0118[id].unk27) {
        func_8002732c(MESSAGE(109), 48, 190, &sp->style[0]);
        func_80027440(vis_table_c.unk0118[id].unk28 | 0x3000000, 104, 190, &sp->style[4]);
        func_80026ecc(74, 136, 190, &sp->style[0]);
        func_80027440(vis_table_c.unk0118[id].unk27 | 0x3000000, 148, 190, &sp->style[4]);
    }
    if (sp->unk2DC & 0x1) {
        func_80027440(vis_table_c.unk0118[id].unk2 | 0x3000000, 244, 190, &sp->style[4]);
        func_8002732c(MESSAGE(102), 216, 190, &sp->style[0]);
    } else if (sp->unk2DC & 0x8) {
        price = vis_table_c.unk0118[id].unk3 * 2 / 3 *
                (vis_table_c.unk0118[id].unk27 - vis_table_c.unk0118[id].unk28);
        if (vis_table_c.unk0118[id].unk28 == 0)
            price *= 2;
        func_80027440((u16)price | 0x4000000, 244, 190, &sp->style[4]);
    }
    func_8002732c(MESSAGE(103), 165, 46, &sp->style[0]);
    y = 46;
    func_8002732c(MESSAGE(103), 165, y, &sp->style[0]);
    for (i = 0; i < 7; i++) {
        func_800276bc(vis_table_c.unk0118[id].unk10[i] | 0x13000000, 207, y, &sp->style[4]);
        func_800276bc(vis_table_c.unk0118[id].unk10[i + 7] | 0x13000000, 273, y, &sp->style[4]);
        y += 14;
    }
    if (vis_table_c.unk0118[id].unk24) {
        DAT_8008819e = vis_table_c.unk0118[id].unk24 * 100 / 454;
        DAT_80088188 = vis_table_c.unk0118[id].unk24 * 1000 / 454;
        func_8002732c(MESSAGE(104), 220, 156, &sp->style[0]);
        func_800276bc(DAT_8008819e | 0x3000000, 248, 156, &sp->style[4]);
        func_80026ecc(78, 269, 156, &sp->style[0]);
        func_800276bc((u16)(DAT_80088188 - DAT_8008819e * 10) | 0x1000000, 275, 156,
                      &sp->style[4]);
        func_80026ecc(108, 281, 156, &sp->style[0]);
    }
    sp->unk350.x = 20;
    sp->unk350.y = 40;
    sp->unk350.w = 128;
    sp->unk350.h = 128;
    sp->unk358.vx = 448;
    sp->unk358.vy = 256;
    func_80028364(&sp->unk350, &sp->unk358, 0, 0, 16);
}

/*
 * Not exact: retail loads the constant 48 before reading +0x2e4;
 * this build hoists the +0x2e4 load, swapping v0 and v1 in the first
 * colour stores.
 */
ADDRESS(0x8002e310, 0x180)
void func_8002e310(void)
{
    MenuScratch *sp = MENU_SCRATCH;

    func_800174c4();
    sp->style[2].b = sp->style[6].b = sp->style[3].g = sp->style[7].g =
        sp->style[3].b = sp->style[7].b = 48;
    sp->style[0].b = sp->style[4].b = sp->style[0].g = sp->style[4].g =
        sp->style[0].r = sp->style[4].r = sp->style[3].r = sp->style[7].r = 96;
    sp->style[2].r = sp->style[6].r = 98;
    sp->style[2].g = sp->style[6].g = 66;
    sp->style[0].cd = sp->style[2].cd = sp->style[3].cd = 57;
    sp->style[4].cd = sp->style[6].cd = sp->style[7].cd = 41;
    if (sp->unk2E4 < 16)
        sp->unk360 = 8 - sp->unk2E4;
    else
        sp->unk360 = sp->unk2E4 - 24;
    sp->unk32C.b = 0;
    sp->unk32C.r = (u8)sp->unk360 * 4 + 96;
    sp->unk32C.g = (u8)sp->unk360 * 2 + 48;
    if (!(sp->unk2DC & 0x8000))
        func_8002d574();
    if (sp->unk2DC & 0x8000)
        func_8002dd70();
    else if (sp->unk2DC & 0x1)
        func_8002d60c();
    else if (sp->unk2DC & 0x8)
        func_8002daa0();
    else
        func_8002d8e8();
    func_8002d1dc();
    func_800175a4();
    func_8002948c();
    func_80017634();
}

/*
 * The status screens: the two window frames, the
 * equipment slot grid and the attribute table with its bars.
 */

/* The slots of the equipment grid. */
#define EQUIP_SLOTS 21
/* Attribute rows. */
#define STATUS_ROWS 14

ADDRESS(0x8002e490, 0x40)
void func_8002e490(void)
{
    func_80029548(8, 8, 300, 224);
    func_80029548(308, 8, 324, 224);
}

/*
 * Draws the equipment slots from their VRAM images, highlights the slot
 * under the cursor and, while +0x2c6 is 1, prints each equipped item's
 * +0x28 figure with a shadow.
 */
ADDRESS(0x8002e4d0, 0x47c)
void func_8002e4d0(void)
{
    MenuScratch *sp;
    RECT rects[EQUIP_SLOTS] = {
        {14, 11, 56, 28},   {14, 40, 56, 28},   {14, 69, 56, 28},   {14, 98, 92, 73},
        {14, 172, 56, 28},  {14, 201, 56, 28},  {72, 15, 48, 24},   {130, 11, 56, 28},
        {130, 40, 56, 28},  {108, 69, 100, 65}, {108, 135, 100, 65}, {130, 201, 56, 28},
        {196, 15, 48, 24},  {246, 11, 56, 28},  {246, 40, 56, 28},  {246, 69, 56, 28},
        {210, 98, 92, 73},  {246, 172, 56, 28}, {246, 201, 56, 28}, {72, 201, 56, 28},
        {188, 201, 56, 28},
    };
    DVECTOR tex[EQUIP_SLOTS] = {
        {740, 152}, {796, 152}, {852, 152}, {932, 0},   {908, 152}, {964, 152}, {740, 124},
        {740, 180}, {796, 180}, {640, 124}, {640, 189}, {852, 180}, {788, 124}, {908, 180},
        {964, 180}, {852, 152}, {932, 73},  {796, 208}, {852, 208}, {908, 208}, {964, 208},
    };
    DVECTOR clut = {0, 511};
    CVECTOR unused = {0};
    RECT unused2;
    int i, slot, mode;
    u16 *equip;

    sp = MENU_SCRATCH;
    if (sp->unk2E0 < 10)
        slot = sp->unk2E0;
    else
        slot = sp->unk2E0 - 1;
    sp->unk330.x = rects[slot].x - 2;
    sp->unk330.y = rects[slot].y - 1;
    sp->unk330.w = rects[slot].w + 4;
    sp->unk330.h = rects[slot].h + 2;
    func_80027d18(&sp->unk330, &sp->unk32C, 513, 1);
    func_800286a8(1, 0, 0);
    sp->style[7].r = 192;
    sp->style[7].g = 158;
    sp->style[7].b = 96;
    sp->style[7].cd = 48;
    equip = viewer.unk38;
    for (i = 0; i < EQUIP_SLOTS; equip++, i++) {
        if (i == 2)
            mode = 48;
        else if ((i == 3 && vis_table_c.unk0118[viewer.unk38[16]].unk26 == 9) ||
                 (i == 4 && vis_table_c.unk0118[viewer.unk38[17]].unk26 == 9))
            mode = 48;
        else
            mode = 16;
        if (i != 6 && i != 12 && sp->unk2C6 == 1 && *equip != 0xffff &&
            vis_table_c.unk0118[*equip].unk27) {
            sp->style[7].r = 200;
            sp->style[7].g = 164;
            sp->style[7].b = 96;
            func_80027440(vis_table_c.unk0118[*equip].unk28 | 0x2000000, rects[i].x + 8,
                          rects[i].y + 4, &sp->style[7]);
            sp->style[7].g = sp->style[7].r = 8;
            sp->style[7].b = 0;
            func_80027440(vis_table_c.unk0118[*equip].unk28 | 0x2000000, rects[i].x + 9,
                          rects[i].y + 5, &sp->style[7]);
        }
        func_80028364(&rects[i], &tex[i], &clut, 0, mode);
    }
}

/*
 * The attribute table: names, current values (with the change a candidate
 * item would make on page 2) and bars scaled to +0x2c4.
 *
 * Not exact: retail schedules the colour stores in another order and
 * keeps the scratchpad base in s2 (this build: s3). The selected-count
 * loop now uses a plain byte argument and advances its cursor in the
 * update clause, matching the retail backedge and pointer walk.
 */
ADDRESS(0x8002e94c, 0x44c)
void func_8002e94c(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    CVECTOR bright = {128, 128, 32, 0};
    CVECTOR dark = {0};
    RECT unused;
    RECT bar;
    short xoff[STATUS_ROWS] = {0};
    int i, y;
    u8 *count;
    s16 *figure;

    sp->style[3].r = sp->style[2].g = 120;
    sp->style[2].r = sp->style[2].b = sp->style[3].b = 40;
    sp->style[3].g = 60;
    sp->style[3].cd = sp->style[2].cd = 73;
    sp->style[7].r = sp->style[6].g = 120;
    sp->style[6].r = sp->style[6].b = sp->style[7].b = 40;
    sp->style[7].g = 60;
    sp->style[7].cd = sp->style[6].cd = 49;
    if (sp->unk2DC == 1) {
        sp->unk320 = sp->unk324 = sp->unk328 = &sp->style[0];
    } else {
        sp->unk320 = &sp->style[0];
        sp->unk324 = &sp->style[2];
        sp->unk328 = &sp->style[3];
    }
    for (i = 0, y = 16; i < STATUS_ROWS; i++, y += 15)
        func_8002732c(MESSAGE(14 + i), xoff[i] + 316, y, sp->unk320);
    if (sp->unk2DC == 1) {
        sp->unk320 = sp->unk324 = sp->unk328 = &sp->style[4];
    } else {
        sp->unk320 = &sp->style[4];
        sp->unk324 = &sp->style[6];
        sp->unk328 = &sp->style[7];
    }
    if (sp->unk2DC == 2 && sp->unk2C6) {
        if (sp->unk2BE != -1) {
            count = vis_table_c.unk0118[sp->unk2C0].unk10;
            for (i = 0, y = 16; i < STATUS_ROWS; count++, y += 15, i++)
                func_80027440(*count | 0x13000000, 590, y, sp->unk320);
        }
        return;
    }
    figure = viewer.unk62;
    for (i = 0, y = 16; i < STATUS_ROWS; i++, y += 15, figure++) {
        if (sp->unk2DC == 2) {
            if (sp->unk25C[i] > 0) {
                func_80027440(sp->unk1B4[i] | 0x3000000, 590, y, sp->unk320);
                func_80026ecc(19, 544, y, sp->unk324);
                func_80027440(sp->unk25C[i] | 0x3000000, 508, y, sp->unk324);
            } else if (sp->unk25C[i] < 0) {
                func_80027440(sp->unk1B4[i] | 0x3000000, 590, y, sp->unk320);
                func_80026ecc(20, 544, y, sp->unk328);
                func_80027440(-sp->unk25C[i] | 0x3000000, 508, y, sp->unk328);
            } else {
                func_80027440(sp->unk1B4[i] | 0x3000000, 590, y, sp->unk320);
            }
        } else if (*figure < 0) {
            func_80027440(0x3000000, 590, y, sp->unk320);
        } else {
            func_80027440((u16)*figure | 0x3000000, 590, y, sp->unk320);
        }
    }
    bar.h = 4;
    for (i = 0, y = 27; i < STATUS_ROWS; i++, y += 15) {
        bar.w = sp->unk1B4[i] * 50 / sp->unk2C4;
        if (bar.w > 100)
            bar.w = 100;
        bar.y = y;
        bar.x = 626 - bar.w;
        func_80028db8(&bar, &bright, &dark, 1);
    }
    func_800286a8(1, 832, 256);
}

/*
 * The status figure page, the cell grid drawer and the item and
 * cell pages.
 */

/* Attribute rows. */

/* Unreferenced: eight zero bytes before this unit's constants. */
DATA(0x80087f68, 0x8, ".sdata")
static s32 DAT_80087f68[2] = {0, 0};

/*
 * The figure page: two current/maximum pairs coloured by how low the
 * current value is, the other figures and two weights in tenths.
 *
 * Not exact: the final scalar rows have their own y lifetime, matching
 * retail's saved registers. Weight conversions still reload after the first
 * shared result store, and the comparison evaluates the other operand
 * first. Initialization schedules differ; the body is four bytes long.
 */
ADDRESS(0x8002ed98, 0x460)
void func_8002ed98(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    s16 *p;
    int y, i;

    sp->style[6].r = 96;
    sp->style[7].r = 96;
    sp->style[7].g = 96;
    sp->style[7].b = 32;
    sp->style[6].g = 48;
    sp->style[6].b = 48;
    sp->style[7].cd = 49;
    sp->style[6].cd = 49;
    for (i = 0, y = 16; i < STATUS_ROWS; y += 15, i++)
        func_8002732c(MESSAGE(i), 316, y, &sp->style[0]);
    p = (s16 *)&viewer;
    for (i = 0, y = 16; i < 2; y += 15, i++) {
        if (p[1] < p[0] / 2)
            sp->unk320 = &sp->style[7];
        else if (p[1] < (short)(p[0] / 5))
            sp->unk320 = &sp->style[6];
        else
            sp->unk320 = &sp->style[4];
        func_80027440((u16)*p++ | 0x5000000, 566, y, sp->unk320);
        func_80026ecc(14, 554, y, sp->unk320);
        func_80027440((u16)*p++ | 0x5000000, 494, y, sp->unk320);
    }
    p++;
    func_80027440((u16)*p++ | 0x3000000, 542, 46, &sp->style[4]);
    func_80026ecc(15, 578, 46, &sp->style[4]);
    func_80027440((u16)*p++ | 0x3000000, 590, 46, &sp->style[4]);
    {
        int figure_y;
        for (i = 3, figure_y = 61; i < 13; figure_y += 15, i++)
            func_80027440((u16)*p++ | 0x3000000, 590, figure_y, &sp->style[4]);
    }
    if ((short)(p[1] / 10) < (short)(p[0] / 10))
        sp->unk320 = &sp->style[6];
    else
        sp->unk320 = &sp->style[4];
    DAT_8008819e = *p * 100 / 454;
    DAT_80088188 = *p++ * 1000 / 454;
    func_80027440(DAT_8008819e | 0x3000000, 494, 211, sp->unk320);
    func_80026ecc(18, 534, 211, sp->unk320);
    func_80027440((u16)(DAT_80088188 - DAT_8008819e * 10) | 0x1000000, 542, 211, sp->unk320);
    func_80026ecc(14, 554, 211, sp->unk320);
    DAT_8008819e = *p * 100 / 454;
    DAT_80088188 = *p * 1000 / 454;
    func_80027440(DAT_8008819e | 0x3000000, 566, 211, sp->unk320);
    func_80026ecc(18, 606, 211, sp->unk320);
    func_80027440((u16)(DAT_80088188 - DAT_8008819e * 10) | 0x1000000, 614, 211, sp->unk320);
}

/*
 * Draws a 25 x 5 grid of 10 x 10 texture cells from tex at pos, 11 pixels
 * apart horizontally and 12 vertically, semi-transparent in color.
 *
 * Not exact: retail keeps pos and tex in t0/t1 with caller-save
 * spills and caches both row UV coordinates after the inner entry guard.
 * This build keeps pos in s8, hoists the addPrim mask, and remains
 * 16 bytes short; the frame and the 11-block loop shape agree.
 */
ADDRESS(0x8002f1f8, 0x2a0)
void func_8002f1f8(DVECTOR *pos, DVECTOR *tex, DVECTOR *clut, CVECTOR *color, int arg4)
{
    int row;
    int u0;
    int cols, rows;
    int col, x, y;
    int u, v;
    StPacketBuffer *buf;
    POLY_FT4 *p;
    CVECTOR c;

    cols = 25;
    rows = 5;
    u0 = (short)(tex->vx % 64) * 4;
    v = (short)(tex->vy % 256);
    for (row = 0, y = 0; row < rows; row++, y += 12, v += 10) {
        for (col = 0, x = 0, u = u0; col < cols; col++, x += 11, u += 10) {
            buf = world_state.packet;
            p = (POLY_FT4 *)buf->cur;
            if (buf->end < (u_long *)(p + 1))
                return;
            buf->cur = (u_long *)(p + 1);
            world_state.unk8328++;
            setUV4(p, u, v, u + 10, v, u, v + 10, u + 10, v + 10);
            p->x0 = p->x2 = pos->vx + x;
            p->x1 = p->x3 = pos->vx + x + 10;
            p->y0 = p->y1 = pos->vy + y;
            p->y2 = p->y3 = pos->vy + y + 10;
            c = *color;
            setRGB0(p, c.r, c.g, c.b);
            p->tpage = GetTPage(0, color->cd & 3, tex->vx, tex->vy);
            p->clut = GetClut(clut->vx, clut->vy);
            setPolyFT4(p);
            setSemiTrans(p, 1);
            addPrim(world_state.ot_cur, p);
        }
    }
}

/* The item page: its arrows, panels, name, cell grid, weight and figures. */
ADDRESS(0x8002f498, 0x4e4)
void func_8002f498(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    DVECTOR pos = {20, 152};
    RECT up = {16, 72, 16, 8};
    RECT down = {300, 72, 16, 8};

    sp->style[2].r = sp->style[6].r = 96;
    sp->style[2].b = sp->style[6].b = 48;
    sp->style[2].g = sp->style[6].g = 48;
    sp->style[2].cd = 73;
    sp->style[6].cd = 49;
    func_8002916c(&down, &sp->unk32C, 0);
    func_8002916c(&up, &sp->unk32C, 2048);
    func_800286a8(1, 0, 0);
    sp->unk350.x = 12;
    sp->unk350.y = 10;
    sp->unk350.w = 192;
    sp->unk350.h = 124;
    sp->unk358.vx = 640;
    sp->unk358.vy = 0;
    func_80028364(&sp->unk350, &sp->unk358, 0, 0, 16);
    sp->unk350.x = 204;
    sp->unk350.w = 100;
    sp->unk358.vx = 832;
    func_80028364(&sp->unk350, &sp->unk358, 0, 0, 16);
    if (sp->unk2BE == -1)
        return;
    sp->style[0].cd |= 0x80;
    func_8002732c((u16 *)(sp->unk2CC->text + sp->unk2CC->offsets[sp->unk2C0]), 14, 136,
                  &sp->style[0]);
    sp->style[0].cd &= 0x7f;
    func_8002f1f8(&pos, &sp->unk0.vram[0], &sp->unk0.vram[1], &sp->style[0], 0xf0004);
    if (vis_table_c.unk0118[sp->unk2C0].unk24) {
        DAT_8008819e = vis_table_c.unk0118[sp->unk2C0].unk24 * 100 / 454;
        DAT_80088188 = vis_table_c.unk0118[sp->unk2C0].unk24 * 1000 / 454;
        func_8002732c(MESSAGE(28), 188, 217, &sp->style[0]);
        func_80027440(DAT_8008819e | 0x3000000, 224, 217, &sp->style[4]);
        func_80026ecc(18, 264, 217, &sp->style[0]);
        func_80027440((u16)(DAT_80088188 - DAT_8008819e * 10) | 0x1000000, 272, 217,
                      &sp->style[4]);
        func_80026ecc(53, 287, 217, &sp->style[0]);
    }
    if (vis_table_c.unk0118[sp->unk2C0].unk27) {
        func_8002732c(MESSAGE(29), 16, 216, &sp->style[0]);
        if (vis_table_c.unk0118[sp->unk2C0].unk28 < 4) {
            func_80027440(vis_table_c.unk0118[sp->unk2C0].unk28 | 0x3000000, 80, 217,
                          &sp->style[6]);
            func_80026ecc(14, 120, 217, &sp->style[2]);
            func_80027440(vis_table_c.unk0118[sp->unk2C0].unk27 | 0x3000000, 136, 217,
                          &sp->style[6]);
        } else {
            func_80027440(vis_table_c.unk0118[sp->unk2C0].unk28 | 0x3000000, 80, 217,
                          &sp->style[4]);
            func_80026ecc(14, 120, 217, &sp->style[0]);
            func_80027440(vis_table_c.unk0118[sp->unk2C0].unk27 | 0x3000000, 136, 217,
                          &sp->style[4]);
        }
    }
}

/* The cell page: its name and grid, the cursor and the cells not yet set. */
ADDRESS(0x8002f97c, 0x2d0)
void func_8002f97c(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    CVECTOR grey = {64, 64, 64, 0};
    DVECTOR pos = {20, 152};
    int row, col, bit;

    sp->style[0].cd |= 0x80;
    func_8002732c((u16 *)(sp->unk2CC->text + sp->unk2CC->offsets[512 + sp->unk2BE]), 14, 136,
                  &sp->style[0]);
    sp->style[0].cd &= 0x7f;
    func_8002f1f8(&pos, &sp->unk0.vram[0], &sp->unk0.vram[1], &sp->style[0], 0xf0004);
    sp->unk330.w = 48;
    sp->unk330.h = 24;
    sp->unk330.x = (short)(sp->unk2BE / 3) * 49 + 11;
    sp->unk330.y = (short)(sp->unk2BE % 3) * 25 + 12;
    func_80027d18(&sp->unk330, &grey, 513, 0);
    sp->unk330.x += 2;
    sp->unk330.y += 1;
    sp->unk330.w -= 4;
    sp->unk330.h -= 2;
    func_80028220(&sp->unk330, &grey, 1);
    func_800286a8(1, 0, 0);
    sp->unk350.w = 48;
    sp->unk350.h = 24;
    sp->unk350.y = 12;
    sp->unk358.vy = 0;
    for (row = 0; row < 2; row++) {
        CVECTOR dim = {64, 64, 64, 0};

        sp->unk350.x = 11;
        sp->unk358.vx = 640;
        for (col = 0, bit = row; col < 6; bit += 3, col++) {
            if (!(*sp->unk2D4 & (1 << bit))) {
                func_80028220(&sp->unk350, &dim, 1);
                func_800286a8(2, 0, 0);
            }
            func_80028364(&sp->unk350, &sp->unk358, 0, 0, 16);
            sp->unk350.x += 49;
            sp->unk358.vx += 48;
        }
        sp->unk350.y += 25;
        sp->unk358.vy += 24;
    }
    func_800286a8(1, 0, 0);
}

/*
 * The status screen frame.
 */

/* Unreferenced: four zero bytes before this unit's constants. */
DATA(0x80087f84, 0x4, ".sdata")
static s32 DAT_80087f84 = 0;

/* The status screen frame: page toggles, colours, arrows and pages. */
ADDRESS(0x8002fc4c, 0x200)
void func_8002fc4c(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    RECT unused[2];
    RECT left = {310, 119, 16, 8};
    RECT right = {630, 119, 16, 8};

    func_800174c4();
    if (DAT_801cd180.unk4 & 0x800)
        DAT_800881aa = 1;
    else if (DAT_801cd180.unk4 & 0x400)
        DAT_800881aa = 0;
    DAT_801cd180.unk4 = 0;
    sp->style[0].r = sp->style[4].r = 80;
    sp->style[0].g = sp->style[4].g = 80;
    sp->style[0].b = sp->style[4].b = 80;
    sp->style[0].cd = 73;
    sp->style[4].cd = 49;
    if (sp->unk2E4 < 16)
        sp->unk360 = 8 - sp->unk2E4;
    else
        sp->unk360 = sp->unk2E4 - 24;
    sp->unk32C.b = 0;
    sp->unk32C.r = (u8)sp->unk360 * 4 + 96;
    sp->unk32C.g = (u8)sp->unk360 * 2 + 48;
    if (DAT_80088192) {
        if (DAT_800881aa == 0)
            func_8002916c(&right, &sp->unk32C, 0);
        else
            func_8002916c(&left, &sp->unk32C, 2048);
    }
    if (sp->unk2DC == 2) {
        if (sp->unk2C7 != 10)
            func_8002f498();
        else
            func_8002f97c();
    } else {
        func_8002e4d0();
    }
    if (DAT_800881aa == 0)
        func_8002ed98();
    else
        func_8002e94c();
    func_8002e490();
    func_8002c9e8();
    func_80017680();
}

/*
 * The stored-item grid page: five cells per row,
 * three rows shown, the selected entry's number and name, and the cell
 * images.
 */

/* The cells per grid row and the rows shown at once. */
#define STORE_COLUMNS 5
#define STORE_ROWS 3

ADDRESS(0x8002fe4c, 0x410)
void func_8002fe4c(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    RECT up = {320, 16, 32, 12};
    RECT down = {320, 208, 32, 12};
    int index, kind;
    int row, col, cols;

    DAT_800881aa = 0;
    sp->unk330.w = 96;
    sp->unk330.h = 48;
    sp->unk330.x = (short)(sp->unk2E0 % STORE_COLUMNS) * 112 + 48;
    sp->unk330.y = (short)(sp->unk2E0 / STORE_COLUMNS) * 56 + 32;
    func_80027d18(&sp->unk330, &sp->unk32C, 513, 1);
    func_800286a8(1, 0, 0);
    if (sp->unk2E2 > 0)
        func_8002916c(&up, &sp->unk32C, 3072);
    if (sp->unk2E2 <= sp->unk0.store.rows - 3)
        func_8002916c(&down, &sp->unk32C, 1024);
    func_800286a8(1, 0, 0);
    index = sp->unk2E2 * STORE_COLUMNS + sp->unk2E0;
    kind = sp->unk0.store.kinds[index];
    func_80027440((u16)(index + 1) | 0x3000000, 486, 214, &sp->style[4]);
    func_80026ecc(14, 531, 214, &sp->style[0]);
    func_80027440(150 | 0x3000000, 551, 214, &sp->style[4]);
    if (index < sp->unk0.store.count) {
        sp->style[0].cd |= 0x80;
        func_8002732c((u16 *)(sp->unk0.store.messages->text +
                              sp->unk0.store.messages->offsets[kind]),
                      40, 214, &sp->style[0]);
        sp->style[0].cd &= 0x7f;
    }
    func_80029548(32, 210, 576, 22);
    sp->unk350.w = 96;
    sp->unk350.h = 48;
    sp->unk350.y = 32;
    index = sp->unk2E2 * STORE_COLUMNS;
    for (row = 0; row < STORE_ROWS; row++) {
        if (sp->unk2E2 + row == sp->unk0.store.rows)
            cols = sp->unk0.store.last;
        else
            cols = STORE_COLUMNS;
        sp->unk350.x = 48;
        for (col = 0; col < cols; index++, col++) {
            if (index < sp->unk0.store.count) {
                func_80027d18(&sp->unk350, &sp->style[0], 513, 1);
                func_80028364(&sp->unk350, &sp->unk0.store.tex[index % 15], 0, 0, 16);
            }
            sp->unk350.x += 112;
        }
        sp->unk350.y += 56;
    }
    func_800286a8(0, 0, 0);
    func_80029548(32, 24, 576, 176);
}

/*
 * The stored-item detail page: the kind's name and image, then either its
 * fourteen attribute figures (page toggle on) or its labels, weight, size,
 * paired figures and up to three trait names.
 */
ADDRESS(0x8003025c, 0x85c)
void func_8003025c(void)
{
    DVECTOR tex = {960, 446};
    DVECTOR clut = {560, 498};
    DVECTOR pos = {34, 154};
    RECT left = {319, 119, 16, 8};
    RECT right = {621, 119, 16, 8};
    MenuScratch *sp = MENU_SCRATCH;
    u16 kind_w[17] = {113, 58, 36, 36, 58, 135, 168, 157, 69, 168, 80, 0, 58, 0, 69, 113, 0};
    MenuStoredItem *item;
    int kind;
    int i;

    kind = sp->unk0.store.kinds[sp->unk2E2 * 5 + sp->unk2E0];
    item = &sp->unk0.store.items[kind];
    if (DAT_801cd180.unk4 & 0x800)
        DAT_800881aa = 1;
    if (DAT_801cd180.unk4 & 0x400)
        DAT_800881aa = 0;
    DAT_801cd180.unk4 = 0;
    if (DAT_800881aa == 0)
        func_8002916c(&right, &sp->unk32C, 0);
    else
        func_8002916c(&left, &sp->unk32C, 2048);
    func_800286a8(1, 0, 0);
    sp->style[0].cd |= 0x80;
    func_8002732c(MESSAGE_IN(sp->unk0.store.messages, kind), 32, 16, &sp->style[0]);
    sp->style[0].cd &= 0x7f;
    func_8002f1f8(&pos, &tex, &clut, &sp->style[0], 0xf0005);
    if (DAT_800881aa) {
        for (i = 0; i < 14; i++) {
            func_8002732c(MESSAGE(100 + i), 336, i * 15 + 16, &sp->style[0]);
            func_80027440((u16)item->figures[i] | DIGITS(2), 582, i * 15 + 16, &sp->style[4]);
        }
    } else {
        for (i = 0; i < 10; i++)
            func_8002732c(MESSAGE(117 + i), 336, i * 15 + 16, &sp->style[0]);
        if (item->kind < 17)
            func_8002732c(MESSAGE(150 + item->kind), 606 - kind_w[item->kind], 16,
                          &sp->style[0]);
        func_80027440(item->unk2 | DIGITS(4), 558, 31, &sp->style[4]);
        func_80027440(item->unk1 | DIGITS(2), 582, 46, &sp->style[4]);
        DAT_8008819e = item->unk4 * 10 / 3048;
        func_80027440(DAT_8008819e | DIGITS(3), 522, 61, &sp->style[4]);
        func_80026ecc(55, 559, 61, &sp->style[4]);
        func_80027440((u16)((u16)(item->unk4 * 10 - DAT_8008819e * 3048) / 254) | DIGITS(2), 570,
                      61, &sp->style[4]);
        func_80026ecc(54, 595, 61, &sp->style[4]);
        DAT_8008819e = item->unk6 * 100 / 454;
        DAT_80088188 = item->unk6 * 1000 / 454;
        func_80027440(DAT_8008819e | DIGITS(4), 522, 76, &sp->style[4]);
        func_80026ecc(18, 574, 76, &sp->style[4]);
        func_80027440((u16)(DAT_80088188 - DAT_8008819e * 10) | DIGITS(1), 582, 76, &sp->style[4]);
        func_80026ecc(53, 594, 76, &sp->style[4]);
        if (item->pairs[0][1]) {
            func_80027440((u16)item->pairs[0][1] | DIGITS(3), 522, 91, &sp->style[4]);
            func_80026ecc(15, 558, 91, &sp->style[4]);
        }
        func_80027440((u16)item->pairs[0][0] | DIGITS(3), 570, 91, &sp->style[4]);
        if (item->pairs[1][1]) {
            func_80027440((u16)item->pairs[1][1] | DIGITS(3), 522, 106, &sp->style[4]);
            func_80026ecc(15, 558, 106, &sp->style[4]);
        }
        func_80027440((u16)item->pairs[1][0] | DIGITS(3), 570, 106, &sp->style[4]);
        if (item->pairs[2][1]) {
            func_80027440((u16)item->pairs[2][1] | DIGITS(3), 522, 121, &sp->style[4]);
            func_80026ecc(15, 558, 121, &sp->style[4]);
        }
        func_80027440((u16)item->pairs[2][0] | DIGITS(3), 570, 121, &sp->style[4]);
        if (item->pairs[3][1]) {
            func_80027440((u16)item->pairs[3][1] | DIGITS(3), 522, 136, &sp->style[4]);
            func_80026ecc(15, 558, 136, &sp->style[4]);
        }
        func_80027440((u16)item->pairs[3][0] | DIGITS(3), 570, 136, &sp->style[4]);
        if (item->traits & 0xf)
            func_8002732c(MESSAGE(129 + (item->traits & 0xf)), 336, 181, &sp->style[0]);
        if (item->traits >> 8 & 0xf)
            func_8002732c(MESSAGE(129 + (item->traits >> 8 & 0xf)), 336, 196, &sp->style[0]);
        if (item->traits >> 16 & 0xf)
            func_8002732c(MESSAGE(129 + (item->traits >> 16 & 0xf)), 336, 211, &sp->style[0]);
    }
    sp->unk350.x = 55;
    sp->unk350.y = 32;
    sp->unk350.w = 230;
    sp->unk350.h = 115;
    func_80027d18(&sp->unk350, &sp->style[0], 513, 1);
    func_800286a8(0, 0, 0);
    func_80028364(&sp->unk350, &sp->unk0.store.image, NULL, NULL, 16);
    func_80029548(24, 12, 292, 216);
    func_80029548(324, 12, 292, 216);
}

/*
 * The stored-item screen frame: colours, cursor blink,
 * then the grid page or the page func_8003025c draws.
 */

ADDRESS(0x80030ab8, 0x12c)
void func_80030ab8(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    RECT unused;
    RECT left = {314, 119, 16, 8};
    RECT right = {625, 119, 16, 8};

    func_800174c4();
    sp->style[0].r = sp->style[4].r = 80;
    sp->style[0].g = sp->style[4].g = 80;
    sp->style[0].b = sp->style[4].b = 80;
    sp->style[0].cd = 73;
    sp->style[4].cd = 49;
    if (sp->unk2E4 < 16)
        sp->unk360 = 8 - sp->unk2E4;
    else
        sp->unk360 = sp->unk2E4 - 24;
    sp->unk32C.b = 0;
    sp->unk32C.r = (u8)sp->unk360 * 4 + 96;
    sp->unk32C.g = (u8)sp->unk360 * 2 + 48;
    if (sp->unk2DC == 2)
        func_8003025c();
    else
        func_8002fe4c();
    func_8002c9e8();
    func_80017680();
}

/*
 * The id grid's left block: the cursor cell, scroll arrows and a position
 * bar, the header, and five rows of four cell images from the first row
 * shown, wrapping after twenty.
 */
ADDRESS(0x80030be4, 0x3bc)
void func_80030be4(void)
{
    RECT unused;
    CVECTOR light = {96, 96, 96};
    RECT up = {26, 40, 24, 8};
    RECT down = {26, 188, 24, 8};
    RECT bar = {20, 47, 16, 3};
    CVECTOR shade = {40, 40, 48};
    RECT edge;
    RECT unused2;
    MenuScratch *sp = MENU_SCRATCH;
    int step_x = 64;
    int step_y = 32;
    short column;
    int cell;
    int row, col;

    column = sp->unk2E0 % 8;
    if (column < 4) {
        sp->unk330.x = column * 64 + 50;
        sp->unk330.y = sp->unk2E0 / 8 * 32 + 35;
        sp->unk330.w = 56;
        sp->unk330.h = 28;
        func_80027d18(&sp->unk330, &sp->unk32C, 513, 1);
    }
    if (sp->unk0.grid.row0 > 0)
        func_8002916c(&up, &sp->unk32C, 3072);
    if (sp->unk0.grid.row0 < 73)
        func_8002916c(&down, &sp->unk32C, 1024);
    bar.y = sp->unk0.grid.row0 * 136 / 74 + 47;
    func_80028220(&bar, &shade, 1);
    func_800286a8(1, 0, 0);
    edge.x = bar.x - 2;
    edge.y = bar.y - 1;
    edge.w = bar.w;
    edge.h = bar.h;
    func_80028220(&edge, &shade, 1);
    func_800286a8(2, 0, 0);
    sp->unk33C.x = 60;
    sp->unk33C.y = 12;
    sp->unk33C.w = 196;
    sp->unk33C.h = 16;
    sp->unk358.vx = 896;
    sp->unk358.vy = 448;
    sp->unk35C.vx = 544;
    sp->unk35C.vy = 502;
    func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, 3);
    cell = sp->unk0.grid.row0 * 4 % 20;
    sp->unk350.y = 35;
    sp->unk350.w = 56;
    sp->unk350.h = 28;
    for (row = 0; row < 5; row++) {
        sp->unk350.x = 50;
        for (col = 0; col < 4; col++) {
            func_80028364(&sp->unk350, &sp->unk0.grid.tex[cell], NULL, NULL, 16);
            cell = (cell + 1) % 20;
            sp->unk350.x += step_x;
        }
        sp->unk350.y += step_y;
    }
    func_80029548(10, 9, 296, 188);
}

/* The id grid's right block, as func_80030be4 for the second id table. */
ADDRESS(0x80030fa0, 0x3d8)
void func_80030fa0(void)
{
    RECT unused;
    CVECTOR light = {96, 96, 96};
    RECT up = {612, 40, 24, 8};
    RECT down = {612, 188, 24, 8};
    RECT bar = {606, 47, 16, 3};
    CVECTOR shade = {40, 40, 48};
    RECT edge;
    RECT unused2;
    MenuScratch *sp = MENU_SCRATCH;
    int step_x = 64;
    int step_y = 32;
    short column;
    int cell;
    int row, col;

    column = sp->unk2E0 % 8;
    if (column >= 4) {
        sp->unk330.x = column * 64 + 82;
        sp->unk330.y = sp->unk2E0 / 8 * 32 + 35;
        sp->unk330.w = 56;
        sp->unk330.h = 28;
        func_80027d18(&sp->unk330, &sp->unk32C, 513, 1);
    }
    if (sp->unk0.grid.row1 > 0)
        func_8002916c(&up, &sp->unk32C, 3072);
    if (sp->unk0.grid.row1 < 73)
        func_8002916c(&down, &sp->unk32C, 1024);
    bar.y = sp->unk0.grid.row1 * 136 / 74 + 47;
    func_80028220(&bar, &shade, 1);
    func_800286a8(1, 0, 0);
    edge.x = bar.x - 2;
    edge.y = bar.y - 1;
    edge.w = bar.w;
    edge.h = bar.h;
    func_80028220(&edge, &shade, 1);
    func_800286a8(2, 0, 0);
    sp->unk33C.x = 384;
    sp->unk33C.y = 12;
    sp->unk33C.w = 196;
    sp->unk33C.h = 16;
    sp->unk358.vx = 896;
    sp->unk358.vy = 464;
    sp->unk35C.vx = 544;
    sp->unk35C.vy = 502;
    func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, 3);
    cell = sp->unk0.grid.row1 * 4 % 20;
    sp->unk350.y = 35;
    sp->unk350.w = 56;
    sp->unk350.h = 28;
    for (row = 0; row < 5; row++) {
        sp->unk350.x = 338;
        for (col = 0; col < 4; col++) {
            func_80028364(&sp->unk350, &sp->unk0.grid.tex[cell + 20], NULL, NULL, 16);
            cell = (cell + 1) % 20;
            sp->unk350.x += step_x;
        }
        sp->unk350.y += step_y;
    }
    func_80029548(334, 9, 296, 188);
}

/*
 * The id grid's detail panels: the name and figures
 * of the id under the grid cursor, and its table C counts.
 */

ADDRESS(0x80031378, 0x2fc)
void func_80031378(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    int id;
    short column;

    column = sp->unk2E0 % 8;
    if (column < 4)
        id = sp->unk0.grid.ids0[(sp->unk0.grid.row0 + sp->unk2E0 / 8) * 4 + column];
    else
        id = sp->unk0.grid.ids1[(sp->unk0.grid.row1 + sp->unk2E0 / 8) * 4 + column - 4];
    sp->style[0].cd |= 0x80;
    if (id != -1)
        func_8002732c((u16 *)(sp->unk0.grid.messages->text + sp->unk0.grid.messages->offsets[id]),
                      18, 202, &sp->style[0]);
    sp->style[0].cd &= 0x7f;
    if (id != -1) {
        if (vis_table_c.unk0118[id].unk27) {
            func_8002732c(MESSAGE(29), 74, 215, &sp->style[0]);
            func_80027440(vis_table_c.unk0118[id].unk28 | 0x3000000, 138, 215, &sp->style[4]);
            func_80026ecc(14, 174, 215, &sp->style[0]);
            func_80027440(vis_table_c.unk0118[id].unk27 | 0x3000000, 190, 215, &sp->style[4]);
        }
        if (vis_table_c.unk0118[id].unk24) {
            func_8002732c(MESSAGE(28), 250, 215, &sp->style[0]);
            DAT_8008819e = vis_table_c.unk0118[id].unk24 * 100 / 454;
            DAT_80088188 = vis_table_c.unk0118[id].unk24 * 1000 / 454;
            func_80027440(DAT_8008819e | 0x3000000, 300, 215, &sp->style[4]);
            func_80026ecc(18, 339, 215, &sp->style[0]);
            func_80027440((u16)(DAT_80088188 - DAT_8008819e * 10) | 0x1000000, 346, 215,
                          &sp->style[4]);
            func_80026ecc(53, 362, 215, &sp->style[0]);
        }
    }
    func_80029548(10, 198, 620, 34);
}

/*
 * Not exact: the 104-byte frame and cached column coordinates agree, but
 * the initial panel setup is merged and the selected ID test is extended
 * before the sentinel branch rather than loaded separately. The saved
 * column register also differs; this build remains 12 bytes short.
 */
ADDRESS(0x80031674, 0x334)
void func_80031674(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    int text_y;
    int i;
    short x, y;
    int name_x;
    int count_x;
    short column, index;
    short id;
    int none;
    u8 *count;
    int n, k, j;

    if ((short)(sp->unk2E0 % 8) < 4) {
        x = 334;
        y = 9;
        text_y = 135;
        count_x = 342;
    } else {
        x = 10;
        y = 9;
        text_y = 135;
        count_x = 18;
    }
    sp->unk350.x = x;
    sp->unk350.w = 192;
    sp->unk350.h = 124;
    sp->unk350.y = y;
    none = 0;
    column = sp->unk2E0 % 8;
    if (column < 4) {
        index = (sp->unk0.grid.row0 + sp->unk2E0 / 8) * 4 + column;
        id = sp->unk0.grid.ids0[index];
    } else {
        index = (sp->unk0.grid.row1 + sp->unk2E0 / 8 - 1) * 4 + column;
        id = sp->unk0.grid.ids1[index];
    }
    if (id == -1)
        none = 1;
    if (!none) {
        sp->unk358.vx = 640;
        sp->unk358.vy = 0;
        func_80028364(&sp->unk350, &sp->unk358, 0, 0, 16);
        n = 0;
        k = 2;
        name_x = count_x;
        sp->unk350.w = 100;
        sp->unk358.vx = 832;
        sp->unk350.x += 192;
        func_80028364(&sp->unk350, &sp->unk358, 0, 0, 16);
        count = vis_table_c.unk0118[id].unk10;
        for (i = 0; i < 4; i++) {
            int label_x = name_x;
            int value_x = name_x + 42;
            for (j = 0; j < 4; j++) {
                if (n == 14)
                    goto done;
                n++;
                if (*count) {
                    func_8002732c(MESSAGE(200 + k), label_x, text_y + j * 15, &sp->style[0]);
                    func_80027440(*count | 0x13000000, value_x, text_y + j * 15, &sp->style[4]);
                }
                count++;
                k++;
            }
            if (n == 14)
                break;
            name_x += 70;
        }
    }
done:
    func_80029548(x, y, 296, 188);
}

/* A two-tile emblem with its caption in a framed panel. */
ADDRESS(0x800319a8, 0x144)
void func_800319a8(void)
{
    CVECTOR light = {96, 96, 96};
    CVECTOR grey = {80, 80, 80};
    MenuScratch *sp = MENU_SCRATCH;

    sp->unk358.vx = 896;
    sp->unk358.vy = 480;
    sp->unk35C.vx = 544;
    sp->unk35C.vy = 503;
    sp->unk350.w = 32;
    sp->unk350.h = 16;
    sp->unk350.x = 353;
    sp->unk350.y = 168;
    func_80028364(&sp->unk350, &sp->unk358, &sp->unk35C, NULL, 0);
    sp->unk358.vx = 904;
    sp->unk35C.vy = 504;
    sp->unk350.x = 232;
    sp->unk350.y = 168;
    func_80028364(&sp->unk350, &sp->unk358, &sp->unk35C, NULL, 0);
    func_8002732c(MESSAGE(216), 188, 150, &sp->style[0]);
    func_80029548(168, 144, 319, 44);
}

/*
 * The id grid screen frame, a VRAM clear and a framed panel.
 */

/*
 * Draws the id grid: both column blocks while the 0x1 page bit is set,
 * otherwise the block under the cursor with its detail panel.
 */
ADDRESS(0x80031aec, 0x210)
void func_80031aec(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    RECT arrow = {319, 110, 24, 16};
    CVECTOR color = {16, 48, 16, 0};

    func_800174c4();
    sp->style[0].r = sp->style[4].r = 96;
    sp->style[0].g = sp->style[4].g = 96;
    sp->style[0].b = sp->style[4].b = 96;
    sp->style[0].cd = 73;
    sp->style[4].cd = 49;
    if (sp->unk2E4 < 16)
        sp->unk360 = 8 - sp->unk2E4;
    else
        sp->unk360 = sp->unk2E4 - 24;
    sp->unk32C.b = 0;
    sp->unk32C.r = (u8)sp->unk360 * 4 + 96;
    sp->unk32C.g = (u8)sp->unk360 * 2 + 48;
    if (sp->unk2DC & 0x8000)
        func_800319a8();
    DAT_800881ae = 1;
    if (sp->unk2DC & 0x1) {
        func_80030be4();
        func_80030fa0();
    } else {
        if ((short)(sp->unk2E0 % 8) < 4)
            func_80030be4();
        else
            func_80030fa0();
        func_80031674();
    }
    DAT_800881ae = 0;
    if ((short)(sp->unk2E0 % 8) < 4)
        func_8002916c(&arrow, &color, 0);
    else
        func_8002916c(&arrow, &color, 2048);
    func_800286a8(1, 0, 0);
    func_80031378();
    func_8002c9e8();
    func_80017680();
}

/* Clears the 640x480 VRAM area from the origin to black. */
ADDRESS(0x80031cfc, 0x54)
void func_80031cfc(void)
{
    RECT r = {0, 0, 640, 480};

    ClearImage(&r, 0, 0, 0);
}

ADDRESS(0x80031d50, 0x94)
void func_80031d50(void)
{
    func_800174c4();
    func_800175a4();
    func_8002948c();
    func_80021518(120, 120, 200, 136, 0, 144, 80, 16, 255, 25, 32738, 96, 96, 96, 0);
    func_80017634();
}
