/*
 * FDAT_272.BIN as one translation unit (WIP module): its handler table,
 * the 21 functions every overlay starts with (src/lib/fdat_common_a.inc,
 * fdat_common.inc, fdat_common_b.inc), then its map handlers through the
 * palette blend. One object: the blend reads map_update_events's RECT
 * initializer (cc1 pools one copy of equal constants), the read-only run
 * is one pool, and the scene words are consecutive statics.
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

RODATA(0x801eb560, 0x27c)

/* A palette word: two 15-bit colours and their mask bits. */
typedef struct {
    u32 r0 : 5, g0 : 5, b0 : 5, s0 : 1;
    u32 r1 : 5, g1 : 5, b1 : 5, s1 : 1;
} ClutPair;

/* Table A entries that must all be beaten for flag 13 (map_update_events). */
DATA(0x801ef504, 0x18, ".data")
static int fdat_801ef504[6] = {16, 17, 18, 19, 20, 21};

/* Steps of the blend left; at 32 the two rows are read first. */
DATA(0x801ef51c, 0x4, ".data")
static int fdat_801ef51c = 32;

DATA(0x801ef520, 0x200, ".bss")
static ClutPair fdat_801ef520[128];

DATA(0x801ef720, 0x200, ".bss")
static ClutPair fdat_801ef720[128];

DATA(0x801ef920, 0x4, ".bss")
static int fdat_801ef920;

DATA(0x801ef924, 0x4, ".bss")
static int fdat_801ef924;

DATA(0x801ef928, 0x4, ".bss")
static int fdat_801ef928;

DATA(0x801ef92c, 0x4, ".bss")
static int fdat_801ef92c;

DATA(0x801ef930, 0x4, ".bss")
static int fdat_801ef930;

DATA(0x801ef934, 0x4, ".bss")
static int fdat_801ef934;

DATA(0x801ef938, 0x4, ".bss")
static int fdat_801ef938;

DATA(0x801ef93c, 0x4, ".bss")
static int fdat_801ef93c;

void map_update_events();
void map_restore_progress();
void map_pick_creature_move();
void map_step_creature();
void map_object_take_equipment();
void map_use_item_on_creature(short id);

const FdatHeader fdat_header = {
    0x20,
    {
        map_hook_none, map_update_events, map_hook_none, map_hook_none,
        map_hook_none, map_restore_progress, map_hook_none, map_hook_none,
        map_hook_none, map_object_take_equipment, map_hook_none, map_hook_none,
        map_use_item_on_creature, map_hook_none, map_hook_none, map_hook_none,
        map_pick_creature_move, map_step_creature, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
    },
    0x53,
};

#include "../lib/fdat_common_a.inc"
#include "../lib/fdat_common.inc"
#include "../lib/fdat_common_b.inc"

void map_fade_palette_row(void);

/*
 * Runs the gate stages (story flag bits 10-12) by table B entry 5's state;
 * sets flag 13 once the six entries of fdat_801ef504 are all beaten, then
 * counts down to map_fade_palette_row's scene; moves the palette row 487 to 489
 * when fdat_801ef934 asks.
 */
ADDRESS(0x801ed4d0, 0x290)
void map_update_events(void)
{
    AreaObject *d5 = &vis_table_b.entries[5];
    Creature *e32 = &vis_table_a.entries[32];
    int i;
    u32 flags;

    if (game_progress.event_flags.field.gate == 0 && d5->anim_frame != 0) {
        game_progress.event_flags.field.gate = 1;
        creature_spawn_now(e32);
        e32->fade = -1;
        e32->visibility = 1;
        e32->status.bytes[2] &= 0xf2;
    } else if (game_progress.event_flags.field.gate == 1 && d5->anim_frame > 0x800) {
        game_progress.event_flags.field.gate = 2;
    }
    if (fdat_801ef938 == 0 && area_state.loading == 0)
        fdat_801ef938 = 1;
    if (!game_progress.event_flags.bit.f13 && fdat_801ef938 == 1) {
        game_progress.event_flags.bit.f13 = 1;
        for (i = 0; i < 6; i++) {
            u8 state = vis_table_a.entries[fdat_801ef504[i]].state;

            if (state != 3 && state != 5) {
                game_progress.event_flags.bit.f13 = 0;
                break;
            }
        }
    }
    if (game_progress.event_flags.bit.f13 == 1 && fdat_801ef930 >= 0) {
        if (fdat_801ef930 == 0) {
            map_fade_palette_row();
            fdat_801ef938 = 0xff;
        } else {
            fdat_801ef930--;
        }
    }
    if (fdat_801ef934 == 1 && area_state.loading == 0) {
        RECT rect = {0, 487, 256, 1};

        MoveImage(&rect, 0, 489);
        flags = vis_lists.floor_kinds_off;
        fdat_801ef930 = -1;
        fdat_801ef934 = 0;
        vis_lists.floor_kinds_off = flags | 0x10;
    }
}

