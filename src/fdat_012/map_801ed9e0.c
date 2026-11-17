/*
 * FDAT_012.BIN: handlers in the middle of its run (WIP module; a unit of its
 * own until its neighbours in the run are exact).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/vis.h>
#include <st/fdat.h>

/* On event 0x10e, places table A entry 48 again unless it is in state 3. */
ADDRESS(0x801ed9e0, 0x58)
void func_801ed9e0(short id)
{
    Vis212 *e = &vis_table_a.entries[48];

    if (id == 0x10e && e->unk1C.bytes[0] != 0 && e->unk05 != 3)
        func_8004ae38(e);
}
