/*
 * FDAT_152.BIN: its first four handlers and the scene word after its code
 * (WIP module; fdat_801ed4a0 after them is not yet exact).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/vis.h>
#include <st/data.h>
#include <st/light.h>
#include <st/menu.h>
#include <st/fdat.h>

/* The scene state: 0 pending, 1 armed, 2 done (flag 15). */
DATA(0x801edb84, 0x4, ".bss")
int fdat_801edb84;

void func_801eda8c(void);

/* Sets up the area by flag 15: table B entries 5..9 shown and item 11 in
 * place, or all hidden and item 11 already moved. */
ADDRESS(0x801ed4a0, 0x90)
void func_801ed4a0(void)
{
    Vis88 *d = &vis_table_b.entries[5];
    Vis48 *item = &vis_table_c.entries[11];

    if (!DAT_801cba08.unk1440.bit.f15) {
        item->unk22 = 4;
        d[0].unk00 = d[1].unk00 = d[2].unk00 = d[3].unk00 = d[4].unk00 = 2;
        fdat_801edb84 = 0;
    } else {
        item->unk22 = 5;
        d[0].unk00 = d[1].unk00 = d[2].unk00 = d[3].unk00 = d[4].unk00 = 0;
        fdat_801edb84 = 2;
        item->unk08.vx += 0x1e0;
    }
}

/* Plays the armed scene: moves item 11, hides table B entries 5..9, sets
 * the area lighting unless flag 16 is set, and sets flag 15. */
ADDRESS(0x801ed530, 0xb0)
void func_801ed530(void)
{
    Vis88 *d = &vis_table_b.entries[5];

    if (fdat_801edb84 == 1) {
        func_801eda8c();
        d[0].unk00 = d[1].unk00 = d[2].unk00 = d[3].unk00 = d[4].unk00 = 0;
        if (!DAT_801cba08.unk1440.bit.f16) {
            lighting.area_mode[11] = 0x24;
            lighting.area_mode[27] = 0x24;
            lighting.area_mode[12] = 0x24;
            lighting.area_mode[28] = 0x24;
            lighting.area_mode[15] = 0x24;
            lighting.area_mode[33] = 0x24;
        }
        fdat_801edb84 = 2;
        DAT_801cba08.unk1440.bit.f15 = 1;
    }
}

/* An empty handler between the others. */
ADDRESS(0x801ed5e0, 0x8)
void func_801ed5e0(void)
{
}

/* Item 0x130 used in front of table B entry 4 sets fdat_801edb84; the
 * result is 1 then, else 2. */
ADDRESS(0x801ed5e8, 0xc8)
void func_801ed5e8(short id)
{
    VECTOR p;
    VisRequest req;

    p.vx = viewer.position.vx;
    p.vy = viewer.position.vy + 1600;
    p.vz = viewer.position.vz;
    req.unk0 = 0;
    req.unk4 = p;
    req.unk1C = 1056;
    req.unk1E = 1600;
    req.unk14.vx = 0;
    req.unk14.vy = 0;
    req.unk14.vz = 0;
    req.unk22 = 0;
    req.unk20 = 0;
    if (id == 0x130) {
        if (func_8004f53c((VisLists *)&req) == 4) {
            fdat_801edb84 = 1;
            DAT_801cd0e0 = 1;
        } else {
            DAT_801cd0e0 = 2;
        }
    }
}
