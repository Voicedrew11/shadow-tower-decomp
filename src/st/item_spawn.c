/*
 * ST.EXE: spawning the table B object an entry carries (WIP module,
 * 0x800549c8..0x80054a54; func_80054a54 after it, a 48-case item-use
 * switch, is not claimed).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/vis.h>
#include <st/funcs.h>
#include <st/item.h>

/*
 * Not exact: register choice only; retail loads the table base into $v1
 * where this source reuses $a1.
 */
/* Spawns table B kind 150 for entry e; e->unk49 remembers it as 106 + its
 * index. */
ADDRESS(0x800549c8, 0x8c)
void func_800549c8(Vis212 *e, int id)
{
    Vis88 *b = func_8004f994(150, 10, -1);

    e->unk49 = b - vis_table_b.entries + 106;
    b->unk0a = id;
    b->unk48 = 0;
}
