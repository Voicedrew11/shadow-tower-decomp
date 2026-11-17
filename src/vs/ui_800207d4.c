/*
 * VS.EXE: a picture drawn as a grid of semi-transparent tiles (WIP
 * module), VS's copy of ST.EXE's func_8002f1f8 (src/st/ui.c): VS
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
/* Not exact: as ST.EXE's copy, retail keeps pos and tex in $t0/$t1 with
 * caller-save spills and hoists v and v + 10 out of the inner loop; this
 * build keeps pos in $s8 (0x10 bytes shorter). */
ADDRESS(0x800207d4, 0x2d0)
void func_800207d4(DVECTOR *pos, DVECTOR *tex, DVECTOR *clut, CVECTOR *color, int arg4)
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
            buf = DAT_801624d0.packet;
            p = (POLY_FT4 *)buf->cur;
            buf->cur = (u_long *)(p + 1);
            if (DAT_801624d0.packet->cur > DAT_801624d0.packet->end)
                return;
            DAT_801624d0.unk8328++;
            setUV4(p, u, v, u + 10, v, u, v + 10, u + 10, v + 10);
            p->x0 = p->x2 = pos->vx + x;
            p->x1 = p->x3 = pos->vx + x + 10;
            p->y0 = p->y1 = pos->vy + y;
            p->y2 = p->y3 = pos->vy + y + 10;
            c = *color;
            c.r -= 12;
            c.g -= 6;
            c.b -= 6;
            setRGB0(p, c.r, c.g, c.b);
            p->tpage = GetTPage(0, color->cd & 3, tex->vx, tex->vy);
            p->clut = GetClut(clut->vx, clut->vy);
            setPolyFT4(p);
            setSemiTrans(p, 1);
            addPrim(DAT_801624d0.ot_cur, p);
        }
    }
}
