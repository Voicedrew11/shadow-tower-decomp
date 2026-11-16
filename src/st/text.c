/*
 * ST.EXE: font glyphs, text strings, numbers and 2D menu primitives (WIP
 * module). Glyphs are textured quads cut from the font pages in VRAM; the
 * scratchpad's font mode picks the 6x12 or 7x10 half-width cell layout.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/data.h>
#include <st/gfx.h>
#include <st/menu.h>
#include <st/text.h>
#include <st/funcs.h>

JUMPTABLES(0x80011490, 0x18)
SDATA(0x80087f1c, 0x18)

/* The text clip window. */
DATA(0x800881ba, 0x2, ".sbss")
s16 DAT_800881ba;
DATA(0x800881bc, 0x2, ".sbss")
s16 DAT_800881bc;
DATA(0x800881c0, 0x4, ".sbss")
s32 DAT_800881c0;
DATA(0x800881c4, 0x4, ".sbss")
s32 DAT_800881c4;
DATA(0x800881c8, 0x4, ".sbss")
s32 DAT_800881c8;

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

/* Not exact: retail assigns the layout constants other registers (cell height
 * in $t5, full width in $t3) and zero-extends the half width before the u multiply. */
ADDRESS(0x80026ecc, 0x460)
void func_80026ecc(u16 c, u16 x, u16 y, CVECTOR *color)
{
    StPacketBuffer *buf;
    POLY_FT4 *p;
    CVECTOR rgb;
    int half, cols, qw, qh, fw, ch;
    u16 split;
    u16 u, v, q, r;
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
        NEW_PRIM(buf, p, POLY_FT4);
        setUV4(p, u, v, half + u, v, u, ch + v, half + u, ch + v);
        if (DAT_800881c4) {
            x -= (DAT_800881ac == 1 ? DAT_800881a0 : DAT_800881a2) * (half + 1);
            DAT_800881c0 += half + 1;
            if (DAT_800881bc < x) {
                DAT_800881c8++;
                return;
            }
            if (x < DAT_800881ba)
                return;
        }
        setXY4(p, x, y, x + half, y, x, y + qh, x + half, y + qh);
    } else {
        n = c - split / 2;
        r = n % cols;
        q = n / cols;
        u = fw * r;
        v = ch * q;
        NEW_PRIM(buf, p, POLY_FT4);
        setUV4(p, u, v, fw + u, v, u, ch + v, fw + u, ch + v);
        if (DAT_800881c4) {
            x -= (DAT_800881ac == 1 ? DAT_800881a0 : DAT_800881a2) * (half + 1);
            DAT_800881c0 += fw;
            if (DAT_800881bc < x) {
                DAT_800881c8++;
                return;
            }
            if (x < DAT_800881ba)
                return;
        }
        setXY4(p, x, y, x + qw, y, x, y + qh, x + qw, y + qh);
    }
    rgb = *color;
    rgb.r += 8;
    rgb.g += 8;
    rgb.b += 8;
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
/* Not exact: retail copies the line start from $a1 (the incoming x), this build
 * from its $s1 copy; the loop-end 0xffff lands in $t0 here. */
ADDRESS(0x8002732c, 0x114)
void func_8002732c(u16 *s, u16 x, u16 y, CVECTOR *color)
{
    int left = x;
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
            s += 2;
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
/* Not exact: retail tests the power-of-ten loop on n - 1 against -1 (no fold to
 * n == 0) and allocates the parameters to other saved registers. */
ADDRESS(0x80027440, 0x27c)
void func_80027440(u32 num, u16 x, u16 y, CVECTOR *color)
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

    if (DAT_800881ae)
        width -= 4;
    if (MENU_SCRATCH->unk2E5 == 0)
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
        func_80026ecc(base, x + width * (n - 1), y, color);
        return;
    }
    pow = 1;
    for (i = 0; i < n; i++)
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
/* Not exact: as func_80027440: the power-of-ten loop test and parameter registers. */
ADDRESS(0x800276bc, 0x244)
void func_800276bc(u32 num, u16 x, u16 y, CVECTOR *color)
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
        func_80026ecc(2, x + width * (n - 1), y, color);
        return;
    }
    pow = 1;
    for (i = 0; i < n; i++)
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
/* Not exact: retail keeps `color` in $t4 and computes `border >> 8` at entry;
 * the bottom bar loads y before h. */
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
/* Not exact: retail keeps separate divides for the fade-in and fade-out cases
 * (this build cross-jumps them) and forms the world_state base later. */
