/*
 * FDAT_092.BIN as one translation unit (WIP module): its handler table,
 * the 21 functions every overlay starts with (src/lib/fdat_common_a.inc,
 * fdat_common.inc, fdat_common_b.inc), its area handlers, the shared move
 * picker and entry handler (src/lib/fdat_801ed4a0.inc, fdat_801ed6e4.inc)
 * and the gate scene's two parts. One object, because the picker's switch
 * table is 8-aligned after creature_run_script_command's in the same read-only run and
 * the eight scene words are consecutive statics.
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

/* 0xff once the gate scene (flag 44) ran. */
DATA(0x801edf54, 0x4, ".bss")
static int fdat_801edf54;

/* 0xff once table B entries 5/6 are open (flag 46). */
DATA(0x801edf58, 0x4, ".bss")
static int fdat_801edf58;

/* 0xff once table B entries 7/8 are open (flag 45). */
DATA(0x801edf5c, 0x4, ".bss")
static int fdat_801edf5c;

/* 1 once both pairs are open, 0xff after the lighting change. */
DATA(0x801edf60, 0x4, ".bss")
static int fdat_801edf60;

/* Table A entry 39's stage: 0 asleep, 1 rising, 2 sinking, 0xff done. */
DATA(0x801edf64, 0x4, ".bss")
static int fdat_801edf64;

/* Table A entry 38's stage, as fdat_801edf64. */
DATA(0x801edf68, 0x4, ".bss")
static int fdat_801edf68;

/* Entry 39's resting height. */
DATA(0x801edf6c, 0x4, ".bss")
static int fdat_801edf6c;

/* Entry 38's resting height. */
DATA(0x801edf70, 0x4, ".bss")
static int fdat_801edf70;

void map_restore_progress();
void map_update_events();

const FdatHeader fdat_header = {
    0xe,
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
    0x41,
};

#include "../lib/fdat_common_a.inc"
#include "../lib/fdat_common.inc"
#include "../lib/fdat_common_b.inc"

void map_play_creature_script(void);
void map_offer_item_at_player(void);

/* Sets up the area by the second flag word's bits 12..17: the gate scene,
 * table B entry pairs 5/6 and 7/8, and table A entries 38 and 39 (raised
 * out of reach until their flags are set). */
ADDRESS(0x801ed4a0, 0x1e0)
void map_restore_progress(void)
{
    Vis212 *e48 = &vis_table_a.entries[48];
    Vis88 *d5 = &vis_table_b.entries[5];
    Vis88 *d6 = &vis_table_b.entries[6];
    Vis88 *d7 = &vis_table_b.entries[7];
    Vis88 *d8 = &vis_table_b.entries[8];
    Vis48 *c5 = &vis_table_c.entries[5];
    Vis212 *e39 = &vis_table_a.entries[39];
    Vis212 *e38 = &vis_table_a.entries[38];

    if (DAT_801cba08.unk1444.bit.f12 == 1) {
        vis_table_a.entries[40].unk00 = 100;
        c5->unk22 = 4;
        fdat_801edf54 = 0xff;
    } else {
        fdat_801edf54 = 0;
        c5->unk22 = 4;
        e48->unk1C.bytes[3] = 1;
    }
    if (DAT_801cba08.unk1444.bit.f13 == 1) {
        d8->unk00 = 2;
        d7->unk00 = 2;
        fdat_801edf5c = 0xff;
    } else {
        fdat_801edf5c = 0;
    }
    if (DAT_801cba08.unk1444.bit.f14 == 1) {
        d6->unk00 = 2;
        d5->unk00 = 2;
        fdat_801edf58 = 0xff;
    } else {
        fdat_801edf58 = 0;
    }
    if (fdat_801edf58 == 0xff && fdat_801edf5c == 0xff)
        fdat_801edf60 = 0xff;
    else
        fdat_801edf60 = 0;
    if (e39->unk05 != 3) {
        if (DAT_801cba08.unk1444.bit.f16 == 1) {
            e39->unk00 = 100;
            fdat_801edf64 = 0xff;
        } else {
            fdat_801edf64 = 0;
            fdat_801edf6c = e39->unk74.vy;
            e39->unk74.vy += 0xc00;
        }
    }
    if (e38->unk05 != 3) {
        if (DAT_801cba08.unk1444.bit.f17 == 1) {
            e38->unk00 = 100;
            fdat_801edf68 = 0xff;
        } else {
            fdat_801edf68 = 0;
            fdat_801edf70 = e38->unk74.vy;
            e38->unk74.vy += 0xa00;
        }
    }
}

/*
 * Runs the area: the gate scene once record 14's stream counter is 1;
 * opens table B entry pairs 7/8 and 5/6 once table A entries 32 and 35 are
 * beaten (lighting change when both are); raises entries 39 and 38 when
 * table B entries 14 and 17 are triggered and lowers them back.
 */
