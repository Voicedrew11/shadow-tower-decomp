/*
 * FDAT_252.BIN as one translation unit (WIP module): its handler table,
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
#include <LIBGPU.H>
#include <st/light.h>

RODATA(0x801eb560, 0x24c)

void map_restore_progress();
void map_update_events();
void map_pick_creature_move();

const FdatHeader fdat_header = {
    0x1e,
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
    0x51,
};

#include "../lib/fdat_common_a.inc"
#include "../lib/fdat_common.inc"
#include "../lib/fdat_common_b.inc"

/* The scene state: 1 once record 8's stream counter reached 1. */
DATA(0x801ed8cc, 0x4, ".bss")
int fdat_801ed8cc;

/* Sets up the area: arms the scene and clears record 8's flags 0x180 once
 * its stream counter has reached 1. */
ADDRESS(0x801ed4a0, 0x4c)
void map_restore_progress(void)
{
    CreatureType *r = &vis_table_a.types[8];

    if (game_progress.talk_counts[r->talk_id] == 1) {
        fdat_801ed8cc = 1;
        r->actions &= ~0x180;
    } else {
        fdat_801ed8cc = 0;
    }
}

/* Until the scene ran, the viewer standing on record 0x33 or 0x36 sends
 * table A entry 32 off (record 8 loses its range bits); otherwise the entry
 * keeps state 0x80. */
ADDRESS(0x801ed4ec, 0xc8)
void map_update_events(void)
{
    Creature *e = &vis_table_a.entries[32];
    CreatureType *r = &vis_table_a.types[8];

    if (fdat_801ed8cc == 0) {
        if (viewer.block == 0x33 || viewer.block == 0x36) {
            viewer.state = VIEWER_STATE_SCRIPTED;
            creature_force_talk(e, 0, 0);
            e->hit = 1;
            fdat_801ed8cc = 1;
            viewer.state = VIEWER_STATE_NORMAL;
            e->flags &= ~0x2000000;
            r->actions &= ~0x180;
        } else {
            e->action = 0x80;
        }
    }
}

/*
 * The entry's move picker, creature_pick_move with one addition: kind 8 waits
 * (action 128) while fdat_801ed8cc is 0.
 */
ADDRESS(0x801ed5b4, 0x268)
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
    if (e->type == 8 && fdat_801ed8cc == 0)
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
