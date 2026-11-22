/*
 * FDAT_122.BIN as one translation unit (WIP module): its handler table,
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

void map_restore_progress();
void map_update_events();
void map_step_creature();

const FdatHeader fdat_header = {
    0x11,
    {
        map_hook_none, map_update_events, map_hook_none, map_hook_none,
        map_hook_none, map_restore_progress, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        creature_pick_move, map_step_creature, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
    },
    0x44,
};

#include "../lib/fdat_common_a.inc"
#include "../lib/fdat_common.inc"
#include "../lib/fdat_common_b.inc"

/* The scene state: 1 armed by flag 47, 2 played. */
DATA(0x801ed938, 0x4, ".bss")
int fdat_801ed938;

void map_spawn_creature_group(void);

/* Sets up the area: clears table A entries 32..34 and arms the scene. */
ADDRESS(0x801ed4a0, 0x44)
void map_restore_progress(void)
{
    vis_table_a.entries[32].spawn_chance = vis_table_a.entries[33].spawn_chance = vis_table_a.entries[34].spawn_chance = 0;
    if (game_progress.event_flags2.bit.f15 == 1)
        fdat_801ed938 = 1;
    else
        fdat_801ed938 = 0;
}

/* Plays the scene once the viewer stands on record 0x35. */
ADDRESS(0x801ed4e4, 0x54)
void map_update_events(void)
{
    if (fdat_801ed938 == 1 && viewer.block == 0x35) {
        map_spawn_creature_group();
        fdat_801ed938 = 2;
    }
}

#include "../lib/fdat_creature_pick_move.inc"