ADDRESS(0x801ed680, 0x4a0)
void map_update_events(void)
{
    Vis212 *e48 = &vis_table_a.entries[48];
    Vis88 *d5 = &vis_table_b.entries[5];
    Vis192 *r14 = &vis_table_a.unk3500[14];
    Vis212 *e40 = &vis_table_a.entries[40];
    Vis212 *e39 = &vis_table_a.entries[39];
    Vis212 *e38 = &vis_table_a.entries[38];
    Vis212 *e32 = &vis_table_a.entries[32];
    Vis212 *e35 = &vis_table_a.entries[35];
    Vis88 *d6 = &vis_table_b.entries[6];
    Vis88 *d7 = &vis_table_b.entries[7];
    Vis88 *d8 = &vis_table_b.entries[8];
    Vis88 *d14 = &vis_table_b.entries[14];
    Vis88 *d17 = &vis_table_b.entries[17];

    if (fdat_801edf54 == 0 && DAT_801cba08.unk1448[r14->unk09] == 1) {
        viewer.unk218 = 5;
        map_play_creature_script();
        if (!(vis_table_c.unk0070[1] & 0x10000))
            map_offer_item_at_player();
        creature_fade_out_despawn(e48);
        creature_spawn_now(e40);
        e40->unk36 = -1;
        e40->unk03 = 1;
        e40->unk1C.bytes[2] &= 0xf2;
        e40->unk70 |= 0x2000000;
        creature_force_talk(e40, 1, 0);
        player_turn_to_creature(e40);
        creature_force_talk(e40, 0, 0);
        e40->unk00 = 100;
        viewer.unk218 = 0;
        fdat_801edf54 = 0xff;
        e40->unk70 &= ~0x2000000;
        DAT_801cba08.unk1444.word |= 0x1000;
    }
    if (fdat_801edf5c == 0 && e32->unk2C == 0) {
        d8->unk00 = 2;
        d7->unk00 = 2;
        fdat_801edf5c = 0xff;
        DAT_801cba08.unk1444.word |= 0x2000;
        if (fdat_801edf58 == 0xff)
            fdat_801edf60 = 1;
    }
    if (fdat_801edf58 == 0 && e35->unk2C == 0) {
        d6->unk00 = 2;
        d5->unk00 = 2;
        fdat_801edf58 = 0xff;
        DAT_801cba08.unk1444.word |= 0x4000;
        if (fdat_801edf5c == 0xff)
            fdat_801edf60 = 1;
    }
    if (fdat_801edf60 == 1) {
        if (!DAT_801cba08.unk1440.bit.f24)
            lighting.area_mode[9] = 0x41;
        fdat_801edf60 = 0xff;
    }
    if (fdat_801edf64 == 0 && d14->unk0C == 1) {
        creature_spawn_now(e39);
        e39->unk36 = -1;
        e39->unk1C.bytes[2] &= 0xf2;
        creature_start_script(e39, 0);
        e39->unk00 = 100;
        e39->unk74.vy += 0xa00;
        e39->unk1C.bytes[3] = 1;
        e39->unk07 = vis_table_a.unk6103;
        fdat_801edf64 = 1;
        DAT_801cba08.unk1444.word |= 0x10000;
    }
    if (fdat_801edf68 == 0 && d17->unk0C == 1) {
        creature_spawn_now(e38);
        e38->unk36 = -1;
        e38->unk1C.bytes[2] &= 0xf2;
        creature_start_script(e38, 0);
        e38->unk00 = 100;
        e38->unk74.vy += 0xa00;
        e38->unk1C.bytes[3] = 1;
        e38->unk07 = vis_table_a.unk6103;
        fdat_801edf68 = 1;
        DAT_801cba08.unk1444.word |= 0x20000;
    }
    if (fdat_801edf64 == 1 && d14->unk10 > 0xc00)
        fdat_801edf64 = 2;
    if (fdat_801edf68 == 1 && d17->unk10 > 0xc00)
        fdat_801edf68 = 2;
    if (fdat_801edf64 == 2) {
        if (e39->unk74.vy <= fdat_801edf6c) {
            e39->unk1C.bytes[3] = 0;
            fdat_801edf64 = 0xff;
        } else {
            e39->unk74.vy -= 0x100;
        }
    }
    if (fdat_801edf68 == 2) {
        if (e38->unk74.vy <= fdat_801edf70) {
            e38->unk1C.bytes[3] = 0;
            fdat_801edf68 = 0xff;
        } else {
            e38->unk74.vy -= 0x100;
        }
    }
}

#include "../lib/fdat_801ed4a0.inc"
#include "../lib/fdat_801ed6e4.inc"

/* Plays script 1 of table A entry 48 to its end. */
ADDRESS(0x801ede14, 0x90)
void map_play_creature_script(void)
{
    Vis212 *e = &vis_table_a.entries[48];

    creature_start_script(e, 1);
    do {
        creature_update_script_sound(e);
        e->unk2A += e->unkB0;
        if ((short)e->unk2A >= 0x1000)
            e->unk2A = 0xfff;
        world_run_frame(NULL, NULL, 0);
    } while ((short)e->unk2A != 0xfff);
}

/*
 * Drops table C item 5 (kind 48) at the viewer once its model is loaded.
 */
ADDRESS(0x801edea4, 0xb0)
void map_offer_item_at_player(void)
{
    StAnimFile *file;
    Vis48 *item = &vis_table_c.entries[5];
    Vis44 *kind = &vis_table_c.unk0118[48];

    item->unk08 = viewer.position;
    item->unk22 = 2;
    item->unk2D -= 3;
    for (;;) {
        anim_get_file(kind->unk4 + 588, &file);
        if (file != NULL)
            break;
        world_run_frame(NULL, NULL, 0);
    }
    ground_item_pick_up(item, 1);
}
