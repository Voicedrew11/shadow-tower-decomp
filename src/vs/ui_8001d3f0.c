/*
 * VS.EXE: the menu screens' glyphs, glyph strings, numbers and rectangle
 * outline (WIP module). VS's copies of ST.EXE's func_80026ecc,
 * func_8002732c, func_80027440, func_800276bc and func_80027d18
 * (src/st/ui.c); VS advances the packet cursor before its bounds check.
 * The text clip window statics are read $gp-relative, so -G8.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <vs/data.h>
#include <vs/menu.h>

/* The text clip window (ST.EXE's DAT_800881ba..DAT_800881c8). */
DATA(0x80063390, 0x2, ".sbss")
static s16 DAT_80063390;
DATA(0x80063392, 0x2, ".sbss")
static s16 DAT_80063392;
DATA(0x80063394, 0x4, ".sbss")
static s32 DAT_80063394;
DATA(0x80063398, 0x4, ".sbss")
static s32 DAT_80063398;
/* Unreferenced. */
DATA(0x8006339c, 0x4, ".sbss")
static s32 DAT_8006339c;
DATA(0x800633a0, 0x4, ".sbss")
static s32 DAT_800633a0;

/*
 * Takes the next primitive from the current packet buffer, or returns once
 * the buffer has overflowed (the cursor is advanced first).
 */
#define NEW_PRIM(buf, p, type)                                       \
    (buf) = DAT_801624d0.packet;                                     \
    (p) = (type *)(buf)->cur;                                        \
    (buf)->cur = (u_long *)((p) + 1);                                \
    if (DAT_801624d0.packet->cur > DAT_801624d0.packet->end)         \
        return;                                                      \
    DAT_801624d0.unk8328++

/* Draws glyph `c` at (x, y): half-width codes below the split, full-width
 * codes after it. */
/* Not exact: retail keeps c and x in $t1/$t0 from entry, zero-extends the
 * half width before the u multiply and only masks x at the clip compare. */
ADDRESS(0x8001d3f0, 0x4c8)
void func_8001d3f0(u16 c, u16 x, u16 y, CVECTOR *color)
{
    StPacketBuffer *buf;
    POLY_FT4 *p;
    CVECTOR rgb;
    int cols, qw, qh, fw, ch;
    u16 half;
    u16 split;
    u16 u, v, q, r;
    int n;

    if (VS_MENU_SCRATCH->unk1D5 == 0) {
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
        NEW_PRIM(buf, p, POLY_FT4);
        setUV4(p, u, v, half + u, v, u, ch + v, half + u, ch + v);
        if (DAT_80063398) {
            x -= (DAT_80063378 == 1 ? DAT_8006333e : DAT_80063340) * (half + 1);
            DAT_80063394 += half + 1;
            if (DAT_80063392 < x) {
                DAT_800633a0++;
                return;
            }
            if (x < DAT_80063390)
                return;
        }
        setXY4(p, x, y, x + half, y, x, y + qh, x + half, y + qh);
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
        NEW_PRIM(buf, p, POLY_FT4);
        setUV4(p, u, v, fw + u, v, u, ch + v, fw + u, ch + v);
        if (DAT_80063398) {
            x -= (DAT_80063378 == 1 ? DAT_8006333e : DAT_80063340) * (half + 1);
            DAT_80063394 += fw;
            if (DAT_80063392 < x) {
                DAT_800633a0++;
                return;
            }
            if (x < DAT_80063390)
                return;
        }
        setXY4(p, x, y, x + qw, y, x, y + qh, x + qw, y + qh);
        rgb = *color;
        rgb.r -= 12;
        rgb.g -= 6;
        rgb.b -= 6;
    }
    setRGB0(p, rgb.r, rgb.g, rgb.b);
    if (color->cd & 0x80) {
        p->tpage = GetTPage(0, color->cd & 3, 896, 256);
        p->clut = GetClut(560, 496);
    } else {
        p->tpage = GetTPage(0, color->cd & 3, 960, 256);
        p->clut = GetClut(560, 495);
    }
    setlen(p, 9);
    setcode(p, 0x2e);
    addPrim(DAT_801624d0.ot_cur, p);
}

