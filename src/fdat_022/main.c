/*
 * FDAT_022.BIN as one translation unit (WIP module): its handler table,
 * the 21 functions every overlay starts with (src/lib/fdat_common_a.inc,
 * fdat_common.inc, fdat_common_b.inc), then its map handlers through the
 * shared fdat_801ed4a0. One object, because fdat_801ed4a0's switch table
 * is 8-aligned after fdat_801ebec4's in the same read-only run.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <psyq/libc.h>
#include <st/cd.h>
#include <st/gfx.h>
#include <st/anim.h>
#include <st/sound.h>
#include <st/data.h>
#include <st/hud.h>
#include <st/xa.h>
#include <st/vis.h>
#include <st/fdat.h>
#include <st/menu.h>

RODATA(0x801eb560, 0x24c)

void func_801ed4a0();
void func_801ed514();

const FdatHeader fdat_header = {
    0x7,
    {
        fdat_801ed498, func_801ed514, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, func_801ed4a0, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed4a0, fdat_801ed6e4, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
    },
    0x3a,
};

#include "../lib/fdat_common_a.inc"
#include "../lib/fdat_common.inc"
#include "../lib/fdat_common_b.inc"

/* The scene state: 0 pending, 1 ready, 0xff done (flag 25). */
DATA(0x801edad0, 0x4, ".bss")
int fdat_801edad0;

void func_801ed8f0(void);

/* Sets up the area: done when flag 25 is set, ready once record 6's stream
 * counter reached 2, else table A entries 48 and 49 set up for the scene. */
ADDRESS(0x801ed4a0, 0x74)
void func_801ed4a0(void)
{
    Vis212 *e48 = &vis_table_a.entries[48];
    Vis212 *e49 = &vis_table_a.entries[49];
    Vis192 *r = &vis_table_a.unk3500[6];

    if (DAT_801cba08.unk1440.bit.f25 == 1) {
        fdat_801edad0 = 0xff;
    } else if (DAT_801cba08.unk1448[r->unk09] == 2) {
        fdat_801edad0 = 1;
    } else {
        e49->unk1C.bytes[3] = 1;
        e48->unk00 = 0;
        fdat_801edad0 = 0;
    }
}

/* Once the viewer stands on record 0x27, entry 49 leaves and entry 48 is
 * marked; when record 2's stream counter reaches 3 the scene plays. */
ADDRESS(0x801ed514, 0xe8)
void func_801ed514(void)
{
    Vis212 *e48 = &vis_table_a.entries[48];
    Vis192 *r = &vis_table_a.unk3500[2];
    Vis212 *e49 = &vis_table_a.entries[49];

    if (fdat_801edad0 == 0 && viewer.unk178 == 0x27) {
        fdat_801ed388(e49, 0, 0);
        fdat_801ed388(e49, 0, 0);
        fdat_801ecf44(e49);
        e48->unk00 = 100;
        fdat_801edad0 = 1;
    }
    if (fdat_801edad0 == 1 && DAT_801cba08.unk1448[r->unk09] == 3) {
        viewer.unk218 = 5;
        func_801ed8f0();
        fdat_801edad0 = 0xff;
        viewer.unk218 = 0;
    }
}

#include "../lib/fdat_801ed4a0.inc"
#include "../lib/fdat_801ed6e4.inc"

/*
 * Entry 48 plays animation 6 and turns its arm up a frame at a time; table
 * C item 5 (6 once 5 is taken) rides on vertex 0x15a until the arm is up.
 * Then the view settles, the entry leaves, the item drops and flag 25 is
 * set.
 */
ADDRESS(0x801ed8f0, 0x1e0)
void func_801ed8f0(void)
{
    Vis212 *e = &vis_table_a.entries[48];
    Vis48 *c5 = &vis_table_c.entries[5];
    Vis48 *c6 = &vis_table_c.entries[6];
    Vis48 *c;
    SVECTOR v;
    SVECTOR unused;

    fdat_801eb7b4(e, 6);
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
