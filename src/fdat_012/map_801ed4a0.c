/*
 * FDAT_012.BIN: the first of its own handlers (WIP module; a unit of its own
 * until its neighbours in the run are exact).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/vis.h>
#include <st/fdat.h>

/* Places table A entry 48 again when it is in use but not active. */
ADDRESS(0x801ed4a0, 0x48)
void func_801ed4a0(void)
{
    Vis212 *e = &vis_table_a.entries[48];

    if (e->unk1C.bytes[0] != 0 && e->unk05 == 0)
        func_8004ae38(e);
}
