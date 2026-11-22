/*
 * FDAT_282.BIN as one translation unit (WIP module): its handler table,
 * the 21 functions every overlay starts with (src/lib/fdat_common_a.inc,
 * fdat_common.inc, fdat_common_b.inc), then its map handlers through the
 * shared creature_pick_move. One object, because creature_pick_move's switch table
 * is 8-aligned after creature_run_script_command's in the same read-only run.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <psyq/libc.h>
#include <st/cd.h>
#include <st/gfx.h>
#include <st/anim.h>
#include <st/sound.h>
#include <st/data.h>
#include <st/hud.h>
#include <st/xa.h>
#include <st/vis.h>
#include <st/fdat.h>
#include <st/menu.h>

RODATA(0x801eb560, 0x24c)

void map_update_events();
void map_restore_progress();
void map_step_creature();
void map_object_hook_none();

const FdatHeader fdat_header = {
    0x21,
    {
        map_hook_none, map_update_events, map_hook_none, map_hook_none,
        map_hook_none, map_restore_progress, map_hook_none, map_hook_none,
        map_hook_none, map_object_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        creature_pick_move, map_step_creature, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
    },
    0x54,
};

#include "../lib/fdat_common_a.inc"
#include "../lib/fdat_common.inc"
#include "../lib/fdat_common_b.inc"

void map_open_object_drop_item(void);

/* Opens door 26 of table B unless it is already open. */
ADDRESS(0x801ed4a0, 0x48)
void map_update_events(void)
{
    AreaObject *door = &vis_table_b.entries[26];

    if (door->examined != 0 && door->state != 0xff)
        map_open_object_drop_item();
}

/* Sets up the area: table C item 2 and table A entries 48..50. */
ADDRESS(0x801ed4e8, 0x78)
void map_restore_progress(void)
{
    Creature *a48 = &vis_table_a.entries[48];
    Creature *a50 = &vis_table_a.entries[50];
    GroundItem *item;
    CreatureType *r;
    Creature *a49;

    r = &vis_table_a.types[vis_table_a.entries[19].type];
    item = &vis_table_c.entries[2];
    a49 = &vis_table_a.entries[49];

    if (vis_table_b.entries[26].state != 0xff)
        item->state = GROUND_ITEM_HIDDEN;
    a49->status.bytes[3] = 2;
    a50->status.bytes[3] = 2;
    a48->status.bytes[3] = 2;
    game_progress.talk_counts[r->talk_id] = 3;
}

#include "../lib/fdat_creature_pick_move.inc"
