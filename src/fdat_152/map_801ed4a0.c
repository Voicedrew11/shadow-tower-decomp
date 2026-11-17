/*
 * FDAT_152.BIN: its first two handlers and the scene word after its code
 * (WIP module; a unit of its own until its neighbours in the run are exact).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/vis.h>
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