/* Draws a 0xffff-terminated glyph string; code 0 starts a new line. */
/* Not exact: the loop-end 0xffff lands in $t0 here, $v0 in retail. */
ADDRESS(0x8001d8b8, 0x114)
void func_8001d8b8(u16 *s, u16 x, u16 y, CVECTOR *color)
{
    u16 left = x;
    u_int space = (color->cd & 0x7c) >> 2;
    int line;
    u_int split;
    u_int half;

    if (VS_MENU_SCRATCH->unk1D5 == 0) {
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
            s += 2;
        } else {
            func_8001d3f0(*s, x, y, color);
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
/* Not exact: retail tests the power-of-ten loop's entry on n - 1 against
 * a -1 loaded at entry (this build folds it to n == 0). */
ADDRESS(0x8001d9cc, 0x27c)
void func_8001d9cc(u32 num, u16 x, u16 y, CVECTOR *color)
{
    u16 digit[8];
    u8 leading = 1;
    u8 neg = 0;
    u16 width = color->cd >> 2;
    u16 base;
    int n;
    u32 sign;
    s16 value;
    int pow;
    u32 div;
    int i;

    if (DAT_8006337a)
        width -= 4;
    if (VS_MENU_SCRATCH->unk1D5 == 0)
        base = 64;
    else
        base = 4;
    n = NUMBER_DIGITS(num);
    value = num;
    sign = num & NUMBER_SIGNED;
    if ((s16)num < 0) {
        value = -num;
        neg = 1;
    }
    if (value == 0) {
        func_8001d3f0(base, x + width * (n - 1), y, color);
        return;
    }
    pow = 1;
    i = n;
    while (i--)
        pow *= 10;
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
        func_8001d3f0(base + 13, x - width, y, color);
    else if (sign)
        func_8001d3f0(base + 12, x - width, y, color);
    i = n;
    while (i--) {
        if (digit[i] != 0)
            leading = 0;
        if (!leading)
            func_8001d3f0(base + digit[i], x + width * (n - i - 1), y, color);
    }
}

/* Draws a number with the fixed 7-pixel digit glyphs (codes 2-11). */
/* Not exact: as func_8001d9cc, the power-of-ten loop's entry test. */
ADDRESS(0x8001dc48, 0x244)
void func_8001dc48(u32 num, u16 x, u16 y, CVECTOR *color)
{
    u16 digit[8];
    u8 leading = 1;
    u8 neg = 0;
    u16 width = 7;
    int n;
    u32 sign;
    s16 value;
    u32 pow;
    u32 div;
    int i;

    n = NUMBER_DIGITS(num);
    value = num;
    sign = num & NUMBER_SIGNED;
    if ((s16)num < 0) {
        value = -num;
        neg = 1;
    }
    if (value == 0) {
        func_8001d3f0(2, x + width * (n - 1), y, color);
        return;
    }
    pow = 1;
    i = n;
    while (i--)
        pow *= 10;
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
        func_8001d3f0(13, x - width, y, color);
    else if (sign)
        func_8001d3f0(12, x - width, y, color);
    i = n;
    while (i--) {
        if (digit[i] != 0)
            leading = 0;
        if (!leading)
            func_8001d3f0(digit[i] + 2, x + width * (n - i - 1), y, color);
    }
}

/* The body of a flat rectangle: the code (semi-transparent when `semi` is
 * 1), the tag linked in front of the ordering table's head, the corners
 * and the colour. */
#define FILL_F4(p, semi, x0, y0, x1, y1, x2, y2, x3, y3, color)      \
    (p)->code = (semi) == 1 ? 0x2a : 0x28;                           \
    (p)->tag = (*DAT_801624d0.ot_cur & 0xffffff) | 0x05000000;       \
    setXY4(p, x0, y0, x1, y1, x2, y2, x3, y3);                       \
    setRGB0(p, (color)->r, (color)->g, (color)->b);                  \
    *DAT_801624d0.ot_cur = (u_long)(p) & 0xffffff

/* Draws a rectangle outline: bars `border & 0xff` high at the top and
 * bottom and `border >> 8` wide at the sides. */
/* Not exact: retail keeps `color` in $t4 and `border >> 8` in $t3 (swapped
 * here) and computes the latter before the overflow test. */
ADDRESS(0x8001de8c, 0x54c)
void func_8001de8c(RECT *r, CVECTOR *color, u16 border, u16 semi)
{
    POLY_F4 *p;
    StPacketBuffer *buf;
    int h = border & 0xff;
    int w = border >> 8;

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
