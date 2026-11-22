/*
 * FDAT_302.BIN as one translation unit (WIP module): its handler table,
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
#include <st/effect.h>
#include <st/pad.h>
#include <st/bank.h>

RODATA(0x801eb560, 0x24c)

/* 1 while table B entries 1..6 are all open (flags 16, 18, 20, 24, 27,
 * 28). */
DATA(0x801ee46c, 0x4, ".bss")
static int fdat_801ee46c;

/* The viewer's progress through the hall (0..3). */
DATA(0x801ee470, 0x4, ".bss")
static int fdat_801ee470;

/* Entry 0's dying scene: 0, 1 dying, 2 due, 0xff done. */
DATA(0x801ee474, 0x4, ".bss")
static int fdat_801ee474;

/* Table A entry 3 beaten: 0, 1 this visit, 0xff gone. */
DATA(0x801ee478, 0x4, ".bss")
static int fdat_801ee478;

/* Table A entry 5's walk: 0, 1 walking, 0xff gone. */
DATA(0x801ee47c, 0x4, ".bss")
static int fdat_801ee47c;

/* Table A entry 5's stage: 0, 1 hit, 2 fleeing, 3 beaten, 0xff gone. */
DATA(0x801ee480, 0x4, ".bss")
static int fdat_801ee480;

void map_restore_progress();
void map_update_events();
void map_step_creature();
void map_talk_on_player_death();

const FdatHeader fdat_header = {
    0x23,
    {
        map_hook_none, map_update_events, map_hook_none, map_hook_none,
        map_hook_none, map_restore_progress, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_talk_on_player_death, map_hook_none, map_hook_none,
        creature_pick_move, map_step_creature, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
    },
    0x56,
};

#include "../lib/fdat_common_a.inc"
#include "../lib/fdat_common.inc"
#include "../lib/fdat_common_b.inc"

void map_shift_player_and_creatures(int which);
void map_warp_player(void);
void map_set_block_zones(u8 zone);
void map_transform_creature(void);

/* Sets up the area: table B entries 1..6 closed unless their flags are
 * set (entry 9 off when all are open), entries 4 and 5 placed, the scene
 * words cleared. */
ADDRESS(0x801ed4a0, 0x110)
void map_restore_progress(void)
{
    AreaObject *d1 = &vis_table_b.entries[1];
    AreaObject *d9 = &vis_table_b.entries[9];
    Creature *e4 = &vis_table_a.entries[4];
    Creature *e5 = &vis_table_a.entries[5];
    u32 w = game_progress.event_flags.word;

    fdat_801ee46c = 1;
    if (!(w >> 16 & 1)) {
        d1[0].visibility = 2;
        fdat_801ee46c = 0;
    }
    if (!(w >> 18 & 1)) {
        d1[1].visibility = 2;
        fdat_801ee46c = 0;
    }
    if (!(w >> 20 & 1)) {
        d1[2].visibility = 2;
        fdat_801ee46c = 0;
    }
    if (!(w >> 24 & 1)) {
        d1[3].visibility = 2;
        fdat_801ee46c = 0;
    }
    if (!(w >> 27 & 1)) {
        d1[4].visibility = 2;
        fdat_801ee46c = 0;
    }
    if (!(w >> 28 & 1)) {
        d1[5].visibility = 2;
        fdat_801ee46c = 0;
    }
    if (fdat_801ee46c)
        d9->kind = OBJECT_KIND_NONE;
    e4->zone = 1;
    e5->zone = 2;
    fdat_801ee474 = 0;
    fdat_801ee478 = 0;
    fdat_801ee47c = 0;
    fdat_801ee480 = 0;
}

/*
 * Runs the area: with all doors open, table B entry 9's square starts the
 * hall scene; the viewer crossing z 0x46800 / 0x40800 moves the hall
 * along; entries 0, 3 and 5 are kept in range by their distance; entry 0
 * dying starts its scene, entries 3 and 5 leave once beaten.
 */
