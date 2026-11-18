/*
 * FDAT_112.BIN: the first of its own handlers (WIP module), then the shared
 * handler fdat_801ed4a0, whose switch table is the unit's read-only claim.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/vis.h>
#include <st/data.h>
#include <st/menu.h>
#include <st/fdat.h>

RODATA(0x801eb730, 0x7c)

/* Sets up the area: table C item 7, and table A entry 48 by flags 14 and 7
 * and the count of item 303. */
ADDRESS(0x801ed4a0, 0x8c)
void func_801ed4a0(void)
{
    Vis212 *e;

    vis_table_c.entries[7].unk22 = 4;
    e = &vis_table_a.entries[48];
    if (DAT_801cba08.unk1440.bit.f14 == 1 || DAT_801c7590.counts[303].count == 1)
        e->unk05 = 3;
    if (DAT_801cba08.unk1440.bit.f7 == 1) {
        e->unk1C.bytes[3] = 0;
        e->unk70 &= ~0x2000000;
    } else {
        e->unk1C.bytes[3] = 2;
    }
}

#include "../lib/fdat_801ed4a0.inc"
