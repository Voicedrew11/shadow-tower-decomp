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

/* When item 0x10e is used, removes table A entry 48 (creature_despawn)
 * unless it is in state 3. */
ADDRESS(0x801ed9e0, 0x58)
void map_use_item_remove_fallen(short id)
{
    Creature *e = &vis_table_a.entries[48];

    if (id == 0x10e && e->status.bytes[0] != 0 && e->state != 3)
        creature_despawn(e);
}
