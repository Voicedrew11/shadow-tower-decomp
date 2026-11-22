/*
 * FDAT_282.BIN: its handlers after creature_pick_move (WIP module): the step of
 * table A entries 19 and 48..50, and the two door scenes.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <psyq/libc.h>
#include <st/vis.h>
#include <st/data.h>
#include <st/hud.h>
#include <st/menu.h>
#include <st/xa.h>
#include <st/fdat.h>

void map_talk_then_open_door(int id);
void map_open_door_then_play_script(void);

/*
 * Steps table A entries 19 and 48..50: 48 and 49 open their doors when the
 * viewer stands on the trigger records, 50 opens the gate, 19 counts its
 * hits; then runs the script like creature_step_script.
 */
ADDRESS(0x801ed7a4, 0x274)
void map_step_creature(void)
{
    Creature *e = vis_table_a.current;
    CreatureType *r = vis_table_a.current_type;
    Creature *a48 = &vis_table_a.entries[48];
    Creature *a50 = &vis_table_a.entries[50];
    Creature *a19 = &vis_table_a.entries[19];
    Creature *a49 = &vis_table_a.entries[49];

    if (e == a48) {
        if (viewer.block == 0x55 || viewer.block == 0x29) {
            map_talk_then_open_door(0x55);
            creature_despawn(e);
            return;
        }
        if (viewer.block == 0x3b)
            e->facing = 0x800;
        else if (viewer.block == 0x13d)
            e->facing = 0;
    }
    if (e == a50 && viewer.block == 0x65) {
        map_open_door_then_play_script();
        creature_despawn(e);
        creature_despawn(a48);
        creature_despawn(a49);
        return;
    }
    if (e == a19) {
        if (e->hit != 0) {
            e->status.bytes[3] = 0;
            game_progress.talk_counts[r->talk_id]++;
        }
        if (game_progress.talk_counts[r->talk_id] == 4)
            e->flags &= ~0x2000000;
    }
    if (e == a49 && viewer.block == 0x61) {
        map_talk_then_open_door(0x61);
        creature_despawn(e);
        return;
    }
    e->anim_frame += e->anim_step;
    if (creature_step_hurt_timer(e) == 1)
        return;
    if (e->anim_step >= 0 ? (short)e->anim_frame >= 4096 : (short)e->anim_frame <= 0) {
        if (e == a48 || e == a49)
            creature_start_script(e, 0);
        else if (e == a50)
            creature_start_script(e, 4);
        else if (e == a19 && game_progress.talk_counts[r->talk_id] == 3)
            creature_start_script(e, 0);
        else {
            e->script_state = 0xff;
            creature_pick_move();
        }
    }
    if (e->script != NULL) {
        creature_run_script_command(e);
        e->script_state = 0xf0;
    }
}

ADDRESS(0x801eda18, 0x8)
void map_object_hook_none(void)
{
}

/*
 * Opens door 26 of table B: raises it to 0xfff, dropping item 2 on the way.
 */
ADDRESS(0x801eda20, 0x184)
void map_open_object_drop_item(void)
{
    int step;
    VECTOR pos;
    SVECTOR rot;
    AreaObject *door = &vis_table_b.entries[26];
    GroundItem *item = &vis_table_c.entries[2];

    viewer.state = VIEWER_STATE_SCRIPTED;
    rot.vy = -1024;
    rot.vx = 0;
    pos.vx = door->position.vx - 3072;
    pos.vz = door->position.vz;
    player_glide_to_pose(&pos, &rot);
    step = 28;
    do {
        if (door->anim_frame < 0xf00 && door->anim_frame + step >= 0xf00)
            object_play_sound(door, door->type * 3 + 208);
        door->anim_frame += step;
        if (door->anim_frame >= 0x1000)
            door->anim_frame = 0xfff;
        if (door->anim_frame > 0x400 && item->state == 4)
            item->state = 2;
        if (door->anim_frame > 0xf00 && item->state == 2)
            item->state = 1;
        ground_items_update();
        world_run_frame(NULL, NULL, 0);
    } while (door->anim_frame != 0xfff);
    door->state = 0xff;
    viewer.state = VIEWER_STATE_NORMAL;
}

/*
 * The door scene of table A entry 48 (`id` 0x55, door 2) or 49 (door 1):
 * plays the entry's opening script, then raises the door and swaps its
 * records.
 */
