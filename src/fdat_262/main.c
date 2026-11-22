/*
 * FDAT_262.BIN as one translation unit (WIP module): its handler table,
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

/* Entry 48: 0 asleep, 0xfd hit, 0xfe awake, 0xff gone (flags 29, 30). */
DATA(0x801ee370, 0x4, ".bss")
static int fdat_801ee370;

/* 0xff once record 13's stream counter reached 1 (or entry 50 was hit). */
DATA(0x801ee374, 0x4, ".bss")
static int fdat_801ee374;

/* Table A entry 33 sent off: 0 not yet, 0xff done. */
DATA(0x801ee378, 0x4, ".bss")
static int fdat_801ee378;

/* 0xff once table A entry 49 is beaten. */
DATA(0x801ee37c, 0x4, ".bss")
static int fdat_801ee37c;

/* Item 0x129 on table B entries 0x16 and 0x17: 1 due, 0xff done (second
 * flag word bits 10 and 11). */
DATA(0x801ee380, 0x4, ".bss")
static int fdat_801ee380;

DATA(0x801ee384, 0x4, ".bss")
static int fdat_801ee384;

void map_restore_progress();
void map_update_events();
void map_use_item_on_object(short id);
void map_pick_creature_move();
void map_step_creature();

const FdatHeader fdat_header = {
    0x1f,
    {
        map_hook_none, map_update_events, map_hook_none, map_hook_none,
        map_hook_none, map_restore_progress, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_use_item_on_object, map_hook_none, map_hook_none, map_hook_none,
        map_pick_creature_move, map_step_creature, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
    },
    0x52,
};

#include "../lib/fdat_common_a.inc"
#include "../lib/fdat_common.inc"
#include "../lib/fdat_common_b.inc"

void map_hand_over_item(void);
void map_slide_ground_item(int which);
void map_redrop_ground_item(int which);

/* Sets up the area by flags 14, 29, 30 and the second word's bits 10/11:
 * entry 48 (asleep, awake or gone), entries 50, 33 and 49, table B entry
 * 0 and table C items 7..10, 28 and 29. */
ADDRESS(0x801ed4a0, 0x2a0)
void map_restore_progress(void)
{
    Creature *e48 = &vis_table_a.entries[48];
    CreatureType *r1 = &vis_table_a.types[1];
    Creature *e50 = &vis_table_a.entries[50];
    CreatureType *r13 = &vis_table_a.types[13];
    Creature *e33 = &vis_table_a.entries[33];
    CreatureType *r4 = &vis_table_a.types[4];
    Creature *e49 = &vis_table_a.entries[49];
    GroundItem *c28 = &vis_table_c.entries[28];
    GroundItem *c29 = &vis_table_c.entries[29];
    GroundItem *c7 = &vis_table_c.entries[7];
    GroundItem *c8 = &vis_table_c.entries[8];
    GroundItem *c9 = &vis_table_c.entries[9];
    GroundItem *c10 = &vis_table_c.entries[10];
    AreaObject *d0 = &vis_table_b.entries[0];

    if (e48->state == 3 || game_progress.event_flags.bit.f29 == 1) {
        e48->spawn_chance = 0;
        fdat_801ee370 = 0xff;
    } else if (game_progress.event_flags.bit.f14) {
        if (game_progress.event_flags.bit.f30 == 1) {
            fdat_801ee370 = 0xfe;
            e48->flags &= ~0x2000000;
        } else {
            fdat_801ee370 = 0;
            e48->status.bytes[3] = 3;
            r1->scripts[13]->command = 0x72;
            r1->scripts[14]->command = 0x80;
        }
    } else {
        e48->spawn_chance = 0;
        fdat_801ee370 = 0xff;
    }
    if (e50->state != 3)
        e50->status.bytes[3] = 2;
    if (game_progress.talk_counts[r13->talk_id] == 1)
        fdat_801ee374 = 0xff;
    else
        fdat_801ee374 = 0;
    if (e33->state != 3) {
        e33->status.bytes[3] = 3;
        d0->state = 11;
    }
    if (game_progress.talk_counts[r4->talk_id] == 1) {
        fdat_801ee378 = 0xff;
        e33->flags &= ~0x2000000;
        r4->actions &= ~0x180;
    } else {
        fdat_801ee378 = 0;
    }
    if (e49->state == 3)
        fdat_801ee37c = 0xff;
    else
        fdat_801ee37c = 0;
    if (!(game_progress.event_flags2.word >> 10 & 1)) {
        c9->state = 4;
        c7->state = 4;
        c28->state = 4;
        fdat_801ee380 = 0;
    } else {
        c28->state = 5;
        fdat_801ee380 = 0xff;
        c28->position.vx -= 0x200;
        c28->depth_bias -= 1;
    }
    if (!(game_progress.event_flags2.word >> 11 & 1)) {
        c10->state = 4;
        c8->state = 4;
        c29->state = 4;
        fdat_801ee384 = 0;
    } else {
        c29->state = 5;
        fdat_801ee384 = 0xff;
        c29->position.vx += 0x200;
        c29->depth_bias -= 1;
    }
}

