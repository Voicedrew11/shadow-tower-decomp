/*
 * FDAT_242.BIN as one translation unit (WIP module): its handler table,
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

RODATA(0x801eb560, 0x28c)

/* Frames the texture animation has run (map_wind_down_texanims). */
DATA(0x801ee0b0, 0x4, ".bss")
static int fdat_801ee0b0;

/* The exit scene state: 3 once the animation ends, 4 played. */
DATA(0x801ee0b4, 0x4, ".bss")
int fdat_801ee0b4;

/* The gate scene state: 2 armed, 3 opened by using item 0x10e. */
DATA(0x801ee0b8, 0x4, ".bss")
int fdat_801ee0b8;

/* The texture animation: 1 to start, 2 to move the finished pages, 0 done. */
DATA(0x801ee0bc, 0x4, ".bss")
int fdat_801ee0bc;

void map_restore_progress();
void map_update_events();
void map_use_item_advance_scene(short id);
void map_step_creature();
void map_talk_on_player_death();

const FdatHeader fdat_header = {
    0x1d,
    {
        map_hook_none, map_update_events, map_hook_none, map_hook_none,
        map_hook_none, map_restore_progress, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_use_item_advance_scene, map_talk_on_player_death, map_hook_none, map_hook_none,
        creature_pick_move, map_step_creature, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
    },
    0x50,
};

#include "../lib/fdat_common_a.inc"
#include "../lib/fdat_common.inc"
#include "../lib/fdat_common_b.inc"

void map_wind_down_texanims(void);
void map_move_texture_pages(void);

/* Sets up the area by flag 17 and the stream counters of records 12 and
 * 13: entries 48..50, the gate scene stage fdat_801ee0b8 and table B
 * entry 2. */
ADDRESS(0x801ed4e0, 0x23c)
void map_restore_progress(void)
{
    Creature *e48 = &vis_table_a.entries[48];
    Creature *e50 = &vis_table_a.entries[50];
    Creature *e49 = &vis_table_a.entries[49];
    Vis88 *d2 = &vis_table_b.entries[2];
    int n;

    if (game_progress.unk1440.word >> 17 & 1) {
        fdat_801ee0bc = 2;
        e48->state = 3;
        fdat_801ee0b4 = 4;
    } else {
        fdat_801ee0bc = 1;
        texanim_clear_all();
        fdat_801ee0b4 = 0;
        e48->status.bytes[3] = 2;
        vis_table_a.entries[33].spawn_chance = 0;
    }
    n = game_progress.unk1448[13];
    if (n == 0)
        fdat_801ee0b8 = 0;
    else if (n == 1) {
        if (game_progress.unk1448[12] == 0)
            fdat_801ee0b8 = n;
        else
            fdat_801ee0b8 = 2;
    } else if (n == 2)
        fdat_801ee0b8 = 3;
    else {
        fdat_801ee0b8 = 4;
        d2->unk4A.door.unk4F = 0xff;
    }
    if (game_progress.unk1448[13] >= 2)
        e50->flags &= ~0x2000000;
    if (game_progress.unk1448[13] == 3)
        e49->flags &= ~0x2000000;
    if (game_progress.unk1448[12] == 1)
        e48->flags &= ~0x2000000;
    if (fdat_801ee0b8 == 2) {
        fdat_801ee0b8 = 3;
        game_progress.unk1448[13]++;
    }
    switch (fdat_801ee0b8) {
    case 0:
        e50->spawn_chance = 100;
        e49->spawn_chance = 0;
        break;
    case 3:
        e50->spawn_chance = 0;
        e49->spawn_chance = 100;
        break;
    case 1:
        e50->home_facing = (e50->home_facing + 0x800) & 0xfff;
    case 4:
        e50->spawn_chance = 0;
        e49->spawn_chance = 0;
        break;
    }
    e49->status.bytes[3] = 1;
    e50->status.bytes[3] = 1;
}

/*
 * Runs the area: table B entries 4 then 3 open the exit scene
 * (map_wind_down_texanims, flag 17), which ends once the viewer leaves area 3; the
 * viewer standing on records 0x126, 0xa0, 0x124 and 0x146 steps the gate
 * scene through entries 50, 48, 50 and 49; and the texture animation
 * starts or its pages are moved.
 */