ADDRESS(0x801edba4, 0x2e4)
void map_talk_then_open_door(int id)
{
    Creature *e;
    AreaObject *door;
    CreatureType *r;
    u16 step;
    u16 n;
    u8 t;

    if (id == 0x55) {
        e = &vis_table_a.entries[48];
        door = &vis_table_b.entries[2];
    } else {
        e = &vis_table_a.entries[49];
        door = &vis_table_b.entries[1];
    }
    r = &vis_table_a.types[e->type];
    if (id == 0x55)
        game_progress.talk_counts[r->talk_id] = 0;
    else
        game_progress.talk_counts[r->talk_id] = 1;
    step = door->unk48 * 4;
    r->scripts[0]->command = 0x70;
    r->scripts[1]->command = 0x71;
    r->scripts[2]->command = 0x72;
    r->scripts[5]->command = 0x80;
    r->scripts[6]->command = 0x80;
    r->scripts[7]->command = 0x80;
    creature_force_talk(e, 0, 0);
    game_progress.talk_counts[r->talk_id] = 3;
    creature_start_script(e, 3);
    do {
        e->anim_frame += e->anim_step;
        if ((short)e->anim_frame >= 0x1000)
            e->anim_frame = 0xfff;
        if ((short)e->anim_frame + e->anim_step >= 0x800 && (short)e->anim_frame < 0x800) {
            viewer.unk190[4][0] = 0;
            viewer.unk190[4][1] = 6000;
            viewer.status_flags |= STATUS_CURSED;
        }
        world_run_frame(NULL, NULL, 0);
    } while ((short)e->anim_frame != 0xfff);
    for (;;) {
        if (door->anim_frame < 1 && door->anim_frame + step >= 1)
            object_play_sound(door, door->type * 3 + 208);
        door->anim_frame += step;
        if (door->anim_frame + step >= 0x1000)
            break;
        world_run_frame(NULL, NULL, 0);
    }
    door->anim_frame = 0xfff;
    door->visibility = 0;
    door->state = 0xff;
    n = door->params.door.n_blocks;
    while (n--) {
        t = vis_records[door->params.door.block_b + n].part;
        vis_records[door->params.door.block_b + n].part = vis_records[door->params.door.block_a + n].part;
        vis_records[door->params.door.block_a + n].part = t;
    }
}

/*
 * The gate scene of table A entry 50: swaps door 3's records, raises it and
 * plays the entry's script to its end.
 */
ADDRESS(0x801ede88, 0x228)
void map_open_door_then_play_script(void)
{
    int raising_step;
    Creature *e = &vis_table_a.entries[50];
    CreatureType *r = &vis_table_a.types[e->type];
    AreaObject *door = &vis_table_b.entries[3];
    u16 step = door->unk48 * 4;
    u16 n;
    u8 t;

    r->scripts[0]->command = 0x80;
    r->scripts[1]->command = 0x80;
    r->scripts[2]->command = 0x80;
    r->scripts[5]->command = 0x70;
    r->scripts[6]->command = 0x71;
    r->scripts[7]->command = 0x72;
    game_progress.talk_counts[r->talk_id] = 2;
    creature_force_talk(e, 0, 0);
    n = door->params.door.n_blocks;
    while (n--) {
        t = vis_records[door->params.door.block_b + n].part;
        vis_records[door->params.door.block_b + n].part = vis_records[door->params.door.block_a + n].part;
        vis_records[door->params.door.block_a + n].part = t;
    }
    /* The look-ahead test uses the promoted halfword step. */
    raising_step = step;
    for (;;) {
        door->anim_frame += step;
        if (door->anim_frame + raising_step >= 0x1000)
            goto gate_raised;
        world_run_frame(NULL, NULL, 0);
    }
gate_raised:
    door->anim_frame = 0xfff;
    door->visibility = 0;
    door->state = 0xff;
    creature_start_script(e, 8);
    do {
        e->anim_frame += e->anim_step;
        if ((short)e->anim_frame >= 0x1000)
            e->anim_frame = 0xfff;
        world_run_frame(NULL, NULL, 0);
    } while ((short)e->anim_frame != 0xfff);
    game_progress.talk_counts[r->talk_id] = 3;
}