ADDRESS(0x80028784, 0x4a8)
void func_80028784(void)
{
    RECT screen = {0, 0, 320, 240};
    RECT card = {32, 132, 255, 255};
    CVECTOR fill;
    CVECTOR tint;
    DVECTOR tex;
    DVECTOR clut;

    tex.vx = 960;
    tex.vy = 256;
    clut.vx = 560;
    clut.vy = 495;
    if (world_state.unk1EE17 == 1) {
        tex.vy = 0;
        clut.vx = 0;
        clut.vy = 511;
    }
    if (world_state.unk1EE18 == 0)
        world_state.unk1EE18 = 1;
    switch (world_state.unk1EE16) {
    case CARD_FADE_IN:
        fill.r = fill.g = fill.b =
            world_state.unk1EE17 == 3 ? 255 : (world_state.unk1EE19 << 4) / world_state.unk1EE18;
        func_80028220(&screen, &fill, 1);
        func_800286a8(2, 0, 0);
        break;
    case CARD_SHOW_IN:
        if (DAT_801cd124 == 1 || world_state.unk1EE17 == 2) {
            tint.r = tint.g = tint.b = world_state.unk1EE19 << 3;
            func_80028364(&card, &tex, &clut, &tint, 3);
            if (world_state.unk1EE19 >= 16)
                world_state.unk1EE16 = CARD_SHOW;
        } else {
            world_state.unk1EE16 = CARD_SHOW;
        }
        break;
    case CARD_SHOW:
        if (DAT_801cd124 == 1 || world_state.unk1EE17 == 2) {
            tint.r = tint.g = tint.b = 128;
            func_80028364(&card, &tex, &clut, &tint,
                          DAT_80088190 == 25 || DAT_80088190 == 30 || DAT_80088190 == 5 ? 2 : 3);
        }
        break;
    case CARD_SHOW_OUT:
        if (DAT_801cd124 == 1 || world_state.unk1EE17 == 2) {
            tint.r = tint.g = tint.b = (16 - world_state.unk1EE19) << 3;
            func_80028364(&card, &tex, &clut, &tint, 3);
            if (world_state.unk1EE19 >= 16)
                world_state.unk1EE16 = CARD_DONE;
        } else {
            world_state.unk1EE16 = CARD_DONE;
        }
        break;
    case CARD_FADE_OUT:
        fill.r = fill.g = fill.b =
            world_state.unk1EE17 == 3
                ? 255
                : ((world_state.unk1EE18 - world_state.unk1EE19) << 4) / world_state.unk1EE18;
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
/* Not exact: retail schedules the halved height around the packet-buffer load
 * with other temporaries. */
ADDRESS(0x80028db8, 0x3b4)
void func_80028db8(RECT *r, CVECTOR *mid, CVECTOR *edge, int semi)
{
    StPacketBuffer *buf;
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
    NEW_PRIM(buf, p, POLY_G4);
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
/* Not exact: the x1 and world_state-base temporaries are swapped ($t2/$t1). */
ADDRESS(0x8002916c, 0x320)
void func_8002916c(RECT *r, CVECTOR *color, u16 dir)
{
    short x0 = 0, y0 = 0, x1 = 0, y1 = 0, x2 = 0, y2 = 0;
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
    setXY3(p, x0 + 1, y0 + 1, x1 + 1, y1 + 1, x2 + 1, y2 + 1);
    setlen(p, 4);
    setcode(p, 0x22);
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
/* Not exact: register allocation: retail spills `right` to the stack and keeps
 * w and h in $s7/$s6; this build spills w. */
ADDRESS(0x80029618, 0x55c)
void func_80029618(short x, short y, short w, short h)
{
    MenuScratch *sp = MENU_SCRATCH;
    RECT r;
    CVECTOR wash = {48, 48, 48};
    short right;
    short bottom;
    int i, j;

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
    i = h / 16 - 4;
    while (i--) {
        sp->unk33C.x = x;
        func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, TILE_HFLIP);
        sp->unk33C.x = right;
        func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, TILE);
        sp->unk33C.y += 16;
    }
    sp->unk33C.x = x + 16;
    sp->unk358.vx = 616;
    i = w / 16 - 2;
    while (i--) {
        sp->unk33C.y = y;
        func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, TILE);
        sp->unk33C.y = y + h - 16;
        func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, TILE_VFLIP);
        sp->unk33C.x += 16;
    }
    sp->unk33C.x = x + 16;
    sp->unk358.vx = 608;
    i = w / 16 - 2;
    while (i--) {
        sp->unk33C.y = y + 16;
        func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, TILE);
        sp->unk33C.y = y + h - 32;
        func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, TILE_VFLIP);
        sp->unk33C.x += 16;
    }
    sp->unk33C.y = y + 32;
    sp->unk358.vx = 600;
    j = h / 16 - 4;
    while (j--) {
        sp->unk33C.x = x + 16;
        i = w / 16 - 2;
        while (i--) {
            func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, TILE);
            sp->unk33C.x += 16;
        }
        sp->unk33C.y += 16;
    }
    r.x = x;
    r.y = y;
    r.w = w;
    r.h = h;
    func_80028220(&r, &wash, 1);
    func_800286a8(2, 0, 0);
}
