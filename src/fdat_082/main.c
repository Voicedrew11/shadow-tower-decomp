/*
 * FDAT_082.BIN as one translation unit (WIP module): its handler table,
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

const FdatHeader fdat_header = {
    0xd,
    {
        map_hook_none, map_update_events, map_hook_none, map_hook_none,
        map_hook_none, map_restore_progress, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        creature_pick_move, creature_step_script, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
    },
    0x40,
};

#include "../lib/fdat_common_a.inc"
#include "../lib/fdat_common.inc"
#include "../lib/fdat_common_b.inc"

/* The scene state: 0 pending, 0xff done. */
DATA(0x801ed87c, 0x4, ".bss")
int fdat_801ed87c;

/* Sets up the area: the scene is pending unless table A entry 48 is dying
 * or record 0's stream counter has started. */
ADDRESS(0x801ed4a0, 0x58)
void map_restore_progress(void)
{
    Creature *e = &vis_table_a.entries[48];
    CreatureType *r = &vis_table_a.types[0];

    if (e->state != 3 && game_progress.unk1448[r->talk_id] == 0)
        fdat_801ed87c = 0;
    else
        fdat_801ed87c = 0xff;
}

/* While pending: ends the scene once entry 48 is beaten, and turns it into
 * item 0x116 once record 0's stream counter reaches 1. */
ADDRESS(0x801ed4f8, 0x90)
void map_update_events(void)
{
    Creature *e = &vis_table_a.entries[48];
    CreatureType *r = &vis_table_a.types[0];

    if (fdat_801ed87c == 0) {
        if (e->hp == 0)
            fdat_801ed87c = 0xff;
        if (game_progress.unk1448[r->talk_id] == 1) {
            creature_drop_item(e, 0x116);
            creature_fade_out_despawn(e);
            fdat_801ed87c = 0xff;
        }
    }
}

#include "../lib/fdat_creature_pick_move.inc"
#include "../lib/fdat_creature_step_script.inc"
