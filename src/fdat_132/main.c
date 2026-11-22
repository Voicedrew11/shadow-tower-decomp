/*
 * FDAT_132.BIN as one translation unit (WIP module): its handler table,
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
#include <LIBGPU.H>
#include <st/menu.h>
#include <st/light.h>

RODATA(0x801eb560, 0x24c)

void map_restore_progress();
void map_update_events();

const FdatHeader fdat_header = {
    0x12,
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
    0x45,
};

#include "../lib/fdat_common_a.inc"
#include "../lib/fdat_common.inc"
#include "../lib/fdat_common_b.inc"

/* 0xff once record 2's stream counter reached 1 (or entry 48 was hit). */
DATA(0x801eda30, 0x4, ".bss")
static int fdat_801eda30;

/* Table A entries 34, 33 and 35 beaten (flags 21, 22, 23): 0 not yet,
 * 1 this visit, 0xff before. */
DATA(0x801eda34, 0x4, ".bss")
static int fdat_801eda34;

DATA(0x801eda38, 0x4, ".bss")
static int fdat_801eda38;

DATA(0x801eda3c, 0x4, ".bss")
static int fdat_801eda3c;

/* Sets up the area: entry 48's range bits unless it is beaten, the scene
 * words by record 2's stream counter and flags 21..23, and records 1, 4
 * and 8 with their bits 7-8 clear. */
ADDRESS(0x801ed4a0, 0x11c)
void map_restore_progress(void)
{
    Creature *e48 = &vis_table_a.entries[48];
    CreatureType *r2 = &vis_table_a.types[2];
    CreatureType *r1 = &vis_table_a.types[1];
    CreatureType *r4 = &vis_table_a.types[4];
    CreatureType *r8 = &vis_table_a.types[8];

    if (e48->state != CREATURE_STATE_GONE)
        e48->status.bytes[3] = 2;
    if (game_progress.talk_counts[r2->talk_id] == 1)
        fdat_801eda30 = 0xff;
    else
        fdat_801eda30 = 0;
    if (!(game_progress.event_flags.word >> 21 & 1))
        fdat_801eda34 = 0;
    else
        fdat_801eda34 = 0xff;
    if (!(game_progress.event_flags.word >> 22 & 1))
        fdat_801eda38 = 0;
    else
        fdat_801eda38 = 0xff;
    if (!(game_progress.event_flags.word >> 23 & 1))
        fdat_801eda3c = 0;
    else
        fdat_801eda3c = 0xff;
    r1->actions &= ~0x180;
    r4->actions &= ~0x180;
    r8->actions &= ~0x180;
}

/* Runs the area: entry 48 leaves once record 2's stream counter is 1 or
 * when hit; entries 34, 33 and 35 beaten set flags 21, 22 and 23. */
ADDRESS(0x801ed5bc, 0x180)
void map_update_events(void)
{
    Creature *e48 = &vis_table_a.entries[48];
    CreatureType *r2 = &vis_table_a.types[2];
    Creature *e34 = &vis_table_a.entries[34];
    Creature *e33 = &vis_table_a.entries[33];
    Creature *e35 = &vis_table_a.entries[35];

    if (fdat_801eda30 != 0xff && game_progress.talk_counts[r2->talk_id] == 1) {
        creature_fade_out_despawn(e48);
        fdat_801eda30 = 0xff;
    }
    if (e48->hit != 0) {
        e48->hit = 0;
        creature_fade_out_despawn(e48);
        fdat_801eda30 = 0xff;
    }
    if (fdat_801eda34 == 0 && e34->hp == 0) {
        fdat_801eda34 = 1;
        game_progress.event_flags.word |= 0x200000;
    }
    if (fdat_801eda38 == 0 && e33->hp == 0) {
        fdat_801eda38 = 1;
        game_progress.event_flags.word |= 0x400000;
    }
    if (fdat_801eda3c == 0 && e35->hp == 0) {
        fdat_801eda3c = 1;
        game_progress.event_flags.word |= 0x800000;
    }
}

#include "../lib/fdat_creature_pick_move.inc"
#include "../lib/fdat_creature_step_script.inc"
