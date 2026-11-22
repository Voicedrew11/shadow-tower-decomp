/*
 * FDAT_142.BIN as one translation unit (WIP module): its handler table,
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
#include <st/menu.h>

RODATA(0x801eb560, 0x24c)

void map_restore_progress();
void map_update_events();
void map_pick_creature_move();

const FdatHeader fdat_header = {
    0x13,
    {
        map_hook_none, map_update_events, map_hook_none, map_hook_none,
        map_hook_none, map_restore_progress, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_pick_creature_move, creature_step_script, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
    },
    0x46,
};

#include "../lib/fdat_common_a.inc"
#include "../lib/fdat_common.inc"
#include "../lib/fdat_common_b.inc"

DATA(0x801edaf4, 0x4, ".bss")
static int fdat_801edaf4;

DATA(0x801edaf8, 0x4, ".bss")
static int fdat_801edaf8;

/* 0xff once record 0's stream counter has started. */
DATA(0x801edafc, 0x4, ".bss")
static int fdat_801edafc;

/* 0xff once record 4's stream counter reached 2. */
DATA(0x801edb00, 0x4, ".bss")
static int fdat_801edb00;

/* Sets up the area: the scene words by records 0 and 4's stream counters;
 * until record 4's reaches 2, table A entries 5..7 are cleared and entry
 * 49's range bits set. */
ADDRESS(0x801ed4a0, 0xac)
void map_restore_progress(void)
{
    CreatureType *r0 = &vis_table_a.types[0];
    Creature *e49 = &vis_table_a.entries[49];
    CreatureType *r4 = &vis_table_a.types[4];
    Creature *e5 = &vis_table_a.entries[5];
    Creature *e6 = &vis_table_a.entries[6];
    Creature *e7 = &vis_table_a.entries[7];

    fdat_801edaf4 = 0;
    fdat_801edaf8 = 0;
    if (game_progress.unk1448[r0->talk_id] != 0)
        fdat_801edafc = 0xff;
    else
        fdat_801edafc = 0;
    if (game_progress.unk1448[r4->talk_id] == 2) {
        fdat_801edb00 = 0xff;
    } else {
        e7->spawn_chance = 0;
        e6->spawn_chance = 0;
        e5->spawn_chance = 0;
        e49->status.bytes[3] = 2;
        fdat_801edb00 = 0;
    }
}

/*
 * Runs the area: the viewer standing on record 0x1f2 or 0x63 picks the
 * scene words; entry 48 asleep until it wakes (scene word 3 armed), then
 * leaves once the area fade is done, keeping the viewer's status 0x188 at
 * 30; record 0x10e with both words set sends entry 49 away and frees
 * entries 5..7.
 */
ADDRESS(0x801ed54c, 0x26c)
void map_update_events(void)
{
    Creature *e48 = &vis_table_a.entries[48];
    Creature *e49 = &vis_table_a.entries[49];
    Creature *e5 = &vis_table_a.entries[5];
    Creature *e6 = &vis_table_a.entries[6];
    Creature *e7 = &vis_table_a.entries[7];

    if (fdat_801edaf8 == 0) {
        if (viewer.unk178 == 0x1f2) {
            fdat_801edaf4 = 1;
            fdat_801edaf8 = 1;
        } else if (viewer.unk178 == 0x63) {
            fdat_801edaf4 = 0;
            fdat_801edaf8 = 1;
        }
    }
    if (fdat_801edaf4 == 1 && fdat_801edaf8 == 1)
        e48->spawn_chance = 0;
    if (fdat_801edaf4 == 0) {
        if (fdat_801edaf8 == 1)
            e49->spawn_chance = 0;
        if (fdat_801edaf4 == 0 && e48->state == 1) {
            fdat_801edafc = 1;
            viewer.unk218 = 5;
        }
    }
    if (fdat_801edafc == 1 && area_state.unk18 == -1 && (short)area_state.unk2 >= 2)
        fdat_801edafc = 2;
    if (fdat_801edafc == 2) {
        creature_force_talk(e48, 0, 0);
        creature_fade_out_despawn(e48);
        fdat_801edafc = 0xff;
        viewer.unk218 = 0;
    }
    if (fdat_801edafc == 0xff && !(viewer.unk2C & 0x800)) {
        viewer.unk188 = 30;
        viewer.unk182 = 0;
    }
    if (fdat_801edb00 == 0 && viewer.unk178 == 0x10e && fdat_801edaf4 == 1
        && fdat_801edaf8 == 1) {
        creature_force_talk(e49, 0, 0);
        creature_force_talk(e49, 0, 0);
        creature_fade_out_despawn(e49);
        creature_spawn_now(e5);
        creature_spawn_now(e6);
        creature_spawn_now(e7);
        fdat_801edb00 = 0xff;
    }
}

/*
 * The entry's move picker, creature_pick_move with one addition: kind 0 waits
 * (action 128) once fdat_801edafc is 0xff, and kind 4 once fdat_801edb00 is.
 */
ADDRESS(0x801ed7b8, 0x28c)
void map_pick_creature_move(void)
{
    Creature *e = vis_table_a.current;
    u16 wall;
    int d;
    short x;
    short z;
    short dx;
    short dz;

    if (e->script_state == 0xf0 || e->script_state == 0)
        return;
    creature_choose_action();
    if (e->type == 0 && fdat_801edafc == 0xff)
        e->action = 128;
    if (e->type == 4 && fdat_801edb00 == 0xff)
        e->action = 128;
    creature_pick_weighted_script();
    switch (e->action) {
    case 8:
        wall = e->wall_heading;
        if ((short)wall == 0x7fff) {
            e->unk5A = e->target_heading = (e->player_heading + 2048) & 0xfff;
        } else {
            d = (((e->player_heading + 2048) & 0xfff) - wall + 1024) & 0xfff;
            if (d < 1024 || d > 3072)
                e->unk5A = e->target_heading = (wall + 3072) & 0xfff;
            else
                e->unk5A = e->target_heading = (wall + 1024) & 0xfff;
        }
        break;
    case 2:
    case 4:
    case 16:
    case 32:
        e->unk5A = e->target_heading = e->player_heading;
        break;
    default:
        e->unk5A = e->target_heading = e->facing;
        break;
    }
    if (e->action != 8)
        e->wall_heading = 0x7fff;
    if (vis_records[e->block].heading_bias_x != 0 || vis_records[e->block].heading_bias_z != 0) {
        x = rcos(e->target_heading - 1024) >> 5;
        z = rsin(e->target_heading - 1024) >> 5;
        dx = x + vis_records[e->block].heading_bias_x;
        dz = z + vis_records[e->block].heading_bias_z;
        e->target_heading = (ratan2(dz, dx) + 1024) & 0xfff;
        x = rcos(e->unk5A) >> 5;
        z = rsin(e->unk5A) >> 5;
        dx = x + vis_records[e->block].heading_bias_x;
        dz = z + vis_records[e->block].heading_bias_z;
        e->unk5A = ratan2(dz, dx);
    }
}

void map_pick_creature_move(void);
#define FDAT_PICK_MOVE map_pick_creature_move

#include "../lib/fdat_creature_step_script.inc"
