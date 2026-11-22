/*
 * FDAT_172.BIN as one translation unit (WIP module): its handler table,
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

/* 0 until item 0x12a is used (1), 0xff once its scene ran (flag 26). */
DATA(0x801edfd0, 0x4, ".bss")
static int fdat_801edfd0;

/* Entry 48: 0 asleep, 0xfd hit, 0xfe awake, 0xff gone (flags 29, 30). */
DATA(0x801edfd4, 0x4, ".bss")
static int fdat_801edfd4;

/* 0 until table A entry 33 is beaten, 1 then (flag 27), 0xff once it left. */
DATA(0x801edfd8, 0x4, ".bss")
static int fdat_801edfd8;

void map_restore_progress();
void map_update_events();
void map_pick_creature_move();
void map_step_creature();
void map_use_item_on_object(short id);
void map_talk_on_player_death();

const FdatHeader fdat_header = {
    0x16,
    {
        map_hook_none, map_update_events, map_hook_none, map_hook_none,
        map_hook_none, map_restore_progress, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_use_item_on_object, map_talk_on_player_death, map_hook_none, map_hook_none,
        map_pick_creature_move, map_step_creature, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
    },
    0x49,
};

#include "../lib/fdat_common_a.inc"
#include "../lib/fdat_common.inc"
#include "../lib/fdat_common_b.inc"

void map_slide_ground_item(void);
void creatures_restart_fade_in(void);

/* Sets up the area by flags 14, 26, 27, 29 and 30: table C item 28,
 * entry 48 (asleep, awake or gone), table B entry 6 and record 14. */
ADDRESS(0x801ed4a0, 0x174)
void map_restore_progress(void)
{
    Creature *e48 = &vis_table_a.entries[48];
    CreatureType *r1 = &vis_table_a.types[1];
    GroundItem *c28 = &vis_table_c.entries[28];
    CreatureType *r14 = &vis_table_a.types[14];
    AreaObject *d6 = &vis_table_b.entries[6];

    if (game_progress.event_flags.bit.f26 == 1) {
        c28->state = GROUND_ITEM_SCRIPTED;
        fdat_801edfd0 = 0xff;
        c28->position.vy += 0x200;
    } else {
        c28->state = GROUND_ITEM_HIDDEN;
        fdat_801edfd0 = 0;
    }
    if (e48->state != CREATURE_STATE_GONE && game_progress.event_flags.bit.f29 != 1
        && game_progress.event_flags.bit.f14) {
        if (game_progress.event_flags.bit.f30 == 1) {
            fdat_801edfd4 = 0xfe;
            e48->flags &= ~0x2000000;
        } else {
            fdat_801edfd4 = 0;
            e48->status.bytes[3] = 3;
            r1->scripts[12]->command = 0x72;
            r1->scripts[13]->command = 0x80;
        }
    } else {
        e48->spawn_chance = 0;
        fdat_801edfd4 = 0xff;
    }
    if (!game_progress.event_flags.bit.f27) {
        fdat_801edfd8 = 0;
        d6->kind = OBJECT_KIND_NONE;
    } else {
        fdat_801edfd8 = 0xff;
    }
    if (game_progress.talk_counts[r14->talk_id] != 0)
        r14->actions &= ~0x180;
}

/*
 * Runs the area: item 0x12a's scene (map_use_item_on_object) lowers table C item 28;
 * entry 48 wakes when hit or once record 1's stream counter is 2, and
 * changes its scripts once its flash is over; entry 33 is sent by the
 * viewer passing z 0x4a000 and, once beaten, changes the lighting and
 * leaves (flag 27).
 */
ADDRESS(0x801ed614, 0x328)
void map_update_events(void)
{
    Creature *e48 = &vis_table_a.entries[48];
    AreaObject *d6 = &vis_table_b.entries[6];
    CreatureType *r1 = &vis_table_a.types[1];
    Creature *e33 = &vis_table_a.entries[33];
    CreatureType *r14 = &vis_table_a.types[14];

    if (fdat_801edfd0 == 1) {
        map_slide_ground_item();
        creatures_restart_fade_in();
        fdat_801edfd0 = 0xff;
        game_progress.event_flags.word |= 0x4000000;
    }
    if (fdat_801edfd4 == 0) {
        if (game_progress.talk_counts[r1->talk_id] == 2) {
            creature_fade_out_despawn(e48);
            fdat_801edfd4 = 0xff;
        }
        if (e48->hit == 1) {
            fdat_801edfd4 = 0xfd;
            e48->status.bytes[2] &= 0xea;
            e48->flags &= ~0x2000000;
            game_progress.event_flags.word |= 0x40000000;
        }
    }
    if (fdat_801edfd4 == 0xfd && e48->hurt_timer == -1) {
        game_progress.talk_counts[r1->talk_id] = 2;
        r1->scripts[12]->command = 0x80;
        r1->scripts[13]->command = 0x72;
        e48->flags |= 0x2000000;
        creature_force_talk(e48, 0, 0);
        fdat_801edfd4 = 0xfe;
        e48->status.bytes[3] = 0;
        e48->flags &= ~0x2000000;
    }
    if (e48->hp == 0)
        game_progress.event_flags.word |= 0x20000000;
    if (game_progress.talk_counts[r14->talk_id] == 0 && viewer.position.vz > 0x4a000) {
        viewer.state = VIEWER_STATE_SCRIPTED;
        creature_force_talk(e33, 0, 0);
        e33->hit = 1;
        viewer.state = VIEWER_STATE_NORMAL;
        e33->flags &= ~0x2000000;
        r14->actions &= ~0x180;
    }
    if (fdat_801edfd8 == 0 && e33->hp == 0) {
        fdat_801edfd8 = 1;
        lighting.area_mode[2] = 0x11;
        lighting.area_mode[17] = 0x11;
        game_progress.event_flags.word |= 0x8000000;
    }
    if (fdat_801edfd8 == 1 && e33->state == CREATURE_STATE_GONE) {
        viewer.state = VIEWER_STATE_SCRIPTED;
        e33->flags |= 0x2000000;
        creature_force_talk(e33, 1, 0);
        fdat_801edfd8 = 0xff;
        d6->kind = OBJECT_KIND_WARP;
        viewer.state = VIEWER_STATE_NORMAL;
    }
}