/*
 * Sets up the area by the story flags: flag 14 (item 0x12f used) or not,
 * flag 29 swaps two records' parts, gate stage 3 and up places entry 32,
 * and the scene stage (bits 8-9) poses entries 50/51 and table C item 28.
 */
ADDRESS(0x801ed760, 0x378)
void map_restore_progress(void)
{
    Creature *e49 = &vis_table_a.entries[49];
    Creature *e48 = &vis_table_a.entries[48];
    Creature *e32 = &vis_table_a.entries[32];
    Creature *e51 = &vis_table_a.entries[51];
    Creature *e50 = &vis_table_a.entries[50];
    CreatureType *r11 = &vis_table_a.types[11];
    GroundItem *c28 = &vis_table_c.entries[28];
    GroundItem *c29 = &vis_table_c.entries[29];
    GroundItem *c10 = &vis_table_c.entries[10];
    GroundItem *c11 = &vis_table_c.entries[11];
    AreaObject *d15 = &vis_table_b.entries[15];
    AreaObject *d16 = &vis_table_b.entries[16];
    int stage;
    u8 part;

    if (game_progress.event_flags.bit.f14 == 1) {
        c29->state = GROUND_ITEM_SCRIPTED;
        fdat_801ef93c = 2;
        d15->kind = OBJECT_KIND_GATE;
        d15->anim = 0;
        d15->anim_frame = 0xfff;
        d15->state = 0xff;
        c29->position.vx += 0x1ea;
    } else {
        fdat_801ef93c = 0;
        e49->status.bytes[3] = 1;
        c29->state = GROUND_ITEM_HIDDEN;
        c11->state = GROUND_ITEM_SCRIPTED;
        c10->state = GROUND_ITEM_SCRIPTED;
        fdat_801ef93c = 0;
        e49->flags &= ~0x2000000;
    }
    e48->status.bytes[3] = 1;
    if (game_progress.event_flags.word >> 29 & 1) {
        part = vis_records[38].part;
        vis_records[38].turn = 3;
        vis_records[38].part = vis_records[330].part;
        vis_records[330].part = part;
    }
    if ((int)(game_progress.event_flags.word >> 10 & 7) >= 3) {
        e32->facing = 0x400;
        e32->home_block = 0x31;
        e32->home_offset_x = 0x800;
        e32->home_offset_z = -0x800;
        e32->home_offset_y = 0;
        e32->spawn_chance = 100;
        e32->region = vis_records[49].region;
        e32->flags &= ~0x2000000;
    } else {
        e32->spawn_chance = 0;
    }
    if (game_progress.event_flags.word >> 29 & 1)
        e32->drop_chance0 = 0;
    stage = game_progress.event_flags.word >> 8 & 3;
    fdat_801ef928 = 0;
    if (stage == 1) {
        creature_start_script(e50, 1);
        e50->status.bytes[3] = 2;
        r11->radius = 0x200;
        r11->height = 0x800;
        e51->state = CREATURE_STATE_GONE;
        fdat_801ef92c = stage;
        c28->state = GROUND_ITEM_HIDDEN;
        fdat_801ef924 = 4;
    } else if (stage == 2) {
        e50->state = CREATURE_STATE_GONE;
        e51->state = CREATURE_STATE_GONE;
        fdat_801ef92c = 2;
        c28->state = GROUND_ITEM_SCRIPTED;
        c28->position.vz = 0x41b88;
        d16->kind = OBJECT_KIND_GATE;
        d16->anim_frame = 0xfff;
        d16->anim = 0;
        d16->state = 0xff;
        fdat_801ef924 = 4;
        c28->depth_bias += 3;
    } else {
        vis_table_a.types[10].scripts[0]->command = 0x70;
        vis_table_a.types[10].scripts[1]->command = 0x71;
        vis_table_a.types[10].scripts[2]->command = 0x72;
        e51->status.bytes[3] = 2;
        creature_start_script(e50, 1);
        e50->anim_step = 0;
        e50->status.bytes[3] = 2;
        creature_start_script(e51, 0);
        c28->state = GROUND_ITEM_HIDDEN;
        fdat_801ef924 = 0;
        fdat_801ef920 = 400;
        fdat_801ef92c = 0;
    }
    if (game_progress.event_flags.bit.f13 == 1) {
        fdat_801ef934 = 1;
        fdat_801ef930 = -1;
        fdat_801ef938 = 0xff;
    } else {
        fdat_801ef934 = 0;
        fdat_801ef930 = 600;
        fdat_801ef938 = 0;
    }
}