ADDRESS(0x801ed5b0, 0x46c)
void map_update_events(void)
{
    AreaObject *d9 = &vis_table_b.entries[9];
    Creature *e0 = &vis_table_a.entries[0];
    Creature *e3 = &vis_table_a.entries[3];
    Creature *e5 = &vis_table_a.entries[5];

    if (fdat_801ee46c != 0
        && player_in_centred_rect(d9->position.vx, d9->position.vz, 0xc00, 0xc00, d9->position.vy)) {
        map_warp_player();
        fdat_801ee470 = 1;
    }
    if (viewer.position.vz > 0x46800) {
        map_shift_player_and_creatures(0);
        fdat_801ee470++;
    }
    if (viewer.position.vz < 0x40800) {
        map_shift_player_and_creatures(1);
        fdat_801ee470--;
    }
    if (e0->player_dist_xz >= 0x6000)
        e0->visibility = 2;
    if (e3->player_dist_xz >= 0x6000)
        e3->visibility = 2;
    if (e5->player_dist_xz >= 0x6000)
        e5->visibility = 2;
    if (e0->player_dist_xz <= 0x6000)
        e0->visibility = 0;
    if (e3->player_dist_xz <= 0x6000)
        e3->visibility = 0;
    if (e5->player_dist_xz <= 0x6000)
        e5->visibility = 0;
    if (fdat_801ee478 != 0xff && fdat_801ee470 >= 2)
        fdat_801ee470 = 1;
    if (e0->state == CREATURE_STATE_DYING) {
        e0->state = CREATURE_STATE_ACTIVE;
        e0->status.bytes[3] = 1;
        e0->hurt_timer = -1;
        e0->status.bytes[2] &= 0xea;
        creature_start_script(e0, 6);
        fdat_801ee474 = 1;
    }
    if (fdat_801ee474 == 2)
        map_transform_creature();
    if (fdat_801ee478 == 0 && e3->hp == 0)
        fdat_801ee478 = 1;
    if (fdat_801ee478 == 1 && e3->state == CREATURE_STATE_GONE) {
        e3->flags |= 0x2000000;
        creature_force_talk(e3, 1, 0);
        fdat_801ee478 = 0xff;
    }
    if (fdat_801ee480 == 0 && e5->hp != (e5->type + vis_table_a.types)->max_hp)
        fdat_801ee480 = 1;
    if (fdat_801ee480 == 2 && e5->hp == 0)
        fdat_801ee480 = 3;
    if (fdat_801ee480 == 1) {
        e5->flags |= 0x2000000;
        creature_force_talk(e5, 1, 0);
        fdat_801ee480 = 2;
        e5->flags &= ~0x2000000;
    }
    if (fdat_801ee480 == 3 && e5->state == CREATURE_STATE_GONE) {
        e5->flags |= 0x2000000;
        creature_force_talk(e5, 1, 0);
        fdat_801ee480 = 0xff;
        game_exit_request = 3;
        game_progress.event_flags2.word |= 0x80;
    }
    if (fdat_801ee47c == 1 && viewer.position.vz > 0x41000) {
        viewer.state = VIEWER_STATE_SCRIPTED;
        creature_force_talk(e5, 0, 0);
        e5->status.bytes[3] = 0;
        fdat_801ee47c = 0xff;
        viewer.state = VIEWER_STATE_NORMAL;
        e5->flags &= ~0x2000000;
    }
    if (fdat_801ee47c == 0 && fdat_801ee470 == 3) {
        map_set_block_zones(2);
        e5->spawn_chance = 100;
        e5->status.bytes[3] = 1;
        fdat_801ee47c = 1;
        e5->flags |= 0x2000000;
    }
}

#include "../lib/fdat_creature_pick_move.inc"

/* The current entry's handler: the animation steps as creature_step_script does;
 * kind 0 at the end of entry 0's dying stage turns to its scripts 7 and 9
 * and leaves (scene word 2). */
