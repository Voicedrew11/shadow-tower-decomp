/*
 * FDAT_022.BIN's handler fdat_801ed6e4, which 19 overlays share, and its
 * last handler (WIP module).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <psyq/libc.h>
#include <st/vis.h>
#include <st/data.h>
#include <st/anim.h>
#include <st/hud.h>
#include <st/menu.h>
#include <st/fdat.h>

#include "../lib/fdat_801ed6e4.inc"

/*
 * Not exact (registers): retail keeps item 6's pointer in s2 (then reused
 * for 0xfff), this source gets a1 (3 retries spent).
 *
 * Entry 48 plays animation 6 and turns its arm up a frame at a time; table
 * C item 5 (6 once 5 is taken) rides on vertex 0x15a until the arm is up.
 * Then the view settles, the entry leaves, the item drops and flag 25 is
 * set.
 */
ADDRESS(0x801ed8f0, 0x1e0)
void func_801ed8f0(void)
{
    Vis212 *e = &vis_table_a.entries[48];
    Vis48 *c5;
    Vis48 *c6;
    Vis48 *c;
    SVECTOR v;
    SVECTOR unused;

    fdat_801eb7b4(e, 6);
    c5 = &vis_table_c.entries[5];
    c6 = &vis_table_c.entries[6];
    c = c5;
    if (vis_table_c.unk0070[c5->unk00 / 32] & (1 << (c5->unk00 % 32)))
        c = c6;
    do {
        e->unk2A += e->unkB0;
        if ((s16)e->unk2A >= 0x1000)
            e->unk2A = 0xfff;
        anim_get_vertex(e->unk0B + 128, e->unk08, (s16)e->unk2A, 0x15a, &v);
        c->unk08.vx = e->unk74.vx - v.vz;
        c->unk08.vy = e->unk74.vy + v.vy;
        c->unk08.vz = e->unk74.vz + v.vx;
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
    fdat_801ecf44(e);
    func_8005335c(c, 1);
    DAT_801cba08.unk1440.word |= 0x2000000;
}
