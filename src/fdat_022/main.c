/*
 * FDAT_022.BIN as one translation unit (WIP module): its handler table,
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
    0x7,
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
    0x3a,
};

#include "../lib/fdat_common_a.inc"
#include "../lib/fdat_common.inc"
#include "../lib/fdat_common_b.inc"

/* The scene state: 0 pending, 1 ready, 0xff done (flag 25). */
DATA(0x801edad0, 0x4, ".bss")
int fdat_801edad0;

void map_hand_over_item(void);

/* Sets up the area: done when flag 25 is set, ready once record 6's stream
 * counter reached 2, else table A entries 48 and 49 set up for the scene. */
ADDRESS(0x801ed4a0, 0x74)
void map_restore_progress(void)
{
    Creature *e48 = &vis_table_a.entries[48];
    Creature *e49 = &vis_table_a.entries[49];
    CreatureType *r = &vis_table_a.types[6];

    if (game_progress.event_flags.bit.f25 == 1) {
        fdat_801edad0 = 0xff;
    } else if (game_progress.talk_counts[r->talk_id] == 2) {
        fdat_801edad0 = 1;
    } else {
        e49->status.bytes[3] = 1;
        e48->spawn_chance = 0;
        fdat_801edad0 = 0;
    }
}

/* Once the viewer stands on record 0x27, entry 49 leaves and entry 48 is
 * marked; when record 2's stream counter reaches 3 the scene plays. */
ADDRESS(0x801ed514, 0xe8)
void map_update_events(void)
{
    Creature *e48 = &vis_table_a.entries[48];
    CreatureType *r = &vis_table_a.types[2];
    Creature *e49 = &vis_table_a.entries[49];

    if (fdat_801edad0 == 0 && viewer.block == 0x27) {
        creature_force_talk(e49, 0, 0);
        creature_force_talk(e49, 0, 0);
        creature_fade_out_despawn(e49);
        e48->spawn_chance = 100;
        fdat_801edad0 = 1;
    }
    if (fdat_801edad0 == 1 && game_progress.talk_counts[r->talk_id] == 3) {
        viewer.state = VIEWER_STATE_SCRIPTED;
        map_hand_over_item();
        fdat_801edad0 = 0xff;
        viewer.state = VIEWER_STATE_NORMAL;
    }
}

#include "../lib/fdat_creature_pick_move.inc"
#include "../lib/fdat_creature_step_script.inc"

/*
 * Entry 48 plays animation 6 and turns its arm up a frame at a time; table
 * C item 5 (6 once 5 is taken) rides on vertex 0x15a until the arm is up.
 * Then the view settles, the entry leaves, the item drops and flag 25 is
 * set.
 */
ADDRESS(0x801ed8f0, 0x1e0)
void map_hand_over_item(void)
{
    Creature *e = &vis_table_a.entries[48];
    GroundItem *c5 = &vis_table_c.entries[5];
    GroundItem *c6 = &vis_table_c.entries[6];
    GroundItem *c;
    SVECTOR v;
    SVECTOR unused;

    creature_start_script(e, 6);
    c = c5;
    if (vis_table_c.collected_bits[c5->item / 32] & (1 << (c5->item % 32)))
        c = c6;
    do {
        e->anim_frame += e->anim_step;
        if ((s16)e->anim_frame >= 0x1000)
            e->anim_frame = 0xfff;
        anim_get_vertex(e->model_slot + 128, e->anim, (s16)e->anim_frame, 0x15a, &v);
        c->position.vx = e->position.vx - v.vz;
        c->position.vy = e->position.vy + v.vy;
        c->position.vz = e->position.vz + v.vx;
        world_run_frame(NULL, &viewer.view_rot, 0);
    } while ((s16)e->anim_frame != 0xfff);
    c->state = 2;
    viewer.pitch = viewer.view_rot.vx + viewer.rot_offset.halves[0] + viewer.shake_pitch + viewer.fall_pitch;
    viewer.yaw = viewer.view_rot.vy + viewer.rot_offset.halves[1] + viewer.shake_yaw + viewer.unk292;
    viewer.roll = viewer.view_rot.vz + viewer.roll_offset + viewer.shake_roll + viewer.unk294;
    viewer.camera_rot.vx = 4096 - viewer.pitch;
    viewer.camera_rot.vy = 4096 - viewer.yaw;
    viewer.camera_rot.vz = 4096 - viewer.roll;
    RotMatrixZYX(&viewer.camera_rot, &viewer.camera_matrix);
    creature_fade_out_despawn(e);
    ground_item_pick_up(c, 1);
    game_progress.event_flags.word |= 0x2000000;
}
