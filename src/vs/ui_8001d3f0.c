/*
 * VS.EXE: the menu text and window drawing (WIP module, from func_8001d3f0
 * through func_8001fcac, which reads the text clip statics): the text and
 * number printers, rectangles, frames, arrows and tiles, the card file
 * list. ST.EXE links copies of most of these routines.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <vs/data.h>
#include <vs/menu.h>
#include <vs/card.h>

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

/* func_8001fcac's fighter picture table (the module's constant pool) and
 * its two RECT initializers (8 bytes each, so .sdata under -G8). */
RODATA(0x8001117c, 0xc0)
SDATA(0x80063260, 0x10)

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
/* Not exact: the num-to-value copy precedes the digit-count shift here;
 * retail schedules it after the shift and mask. */
ADDRESS(0x8001d9cc, 0x27c)
void func_8001d9cc(u32 num, u16 x, u16 y, CVECTOR *color)
{
    u16 digit[8];
    int end;
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
    end = -1;
    if ((s16)num < 0) {
        value = -num;
        neg = 1;
    }
    if (value == 0) {
        func_8001d3f0(base, x + width * (n - 1), y, color);
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
/* Not exact: the digit-count decode follows the parameter copies here;
 * retail schedules it before them. */
ADDRESS(0x8001dc48, 0x244)
void func_8001dc48(u32 num, u16 x, u16 y, CVECTOR *color)
{
    u16 digit[8];
    int end;
    u8 leading = 1;
    u8 neg = 0;
    int n = NUMBER_DIGITS(num);
    u16 width = 7;
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
        func_8001d3f0(2, x + width * (n - 1), y, color);
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

/*
 * Takes the next primitive from the current packet buffer, or returns once
 * the buffer has overflowed (the cursor is advanced first). A statement
 * sequence, as ST.EXE's NEW_PRIM.
 */


/* Fills a rectangle (semi-transparent when `semi` is 1). */
ADDRESS(0x8001e3d8, 0x158)
void func_8001e3d8(RECT *r, CVECTOR *color, u16 semi)
{
    POLY_F4 *p;
    StPacketBuffer *buf;

    NEW_PRIM(buf, p, POLY_F4);
    FILL_F4(p, semi, r->x, r->y, r->x + r->w, r->y, r->x, r->y + r->h, r->x + r->w, r->y + r->h,
            color);
}

/*
 * Draws the texture rectangle `tex` over `r` with CLUT `clut`, tinted by
 * `color` (grey when NULL); `mode` as ST.EXE's func_80028364, plus bit 8: twice the
 * texture width.
 */
ADDRESS(0x8001e530, 0x360)
void func_8001e530(RECT *r, DVECTOR *tex, DVECTOR *clut, CVECTOR *color, int mode)
{
    POLY_FT4 *p;
    StPacketBuffer *buf;
    int semi = mode & 1;
    int abr = (mode >> 1) & 3;
    int tp = (mode >> 3) & 3;
    int flip = (mode >> 5) & 3;
    int wide = (mode >> 8) & 1;
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
    if (u0 + r->w < 256) {
        if (wide)
            u1 = u0 + r->w * 2;
        else
            u1 = u0 + r->w;
    }
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
    p->tag = (*DAT_801624d0.ot_cur & 0xffffff) | 0x09000000;
    setXY4(p, r->x, r->y, r->x + r->w, r->y, r->x, r->y + r->h, r->x + r->w, r->y + r->h);
    if (flip == 1)
        setUV4(p, u1 - 1, v0, u0 - 1, v0, u1 - 1, v1, u0 - 1, v1);
    else if (flip == 2)
        setUV4(p, u0, v1 - 1, u1, v1 - 1, u0, v0 - 1, u1, v0 - 1);
    else
        setUV4(p, u0, v0, u1, v0, u0, v1, u1, v1);
    p->code = semi ? 0x2e : 0x2c;
    *DAT_801624d0.ot_cur = (u_long)p & 0xffffff;
}

/* Adds a draw-mode primitive selecting the texture page at (x, y) with
 * blend rate `abr`. */
ADDRESS(0x8001e890, 0xe8)
void func_8001e890(int abr, int x, int y)
{
    DR_MODE *p;
    StPacketBuffer *buf;

    NEW_PRIM(buf, p, DR_MODE);
    SetDrawMode(p, 0, 0, GetTPage(0, abr, x, y), NULL);
    addPrim(DAT_801624d0.ot_cur, p);
}

/* Draws a Gouraud quad: corner positions and colours in setXY4 order. */
ADDRESS(0x8001e978, 0x1a0)
void func_8001e978(DVECTOR *xy, CVECTOR *rgb, int semi)
{
    StPacketBuffer *buf;
    POLY_G4 *p;

    NEW_PRIM(buf, p, POLY_G4);
    p->tag = (*DAT_801624d0.ot_cur & 0xffffff) | 0x08000000;
    setRGB0(p, rgb[0].r, rgb[0].g, rgb[0].b);
    setRGB1(p, rgb[1].r, rgb[1].g, rgb[1].b);
    setRGB2(p, rgb[2].r, rgb[2].g, rgb[2].b);
    setRGB3(p, rgb[3].r, rgb[3].g, rgb[3].b);
    setXY4(p, xy[0].vx, xy[0].vy, xy[1].vx, xy[1].vy, xy[2].vx, xy[2].vy, xy[3].vx, xy[3].vy);
    p->code = semi == 0 ? 0x38 : 0x3a;
    *DAT_801624d0.ot_cur = (u_long)p & 0xffffff;
}

/*
 * Takes the next primitive from the current packet buffer, or returns once
 * the buffer has overflowed (the cursor is advanced first).
 */

/* Fills `r` with a vertical gradient from `edge` at the top and bottom to
 * `mid` across the middle. */
/* Not exact: as ST.EXE's copy, retail schedules the halved height around
 * the packet-buffer load with other temporaries. */
ADDRESS(0x8001eb18, 0x3dc)
void func_8001eb18(RECT *r, CVECTOR *mid, CVECTOR *edge, int semi)
{
    StPacketBuffer *buf;
    POLY_G4 *p;
    RECT b;

    b = *r;
    b.h = b.h >> 1;
    NEW_PRIM(buf, p, POLY_G4);
    p->code = semi == 0 ? 0x38 : 0x3a;
    p->tag = (*DAT_801624d0.ot_cur & 0xffffff) | 0x08000000;
    setRGB0(p, edge->r, edge->g, edge->b);
    setRGB1(p, edge->r, edge->g, edge->b);
    setRGB2(p, mid->r, mid->g, mid->b);
    setRGB3(p, mid->r, mid->g, mid->b);
    setXY4(p, b.x, b.y, b.x + b.w, b.y, b.x, b.y + b.h, b.x + b.w, b.y + b.h);
    *DAT_801624d0.ot_cur = (u_long)p & 0xffffff;
    b = *r;
    b.h = b.h >> 1;
    b.y = b.h + b.y;
    NEW_PRIM(buf, p, POLY_G4);
    p->code = semi == 0 ? 0x38 : 0x3a;
    p->tag = (*DAT_801624d0.ot_cur & 0xffffff) | 0x08000000;
    setRGB0(p, mid->r, mid->g, mid->b);
    setRGB1(p, mid->r, mid->g, mid->b);
    setRGB2(p, edge->r, edge->g, edge->b);
    setRGB3(p, edge->r, edge->g, edge->b);
    setXY4(p, b.x, b.y, b.x + b.w, b.y, b.x, b.y + b.h, b.x + b.w, b.y + b.h);
    *DAT_801624d0.ot_cur = (u_long)p & 0xffffff;
}

/* Draws a triangle filling `r` that points along `dir` (0, 1024, 2048 or
 * 3072), over a copy offset by one pixel. */
ADDRESS(0x8001eef4, 0x348)
void func_8001eef4(RECT *r, CVECTOR *color, u16 dir)
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
    addPrim(DAT_801624d0.ot_cur, p);
    NEW_PRIM(buf, p, POLY_F3);
    setlen(p, 4);
    setcode(p, 0x22);
    setXY3(p, x0 + 1, y0 + 1, x1 + 1, y1 + 1, x2 + 1, y2 + 1);
    setRGB0(p, color->r, color->g, color->b);
    addPrim(DAT_801624d0.ot_cur, p);
}

/* Draws a menu panel: a dark fill over a lighter border, both blended. */
ADDRESS(0x8001f23c, 0xd0)
void func_8001f23c(int x, int y, int w, int h)
{
    VS_MENU_SCRATCH->unk220.g = 80;
    VS_MENU_SCRATCH->unk220.r = 80;
    VS_MENU_SCRATCH->unk220.b = 96;
    VS_MENU_SCRATCH->unk22C.g = 40;
    VS_MENU_SCRATCH->unk22C.r = 40;
    VS_MENU_SCRATCH->unk224.x = x;
    VS_MENU_SCRATCH->unk224.y = y;
    VS_MENU_SCRATCH->unk224.w = w;
    VS_MENU_SCRATCH->unk224.h = h;
    VS_MENU_SCRATCH->unk22C.b = 48;
    VS_MENU_SCRATCH->unk230.x = x + 2;
    VS_MENU_SCRATCH->unk230.y = y + 1;
    VS_MENU_SCRATCH->unk230.w = w - 4;
    VS_MENU_SCRATCH->unk230.h = h - 2;
    func_8001e3d8(&VS_MENU_SCRATCH->unk230, &VS_MENU_SCRATCH->unk22C, 1);
    func_8001e890(2, 0, 0);
    func_8001e3d8(&VS_MENU_SCRATCH->unk224, &VS_MENU_SCRATCH->unk220, 1);
    func_8001e890(0, 0, 0);
}

/* func_8001e530 modes: a semi-transparent tile, flipped. */
#define TILE (1)
#define TILE_HFLIP (1 | 1 << 5)
#define TILE_VFLIP (1 | 2 << 5)

/*
 * Draws a window frame from 16x16 tiles of the menu texture page: corners,
 * edges, an inner bevel and the filled middle.
 */
/* Not exact: register allocation, as ST.EXE's copy: retail keeps h in $s6
 * and spills only w; this build spills both. */
ADDRESS(0x8001f30c, 0x51c)
void func_8001f30c(short x, short y, short w, short h)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    short right;
    short bottom;
    int i, j;

    sp->unk224.w = 16;
    sp->unk224.h = 16;
    sp->unk244.vx = 544;
    sp->unk244.vy = 504;
    sp->unk240.vx = 612;
    sp->unk224.x = x;
    sp->unk224.y = y;
    sp->unk240.vy = 400;
    func_8001e530(&sp->unk224, &sp->unk240, &sp->unk244, NULL, TILE);
    sp->unk224.y = y + h - 16;
    func_8001e530(&sp->unk224, &sp->unk240, &sp->unk244, NULL, TILE_VFLIP);
    right = x + w - 16;
    sp->unk224.y = y;
    sp->unk240.vx = 620;
    sp->unk224.x = right;
    func_8001e530(&sp->unk224, &sp->unk240, &sp->unk244, NULL, TILE);
    sp->unk224.y = y + h - 16;
    func_8001e530(&sp->unk224, &sp->unk240, &sp->unk244, NULL, TILE_VFLIP);
    sp->unk224.x = x;
    sp->unk224.y = y + 16;
    sp->unk240.vx = 604;
    func_8001e530(&sp->unk224, &sp->unk240, &sp->unk244, NULL, TILE);
    sp->unk224.y = y + h - 32;
    func_8001e530(&sp->unk224, &sp->unk240, &sp->unk244, NULL, TILE_VFLIP);
    sp->unk224.y = y + 16;
    sp->unk240.vx = 596;
    sp->unk224.x = right;
    func_8001e530(&sp->unk224, &sp->unk240, &sp->unk244, NULL, TILE);
    sp->unk224.y = y + h - 32;
    func_8001e530(&sp->unk224, &sp->unk240, &sp->unk244, NULL, TILE_VFLIP);

    sp->unk224.y = y + 32;
    sp->unk224.x = x;
    sp->unk240.vx = 624;
    i = h / 16 - 4;
    while (i--) {
        sp->unk224.x = x;
        func_8001e530(&sp->unk224, &sp->unk240, &sp->unk244, NULL, TILE_HFLIP);
        sp->unk224.x = right;
        func_8001e530(&sp->unk224, &sp->unk240, &sp->unk244, NULL, TILE);
        sp->unk224.y += 16;
    }
    sp->unk224.x = x + 16;
    sp->unk240.vx = 616;
    i = w / 16 - 2;
    while (i--) {
        sp->unk224.y = y;
        func_8001e530(&sp->unk224, &sp->unk240, &sp->unk244, NULL, TILE);
        sp->unk224.y = y + h - 16;
        func_8001e530(&sp->unk224, &sp->unk240, &sp->unk244, NULL, TILE_VFLIP);
        sp->unk224.x += 16;
    }
    sp->unk224.x = x + 16;
    sp->unk240.vx = 608;
    i = w / 16 - 2;
    while (i--) {
        sp->unk224.y = y + 16;
        func_8001e530(&sp->unk224, &sp->unk240, &sp->unk244, NULL, TILE);
        sp->unk224.y = y + h - 32;
        func_8001e530(&sp->unk224, &sp->unk240, &sp->unk244, NULL, TILE_VFLIP);
        sp->unk224.x += 16;
    }
    sp->unk224.y = y + 32;
    sp->unk240.vx = 600;
    j = h / 16 - 4;
    while (j--) {
        sp->unk224.x = x + 16;
        i = w / 16 - 2;
        while (i--) {
            func_8001e530(&sp->unk224, &sp->unk240, &sp->unk244, NULL, TILE);
            sp->unk224.x += 16;
        }
        sp->unk224.y += 16;
    }
}

ADDRESS(0x8001f828, 0x9c)
void func_8001f828(void)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    int n;

    sp->unk238.w = 128;
    sp->unk238.h = 240;
    sp->unk240.vx = 768;
    sp->unk238.y = 0;
    sp->unk238.x = 0;
    sp->unk240.vy = 256;
    n = 5;
    while (n--) {
        func_8001e530(&sp->unk238, &sp->unk240, 0, 0, 16);
        sp->unk238.x += 128;
    }
}

