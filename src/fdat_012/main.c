/*
 * FDAT_012.BIN as one translation unit (WIP module): its handler table,
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

void map_remove_fallen_creature();
void map_restore_progress();
void map_step_creature();
void map_use_item_remove_fallen();

const FdatHeader fdat_header = {
    0x6,
    {
        map_hook_none, map_remove_fallen_creature, map_hook_none, map_hook_none,
        map_hook_none, map_restore_progress, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_use_item_remove_fallen, map_hook_none, map_hook_none, map_hook_none,
        creature_pick_move, map_step_creature, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
    },
    0x39,
};

#include "../lib/fdat_common_a.inc"
#include "../lib/fdat_common.inc"
#include "../lib/fdat_common_b.inc"

/* The scene state: 0 idle, 1 armed, 2 played, 3 done (flag 0 set). */
DATA(0x801edfac, 0x4, ".bss")
int fdat_801edfac;

/* Removes table A entry 48 (creature_despawn) when it is in use but not active. */
ADDRESS(0x801ed4a0, 0x48)
void map_remove_fallen_creature(void)
{
    Creature *e = &vis_table_a.entries[48];

    if (e->status.bytes[0] != 0 && e->state == CREATURE_STATE_WAITING)
        creature_despawn(e);
}

/* Sets up the area: the scene state from flag 0, entry 48's range bits. */
ADDRESS(0x801ed4e8, 0x40)
void map_restore_progress(void)
{
    Creature *e = &vis_table_a.entries[48];

    if (game_progress.event_flags.bit.f0)
        fdat_801edfac = 3;
    else
        fdat_801edfac = 0;
    e->status.bytes[3] = 2;
}

#include "../lib/fdat_creature_pick_move.inc"
