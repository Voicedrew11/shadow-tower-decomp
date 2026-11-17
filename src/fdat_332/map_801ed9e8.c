/*
 * FDAT_332.BIN: its last handler (WIP module; a unit of its own until its neighbours in
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

/* Not exact (one attempt, retries off; 77.1%): first divergence retail `addiu sp,sp,-40`, this source `addiu sp,sp,-32`. */
/* Entry 33's end: it faces the viewer, finishes its animation, leaves its
 * drops (20/30/100 percent), dies (animation 12) and table B entry 3
 * opens; the area's history is reset (scene word 2). */
ADDRESS(0x801ed9e8, 0x154)
void func_801ed9e8(void)
{
    Vis212 *e = &vis_table_a.entries[33];
    Vis88 *d3 = &vis_table_b.entries[3];

    viewer.unk218 = 5;
    fdat_801ed1e4(e);
    for (;;) {
        e->unk2A += e->unkB0;
        if ((short)e->unk2A >= 0x1000)
            break;
        func_80026dd4(NULL, NULL, 0);
    }
    fdat_801ed388(e, 0, 0);
    fdat_801ed388(e, 0, 0);
    vis_table_a.unk6158[3] = 0xff;
    vis_table_a.unk6158[2] = 0xff;
    vis_table_a.unk6158[1] = 0xff;
    vis_table_a.unk6158[0] = 0xff;
    e->unk65 = 20;
    e->unk66 = 30;
    e->unk67 = 100;
    func_8004da2c(e);
    func_8004db0c(e);
    fdat_801eb7b4(e, 12);
    e->unk05 = 5;
    e->unk34 = 0xfff;
    e->unk1A = 0xf0;
    e->unk2A = 0;
    e->unk2C = 0;
    d3->unk05 = 0xed;
    viewer.unk218 = 0;
    fdat_801edb3c = 2;
    vis_table_a.unk612C = 0;
    vis_table_a.unk6128 = 10;
    e->unk1C.bytes[2] |= 0x15;
}
