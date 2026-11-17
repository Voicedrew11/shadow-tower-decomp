/*
 * FDAT_272.BIN: table C item 10's ride (WIP module; a unit of its own until its neighbours in
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

/* Not exact (one attempt, retries off; 98.4%): first divergence retail `addiu s2,s1,48`, this source `addiu a1,s1,48`. */
/*
 * Entry 49 plays animation 4 and lifts its arm a frame at a time; table C
 * item 10 (11 once 10 is taken) rides on vertex 0x160 until the arm is up.
 * Then the view settles and the item drops.
 */
ADDRESS(0x801eeeb4, 0x1c8)
void func_801eeeb4(void)
{
    Vis212 *e = &vis_table_a.entries[49];
    Vis48 *c10;
    Vis48 *c11;
    Vis48 *c;
    SVECTOR v;
    SVECTOR unused;

    fdat_801eb7b4(e, 4);
    c10 = &vis_table_c.entries[10];
    c11 = &vis_table_c.entries[11];
    c = c10;
    if (vis_table_c.unk0070[c10->unk00 / 32] & (1 << (c10->unk00 % 32)))
        c = c11;
    do {
        fdat_801eba4c(e);
        e->unk2A += e->unkB0;
        if ((s16)e->unk2A >= 0x1000)
            e->unk2A = 0xfff;
        anim_get_vertex(e->unk0B + 128, e->unk08, (s16)e->unk2A, 0x160, &v);
        c->unk08.vx = e->unk74.vx - v.vx;
        c->unk08.vy = e->unk74.vy + v.vy;
        c->unk08.vz = e->unk74.vz - v.vz;
        func_80026dd4(NULL, &viewer.unk278, 0);
    } while ((s16)e->unk2A != 0xfff);
    c->unk22 = 2;
    viewer.unk268 = viewer.unk278.vx + viewer.unk280.halves[0] + viewer.unk288 + viewer.unk290;
    viewer.unk26A = viewer.unk278.vy + viewer.unk280.halves[1] + viewer.unk28A + viewer.unk292;
    viewer.unk26C = viewer.unk278.vz + viewer.unk284 + viewer.unk28C + viewer.unk294;
    viewer.unk270.vx = 4096 - viewer.unk268;
    viewer.unk270.vy = 4096 - viewer.unk26A;
    viewer.unk270.vz = 4096 - viewer.unk26C;
    RotMatrixZYX(&viewer.unk270, &viewer.unk298);
    func_8005335c(c, 1);
}