ADDRESS(0x801edc60, 0x13c)
void map_step_creature(void)
{
    Creature *e = vis_table_a.current;
    CreatureType *r = vis_table_a.current_type;

    e->anim_frame += e->anim_step;
    if (creature_step_hurt_timer(e) == 1)
        return;
    if (e->anim_step >= 0 ? (short)e->anim_frame >= 4096 : (short)e->anim_frame <= 0) {
        if (e->type == 0 && fdat_801ee474 == 1) {
            e->flags |= 0x2000000;
            r->scripts[7]->command = 0x72;
            r->scripts[9]->command = 0x70;
            r->scripts[9]->anim_step = -0xc0;
            creature_force_talk(e, 0, 0);
            fdat_801ee474 = 2;
        } else {
            e->script_state = 0xff;
            creature_pick_move();
        }
    }
    if (e->script != NULL) {
        creature_run_script_command(e);
        e->script_state = 0xf0;
    }
}

/* Moves the viewer, every live table A entry and every live effect 0x6000
 * along z (`up`) or back. */
ADDRESS(0x801edd9c, 0x104)
void map_shift_player_and_creatures(int up)
{
    Creature *e = vis_table_a.entries;
    StEffect *fx = effects.slots;
    int dz = 0x6000;
    int i;

    if (up == 0)
        dz = -0x6000;
    viewer.position.vz += dz;
    i = 64;
    while (i--) {
        if (e->spawn_chance != 0xff) {
            e->position.vz += dz;
            creature_move(e);
        }
        e++;
    }
    i = ST_EFFECT_SLOTS;
    while (i--) {
        if (fx->flags != ST_EFFECT_FREE)
            fx->position.vz += dz;
        fx++;
    }
}

void fade_run_frames(int arg0, int arg1, int arg2, int arg3);
void creatures_update(void);
void player_enter_normal_state(void);

/* The hall scene: fades out, puts the viewer at the hall's start, fades
 * in with table A entry 0 awake (loading its model if needed); unless
 * record 0's stream counter has started it is handed to its exit. */
ADDRESS(0x801edea0, 0x1f0)
void map_warp_player(void)
{
    Creature *e = vis_table_a.entries;
    int index = e->type;
    CreatureType *r = &vis_table_a.types[index];
    int slot = e->model_slot + 128;

    play_warp_sound();
    fade_run_frames(2, 0, 0x1000, 0x100);
    creatures_bind_previous_zone();
    viewer.position.vx = 0x45800;
    viewer.position.vy = 0x3f9c0;
    viewer.position.vz = 0x40000;
    viewer.view_rot.vy = 0;
    viewer.view_rot.vx = 0;
    viewer.state = VIEWER_STATE_NORMAL;
    viewer.shake_roll = 0;
    viewer.shake_yaw = 0;
    viewer.shake_pitch = 0;
    viewer.rotation = viewer.view_rot;
    viewer.state = VIEWER_STATE_SCRIPTED;
    fade_run_frames(2, 0x1000, 0x1000, 0);
    creatures_update();
    creature_spawn_now(e);
    if (world_state.model_files[slot] == NULL)
        creature_model_load(ARCHIVE_CHR, index * 5, slot, e->vram_tag & 0xf);
    e->fade = -1;
    e->status.bytes[2] &= 0xf2;
    creature_start_script(e, 7);
    vis_table_a.zone = 0;
    cd_wait_idle();
    play_warp_sound();
    fade_run_frames(2, 0x1000, 0, -0x100);
    pad_clear_input();
    if (game_progress.talk_counts[r->talk_id] == 0) {
        creature_force_talk(e, 0, 0);
        e->flags &= ~0x2000000;
    }
    vis_table_a.current = e;
    vis_table_a.current_type = r;
    creature_pick_move();
    player_enter_normal_state();
}

/* Sets the zone of all 512 map-block records (vis_records). */
ADDRESS(0x801ee090, 0x28)
void map_set_block_zones(u8 zone)
{
    MapBlock *r = vis_records;
    int i = 512;

    while (i--) {
        r->zone = zone;
        r++;
    }
}