/*
 * The entry's move picker, creature_pick_move with one difference: the headings
 * stay as they are when no script was picked.
 */
ADDRESS(0x801edad8, 0x24c)
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
    if (!creature_pick_weighted_script())
        return;
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
void map_slide_ground_item(void);
void map_open_object(void);
void map_turn_view_during_script(void);
void map_hand_over_item(void);
void map_start_creature_pair_scene(void);
void map_walk_creature_off(void);

/*
 * The current entry's handler: kind 4 runs the item 0x12f scene once,
 * kind 0 steps the gate stages (story flag bits 10-12), kind 10 the scene
 * stages in fdat_801ef924, kind 11 holds its pose; then the animation
 * steps as creature_step_script does.
 */
ADDRESS(0x801edd24, 0x718)
void map_step_creature(void)
{
    CreatureTable *a = &vis_table_a;
    Creature *e48 = &a->entries[48];
    Creature *e50 = &a->entries[50];
    Creature *e = a->current;
    CreatureType *r = a->current_type;
    VECTOR pos;
    SVECTOR rot;
    u8 part;

    if (e->type == 4 && fdat_801ef93c == 1) {
        rot.vx = 0;
        rot.vy = -0x400;
        viewer.state = VIEWER_STATE_SCRIPTED;
        pos.vx = e48->position.vx - 0xa00;
        pos.vz = e48->position.vz;
        player_glide_to_pose(&pos, &rot);
        map_slide_ground_item();
        rot.vx = 0xc0;
        rot.vy = -0x500;
        player_glide_to_pose(&pos, &rot);
        map_open_object();
        map_turn_view_during_script();
        creature_talk(e, 0, 0);
        map_hand_over_item();
        creature_fade_out_despawn(e);
        fdat_801ef93c = 2;
        viewer.state = VIEWER_STATE_NORMAL;
        game_progress.event_flags.word |= 0x4000;
    }
    if (e->type == 0) {
        switch (game_progress.event_flags.word >> 10 & 7) {
        case 2:
            viewer.state = VIEWER_STATE_SCRIPTED;
            creature_force_talk(e, 0, 0);
            player_turn_to_creature(e);
            map_walk_creature_off();
            viewer.state = VIEWER_STATE_NORMAL;
            e->spawn_chance = 100;
            e->facing = 0x400;
            e->home_block = 0x31;
            e->position.vx = 0x40800;
            e->position.vy = 0x41000;
            e->position.vz = 0x38800;
            part = vis_records[49].region;
            e->home_offset_x = 0x800;
            e->home_offset_y = 0;
            e->home_offset_z = -0x800;
            e->region = part;
            game_progress.event_flags.word = (game_progress.event_flags.word & ~0x1c00) | 0xc00;
            break;
        case 3:
            if (viewer.block == 0x2f) {
                creature_force_talk(e, 0, 0);
                game_progress.event_flags.word = (game_progress.event_flags.word & ~0x1c00) | 0x1000;
            } else if (viewer.block == 0x16) {
                game_progress.talk_counts[r->talk_id]++;
                game_progress.event_flags.word = (game_progress.event_flags.word & ~0x1c00) | 0x1400;
            }
            break;
        case 4:
            if (viewer.block == 0x16)
                game_progress.event_flags.word = (game_progress.event_flags.word & ~0x1c00) | 0x1400;
            break;
        case 5:
            if (game_progress.event_flags.word >> 29 & 1) {
                game_progress.talk_counts[r->talk_id]++;
                game_progress.event_flags.word = (game_progress.event_flags.word & ~0x1c00) | 0x1800;
            } else {
                e->flags |= 0x2000000;
                creature_force_talk(e, 0, 0);
                e->status.bytes[3] = 2;
                game_progress.event_flags.word = (game_progress.event_flags.word & ~0x1c00) | 0x1800;
                e->flags &= ~0x2000000;
            }
            break;
        case 6:
            if (e->hit != 0) {
                e->flags |= 0x2000000;
                creature_talk(e, 0, 0);
                e->fade = -1;
                e->hurt_timer = -1;
                e->status.bytes[3] = 0;
                e->status.bytes[2] &= 0xe2;
                if (e->hp == 0)
                    e->hp = 1;
                game_progress.event_flags.word |= 0x1c00;
                e->flags &= ~0x2000000;
            }
            break;
        case 1:
        case 7:
            break;
        }
    }
    if (e->type == 10) {
        if (e->hit != 0) {
            map_start_creature_pair_scene();
            fdat_801ef924 = 3;
        }
        switch (fdat_801ef924) {
        case 0:
            if (viewer.block == 0xa4) {
                e50->fade = -1;
                e50->status.bytes[2] &= 0xf2;
                creature_force_talk(e, 0, 0);
                fdat_801ef924 = 1;
                e->flags &= ~0x2000000;
                game_progress.event_flags.word = (game_progress.event_flags.word & ~0x300) | 0x100;
            }
            break;
        case 1:
            if (fdat_801ef920 == 0) {
                map_start_creature_pair_scene();
                fdat_801ef924 = 3;
            } else {
                fdat_801ef920--;
            }
            break;
        case 3:
            if ((short)e->anim_frame + e->anim_step >= 0x1000) {
                creature_despawn(e);
                fdat_801ef924 = 4;
            }
            break;
        }
    }
    if (e->type == 11) {
        if (fdat_801ef924 == 3 && (short)e->anim_frame + e->anim_step >= 0x1000) {
            e->anim_frame = 0xfff;
            e->anim_step = 0;
        }
        if (fdat_801ef924 == 4) {
            if (fdat_801ef92c == 2)
                creature_start_script(e, 0);
            else
                creature_start_script(e, 1);
            e->anim_frame = 0xfff;
            e->anim_step = 0;
        }
    }
    e->anim_frame += e->anim_step;
    if (creature_step_hurt_timer(e) == 1)
        return;
    if (e->anim_step >= 0 ? (short)e->anim_frame >= 4096 : (short)e->anim_frame <= 0) {
        if (e->type == 4 && fdat_801ef93c == 0) {
            creature_start_script(e, 0);
            e->anim_step = 0;
        }
        if (e->type == 0) {
            if ((game_progress.event_flags.word >> 10 & 7) == 1) {
                creature_start_script(e, 0);
            } else {
                e->script_state = 0xff;
                map_pick_creature_move();
            }
        } else if (e->type == 10) {
            if (fdat_801ef924 < 2) {
                if (fdat_801ef924 < 0)
                    goto repeat_current_script;
                creature_start_script(e, 0);
            } else {
repeat_current_script:
                creature_start_script(e, e->script_index);
                return;
            }
        } else if (e->type == 11) {
            if (fdat_801ef92c == 2)
                creature_start_script(e, 0);
            else
                creature_start_script(e, 1);
            e->anim_step = 0;
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

void map_take_equipment_raise_object(void);
void map_play_two_creature_scripts(void);
void map_talk_then_remove_creatures(void);

/* Standing at table B entry 16 with item slot 3 below 50 and the scene not
 * yet run: walks up to it and plays the scene (scene stage 2). */
ADDRESS(0x801ee43c, 0x108)
void map_object_take_equipment(void)
{
    AreaObject *d = vis_table_b.current;
    Creature *e51 = &vis_table_a.entries[51];
    VECTOR pos;
    SVECTOR rot;

    if (d == &vis_table_b.entries[16] && viewer.equipment[EQUIP_SLOT_RIGHT_HAND] < 50 && d->state == 1
        && fdat_801ef92c == 0) {
        pos.vx = d->position.vx;
        pos.vz = d->position.vz - 0x600;
        rot.vx = 0;
        rot.vy = 0;
        viewer.state = VIEWER_STATE_SCRIPTED;
        player_glide_to_pose(&pos, &rot);
        map_take_equipment_raise_object();
        player_turn_to_creature(e51);
        map_play_two_creature_scripts();
        map_talk_then_remove_creatures();
        viewer.state = VIEWER_STATE_NORMAL;
        fdat_801ef924 = 2;
        fdat_801ef92c = 2;
        game_progress.event_flags.word = (game_progress.event_flags.word & ~0x300) | 0x200;
    }
}

/* Item 0x12f used in front of table A entry 0x30 sets fdat_801ef93c; the
 * result is 1 then, else 2. */
ADDRESS(0x801ee544, 0xcc)
void map_use_item_on_creature(short id)
{
    VECTOR p;
    CollisionRequest req;

    p.vx = viewer.position.vx;
    p.vy = viewer.position.vy + 1600;
    p.vz = viewer.position.vz;
    req.tests = 0;
    req.position = p;
    req.radius = 1056;
    req.height = 1600;
    req.velocity.vx = 0;
    req.velocity.vy = 0;
    req.velocity.vz = 0;
    req.speed = 0;
    req.heading = 0;
    if (id == 0x12f) {
        if (creature_find_in_reach((CollisionState *)&req, 1) == 0x30) {
            fdat_801ef93c = 1;
            DAT_801cd0e0 = 1;
        } else {
            DAT_801cd0e0 = 2;
        }
    }
}

/* Takes the item of slot 3, recomputes the figures and slots, raises table
 * B entry 16 and slides table C item 28 along. */
ADDRESS(0x801ee610, 0x1c4)
void map_take_equipment_raise_object(void)
{
    AreaObject *d16 = &vis_table_b.entries[16];
    u8 *speed = &d16->unk48;
    GroundItem *c28 = &vis_table_c.entries[28];
    Inventory *items = &inventory;
    ItemType *kind;
    int step;

    kind = &vis_table_c.types[c28->item];
    *(u8 *)&items->counts[viewer.equipment[EQUIP_SLOT_RIGHT_HAND]] = 0;
    viewer.equipment[EQUIP_SLOT_RIGHT_HAND] = EQUIP_NONE;
    player_compute_derived_stats(0);
    player_update_durability_warning();
    player_load_equipment_model(viewer.equipment[EQUIP_SLOT_RIGHT_HAND], 0, 0);
    player_load_equipment_model(viewer.equipment[EQUIP_SLOT_LEFT_HAND], 1, 0);
    d16->kind = OBJECT_KIND_GATE;
    d16->anim = 0;
    d16->anim_frame = 0;
    c28->state = GROUND_ITEM_SCRIPTED;
    c28->depth_bias += 3;
    for (;;) {
        StAnimFile *file;

        if (c28->position.vz > d16->position.vz - 0x300)
            goto item_placed;
        anim_get_file(kind->model + 588, &file);
        if (file != NULL)
            c28->position.vz += 36;
        world_run_frame(NULL, NULL, 0);
    }
item_placed:
    step = *speed * 4;
    for (;;) {
        int v;
        int next;

        object_play_sound(d16, d16->type * 3 + 0xd0);
        v = d16->anim_frame;
        next = v + step;
        if (v == 0xfff) {
            d16->state = 0xff;
            break;
        }
        if (next >= 0x1000)
            d16->anim_frame = 0xfff;
        else
            d16->anim_frame = next;
        c28->position.vz += 12;
        world_run_frame(NULL, NULL, 0);
    }
}

/* Entries 51 and 50 play animations 3 and 0 to the end (51 once 50 is half
 * through), table B entry 19 steps from state 11 to 12, and the view
 * pitches up 16 a frame for 16 frames. */
ADDRESS(0x801ee7d4, 0x174)
void map_play_two_creature_scripts(void)
{
    Creature *e51 = &vis_table_a.entries[51];
    AreaObject *d19 = &vis_table_b.entries[19];
    Creature *e50;
    int n;

    creature_start_script(e51, 3);
    e50 = e51 - 1;
    creature_start_script(e50, 0);
    e51->anim_frame = 0;
    e50->anim_frame = 0;
    do {
        creature_update_script_sound(e50);
        creature_update_script_sound(e51);
        e50->anim_frame += e50->anim_step;
        if ((short)e50->anim_frame > 0x800)
            e51->anim_frame += e51->anim_step;
        if ((short)e51->anim_frame >= 0x1000)
            e51->anim_frame = 0xfff;
        if ((short)e50->anim_frame >= 0x1000)
            e50->anim_frame = 0xfff;
        world_run_frame(NULL, NULL, 0);
    } while ((short)e51->anim_frame != 0xfff);
    if (d19->state == 11)
        d19->state = 12;
    n = 16;
    while (n--) {
        viewer.view_rot.vx += 16;
        viewer.pitch = viewer.view_rot.vx + viewer.rot_offset.halves[0] + viewer.shake_pitch + viewer.fall_pitch;
        world_run_frame(NULL, (SVECTOR *)&viewer.pitch, 0);
    }
}

/* Bumps record 10's stream counter, sends entry 51 away (its scripts 0..2
 * become 0x80, 5..7 0x70..0x72) and restarts entry 50. */
ADDRESS(0x801ee948, 0xcc)
void map_talk_then_remove_creatures(void)
{
    Creature *e = &vis_table_a.entries[51];
    CreatureType *r = &vis_table_a.types[10];

    game_progress.talk_counts[r->talk_id]++;
    e->flags |= 0x2000000;
    r->scripts[0]->command = 0x80;
    r->scripts[1]->command = 0x80;
    r->scripts[2]->command = 0x80;
    r->scripts[5]->command = 0x70;
    r->scripts[6]->command = 0x71;
    r->scripts[7]->command = 0x72;
    creature_talk(e, 0, 0);
    creature_fade_out_despawn(e);
    e--;
    creature_start_script(e, 0);
    creature_despawn(e);
}

/* Once: entry 51 plays animation 8 and entry 50 animation 1, record 11's
 * ranges are set and the scene stage becomes 1. */
ADDRESS(0x801eea14, 0xc4)
void map_start_creature_pair_scene(void)
{
    Creature *e51 = &vis_table_a.entries[51];
    Creature *e50 = e51 - 1;
    CreatureType *r = &vis_table_a.types[11];

    if (fdat_801ef928 == 0) {
        e51->status.bytes[3] = 1;
        fdat_801ef920 = -1;
        e51->flags &= ~0x2000000;
        creature_start_script(e51, 8);
        creature_start_script(e50, 1);
        fdat_801ef92c = 1;
        r->radius = 0x200;
        r->height = 0x800;
        fdat_801ef928 = 1;
        game_progress.event_flags.word = (game_progress.event_flags.word & ~0x300) | 0x100;
    }
}

/* Slides table C item 29 by 0x1ea, 24 a frame while its model is loaded. */
ADDRESS(0x801eead8, 0xdc)
void map_slide_ground_item(void)
{
    GroundItem *item = &vis_table_c.entries[29];
    ItemType *kind = &vis_table_c.types[303];
    int step = 24;
    int top;

    item->state = GROUND_ITEM_SCRIPTED;
    top = item->position.vx + 0x1ea;
    item->depth_bias -= 2;
    for (;;) {
        StAnimFile *file;

        if (item->position.vx == top)
            break;
        anim_get_file(kind->model + 588, &file);
        if (file != NULL)
            item->position.vx += step;
        if (item->position.vx > top)
            item->position.vx = top;
        world_run_frame(NULL, NULL, 0);
    }
    item->depth_bias += 2;
}

/* Raises table B entry 15 until its state is 0xfff, then marks it open. */
ADDRESS(0x801eebb4, 0xc0)
void map_open_object(void)
{
    AreaObject *d = &vis_table_b.entries[15];
    int step;

    d->kind = OBJECT_KIND_GATE;
    d->anim = 0;
    d->anim_frame = 0;
    step = d->unk48 * 4;
    do {
        if (d->anim_frame == 0)
            object_play_sound(d, d->type * 3 + 0xd0);
        d->anim_frame += step;
        if (d->anim_frame >= 0x1000)
            d->anim_frame = 0xfff;
        world_run_frame(NULL, NULL, 0);
    } while (d->anim_frame != 0xfff);
    d->state = 0xff;
}

/* Entry 49 plays animation 0 to its end while the view turns from its
 * pitch and yaw to (0, -0x400). */
ADDRESS(0x801eec74, 0x240)
void map_turn_view_during_script(void)
{
    SVECTOR from;
    SVECTOR to;
    Creature *e;
    int n;
    int total;

    from = viewer.view_rot;
    e = &vis_table_a.entries[49];
    to.vy = -0x400;
    to.vx = 0;
    if (from.vx > 2048)
        from.vx -= 4096;
    if (from.vy > 2048)
        from.vy -= 4096;
    creature_start_script(e, 0);
    n = 0x1000 / e->anim_step;
    total = n;
    do {
        creature_update_script_sound(e);
        e->anim_frame += e->anim_step;
        if ((short)e->anim_frame >= 0x1000)
            e->anim_frame = 0xfff;
        viewer.view_rot.vx = (from.vx * n + to.vx * (total - n)) / total;
        viewer.pitch = viewer.view_rot.vx + viewer.rot_offset.halves[0] + viewer.shake_pitch + viewer.fall_pitch;
        viewer.view_rot.vy = (from.vy * n + to.vy * (total - n)) / total;
        viewer.yaw = viewer.view_rot.vy + viewer.rot_offset.halves[1] + viewer.shake_yaw + viewer.unk292;
        world_run_frame(NULL, (SVECTOR *)&viewer.pitch, 0);
        n--;
    } while ((short)e->anim_frame != 0xfff);
    e->flags |= 0x2000000;
}

/*
 * Entry 49 plays animation 4 and lifts its arm a frame at a time; table C
 * item 10 (11 once 10 is taken) rides on vertex 0x160 until the arm is up.
 * Then the view settles and the item drops.
 */
ADDRESS(0x801eeeb4, 0x1c8)
void map_hand_over_item(void)
{
    Creature *e = &vis_table_a.entries[49];
    GroundItem *c10 = &vis_table_c.entries[10];
    GroundItem *c11 = &vis_table_c.entries[11];
    GroundItem *c;
    SVECTOR v;
    SVECTOR unused;

    creature_start_script(e, 4);
    c = c10;
    if (vis_table_c.collected_bits[c10->item / 32] & (1 << (c10->item % 32)))
        c = c11;
    do {
        creature_update_script_sound(e);
        e->anim_frame += e->anim_step;
        if ((s16)e->anim_frame >= 0x1000)
            e->anim_frame = 0xfff;
        anim_get_vertex(e->model_slot + 128, e->anim, (s16)e->anim_frame, 0x160, &v);
        c->position.vx = e->position.vx - v.vx;
        c->position.vy = e->position.vy + v.vy;
        c->position.vz = e->position.vz - v.vz;
        world_run_frame(NULL, &viewer.view_rot, 0);
    } while ((s16)e->anim_frame != 0xfff);
    c->state = GROUND_ITEM_RESTING;
    viewer.pitch = viewer.view_rot.vx + viewer.rot_offset.halves[0] + viewer.shake_pitch + viewer.fall_pitch;
    viewer.yaw = viewer.view_rot.vy + viewer.rot_offset.halves[1] + viewer.shake_yaw + viewer.unk292;
    viewer.roll = viewer.view_rot.vz + viewer.roll_offset + viewer.shake_roll + viewer.unk294;
    viewer.camera_rot.vx = 4096 - viewer.pitch;
    viewer.camera_rot.vy = 4096 - viewer.yaw;
    viewer.camera_rot.vz = 4096 - viewer.roll;
    RotMatrixZYX(&viewer.camera_rot, &viewer.camera_matrix);
    ground_item_pick_up(c, 1);
}

/* Entry 32 plays animation 9 (the ground shakes through its middle part
 * and it starts moving near the end), then walks off along x to 0x3f000. */
ADDRESS(0x801ef07c, 0x144)
void map_walk_creature_off(void)
{
    Creature *e = &vis_table_a.entries[32];

    creature_start_script(e, 9);
    do {
        creature_update_script_sound(e);
        e->anim_frame += e->anim_step;
        if ((short)e->anim_frame >= 0x1000)
            e->anim_frame = 0xfff;
        if (e->anim_frame > 0xa00 && e->anim_frame < 0xdfc)
            creature_turn_toward(0x400, 0x80);
        if ((short)e->anim_frame > 0xdfc)
            e->position.vx += 0x40;
        world_run_frame(NULL, NULL, 0);
    } while ((short)e->anim_frame != 0xfff);
    creature_start_script(e, 1);
    do {
        creature_update_script_sound(e);
        e->anim_frame += e->anim_step;
        if ((short)e->anim_frame > 0x1000)
            e->anim_frame = 0;
        e->position.vx += 0x40;
        world_run_frame(NULL, NULL, 0);
    } while (e->position.vx <= 0x3f000);
}

/*
 * Blends palette row 489 from row 487 by fdat_801ef51c / 32, a step a
 * call; at 0 loads row 487's copy and ends.
 */
ADDRESS(0x801ef1c0, 0x344)
void map_fade_palette_row(void)
{
    RECT dst = {0, 489, 256, 1};
    RECT src = {0, 487, 256, 1};
    ClutPair buf[128];
    int i;
    int t;
    int u;
    u32 flags;

    if (fdat_801ef51c == 32) {
        StoreImage(&dst, (u_long *)fdat_801ef520);
        StoreImage(&src, (u_long *)fdat_801ef720);
    }
    t = fdat_801ef51c;
    u = 32 - t;
    for (i = 0; i < 128; i++) {
        buf[i].b0 = (fdat_801ef520[i].b0 * t + fdat_801ef720[i].b0 * u) >> 5;
        buf[i].g0 = (fdat_801ef520[i].g0 * t + fdat_801ef720[i].g0 * u) >> 5;
        buf[i].r0 = (fdat_801ef520[i].r0 * t + fdat_801ef720[i].r0 * u) >> 5;
        buf[i].b1 = (fdat_801ef520[i].b1 * t + fdat_801ef720[i].b1 * u) >> 5;
        buf[i].g1 = (fdat_801ef520[i].g1 * t + fdat_801ef720[i].g1 * u) >> 5;
        buf[i].r1 = (fdat_801ef520[i].r1 * t + fdat_801ef720[i].r1 * u) >> 5;
        buf[i].s0 = fdat_801ef520[i].s0;
        buf[i].s1 = fdat_801ef520[i].s1;
    }
    LoadImage(&dst, (u_long *)buf);
    if (fdat_801ef51c == 0) {
        LoadImage(&dst, (u_long *)fdat_801ef720);
        flags = vis_lists.floor_kinds_off;
        fdat_801ef930 = -1;
        vis_lists.floor_kinds_off = flags | 0x10;
    } else {
        fdat_801ef51c--;
    }
}