/*
 * Runs the area: entry 48 wakes when hit or once record 1's counter is 2
 * (map_hand_over_item's scene) and changes its scripts after its flash; entry
 * 50 leaves; the viewer on records 0x9c, 0xe9, 0xa0, 0xa2 or 0xa4 sends
 * entry 33 until entry 49 is beaten; item 0x129's two scenes run.
 */
ADDRESS(0x801ed740, 0x3cc)
void map_update_events(void)
{
    Creature *e48 = &vis_table_a.entries[48];
    CreatureType *r1 = &vis_table_a.types[1];
    Creature *e50 = &vis_table_a.entries[50];
    CreatureType *r13 = &vis_table_a.types[13];
    Creature *e33 = &vis_table_a.entries[33];
    CreatureType *r4 = &vis_table_a.types[4];
    Creature *e49 = &vis_table_a.entries[49];
    AreaObject *d0 = &vis_table_b.entries[0];

    if (fdat_801ee370 == 0) {
        if (game_progress.talk_counts[r1->talk_id] == 2) {
            map_hand_over_item();
            creature_fade_out_despawn(e48);
            viewer.state = 0;
            fdat_801ee370 = 0xff;
        }
        if (e48->hit == 1) {
            fdat_801ee370 = 0xfd;
            e48->status.bytes[2] &= 0xea;
            e48->flags &= ~0x2000000;
            game_progress.event_flags.word |= 0x40000000;
        }
    }
    if (fdat_801ee370 == 0xfd && e48->hurt_timer == -1) {
        game_progress.talk_counts[r1->talk_id] = 2;
        r1->scripts[13]->command = 0x80;
        r1->scripts[14]->command = 0x72;
        e48->flags |= 0x2000000;
        creature_force_talk(e48, 0, 0);
        fdat_801ee370 = 0xfe;
        e48->status.bytes[3] = 0;
        e48->flags &= ~0x2000000;
    }
    if (e48->hp == 0)
        game_progress.event_flags.word |= 0x20000000;
    if (fdat_801ee374 != 0xff && game_progress.talk_counts[r13->talk_id] == 1) {
        creature_fade_out_despawn(e50);
        fdat_801ee374 = 0xff;
    }
    if (e50->hit != 0) {
        e50->hit = 0;
        creature_fade_out_despawn(e50);
        fdat_801ee374 = 0xff;
    }
    if (fdat_801ee378 == 0) {
        if (viewer.block == 0x9c || viewer.block == 0xe9 || viewer.block == 0xa0
            || viewer.block == 0xa2 || viewer.block == 0xa4) {
            viewer.state = 5;
            creature_force_talk(e33, 0, 0);
            e33->hit = 1;
            fdat_801ee378 = 0xff;
            viewer.state = 0;
            e33->flags &= ~0x2000000;
            r4->actions &= ~0x180;
        } else {
            e33->action = 0x80;
        }
    }
    if (fdat_801ee37c == 0 && e49->hp == 0) {
        creature_despawn(e33);
        fdat_801ee37c = 0xff;
        game_progress.talk_counts[r4->talk_id] = 1;
        fdat_801ee378 = 0xff;
        d0->state = 0;
    } else if (fdat_801ee37c != 0xff) {
        e33->status.bytes[3] = 3;
    }
    if (fdat_801ee380 == 1) {
        map_slide_ground_item(0);
        map_redrop_ground_item(0);
        fdat_801ee380 = 0xff;
        game_progress.event_flags2.word |= 0x400;
    }
    if (fdat_801ee384 == 1) {
        map_slide_ground_item(1);
        map_redrop_ground_item(1);
        fdat_801ee384 = 0xff;
        game_progress.event_flags2.word |= 0x800;
    }
}

