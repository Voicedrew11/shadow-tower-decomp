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
    Vis212 *e3 = &vis_table_a.entries[3];
    Vis212 *e2 = &vis_table_a.entries[2];
    Vis212 *e1 = &vis_table_a.entries[1];
    Vis212 *e0 = &vis_table_a.entries[0];
    Vis212 *e4 = &vis_table_a.entries[4];

    if (game_progress.unk1440.bit.f2 == 1) {
        if (e3->unk00 != 0xff)
            e3->unk00 = 100;
        if (e4->unk00 != 0xff)
            e4->unk00 = 100;
    }
    if (game_progress.unk1440.bit.f3 == 1 && e2->unk00 != 0xff)
        e2->unk00 = 100;
    if (game_progress.unk1440.bit.f4 == 1 && e1->unk00 != 0xff)
        e1->unk00 = 100;
    if (game_progress.unk1440.bit.f5 == 1 && e0->unk00 != 0xff)
        e0->unk00 = 100;
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
    Vis212 *e3 = &vis_table_a.entries[3];
    Vis212 *e4 = &vis_table_a.entries[4];
    Vis212 *e2 = &vis_table_a.entries[2];
    Vis212 *e1 = &vis_table_a.entries[1];
    Vis212 *e0 = &vis_table_a.entries[0];
    Vis88 *d20 = &vis_table_b.entries[20];
    Vis88 *d1 = &vis_table_b.entries[1];
    Vis88 *d12 = &vis_table_b.entries[12];

    if (!game_progress.unk1440.bit.f2 && d20->unk0C == 1) {
        if (e3->unk05 == 0)
            creature_spawn_now(e3);
        if (e4->unk05 == 0)
            creature_spawn_now(e4);
        e3->unk00 = 100;
        e4->unk00 = 100;
        game_progress.unk1440.word |= 4;
    }
    if (!game_progress.unk1440.bit.f3 && d1->unk0C == 255 && e2->unk05 == 0) {
        creature_spawn_now(e2);
        e2->unk00 = 100;
        game_progress.unk1440.word |= 8;
    }
    if (!game_progress.unk1440.bit.f4 && d12->unk4A.door.unk4F == 255 && e1->unk05 == 0) {
        creature_spawn_now(e1);
        e1->unk00 = 100;
        game_progress.unk1440.word |= 0x10;
    }
    if (!game_progress.unk1440.bit.f5 && viewer.unk178 == 57 && e0->unk05 == 0) {
        creature_spawn_now(e0);
        e0->unk00 = 100;
        game_progress.unk1440.word |= 0x20;
    }
    if (viewer.unk218 != 4 && viewer.unk178 == 68)
        player_kill(0);
}

#include "../lib/fdat_creature_pick_move.inc"
#include "../lib/fdat_creature_step_script.inc"