/* As func_8001f828, three strips wide. */
ADDRESS(0x8001f8c4, 0x9c)
void func_8001f8c4(void)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    int n;

    sp->unk238.w = 128;
    sp->unk238.h = 240;
    sp->unk240.vx = 768;
    sp->unk238.y = 0;
    sp->unk238.x = 0;
    sp->unk240.vy = 256;
    n = 3;
    while (n--) {
        func_8001e530(&sp->unk238, &sp->unk240, 0, 0, 16);
        sp->unk238.x += 128;
    }
}

ADDRESS(0x8001f960, 0x40)
void func_8001f960(void)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;

    func_8001d8b8(VS_MESSAGE(310), 32, 74, &sp->style[0]);
}

/*
 * Draws message 370 + unk1BC, the highlight bar over the choice unk1CC
 * selects, clears the two seven-entry tables and draws four tiles of the
 * picture at (896, 512).
 */
ADDRESS(0x8001f9a0, 0x1f8)
void func_8001f9a0(void)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    int i;

    func_8001d8b8(VS_MESSAGE(370 + sp->unk1BC), 50, 60, &sp->style[0]);
    sp->unk218.x = 112;
    sp->unk218.w = 100;
    sp->unk218.h = 18;
    if (sp->unk1CC == 2)
        sp->unk218.y = 140;
    else
        sp->unk218.y = 164;
    func_8001de8c(&sp->unk218, &sp->unk214, 257, 1);
    func_8001e890(1, 0, 0);
    for (i = 0; i < 7; i++) {
        DAT_801c4a38[i] = 0;
        DAT_80117598[i] = 0;
    }
    sp->unk240.vx = 896;
    sp->unk240.vy = 512;
    sp->unk244.vx = 544;
    sp->unk244.vy = 511;
    sp->unk224.x = 166;
    sp->unk224.y = 142;
    sp->unk224.w = 40;
    sp->unk224.h = 16;
    func_8001e530(&sp->unk224, &sp->unk240, &sp->unk244, NULL, 3);
    sp->unk224.y = 166;
    func_8001e530(&sp->unk224, &sp->unk240, &sp->unk244, NULL, 3);
    sp->unk240.vx = 906;
    sp->unk224.x = 118;
    sp->unk224.y = 141;
    sp->unk224.w = 40;
    func_8001e530(&sp->unk224, &sp->unk240, &sp->unk244, NULL, 3);
    sp->unk240.vx = 934;
    sp->unk224.x = 118;
    sp->unk224.y = 165;
    sp->unk224.w = 40;
    func_8001e530(&sp->unk224, &sp->unk240, &sp->unk244, NULL, 3);
}

