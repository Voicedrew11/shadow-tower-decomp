/*
 * FDAT_182.BIN as one translation unit (WIP module): its handler table,
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
#include <st/light.h>
#include <st/menu.h>

RODATA(0x801eb560, 0x24c)

void map_reset_scene_state();
void map_update_events();
void map_use_item_request_warp(short id);
void map_talk_on_player_death();

const FdatHeader fdat_header = {
    0x17,
    {
        map_hook_none, map_update_events, map_hook_none, map_hook_none,
        map_hook_none, map_reset_scene_state, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_use_item_request_warp, map_talk_on_player_death, map_hook_none, map_hook_none,
        creature_pick_move, creature_step_script, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
    },
    0x4a,
};

#include "../lib/fdat_common_a.inc"
#include "../lib/fdat_common.inc"
#include "../lib/fdat_common_b.inc"

/* 1 or 2: the gate the event 0x131 opened (map_use_item_request_warp); 0 none. */
DATA(0x801edc50, 0x4, ".bss")
int fdat_801edc50;

void map_warp_player(int gate);

/* Clears the scene statics. */
ADDRESS(0x801ed4a0, 0x14)
void map_reset_scene_state(void)
{
    fdat_801edc50 = 0;
    fdat_801edc54 = 0;
}

/*
 * Runs the opened gate's scene; arms the exit scene (flag 18, area lighting)
 * once table A entry 32 is beaten, then hands the entry to its exit when it
 * dies.
 */
ADDRESS(0x801ed4b4, 0x150)
void map_update_events(void)
{
    Creature *e = &vis_table_a.entries[32];

    if (fdat_801edc50 == 1)
        map_warp_player(0);
    else if (fdat_801edc50 == 2)
        map_warp_player(1);
    if (!(game_progress.event_flags.word >> 18 & 1) && e->hp == 0) {
        viewer.state = 5;
        game_progress.event_flags.word |= 0x40000;
        lighting.area_mode[3] = 0x11;
        lighting.area_mode[20] = 0x11;
        lighting.area_mode[24] = 0x11;
        lighting.area_mode[18] = 0x11;
        fdat_801edc54 = 1;
    }
    if (fdat_801edc54 == 1) {
        player_update_hand_action(0);
        player_update_hand_action(1);
        if (fdat_801edc54 == 1 && e->state == 3) {
            viewer.state = 5;
            e->flags |= 0x2000000;
            creature_force_talk(e, 1, 0);
            fdat_801edc54 = 0xff;
            viewer.state = 0;
        }
    }
}

/* Event 0x131 opens a gate: 2 when the viewer stands in area 3, else 1. */
ADDRESS(0x801ed604, 0x58)
void map_use_item_request_warp(short id)
{
    if (id == 0x131 && fdat_801edc50 == 0) {
        if (((MapBlock *)viewer.block_record)->zone == 3)
            fdat_801edc50 = 2;
        else
            fdat_801edc50 = 1;
        DAT_801cd0e0 = 1;
    }
}

#include "../lib/fdat_creature_pick_move.inc"
#include "../lib/fdat_creature_step_script.inc"
