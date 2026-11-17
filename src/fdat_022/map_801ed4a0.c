/*
 * FDAT_022.BIN: the first of its own handlers and the scene word after its
 * code (WIP module; a unit of its own until its neighbours in the run are
 * exact).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/vis.h>
#include <st/menu.h>
#include <st/fdat.h>

/* The scene state: 0 pending, 1 ready, 0xff done (flag 25). */
DATA(0x801edad0, 0x4, ".bss")
int fdat_801edad0;

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
