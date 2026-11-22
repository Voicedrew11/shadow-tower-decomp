/*
 * VS.EXE: a picture drawn as a grid of semi-transparent tiles (WIP
 * module), VS's copy of ST.EXE's ui_draw_cell_grid (src/st/render.c): VS
 * advances the packet cursor before its bounds check and darkens the
 * colour.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <vs/data.h>

/*
 * Draws 25 x 5 tiles of 10 x 10 texels from `tex` at `pos`, 11 pixels
 * apart horizontally and 12 vertically, semi-transparent in `color`
 * darkened by (12, 6, 6).
 */
ADDRESS(0x800207d4, 0x2d0)
void ui_draw_cell_grid(DVECTOR *pos, DVECTOR *tex, DVECTOR *clut, CVECTOR *color, int arg4)
{
    int row, col;
    int cols, rows; /* tiles across and down */
    int w, h;       /* a tile on screen */
    int tw, th;     /* a tile in the texture */
    int dx, dy;     /* tile pitch on screen */
    int u0, v0;
    int u, v;
    StPacketBuffer *buf;
    POLY_FT4 *p;
    CVECTOR c;

    cols = 25;
    rows = 5;
    w = 10;
    h = 10;
    tw = 10;
    th = 10;
    dx = 11;
    dy = 12;
    u0 = (short)(tex->vx % 64) * 4;
    v0 = (short)(tex->vy % 256);
    for (row = 0; row < rows; row++) {
        for (col = 0; col < cols; col++) {
            u = u0 + col * tw;
            v = v0 + row * th;
            buf = vs_gfx_state.packet;
            p = (POLY_FT4 *)buf->cur;
            buf->cur = (u_long *)(p + 1);
            if (vs_gfx_state.packet->cur > vs_gfx_state.packet->end)
                return;
            vs_gfx_state.unk8328++;
            setUV4(p, u, v, u + tw, v, u, v + th, u + tw, v + th);
            p->x0 = p->x2 = pos->vx + col * dx;
            p->x1 = p->x3 = pos->vx + col * dx + w;
            p->y0 = p->y1 = pos->vy + row * dy;
            p->y2 = p->y3 = pos->vy + row * dy + h;
            c = *color;
            c.r -= 12;
            c.g -= 6;
            c.b -= 6;
            setRGB0(p, c.r, c.g, c.b);
            p->tpage = GetTPage(0, color->cd & 3, tex->vx, tex->vy);
            p->clut = GetClut(clut->vx, clut->vy);
            setPolyFT4(p);
            setSemiTrans(p, 1);
            addPrim(vs_gfx_state.ot_cur, p);
        }
    }
}
