/*
 * FDAT_142.BIN: the first of its own handlers and the four scene words after
 * its code (WIP module; a unit of its own until its neighbours in the run
 * are exact).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/vis.h>
#include <st/menu.h>
#include <st/fdat.h>

DATA(0x801edaf4, 0x4, ".bss")
static int fdat_801edaf4;

DATA(0x801edaf8, 0x4, ".bss")
static int fdat_801edaf8;

/* 0xff once record 0's stream counter has started. */
DATA(0x801edafc, 0x4, ".bss")
static int fdat_801edafc;

/* 0xff once record 4's stream counter reached 2. */
DATA(0x801edb00, 0x4, ".bss")
static int fdat_801edb00;

/* Sets up the area: the scene words by records 0 and 4's stream counters;
 * until record 4's reaches 2, table A entries 5..7 are cleared and entry
 * 49's range bits set. */
ADDRESS(0x801ed4a0, 0xac)
void func_801ed4a0(void)
{
    Vis192 *r0 = &vis_table_a.unk3500[0];
    Vis212 *e49 = &vis_table_a.entries[49];
    Vis192 *r4 = &vis_table_a.unk3500[4];
    Vis212 *e5 = &vis_table_a.entries[5];
    Vis212 *e6 = &vis_table_a.entries[6];
    Vis212 *e7 = &vis_table_a.entries[7];

    fdat_801edaf4 = 0;
    fdat_801edaf8 = 0;
    if (DAT_801cba08.unk1448[r0->unk09] != 0)
        fdat_801edafc = 0xff;
    else
        fdat_801edafc = 0;
    if (DAT_801cba08.unk1448[r4->unk09] == 2) {
        fdat_801edb00 = 0xff;
    } else {
        e7->unk00 = 0;
        e6->unk00 = 0;
        e5->unk00 = 0;
        e49->unk1C.bytes[3] = 2;
        fdat_801edb00 = 0;
    }
}
