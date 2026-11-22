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
void func_801ed5b0();
void func_801edc60();
void func_801ee37c();

const FdatHeader fdat_header = {
    0x23,
    {
        map_hook_none, func_801ed5b0, map_hook_none, map_hook_none,
        map_hook_none, map_restore_progress, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, func_801ee37c, map_hook_none, map_hook_none,
        creature_pick_move, func_801edc60, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
    },
    0x56,
};

#include "../lib/fdat_common_a.inc"
#include "../lib/fdat_common.inc"
#include "../lib/fdat_common_b.inc"

void func_801edd9c(int which);
void func_801edea0(void);
void func_801ee090(u8 area);
void func_801ee0b8(void);

/* Sets up the area: table B entries 1..6 closed unless their flags are
 * set (entry 9 off when all are open), entries 4 and 5 placed, the scene
 * words cleared. */
ADDRESS(0x801ed4a0, 0x110)
void map_restore_progress(void)
{
    Vis88 *d1 = &vis_table_b.entries[1];
    Vis88 *d9 = &vis_table_b.entries[9];
    Vis212 *e4 = &vis_table_a.entries[4];
    Vis212 *e5 = &vis_table_a.entries[5];
    u32 w = DAT_801cba08.unk1440.word;

    fdat_801ee46c = 1;
    if (!(w >> 16 & 1)) {
        d1[0].unk00 = 2;
        fdat_801ee46c = 0;
    }
    if (!(w >> 18 & 1)) {
        d1[1].unk00 = 2;
        fdat_801ee46c = 0;
    }
    if (!(w >> 20 & 1)) {
        d1[2].unk00 = 2;
        fdat_801ee46c = 0;
    }
    if (!(w >> 24 & 1)) {
        d1[3].unk00 = 2;
        fdat_801ee46c = 0;
    }
    if (!(w >> 27 & 1)) {
        d1[4].unk00 = 2;
        fdat_801ee46c = 0;
    }
    if (!(w >> 28 & 1)) {
        d1[5].unk00 = 2;
        fdat_801ee46c = 0;
    }
    if (fdat_801ee46c)
        d9->unk05 = 0xff;
    e4->unk07 = 1;
    e5->unk07 = 2;
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
void func_801ed5b0(void)
{
    Vis88 *d9 = &vis_table_b.entries[9];
    Vis212 *e0 = &vis_table_a.entries[0];
    Vis212 *e3 = &vis_table_a.entries[3];
    Vis212 *e5 = &vis_table_a.entries[5];

    if (fdat_801ee46c != 0
        && player_in_centred_rect(d9->unk24.vx, d9->unk24.vz, 0xc00, 0xc00, d9->unk24.vy)) {
        func_801edea0();
        fdat_801ee470 = 1;
    }
    if (viewer.position.vz > 0x46800) {
        func_801edd9c(0);
        fdat_801ee470++;
    }
    if (viewer.position.vz < 0x40800) {
        func_801edd9c(1);
        fdat_801ee470--;
    }
    if (e0->unk40 >= 0x6000)
        e0->unk03 = 2;
    if (e3->unk40 >= 0x6000)
        e3->unk03 = 2;
    if (e5->unk40 >= 0x6000)
        e5->unk03 = 2;
    if (e0->unk40 <= 0x6000)
        e0->unk03 = 0;
    if (e3->unk40 <= 0x6000)
        e3->unk03 = 0;
    if (e5->unk40 <= 0x6000)
        e5->unk03 = 0;
    if (fdat_801ee478 != 0xff && fdat_801ee470 >= 2)
        fdat_801ee470 = 1;
    if (e0->unk05 == 5) {
        e0->unk05 = 1;
        e0->unk1C.bytes[3] = 1;
        e0->unk34 = -1;
        e0->unk1C.bytes[2] &= 0xea;
        creature_start_script(e0, 6);
        fdat_801ee474 = 1;
    }
    if (fdat_801ee474 == 2)
        func_801ee0b8();
    if (fdat_801ee478 == 0 && e3->unk2C == 0)
        fdat_801ee478 = 1;
    if (fdat_801ee478 == 1 && e3->unk05 == 3) {
        e3->unk70 |= 0x2000000;
        creature_force_talk(e3, 1, 0);
        fdat_801ee478 = 0xff;
    }
    if (fdat_801ee480 == 0 && e5->unk2C != (e5->unk02 + vis_table_a.unk3500)->unk32)
        fdat_801ee480 = 1;
    if (fdat_801ee480 == 2 && e5->unk2C == 0)
        fdat_801ee480 = 3;
    if (fdat_801ee480 == 1) {
        e5->unk70 |= 0x2000000;
        creature_force_talk(e5, 1, 0);
        fdat_801ee480 = 2;
        e5->unk70 &= ~0x2000000;
    }
    if (fdat_801ee480 == 3 && e5->unk05 == 3) {
        e5->unk70 |= 0x2000000;
        creature_force_talk(e5, 1, 0);
        fdat_801ee480 = 0xff;
        DAT_800881a4 = 3;
        DAT_801cba08.unk1444.word |= 0x80;
    }
    if (fdat_801ee47c == 1 && viewer.position.vz > 0x41000) {
        viewer.unk218 = 5;
        creature_force_talk(e5, 0, 0);
        e5->unk1C.bytes[3] = 0;
        fdat_801ee47c = 0xff;
        viewer.unk218 = 0;
        e5->unk70 &= ~0x2000000;
    }
    if (fdat_801ee47c == 0 && fdat_801ee470 == 3) {
        func_801ee090(2);
        e5->unk00 = 100;
        e5->unk1C.bytes[3] = 1;
        fdat_801ee47c = 1;
        e5->unk70 |= 0x2000000;
    }
}

#include "../lib/fdat_801ed4a0.inc"

/* The current entry's handler: the animation steps as creature_step_script does;
 * kind 0 at the end of entry 0's dying stage turns to its scripts 7 and 9
 * and leaves (scene word 2). */
ADDRESS(0x801edc60, 0x13c)
void func_801edc60(void)
{
    Vis212 *e = vis_table_a.unk6138;
    Vis192 *r = vis_table_a.unk6134;

    e->unk2A += e->unkB0;
    if (creature_step_hurt_timer(e) == 1)
        return;
    if (e->unkB0 >= 0 ? (short)e->unk2A >= 4096 : (short)e->unk2A <= 0) {
        if (e->unk02 == 0 && fdat_801ee474 == 1) {
            e->unk70 |= 0x2000000;
            r->unk60[7]->unk00 = 0x72;
            r->unk60[9]->unk00 = 0x70;
            *(s16 *)&r->unk60[9]->unk0C = -0xc0;
            creature_force_talk(e, 0, 0);
            fdat_801ee474 = 2;
        } else {
            e->unk1A = 0xff;
            creature_pick_move();
        }
    }
    if (e->unkA8 != NULL) {
        creature_run_script_command(e);
        e->unk1A = 0xf0;
    }
}

/* Moves the viewer, every live table A entry and every live effect 0x6000
 * along z (`up`) or back. */
ADDRESS(0x801edd9c, 0x104)
void func_801edd9c(int up)
{
    Vis212 *e = vis_table_a.entries;
    StEffect *fx = effects.slots;
    int dz = 0x6000;
    int i;

    if (up == 0)
        dz = -0x6000;
    viewer.position.vz += dz;
    i = 64;
    while (i--) {
        if (e->unk00 != 0xff) {
            e->unk74.vz += dz;
            creature_move(e);
        }
        e++;
    }
    i = ST_EFFECT_SLOTS;
    while (i--) {
        if (fx->unk0 != ST_EFFECT_FREE)
            fx->unk20.vz += dz;
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
void func_801edea0(void)
{
    Vis212 *e = vis_table_a.entries;
    int index = e->unk02;
    Vis192 *r = &vis_table_a.unk3500[index];
    int slot = e->unk0B + 128;

    play_warp_sound();
    fade_run_frames(2, 0, 0x1000, 0x100);
    creatures_bind_previous_area();
    viewer.position.vx = 0x45800;
    viewer.position.vy = 0x3f9c0;
    viewer.position.vz = 0x40000;
    viewer.unk278.vy = 0;
    viewer.unk278.vx = 0;
    viewer.unk218 = 0;
    viewer.unk28C = 0;
    viewer.unk28A = 0;
    viewer.unk288 = 0;
    *(SVECTOR *)&viewer.unk268 = viewer.unk278;
    viewer.unk218 = 5;
    fade_run_frames(2, 0x1000, 0x1000, 0);
    creatures_update();
    creature_spawn_now(e);
    if (world_state.unk834C[slot] == NULL)
        creature_model_load(2, index * 5, slot, e->unk0C & 0xf);
    e->unk36 = -1;
    e->unk1C.bytes[2] &= 0xf2;
    creature_start_script(e, 7);
    vis_table_a.unk6168 = 0;
    cd_wait_idle();
    play_warp_sound();
    fade_run_frames(2, 0x1000, 0, -0x100);
    pad_clear_input();
    if (DAT_801cba08.unk1448[r->unk09] == 0) {
        creature_force_talk(e, 0, 0);
        e->unk70 &= ~0x2000000;
    }
    vis_table_a.unk6138 = e;
    vis_table_a.unk6134 = r;
    creature_pick_move();
    player_enter_normal_state();
}

/* Sets the area id of every visible record. */
ADDRESS(0x801ee090, 0x28)
void func_801ee090(u8 area)
{
    VisRecord *r = vis_records;
    int i = 512;

    while (i--) {
        r->unk07 = area;
        r++;
    }
}

/* Entry 0 finishes animation 10; entry 48 takes its place and goes, entry
 * 3 rises there (animation 10) and entry 49 replaces 48 until 3 is up. */
ADDRESS(0x801ee0b8, 0x2c4)
void func_801ee0b8(void)
{
    Vis212 *e0;
    Vis212 *e48;
    Vis212 *e49;
    Vis212 *e3;
    Vis192 *r2;
    StAnimFile *file;

    viewer.unk218 = 5;
    e0 = vis_table_a.entries;
    e48 = &vis_table_a.entries[48];
    e49 = &vis_table_a.entries[49];
    e3 = &vis_table_a.entries[3];
    r2 = &vis_table_a.unk3500[2];
    creature_start_script(e0, 10);
    for (;;) {
        e0->unk2A += e0->unkB0;
        if ((short)e0->unk2A >= 0x1000)
            goto first_animation_done;
        world_run_frame(NULL, NULL, 0);
    }
first_animation_done:
    e48->unk74 = e0->unk74;
    *(SVECTOR *)&e48->unk8C = *(SVECTOR *)&e0->unk8C;
    e48->unk26 = e0->unk26;
    creature_despawn(e0);
    viewer.unk30--;
    creature_spawn_now(e3);
    e3->unk74 = e48->unk74;
    *(SVECTOR *)&e3->unk8C = *(SVECTOR *)&e48->unk8C;
    e3->unk36 = -1;
    e3->unk26 = e48->unk26;
    e3->unk1C.bytes[2] &= 0xf2;
    e3->unk70 &= ~0x2000000;
    creature_start_script(e3, 10);
    do {
        e3->unk2A = 0;
        world_run_frame(NULL, NULL, 0);
        anim_get_file(e3->unk0B + 128, &file);
    } while (file == NULL);
    e49->unk74 = e3->unk74;
    *(SVECTOR *)&e49->unk8C = *(SVECTOR *)&e3->unk8C;
    e49->unk26 = e3->unk26;
    r2->unk60[0]->unk0C = 0x90;
    creature_start_script(e49, 0);
    creature_despawn(e48);
    for (;;) {
        e49->unk2A += e49->unkB0;
        e3->unk2A += e3->unkB0;
        if ((short)e49->unk2A >= 0x1000)
            e49->unk2A = 0xfff;
        if ((short)e3->unk2A >= 0x1000)
            break;
        creature_update_script_sound(e49);
        creature_update_script_sound(e3);
        world_run_frame(NULL, NULL, 0);
    }
    creature_despawn(e49);
    viewer.unk218 = 0;
    fdat_801ee474 = 0xff;
}

/* While record 0's stream counter has started: wakes entry 0 (counter 2)
 * until its scene ran, else entry 3 (record 3's counter 1) until it is
 * gone, else entry 5 (record 8's counter 3) until it is gone. */
ADDRESS(0x801ee37c, 0xf0)
void func_801ee37c(void)
{
    Vis212 *e0 = vis_table_a.entries;
    Vis212 *e3 = &vis_table_a.entries[3];
    Vis192 *r3 = &vis_table_a.unk3500[3];
    Vis212 *e5 = &vis_table_a.entries[5];
    Vis192 *r8 = &vis_table_a.unk3500[8];
    Data801cba08 *story = &DAT_801cba08;

    if (story->unk1448[vis_table_a.unk3500[0].unk09] == 0)
        return;
    if (fdat_801ee474 != 0xff) {
        story->unk1448[vis_table_a.unk3500[0].unk09] = 2;
        e0->unk70 |= 0x2000000;
        creature_force_talk(e0, 1, 1);
    } else if (fdat_801ee478 != 0xff) {
        story->unk1448[r3->unk09] = 1;
        e3->unk70 |= 0x2000000;
        creature_force_talk(e3, 1, 1);
    } else if (story->unk1448[r8->unk09] != 0 && fdat_801ee480 != 0xff) {
        story->unk1448[r8->unk09] = 3;
        e5->unk70 |= 0x2000000;
        creature_force_talk(e5, 1, 1);
    } else {
        return;
    }
}
