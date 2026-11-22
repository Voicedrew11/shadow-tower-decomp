/*
 * FDAT_382.BIN as one translation unit (WIP module): its handler table,
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
#include <LIBGPU.H>
#include <st/menu.h>
#include <st/light.h>

RODATA(0x801eb560, 0x24c)

/* 1 when entries 0..2 and 32 stand (flags 21..23). */
DATA(0x801edafc, 0x4, ".bss")
static int fdat_801edafc;

/* 0xff once the scene ran (record 1's stream counter 2). */
DATA(0x801edb00, 0x4, ".bss")
static int fdat_801edb00;

/* Table A entry 32 beaten: 0, 1 this visit, 0xff gone (flag 24). */
DATA(0x801edb04, 0x4, ".bss")
static int fdat_801edb04;

void map_restore_progress();
void map_update_events();
void map_pick_creature_move();
void map_step_creature();
void map_talk_on_player_death();

const FdatHeader fdat_header = {
    0x2b,
    {
        map_hook_none, map_update_events, map_hook_none, map_hook_none,
        map_hook_none, map_restore_progress, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_talk_on_player_death, map_hook_none, map_hook_none,
        map_pick_creature_move, map_step_creature, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
    },
    0x5e,
};

#include "../lib/fdat_common_a.inc"
#include "../lib/fdat_common.inc"
#include "../lib/fdat_common_b.inc"

/* Sets up the area: with flags 21..23 all set entries 0..2 and 32 stand
 * (scene word 1), else they wait; flag 24 and record 1's stream counter
 * set the other words. */
ADDRESS(0x801ed4a0, 0xe8)
void map_restore_progress(void)
{
    CreatureTable *a = &vis_table_a;
    Creature *e32 = &a->entries[32];
    CreatureType *r1 = &a->types[1];

    if ((game_progress.event_flags.word & 0xe00000) == 0xe00000) {
        vis_table_a.entries[32].spawn_chance = 100;
        vis_table_a.entries[2].spawn_chance = 100;
        vis_table_a.entries[1].spawn_chance = 100;
        vis_table_a.entries[0].spawn_chance = 100;
        e32->status.bytes[3] = 1;
        fdat_801edafc = 1;
        e32->flags &= ~0x2000000;
    } else {
        vis_table_a.entries[32].spawn_chance = 0;
        vis_table_a.entries[2].spawn_chance = 0;
        vis_table_a.entries[1].spawn_chance = 0;
        vis_table_a.entries[0].spawn_chance = 0;
        fdat_801edafc = 0;
    }
    if (!game_progress.event_flags.bit.f24)
        fdat_801edb04 = 0;
    else
        fdat_801edb04 = 0xff;
    if (game_progress.talk_counts[r1->talk_id] >= 2)
        fdat_801edb00 = 0xff;
    else
        fdat_801edb00 = 0;
}

/* Runs the area: the viewer reaching z 0x3a800 wakes entry 32 (or sends
 * it, with entries 0 and 32 out of range); beaten, it changes the lighting
 * of four areas (flag 24) and leaves. */
ADDRESS(0x801ed588, 0x1c4)
void map_update_events(void)
{
    Creature *e0 = vis_table_a.entries;
    Creature *e32 = &vis_table_a.entries[32];
    CreatureType *r1 = &vis_table_a.types[1];

    if (fdat_801edb00 == 0 && viewer.position.vz < 0x3a800) {
        if (fdat_801edafc == 0) {
            game_progress.talk_counts[r1->talk_id] = 0;
            creature_force_talk(e32, 1, 0);
            e32->flags &= ~0x2000000;
        } else {
            game_progress.talk_counts[r1->talk_id] = 1;
            creature_force_talk(e32, 0, 0);
            e32->flags &= ~0x2000000;
            e32->status.bytes[3] = 0;
            e0->status.bytes[3] = 0;
        }
        fdat_801edb00 = 0xff;
    }
    if (fdat_801edb04 == 0 && e32->hp == 0) {
        fdat_801edb04 = 1;
        lighting.area_mode[9] = 0x11;
        lighting.area_mode[8] = 0x11;
        lighting.area_mode[13] = 0x11;
        lighting.area_mode[38] = 0x11;
        game_progress.event_flags.word |= 0x1000000;
    }
    if (fdat_801edb04 == 1 && e32->state == CREATURE_STATE_GONE) {
        viewer.state = VIEWER_STATE_SCRIPTED;
        e32->flags |= 0x2000000;
        game_progress.talk_counts[r1->talk_id] = 2;
        creature_force_talk(e32, 1, 0);
        fdat_801edb04 = 0xff;
        viewer.state = VIEWER_STATE_NORMAL;
    }
}

/*
 * The entry's move picker, creature_pick_move with one addition: kind 1 waits
 * (action 128) while fdat_801edb00 is 0.
 */
ADDRESS(0x801ed74c, 0x268)
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
    if (e->type == 1 && fdat_801edb00 == 0)
        e->action = CREATURE_ACTION_DORMANT;
    creature_pick_weighted_script();
    switch (e->action) {
    case CREATURE_ACTION_TURN_AWAY:
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
    if (e->action != CREATURE_ACTION_TURN_AWAY)
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

/* The current entry's handler: the animation steps as creature_step_script does;
 * the next move is picked only once the scene ran. */
ADDRESS(0x801ed9b4, 0xc0)
void map_step_creature(void)
{
    Creature *e = vis_table_a.current;

    e->anim_frame += e->anim_step;
    if (creature_step_hurt_timer(e) == 1)
        return;
    if ((e->anim_step >= 0 ? (short)e->anim_frame >= 4096 : (short)e->anim_frame <= 0) && fdat_801edb00 == 0xff) {
        e->script_state = 0xff;
        map_pick_creature_move();
    }
    if (e->script != NULL) {
        creature_run_script_command(e);
        e->script_state = 0xf0;
    }
}

/* In area 0, while flag 24 is clear, hands table A entry 32 to its exit once
 * its record's stream counter has reached 2. */
ADDRESS(0x801eda74, 0x88)
void map_talk_on_player_death(void)
{
    Creature *e;
    CreatureType *r;

    e = &vis_table_a.entries[32];
    r = &vis_table_a.types[1];

    if (game_progress.talk_counts[r->talk_id] >= 2 && !(game_progress.event_flags.word >> 24 & 1)
        && vis_table_a.zone == 0) {
        game_progress.talk_counts[r->talk_id] = 3;
        e->flags |= 0x2000000;
        creature_force_talk(e, 1, 1);
    }
}
