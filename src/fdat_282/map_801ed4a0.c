/*
 * FDAT_282.BIN: the first two of its own handlers (WIP module; a unit of its
 * own because fdat_801ed4a0 after it is not yet exact).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/vis.h>
#include <st/menu.h>
#include <st/fdat.h>

void func_801eda20(void);

/* Opens door 26 of table B unless it is already open. */
ADDRESS(0x801ed4a0, 0x48)
void func_801ed4a0(void)
{
    Vis88 *door = &vis_table_b.entries[26];

    if (door->unk0E != 0 && door->unk0C != 0xff)
        func_801eda20();
}

/* Sets up the area: table C item 2 and table A entries 48..50. */
ADDRESS(0x801ed4e8, 0x78)
void func_801ed4e8(void)
{
    Vis212 *a48 = &vis_table_a.entries[48];
    Vis212 *a50 = &vis_table_a.entries[50];
    Vis48 *item;
    Vis192 *r;
    Vis212 *a49;

    r = &vis_table_a.unk3500[vis_table_a.entries[19].unk02];
    item = &vis_table_c.entries[2];
    a49 = &vis_table_a.entries[49];

    if (vis_table_b.entries[26].unk0C != 0xff)
        item->unk22 = 4;
    a49->unk1C.bytes[3] = 2;
    a50->unk1C.bytes[3] = 2;
    a48->unk1C.bytes[3] = 2;
    DAT_801cba08.unk1448[r->unk09] = 3;
}
