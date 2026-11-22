/*
 * FDAT_062.BIN as one translation unit (WIP module): its handler table,
 * the 21 functions every overlay starts with (src/lib/fdat_common_a.inc,
 * fdat_common.inc, fdat_common_b.inc), then its map handlers through its
 * own move picker. One object, because the picker's switch table is
 * 8-aligned after creature_run_script_command's in the same read-only run.
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
#include <st/player.h>
#include <st/menu.h>

RODATA(0x801eb560, 0x24c)

void map_restore_progress();
void map_update_events();

const FdatHeader fdat_header = {
    0xb,
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
    0x3e,
};

#include "../lib/fdat_common_a.inc"
#include "../lib/fdat_common.inc"
#include "../lib/fdat_common_b.inc"

/*
 * Arms the live table A entries 0..4 whose flags 2..5 are set.
 */
ADDRESS(0x801ed4a0, 0xec)
void map_restore_progress(void)
{
    Creature *e3 = &vis_table_a.entries[3];
    Creature *e2 = &vis_table_a.entries[2];
    Creature *e1 = &vis_table_a.entries[1];
    Creature *e0 = &vis_table_a.entries[0];
    Creature *e4 = &vis_table_a.entries[4];

    if (game_progress.event_flags.bit.f2 == 1) {
        if (e3->spawn_chance != 0xff)
            e3->spawn_chance = 100;
        if (e4->spawn_chance != 0xff)
            e4->spawn_chance = 100;
    }
    if (game_progress.event_flags.bit.f3 == 1 && e2->spawn_chance != 0xff)
        e2->spawn_chance = 100;
    if (game_progress.event_flags.bit.f4 == 1 && e1->spawn_chance != 0xff)
        e1->spawn_chance = 100;
    if (game_progress.event_flags.bit.f5 == 1 && e0->spawn_chance != 0xff)
        e0->spawn_chance = 100;
}

/*
 * Each frame: beats the live table A entries 0..4 whose trigger has fired
 * and sets their flag (2..5): entries 3 and 4 once table B entry 20's state
 * is 1, entry 2 once entry 1 is open, entry 1 once door 12 is idle, entry 0
 * while the viewer's unk178 is 57. Kills the viewer on unk178 68.
 */
ADDRESS(0x801ed58c, 0x244)
void map_update_events(void)
{
    Creature *e3 = &vis_table_a.entries[3];
    Creature *e4 = &vis_table_a.entries[4];
    Creature *e2 = &vis_table_a.entries[2];
    Creature *e1 = &vis_table_a.entries[1];
    Creature *e0 = &vis_table_a.entries[0];
    AreaObject *d20 = &vis_table_b.entries[20];
    AreaObject *d1 = &vis_table_b.entries[1];
    AreaObject *d12 = &vis_table_b.entries[12];

    if (!game_progress.event_flags.bit.f2 && d20->state == 1) {
        if (e3->state == 0)
            creature_spawn_now(e3);
        if (e4->state == 0)
            creature_spawn_now(e4);
        e3->spawn_chance = 100;
        e4->spawn_chance = 100;
        game_progress.event_flags.word |= 4;
    }
    if (!game_progress.event_flags.bit.f3 && d1->state == 255 && e2->state == 0) {
        creature_spawn_now(e2);
        e2->spawn_chance = 100;
        game_progress.event_flags.word |= 8;
    }
    if (!game_progress.event_flags.bit.f4 && d12->params.door.lock == 255 && e1->state == 0) {
        creature_spawn_now(e1);
        e1->spawn_chance = 100;
        game_progress.event_flags.word |= 0x10;
    }
    if (!game_progress.event_flags.bit.f5 && viewer.block == 57 && e0->state == 0) {
        creature_spawn_now(e0);
        e0->spawn_chance = 100;
        game_progress.event_flags.word |= 0x20;
    }
    if (viewer.state != VIEWER_STATE_DYING && viewer.block == 68)
        player_kill(0);
}

#include "../lib/fdat_creature_pick_move.inc"
#include "../lib/fdat_creature_step_script.inc"