/* Entry 0 finishes animation 10; entry 48 takes its place and goes, entry
 * 3 rises there (animation 10) and entry 49 replaces 48 until 3 is up. */
ADDRESS(0x801ee0b8, 0x2c4)
void map_transform_creature(void)
{
    Creature *e0;
    Creature *e48;
    Creature *e49;
    Creature *e3;
    CreatureType *r2;
    StAnimFile *file;

    viewer.state = VIEWER_STATE_SCRIPTED;
    e0 = vis_table_a.entries;
    e48 = &vis_table_a.entries[48];
    e49 = &vis_table_a.entries[49];
    e3 = &vis_table_a.entries[3];
    r2 = &vis_table_a.types[2];
    creature_start_script(e0, 10);
    for (;;) {
        e0->anim_frame += e0->anim_step;
        if ((short)e0->anim_frame >= 0x1000)
            goto first_animation_done;
        world_run_frame(NULL, NULL, 0);
    }
first_animation_done:
    e48->position = e0->position;
    e48->rotation = e0->rotation;
    e48->block = e0->block;
    creature_despawn(e0);
    viewer.creatures_killed--;
    creature_spawn_now(e3);
    e3->position = e48->position;
    e3->rotation = e48->rotation;
    e3->fade = -1;
    e3->block = e48->block;
    e3->status.bytes[2] &= 0xf2;
    e3->flags &= ~0x2000000;
    creature_start_script(e3, 10);
    do {
        e3->anim_frame = 0;
        world_run_frame(NULL, NULL, 0);
        anim_get_file(e3->model_slot + MODEL_SLOT_CREATURES, &file);
    } while (file == NULL);
    e49->position = e3->position;
    e49->rotation = e3->rotation;
    e49->block = e3->block;
    r2->scripts[0]->anim_step = 0x90;
    creature_start_script(e49, 0);
    creature_despawn(e48);
    for (;;) {
        e49->anim_frame += e49->anim_step;
        e3->anim_frame += e3->anim_step;
        if ((short)e49->anim_frame >= 0x1000)
            e49->anim_frame = 0xfff;
        if ((short)e3->anim_frame >= 0x1000)
            break;
        creature_update_script_sound(e49);
        creature_update_script_sound(e3);
        world_run_frame(NULL, NULL, 0);
    }
    creature_despawn(e49);
    viewer.state = VIEWER_STATE_NORMAL;
    fdat_801ee474 = 0xff;
}

/* While record 0's stream counter has started: wakes entry 0 (counter 2)
 * until its scene ran, else entry 3 (record 3's counter 1) until it is
 * gone, else entry 5 (record 8's counter 3) until it is gone. */
ADDRESS(0x801ee37c, 0xf0)
void map_talk_on_player_death(void)
{
    Creature *e0 = vis_table_a.entries;
    Creature *e3 = &vis_table_a.entries[3];
    CreatureType *r3 = &vis_table_a.types[3];
    Creature *e5 = &vis_table_a.entries[5];
    CreatureType *r8 = &vis_table_a.types[8];
    GameProgress *story = &game_progress;

    if (story->talk_counts[vis_table_a.types[0].talk_id] == 0)
        return;
    if (fdat_801ee474 != 0xff) {
        story->talk_counts[vis_table_a.types[0].talk_id] = 2;
        e0->flags |= 0x2000000;
        creature_force_talk(e0, 1, 1);
    } else if (fdat_801ee478 != 0xff) {
        story->talk_counts[r3->talk_id] = 1;
        e3->flags |= 0x2000000;
        creature_force_talk(e3, 1, 1);
    } else if (story->talk_counts[r8->talk_id] != 0 && fdat_801ee480 != 0xff) {
        story->talk_counts[r8->talk_id] = 3;
        e5->flags |= 0x2000000;
        creature_force_talk(e5, 1, 1);
    } else {
        return;
    }
}
