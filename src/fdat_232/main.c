/*
 * FDAT_232.BIN as one translation unit (WIP module): its handler table,
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

void map_restore_progress();
void map_update_events();
void map_pick_creature_move();
void map_talk_on_player_death();

const FdatHeader fdat_header = {
    0x1c,
    {
        map_hook_none, map_update_events, map_hook_none, map_hook_none,
        map_hook_none, map_restore_progress, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_talk_on_player_death, map_hook_none, map_hook_none,
        map_pick_creature_move, creature_step_script, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
    },
    0x4f,
};

#include "../lib/fdat_common_a.inc"
#include "../lib/fdat_common.inc"
#include "../lib/fdat_common_b.inc"

/* Table A entry 32 beaten: 0 not yet, 1 this visit, 0xff (flag 28). */
DATA(0x801edb94, 0x4, ".bss")
static int fdat_801edb94;

/* 1 while the viewer's status 0x188 is held at 30. */
DATA(0x801edb98, 0x4, ".bss")
static int fdat_801edb98;

/* The entry scene: 0 waiting for the fade, 1 due, 0xff done. */
DATA(0x801edb9c, 0x4, ".bss")
static int fdat_801edb9c;

/* Sets up the area by flag 28 and record 12's stream counter. */
ADDRESS(0x801ed4a0, 0xfc)
void map_restore_progress(void)
{
    CreatureType *r = &vis_table_a.types[12];

    if (!(game_progress.event_flags.word >> 28 & 1))
        fdat_801edb94 = 0;
    else
        fdat_801edb94 = 0xff;
    if (game_progress.talk_counts[r->talk_id] >= 2)
        r->actions &= ~0x180;
    if (game_progress.talk_counts[r->talk_id] == 0) {
        viewer.state = VIEWER_STATE_SCRIPTED;
        fdat_801edb9c = 0;
    } else {
        fdat_801edb9c = 0xff;
    }
    if (game_progress.talk_counts[r->talk_id] != 0 && !(game_progress.event_flags.word >> 28 & 1))
        fdat_801edb98 = 1;
    else
        fdat_801edb98 = 0;
}

/*
 * Runs the area: once the fade is done table A entry 32 wakes; the
 * viewer's status 0x188 is held at 30 meanwhile; the viewer passing (x
 * 0x4c000, z 0x55000) while record 12's counter is 1 sends it; beaten, it
 * leaves (flag 28).
 */
ADDRESS(0x801ed59c, 0x23c)
void map_update_events(void)
{
    Creature *e = &vis_table_a.entries[32];
    CreatureType *r = &vis_table_a.types[12];
    int n;

    if (fdat_801edb9c == 0 && area_state.entry_fade_start == -1 && area_state.load_step != 0)
        fdat_801edb9c = 1;
    if (fdat_801edb9c == 1) {
        e->flags |= 0x2000000;
        creature_force_talk(e, 1, 0);
        fdat_801edb98 = 1;
        fdat_801edb9c = 0xff;
        viewer.state = VIEWER_STATE_NORMAL;
    }
    if (fdat_801edb98 == 1 && !(viewer.status_flags & 0x800)) {
        viewer.magic_gauge_delay = 30;
        viewer.magic_gauge = 0;
    }
    n = game_progress.talk_counts[r->talk_id];
    if (n == 1 && viewer.position.vx > 0x4c000 && viewer.position.vz > 0x55000) {
        viewer.state = VIEWER_STATE_SCRIPTED;
        creature_force_talk(e, 0, 0);
        e->hit = n;
        viewer.state = VIEWER_STATE_NORMAL;
        e->flags &= ~0x2000000;
        r->actions &= ~0x180;
    }
    if (fdat_801edb94 == 0 && e->hp == 0) {
        fdat_801edb94 = 1;
        game_progress.event_flags.word |= 0x10000000;
    }
    if (fdat_801edb94 == 1 && e->state == CREATURE_STATE_GONE) {
        viewer.state = VIEWER_STATE_SCRIPTED;
        e->flags |= 0x2000000;
        creature_force_talk(e, 1, 0);
        fdat_801edb94 = 0xff;
        fdat_801edb98 = 0;
        viewer.state = VIEWER_STATE_NORMAL;
    }
}

/*
 * The entry's move picker, creature_pick_move with one addition: kind 12 waits
 * (action 128) until its record's stream counter reaches 2.
 */
ADDRESS(0x801ed7d8, 0x280)
void map_pick_creature_move(void)
{
    Creature *e = vis_table_a.current;
    CreatureType *r = vis_table_a.current_type;
    u16 wall;
    int d;
    short x;
    short z;
    short dx;
    short dz;

    if (e->script_state == 0xf0 || e->script_state == 0)
        return;
    creature_choose_action();
    if (e->type == 12 && game_progress.talk_counts[r->talk_id] < 2)
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
        e->unk5A = e->target_heading = e->rotation.vy;
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
#define FDAT_PICK_MOVE map_pick_creature_move

#include "../lib/fdat_creature_step_script.inc"

/* In area 3, while flag 28 is clear, hands table A entry 32 to its exit once
 * its record's stream counter has reached 2. */
ADDRESS(0x801edb08, 0x8c)
void map_talk_on_player_death(void)
{
    Creature *e;
    CreatureType *r;

    e = &vis_table_a.entries[32];
    r = &vis_table_a.types[12];

    if (game_progress.talk_counts[r->talk_id] >= 2 && !(game_progress.event_flags.word >> 28 & 1)
        && vis_table_a.zone == 3) {
        game_progress.talk_counts[r->talk_id] = 3;
        e->flags |= 0x2000000;
        creature_force_talk(e, 1, 1);
    }
}
