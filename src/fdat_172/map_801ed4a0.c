/*
 * FDAT_172.BIN: its first two handlers and the scene word fdat_801edfd8 (WIP module; a unit of its own until its neighbours in
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

/* 0 until table A entry 33 is beaten, 1 then (flag 27), 0xff once it left. */
DATA(0x801edfd8, 0x4, ".bss")
int fdat_801edfd8;

void func_801ede00(void);
void func_801edef8(void);

/* Not exact (one attempt, retries off; 97.6%): first divergence retail `lui v1,0x0`, this source `lui v0,0x0`. */
/* Sets up the area by flags 14, 26, 27, 29 and 30: table C item 28,
 * entry 48 (asleep, awake or gone), table B entry 6 and record 14. */
ADDRESS(0x801ed4a0, 0x174)
void func_801ed4a0(void)
{
    Vis212 *e48 = &vis_table_a.entries[48];
    Vis192 *r1 = &vis_table_a.unk3500[1];
    Vis48 *c28 = &vis_table_c.entries[28];
    Vis88 *d6 = &vis_table_b.entries[6];
    Vis192 *r14 = &vis_table_a.unk3500[14];

    if (DAT_801cba08.unk1440.bit.f26 == 1) {
        c28->unk22 = 5;
        fdat_801edfd0 = 0xff;
        c28->unk08.vy += 0x200;
    } else {
        c28->unk22 = 4;
        fdat_801edfd0 = 0;
    }
    if (e48->unk05 != 3 && DAT_801cba08.unk1440.bit.f29 != 1 && DAT_801cba08.unk1440.bit.f14) {
        if (DAT_801cba08.unk1440.bit.f30 == 1) {
            fdat_801edfd4 = 0xfe;
            e48->unk70 &= ~0x2000000;
        } else {
            fdat_801edfd4 = 0;
            e48->unk1C.bytes[3] = 3;
            r1->unk60[12]->unk00 = 0x72;
            r1->unk60[13]->unk00 = 0x80;
        }
    } else {
        e48->unk00 = 0;
        fdat_801edfd4 = 0xff;
    }
    if (!DAT_801cba08.unk1440.bit.f27) {
        fdat_801edfd8 = 0;
        d6->unk05 = 0xff;
    } else {
        fdat_801edfd8 = 0xff;
    }
    if (DAT_801cba08.unk1448[r14->unk09] != 0)
        r14->unk1C &= ~0x180;
}

/*
 * Runs the area: item 0x12a's scene (func_801edd38) lifts table C item 28;
 * entry 48 wakes when hit or once record 1's stream counter is 2, and
 * changes its scripts once its flash is over; entry 33 is sent by the
 * viewer passing z 0x4a000 and, once beaten, changes the lighting and
 * leaves (flag 27).
 */
ADDRESS(0x801ed614, 0x328)
void func_801ed614(void)
{
    Vis212 *e48 = &vis_table_a.entries[48];
    Vis88 *d6 = &vis_table_b.entries[6];
    Vis192 *r1 = &vis_table_a.unk3500[1];
    Vis212 *e33 = &vis_table_a.entries[33];
    Vis192 *r14 = &vis_table_a.unk3500[14];

    if (fdat_801edfd0 == 1) {
        func_801ede00();
        func_801edef8();
        fdat_801edfd0 = 0xff;
        DAT_801cba08.unk1440.word |= 0x4000000;
    }
    if (fdat_801edfd4 == 0) {
        if (DAT_801cba08.unk1448[r1->unk09] == 2) {
            fdat_801ecf44(e48);
            fdat_801edfd4 = 0xff;
        }
        if (e48->unk64 == 1) {
            fdat_801edfd4 = 0xfd;
            e48->unk1C.bytes[2] &= 0xea;
            e48->unk70 &= ~0x2000000;
            DAT_801cba08.unk1440.word |= 0x40000000;
        }
    }
    if (fdat_801edfd4 == 0xfd && e48->unk34 == -1) {
        DAT_801cba08.unk1448[r1->unk09] = 2;
        r1->unk60[12]->unk00 = 0x80;
        r1->unk60[13]->unk00 = 0x72;
        e48->unk70 |= 0x2000000;
        fdat_801ed388(e48, 0, 0);
        fdat_801edfd4 = 0xfe;
        e48->unk1C.bytes[3] = 0;
        e48->unk70 &= ~0x2000000;
    }
    if (e48->unk2C == 0)
        DAT_801cba08.unk1440.word |= 0x20000000;
    if (DAT_801cba08.unk1448[r14->unk09] == 0 && viewer.position.vz > 0x4a000) {
        viewer.unk218 = 5;
        fdat_801ed388(e33, 0, 0);
        e33->unk64 = 1;
        viewer.unk218 = 0;
        e33->unk70 &= ~0x2000000;
        r14->unk1C &= ~0x180;
    }
    if (fdat_801edfd8 == 0 && e33->unk2C == 0) {
        fdat_801edfd8 = 1;
        lighting.area_mode[2] = 0x11;
        lighting.area_mode[17] = 0x11;
        DAT_801cba08.unk1440.word |= 0x8000000;
    }
    if (fdat_801edfd8 == 1 && e33->unk05 == 3) {
        viewer.unk218 = 5;
        e33->unk70 |= 0x2000000;
        fdat_801ed388(e33, 1, 0);
        fdat_801edfd8 = 0xff;
        d6->unk05 = 0xed;
        viewer.unk218 = 0;
    }
}