ADDRESS(0x8001fb98, 0x114)
void func_8001fb98(void)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;

    if (sp->unk0.menu.half[1] == 0) {
        func_8001d8b8(VS_MESSAGE(311), 32, 74, &sp->style[0]);
        if (sp->unk1C6) {
            func_8001d8b8(VS_MESSAGE(325), 100, 160, &sp->style[1]);
            func_8001d8b8(VS_MESSAGE(326), 170, 160, &sp->style[0]);
        } else {
            func_8001d8b8(VS_MESSAGE(325), 100, 160, &sp->style[0]);
            func_8001d8b8(VS_MESSAGE(326), 170, 160, &sp->style[1]);
        }
    } else {
        func_8001d8b8(VS_MESSAGE(312), 32, 74, &sp->style[0]);
        func_8001d8b8(VS_MESSAGE(332), 40, 100, &sp->style[0]);
    }
}

/*
 * Draws the card file list: on the confirmation page (half[1] 1) the
 * save/load question with its two choices, then the scroll arrows, the
 * cursor bar and up to three rows (file number, the fighter of the save
 * and its won count), each framed; the clipped text advance of each row
 * goes to DAT_801c4a38.
 * Not exact: retail's frame is 272 bytes, this build's 264 (first
 * divergence at +0x0), and retail rereads the file row for each field;
 * retail is 0x6d8 bytes, this build 0x6a4.
 */