/* Item 0x129 used in front of table B entry 0x16 or 0x17 starts its
 * scene; the result is 1 then, else 2. */
ADDRESS(0x801edb0c, 0x10c)
void map_use_item_on_object(short id)
{
    VECTOR p;
    CollisionRequest req;
    int n;

    if (id == 0x129) {
        p.vx = viewer.position.vx;
        p.vy = viewer.position.vy + 1600;
        p.vz = viewer.position.vz;
        req.tests = 0;
        req.position = p;
        req.radius = 1056;
        req.velocity.vx = 0;
        req.velocity.vy = 0;
        req.velocity.vz = 0;
        req.speed = 0;
        req.height = 1600;
        req.heading = 0;
        n = object_find_usable(0, (CollisionState *)&req);
        if (fdat_801ee380 == 0 && n == 0x16) {
            fdat_801ee380 = 1;
            game_progress.item_use_result = 1;
        } else if (fdat_801ee384 == 0 && n == 0x17) {
            fdat_801ee384 = 1;
            game_progress.item_use_result = 1;
        } else {
            game_progress.item_use_result = 2;
        }
    }
}

/*
 * The entry's move picker, creature_pick_move with two additions: kind 4 waits
 * (action 128) while fdat_801ee378 is 0, and kind 1 only plays animation
 * 10 while fdat_801ee370 is 0 or 253.
 */
ADDRESS(0x801edc18, 0x2ac)
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
    if (e->type == 4 && fdat_801ee378 == 0)
        e->action = 128;
    if (e->type == 1 && (fdat_801ee370 == 0 || fdat_801ee370 == 253)) {
        creature_start_script(e, 10);
        return;
    }
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
        e->unk5A = e->target_heading = e->rotation.vy;
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

/* The current entry's handler: the animation steps as creature_step_script does,
 * kind 1 replaying animation 10 while entry 48 sleeps. */
ADDRESS(0x801edec4, 0xf0)
void map_step_creature(void)
{
    Creature *e = vis_table_a.current;

    e->anim_frame += e->anim_step;
    if (creature_step_hurt_timer(e) == 1)
        return;
    if (e->anim_step >= 0 ? (short)e->anim_frame >= 4096 : (short)e->anim_frame <= 0) {
        if (e->type == 1 && (fdat_801ee370 == 0 || fdat_801ee370 == 0xfd)) {
            creature_start_script(e, 10);
        } else {
            e->script_state = 0xff;
            map_pick_creature_move();
        }
    }
    if (e->script != NULL) {
        creature_run_script_command(e);
        e->script_state = 0xf0;
    }
}

/*
 * Entry 48 plays animation 15 and lifts its arm a frame at a time; table C
 * item 5 rides on vertex 0x160 until the arm is up. Then the view settles
 * and the item drops.
 */
