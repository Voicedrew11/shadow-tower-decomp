/*
 * VS.EXE: the menu screens' 2D primitives: filled and textured rectangles,
 * a draw-mode primitive and a Gouraud quad (WIP module). VS's copies of
 * ST.EXE's func_80028220, func_80028364, func_800286a8 and func_80028c2c
 * (src/st/ui.c); VS advances the packet cursor before its bounds check.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <vs/data.h>
#include <vs/menu.h>

/*
 * Takes the next primitive from the current packet buffer, or returns once
 * the buffer has overflowed (the cursor is advanced first). A statement
 * sequence, as ST.EXE's NEW_PRIM.
 */
#define NEW_PRIM(buf, p, type)                                       \
    (buf) = DAT_801624d0.packet;                                     \
    (p) = (type *)(buf)->cur;                                        \
    (buf)->cur = (u_long *)((p) + 1);                                \
    if (DAT_801624d0.packet->cur > DAT_801624d0.packet->end)         \
        return;                                                      \
    DAT_801624d0.unk8328++

#define FILL_F4(p, semi, x0, y0, x1, y1, x2, y2, x3, y3, color)      \
    (p)->code = (semi) == 1 ? 0x2a : 0x28;                           \
    (p)->tag = (*DAT_801624d0.ot_cur & 0xffffff) | 0x05000000;       \
    setXY4(p, x0, y0, x1, y1, x2, y2, x3, y3);                       \
    setRGB0(p, (color)->r, (color)->g, (color)->b);                  \
    *DAT_801624d0.ot_cur = (u_long)(p) & 0xffffff

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