ADDRESS(0x8001fcac, 0x6d8)
void func_8001fcac(void)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    s32 picture[48] = {
        -1, 0, 5, 2, 0, 0, 0, -1, 4, 4, 3, 1, 1, 4, 6, 1,
        -1, 5, 2, -1, 2, 3, -1, 6, 2, 6, 6, 1, 1, -1, -1, -1,
        3, 1, -1, -1, -1, 3, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    };
    RECT unused_left = {25, 93, 16, 8};
    RECT unused_right = {294, 93, 16, 8};
    VsCardFile *file;
    int rows;
    int i;

    if (sp->unk0.menu.half[1] == 1) {
        func_8001d8b8(VS_MESSAGE((sp->unk1CC & 0xff) ? 323 : 324), 68, 128, &sp->style[0]);
        sp->style[0].r = sp->style[0].g = sp->style[0].b = 24;
        if (sp->unk1C6) {
            func_8001d8b8(VS_MESSAGE(325), 120, 150, &sp->style[1]);
            func_8001d8b8(VS_MESSAGE(326), 190, 150, &sp->style[0]);
        } else {
            func_8001d8b8(VS_MESSAGE(325), 120, 150, &sp->style[0]);
            func_8001d8b8(VS_MESSAGE(326), 190, 150, &sp->style[1]);
        }
        sp->style[0].r = sp->style[0].g = sp->style[0].b = 72;
        sp->unk224.x = 60;
        sp->unk224.y = 120;
        sp->unk224.w = 200;
        sp->unk224.h = 50;
        sp->unk230.x = 61;
        sp->unk230.y = 121;
        sp->unk230.w = 198;
        sp->unk230.h = 48;
        sp->unk220.b = sp->unk220.g = sp->unk220.r = 64;
        sp->unk22C.b = sp->unk22C.g = sp->unk22C.r = 32;
        func_8001e3d8(&sp->unk230, &sp->unk22C, 1);
        func_8001e890(2, 0, 0);
        func_8001e3d8(&sp->unk224, &sp->unk220, 0);
    }
    if (sp->unk1BE >= 4) {
        if (sp->unk1C4 > 0) {
            sp->unk224.x = 152;
            sp->unk224.y = 68;
            sp->unk224.w = 16;
            sp->unk224.h = 8;
            func_8001eef4(&sp->unk224, &sp->unk214, 3072);
            func_8001e890(1, 0, 0);
        }
        if (sp->unk1C4 < sp->unk1BE - 3) {
            sp->unk224.x = 152;
            sp->unk224.y = 215;
            sp->unk224.w = 16;
            sp->unk224.h = 8;
            func_8001eef4(&sp->unk224, &sp->unk214, 1024);
            func_8001e890(1, 0, 0);
        }
    }
    sp->unk218.x = 32;
    sp->unk218.w = 256;
    sp->unk218.h = 40;
    sp->unk218.y = sp->unk1C2 * 48 + 74;
    func_8001de8c(&sp->unk218, &sp->unk214, 257, 1);
    func_8001e890(1, 0, 0);
    sp->unk218.y = 74;
    rows = 3;
    if (sp->unk1BE < 3)
        rows = sp->unk1BE;
    for (i = 0; i < rows; i++) {
        if (sp->unk1C0 == 1 && i + sp->unk1C4 == sp->unk1BE - 1) {
            func_8001d8b8(VS_MESSAGE(322), 64, sp->unk218.y + 6, &sp->style[0]);
        } else {
            if (sp->unk1CC & 0xff00)
                func_8001dc48(0x2000000 | (DAT_80063328[sp->unk1BC] + 1), 38,
                              sp->unk218.y + 6, &sp->style[0]);
            else
                func_8001dc48(0x2000000 | (u16)(sp->unk1C4 + i + 1), 38,
                              sp->unk218.y + 6, &sp->style[0]);
            func_8001d8b8(VS_MESSAGE(328), 218, sp->unk218.y + 6, &sp->style[0]);
            DAT_80063378 = 1;
            DAT_80063398 = 0;
            DAT_80063390 = 0;
            DAT_80063392 = 280;
            file = &sp->unk1B8->files[i + sp->unk1C4];
            if (picture[file->unk15] != -1)
                func_8001d8b8(VS_MESSAGE(610 + picture[file->unk15]), 64, sp->unk218.y + 6,
                              &sp->style[0]);
            file = &sp->unk1B8->files[i + sp->unk1C4];
            if (picture[file->unk15] != -1)
                func_8001d8b8(VS_MESSAGE(700 + file->unk15), DAT_80063394 + 64,
                              sp->unk218.y + 22, &sp->style[0]);
            else
                func_8001d8b8(VS_MESSAGE(700 + sp->unk1B8->files[i + sp->unk1C4].unk15),
                              DAT_80063394 + 64, sp->unk218.y + 6, &sp->style[0]);
            func_8001dc48(0x4000000 | sp->unk1B8->files[i + sp->unk1C4].unk16, 248,
                          sp->unk218.y + 6, &sp->style[0]);
        }
        DAT_801c4a38[i + sp->unk1C4] = DAT_800633a0;
        func_8001de8c(&sp->unk218, &sp->unk220, 257, 1);
        sp->unk218.y += 48;
    }
    func_8001e890(1, 0, 0);
}