ADDRESS(0x801edfb4, 0x18c)
void map_hand_over_item(void)
{
    Creature *e = &vis_table_a.entries[48];
    GroundItem *c = &vis_table_c.entries[5];
    SVECTOR v;
    SVECTOR unused;

    creature_start_script(e, 15);
    do {
        e->anim_frame += e->anim_step;
        if ((s16)e->anim_frame >= 0x1000)
            e->anim_frame = 0xfff;
        anim_get_vertex(e->model_slot + 128, e->anim, (s16)e->anim_frame, 0x160, &v);
        c->position.vx = e->position.vx - v.vz;
        c->position.vy = e->position.vy + v.vy;
        c->position.vz = e->position.vz + v.vx;
        world_run_frame(NULL, &viewer.view_rot, 0);
    } while ((s16)e->anim_frame != 0xfff);
    c->state = 2;
    viewer.rotation.vx = viewer.view_rot.vx + viewer.rot_offset.halves[0] + viewer.shake_pitch + viewer.fall_pitch;
    viewer.rotation.vy = viewer.view_rot.vy + viewer.rot_offset.halves[1] + viewer.shake_yaw + viewer.unk292;
    viewer.rotation.vz = viewer.view_rot.vz + viewer.roll_offset + viewer.shake_roll + viewer.unk294;
    viewer.camera_rot.vx = 4096 - viewer.rotation.vx;
    viewer.camera_rot.vy = 4096 - viewer.rotation.vy;
    viewer.camera_rot.vz = 4096 - viewer.rotation.vz;
    RotMatrixZYX(&viewer.camera_rot, &viewer.camera_matrix);
    ground_item_pick_up(c, 1);
}

/* Slides table C item 28 (`which` 0, by -0x200) or 29 (by 0x200) along x,
 * 24 a frame while its model is loaded. */
ADDRESS(0x801ee140, 0x138)
void map_slide_ground_item(int which)
{
    GroundItem *item;
    int end;
    int step;
    ItemType *kind;

    if (which == 0) {
        item = &vis_table_c.entries[28];
        step = -24;
        end = item->position.vx - 0x200;
    } else {
        item = &vis_table_c.entries[29];
        step = 24;
        end = item->position.vx + 0x200;
    }
    item->state = 5;
    kind = &vis_table_c.types[item->item];
    item->depth_bias -= 1;
    for (;;) {
        StAnimFile *file;

        if (item->position.vx == end)
            break;
        anim_get_file(kind->model + 588, &file);
        if (file != NULL)
            item->position.vx += step;
        if ((step > 0 && item->position.vx > end) || (step < 0 && item->position.vx < end))
            item->position.vx = end;
        world_run_frame(NULL, NULL, 0);
    }
    play_unlock_sound();
}

/*
 * Turns one of two table C items into its table A form: items 7 and 9
 * (`which` 0) or 8 and 10, the one whose id bit is clear first. Both items
 * are cleared.
 */
ADDRESS(0x801ee278, 0xf8)
void map_redrop_ground_item(int which)
{
    Creature e;
    GroundItem *a;
    GroundItem *b;

    if (which == 0) {
        if (vis_table_c.collected_bits[1] & 0x20) {
            a = &vis_table_c.entries[9];
            b = &vis_table_c.entries[7];
        } else {
            a = &vis_table_c.entries[7];
            b = &vis_table_c.entries[9];
        }
    } else {
        if (vis_table_c.collected_bits[1] & 0x800) {
            a = &vis_table_c.entries[10];
            b = &vis_table_c.entries[8];
        } else {
            a = &vis_table_c.entries[8];
            b = &vis_table_c.entries[10];
        }
    }
    e.block = a->block;
    e.position = a->position;
    e.height = 0;
    e.rotation = a->rotation;
    e.player_dist_xz = a->player_dist_xz;
    creature_drop_item(&e, a->item);
    b->state = 0;
    a->state = 0;
}