/*
 * The entry's move picker, creature_pick_move with two additions: kind 14 waits
 * (action 128) while its record's stream counter is 0, and kind 1 only
 * plays animation 9 while fdat_801edfd4 is 0 or 253.
 */
ADDRESS(0x801ed93c, 0x2bc)
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
    if (e->type == 14 && game_progress.talk_counts[r->talk_id] == 0)
        e->action = CREATURE_ACTION_DORMANT;
    if (e->type == 1 && (fdat_801edfd4 == 0 || fdat_801edfd4 == 253)) {
        creature_start_script(e, 9);
        return;
    }
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

/* The current entry's handler: entries below 56 other than kind 1 are kept
 * in range (unk03 3) until the item scene ran; then the animation steps as
 * creature_step_script does, kind 1 replaying animation 9 while entry 48 sleeps. */
ADDRESS(0x801edbf8, 0x140)
void map_step_creature(void)
{
    Creature *e = vis_table_a.current;

    if (fdat_801edfd0 != 0xff && e->type != 1 && e < &vis_table_a.entries[56]
        && e->state != CREATURE_STATE_DYING)
        e->visibility = 3;
    else
        e->visibility = 0;
    e->anim_frame += e->anim_step;
    if (creature_step_hurt_timer(e) == 1)
        return;
    if (e->anim_step >= 0 ? (short)e->anim_frame >= 4096 : (short)e->anim_frame <= 0) {
        if (e->type == 1 && (fdat_801edfd4 == 0 || fdat_801edfd4 == 0xfd)) {
            creature_start_script(e, 9);
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

/* Item 0x12a used in front of table B entry 12 sets fdat_801edfd0; the
 * result is 1 then, else 2. */
ADDRESS(0x801edd38, 0xc8)
void map_use_item_on_object(short id)
{
    VECTOR p;
    CollisionRequest req;

    if (id == 0x12a) {
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
        if (object_find_in_reach((CollisionState *)&req) == 12) {
            game_progress.item_use_result = 1;
            fdat_801edfd0 = 1;
        } else {
            game_progress.item_use_result = 2;
        }
    }
}

/* Moves table C item 28 0x200 down (y grows downwards), 24 a frame while
 * its model is loaded. */
ADDRESS(0x801ede00, 0xf8)
void map_slide_ground_item(void)
{
    int step = 24;
    GroundItem *item = &vis_table_c.entries[28];
    ItemType *kind;
    int top;

    item->state = GROUND_ITEM_SCRIPTED;
    kind = &vis_table_c.types[item->item];
    top = item->position.vy + 0x200;
    item->depth_bias -= 2;
    for (;;) {
        StAnimFile *file;

        if (item->position.vy == top)
            break;
        anim_get_file(kind->model + MODEL_SLOT_ITEMS, &file);
        if (file != NULL)
            item->position.vy += step;
        if (item->position.vy > top)
            item->position.vy = top;
        world_run_frame(NULL, NULL, 0);
    }
    item->depth_bias += 2;
}

/* Marks every active table A entry: unk36 0xfff and range bits 0xd. */
ADDRESS(0x801edef8, 0x50)
void creatures_restart_fade_in(void)
{
    Creature *e = vis_table_a.entries;
    int i = 64;

    while (i--) {
        if (e->state == CREATURE_STATE_ACTIVE) {
            e->fade = 0xfff;
            e->status.bytes[2] |= 0xd;
        }
        e++;
    }
}

/* In area 3, while flag 27 is clear, hands table A entry 33 to its exit once
 * its record's stream counter has started. */
ADDRESS(0x801edf48, 0x88)
void map_talk_on_player_death(void)
{
    Creature *e;
    CreatureType *r;

    e = &vis_table_a.entries[33];
    r = &vis_table_a.types[14];

    if (game_progress.talk_counts[r->talk_id] != 0
        && !(game_progress.event_flags.word >> 27 & 1) && vis_table_a.zone == 3) {
        game_progress.talk_counts[r->talk_id] = 2;
        e->flags |= 0x2000000;
        creature_force_talk(e, 1, 1);
    }
}
