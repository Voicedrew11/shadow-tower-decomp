/*
 * FDAT_202.BIN: the first of its own handlers and the scene word after its
 * code (WIP module; a unit of its own until its neighbours in the run are
 * exact).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/vis.h>
#include <st/light.h>
#include <st/menu.h>
#include <st/fdat.h>

/* 0xff once door 33 of table B is open. */
DATA(0x801eda08, 0x4, ".bss")
int fdat_801eda08;

/* Sets up the area by door 33: once it is open, retires every table B entry
 * of records 0x29, 0x2b and 0x94; the lighting of area 20 unless flag 18 is
 * set. */
ADDRESS(0x801ed4a0, 0xc0)
void func_801ed4a0(void)
{
    Vis88 *door = &vis_table_b.entries[33];
    Vis88 *b;
    int i;

    if (door->unk0C == 0xff) {
        b = vis_table_b.entries;
        i = 240;
        while (i--) {
            if (b->unk0a == 0x29 || b->unk0a == 0x2b || b->unk0a == 0x94)
                b->unk05 = 0xff;
            b++;
        }
        fdat_801eda08 = 0xff;
        if (!DAT_801cba08.unk1440.bit.f18)
            lighting.area_mode[20] = 0x41;
    } else {
        fdat_801eda08 = 0;
        if (!DAT_801cba08.unk1440.bit.f18)
            lighting.area_mode[20] = 0x22;
    }
}
