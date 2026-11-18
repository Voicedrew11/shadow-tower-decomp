/*
 * FDAT_242.BIN: an event handler and the scene word it steps (WIP module), then
 * the shared handler fdat_801ed4a0, whose switch table is the unit's
 * read-only claim.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/vis.h>
#include <st/menu.h>
#include <st/fdat.h>

RODATA(0x801eb740, 0x7c)

/* The gate scene state: 2 armed, 3 opened by event 0x10e. */
DATA(0x801ee0b8, 0x4, ".bss")
int fdat_801ee0b8;

/* Event 0x10e opens the armed gate: table A entry 50 dies, 49 wakes, and
 * record stream counter 13 advances. */
ADDRESS(0x801edaa4, 0x6c)
void func_801edaa4(short id)
{
    Vis212 *e = &vis_table_a.entries[50];

    if (id == 0x10e && fdat_801ee0b8 == 2) {
        fdat_801ee0b8 = 3;
        e->unk05 = 3;
        vis_table_a.entries[49].unk00 = 100;
        e->unk70 &= ~0x2000000;
        DAT_801cba08.unk1448[13]++;
    }
}

#include "../lib/fdat_801ed4a0.inc"
