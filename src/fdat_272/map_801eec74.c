/*
 * FDAT_272.BIN: entry 49's turn (WIP module; a unit of its own until its neighbours in
 * the run are exact).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <psyq/libc.h>
#include <st/vis.h>
#include <st/data.h>
#include <st/anim.h>
#include <st/hud.h>
#include <st/menu.h>
#include <st/light.h>
#include <st/xa.h>
#include <st/fdat.h>

/* Not exact (registers): retail takes the two to-products in a3 and v1,
 * this source in v0, and loads unk280 before unk288. Here global-alloc
 * puts both products in LO, reload spills LO for the from-product's output
 * reload and the retry gives them v0; operand order, a `total - n` local
 * and x/y locals were ruled out. */
/* Entry 49 plays animation 0 to its end while the view turns from its
 * pitch and yaw to (0, -0x400). */
ADDRESS(0x801eec74, 0x240)
void func_801eec74(void)
{
    SVECTOR from;
    SVECTOR to;
    Vis212 *e;
    int n;
    int total;

    from = viewer.unk278;
    e = &vis_table_a.entries[49];
    to.vy = -0x400;
    to.vx = 0;
    if (from.vx > 2048)
        from.vx -= 4096;
    if (from.vy > 2048)
        from.vy -= 4096;
    fdat_801eb7b4(e, 0);
    n = 0x1000 / e->unkB0;
    total = n;
    do {
        fdat_801eba4c(e);
        e->unk2A += e->unkB0;
        if ((short)e->unk2A >= 0x1000)
            e->unk2A = 0xfff;
        viewer.unk278.vx = (from.vx * n + to.vx * (total - n)) / total;
        viewer.unk268 = viewer.unk278.vx + viewer.unk280.halves[0] + viewer.unk288 + viewer.unk290;
        viewer.unk278.vy = (from.vy * n + to.vy * (total - n)) / total;
        viewer.unk26A = viewer.unk278.vy + viewer.unk280.halves[1] + viewer.unk28A + viewer.unk292;
        func_80026dd4(NULL, (SVECTOR *)&viewer.unk268, 0);
        n--;
    } while ((short)e->unk2A != 0xfff);
    e->unk70 |= 0x2000000;
}
