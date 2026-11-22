/*
 * ST.EXE: spawning the table B object an entry carries (WIP module,
 * 0x800549c8..0x80054a54; item_use after it, a 48-case item-use
 * switch, is not claimed).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/vis.h>
#include <st/funcs.h>
#include <st/item.h>

/* Spawns table B kind 150 for entry e and returns it; e->spawned_object remembers it
 * as 106 + its index. */
ADDRESS(0x800549c8, 0x8c)
AreaObject *creature_alloc_object(Creature *e, int id)
{
    AreaObject *b = object_alloc(150, 10, -1);

    e->unk48.b.spawned_object = b - vis_table_b.entries + 106;
    b->type = id;
    b->unk48 = 0;
    return b;
}