ADDRESS(0x801ed71c, 0x388)
void map_update_events(void)
{
    Vis88 *d4 = &vis_table_b.entries[4];
    Creature *e48 = &vis_table_a.entries[48];
    Vis88 *d3 = &vis_table_b.entries[3];
    Creature *e33 = &vis_table_a.entries[33];
    Creature *e50 = &vis_table_a.entries[50];
    Creature *e49 = &vis_table_a.entries[49];
    Vis88 *d2 = &vis_table_b.entries[2];

    if (fdat_801ee0b4 == 0 && d4->unk0C == 0xff) {
        d3->unk0C = 12;
        fdat_801ee0b4 = 1;
        game_progress.unk1440.word |= 0x20000;
    }
    if (fdat_801ee0b4 == 1 && d3->unk0C == 0xff)
        fdat_801ee0b4 = 2;
    if (fdat_801ee0b4 == 2)
        map_wind_down_texanims();
    if (fdat_801ee0b4 == 3 && ((VisRecord *)viewer.unk174)->unk07 != 3) {
        map_move_texture_pages();
        creature_despawn(e48);
        e33->spawn_chance = 100;
    }
    if (fdat_801ee0b8 == 0 && viewer.unk178 == 0x126) {
        creature_force_talk(e50, 0, 0);
        creature_start_script(e50, 0);
        creature_fade_out_despawn(e50);
        e50->state = 0;
        e50->spawn_chance = 0;
        fdat_801ee0b8 = 1;
        e50->home_facing = (e50->home_facing + 0x800) & 0xfff;
    }
    if (fdat_801ee0b8 == 1 && viewer.unk178 == 0xa0) {
        creature_force_talk(e48, 0, 0);
        fdat_801ee0b8 = 2;
        e50->spawn_chance = 100;
        e48->flags &= ~0x2000000;
    }
    if (fdat_801ee0b8 == 2 && viewer.unk178 == 0x124) {
        creature_force_talk(e50, 0, 0);
        creature_start_script(e50, 0);
        creature_fade_out_despawn(e50);
        fdat_801ee0b8 = 3;
        e49->spawn_chance = 100;
        e50->flags &= ~0x2000000;
    }
    if (fdat_801ee0b8 == 3 && viewer.unk178 == 0x146) {
        creature_force_talk(e49, 0, 0);
        creature_start_script(e49, 0);
        play_unlock_sound();
        d2->unk4A.door.unk4F = 0xff;
        creature_fade_out_despawn(e49);
        fdat_801ee0b8 = 4;
        e49->flags &= ~0x2000000;
    }
    if (fdat_801ee0bc == 1) {
        RECT a = {480, 64, 64, 96};
        RECT b = {512, 483, 16, 1};

        texanim_start(&a, 0, 4, 2, 24);
        texanim_start(&b, 2, 1, 0x31, 24);
        fdat_801ee0bc = 0;
    } else if (fdat_801ee0bc == 2 && area_state.unk0 == 0) {
        map_move_texture_pages();
        fdat_801ee0bc = 0;
    }
}

/* Using item 0x10e opens the armed gate: table A entry 50 dies, 49 wakes, and
 * record stream counter 13 advances. */
ADDRESS(0x801edaa4, 0x6c)
void map_use_item_advance_scene(short id)
{
    Creature *e = &vis_table_a.entries[50];

    if (id == 0x10e && fdat_801ee0b8 == 2) {
        fdat_801ee0b8 = 3;
        e->state = 3;
        vis_table_a.entries[49].spawn_chance = 100;
        e->flags &= ~0x2000000;
        game_progress.unk1448[13]++;
    }
}

#include "../lib/fdat_creature_pick_move.inc"

/*
 * The step of the overlay's entries, like creature_step_script: entry record 0
 * restarts script 0 at the end of a run until stream counter 12 starts.
 */
ADDRESS(0x801edd54, 0xe8)
void map_step_creature(void)
{
    Creature *e = vis_table_a.current;

    e->anim_frame += e->anim_step;
    if (creature_step_hurt_timer(e) == 1)
        return;
    if (e->anim_step >= 0 ? (short)e->anim_frame >= 4096 : (short)e->anim_frame <= 0) {
        if (e->type == 0 && game_progress.unk1448[12] == 0) {
            creature_start_script(e, 0);
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

/* In area 0, unless the scene was played, hands table A entry 48 to its
 * exit once its record's stream counter has started. */
ADDRESS(0x801ede3c, 0x80)
void map_talk_on_player_death(void)
{
    Creature *e;
    CreatureType *r;

    e = &vis_table_a.entries[48];
    r = &vis_table_a.types[0];

    if (game_progress.unk1448[r->talk_id] != 0 && fdat_801ee0b4 != 4
        && vis_table_a.zone == 0) {
        game_progress.unk1448[r->talk_id] = 1;
        e->flags |= 0x2000000;
        creature_force_talk(e, 1, 1);
    }
}

/* Slows texture animations 0 and 1 every 4th frame for 32 frames, then
 * stops them and ends the scene. */
ADDRESS(0x801edebc, 0x70)
void map_wind_down_texanims(void)
{
    StTexAnim *t0 = &world_state.texanim[0];
    StTexAnim *t1 = &world_state.texanim[1];

    if (fdat_801ee0b0 < 32) {
        if ((fdat_801ee0b0 & 3) == 0) {
            t0->period++;
            t1->period++;
        }
        fdat_801ee0b0++;
    } else {
        t0->area = 0xfe;
        t1->area = 0xfe;
        fdat_801ee0b4 = 3;
    }
}

/* Moves the finished texture animation's pages and palette rows into
 * place and ends the exit scene. */
ADDRESS(0x801edf2c, 0x184)
void map_move_texture_pages(void)
{
    RECT a = {496, 0, 16, 64};
    RECT b = {480, 160, 16, 96};
    RECT c = {800, 224, 32, 32};
    RECT d = {512, 485, 16, 1};
    RECT e = {512, 486, 16, 1};
    RECT f = {0, 490, 256, 1};
    u32 flags;

    MoveImage(&a, 480, 0);
    MoveImage(&b, 480, 64);
    MoveImage(&c, 768, 224);
    MoveImage(&d, 512, 483);
    MoveImage(&e, 512, 484);
    MoveImage(&f, 0, 490);
    flags = vis_lists.unk47E0;
    fdat_801ee0b4 = 4;
    vis_lists.unk47E0 = flags | 8;
}
