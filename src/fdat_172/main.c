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
    Vis212 *e48 = &vis_table_a.entries[48];
    Vis192 *r1 = &vis_table_a.unk3500[1];
    Vis48 *c28 = &vis_table_c.entries[28];
    Vis192 *r14 = &vis_table_a.unk3500[14];
    Vis88 *d6 = &vis_table_b.entries[6];

    if (DAT_801cba08.unk1440.bit.f26 == 1) {
        c28->unk22 = 5;
        fdat_801edfd0 = 0xff;
        c28->unk08.vy += 0x200;
    } else {
        c28->unk22 = 4;
        fdat_801edfd0 = 0;
    }
    if (e48->unk05 != 3 && DAT_801cba08.unk1440.bit.f29 != 1 && DAT_801cba08.unk1440.bit.f14) {
        if (DAT_801cba08.unk1440.bit.f30 == 1) {
            fdat_801edfd4 = 0xfe;
            e48->unk70 &= ~0x2000000;
        } else {
            fdat_801edfd4 = 0;
            e48->unk1C.bytes[3] = 3;
            r1->unk60[12]->unk00 = 0x72;
            r1->unk60[13]->unk00 = 0x80;
        }
    } else {
        e48->unk00 = 0;
        fdat_801edfd4 = 0xff;
    }
    if (!DAT_801cba08.unk1440.bit.f27) {
        fdat_801edfd8 = 0;
        d6->unk05 = 0xff;
    } else {
        fdat_801edfd8 = 0xff;
    }
    if (DAT_801cba08.unk1448[r14->unk09] != 0)
        r14->unk1C &= ~0x180;
}

/*
 * Runs the area: item 0x12a's scene (map_use_item_on_object) lifts table C item 28;
 * entry 48 wakes when hit or once record 1's stream counter is 2, and
 * changes its scripts once its flash is over; entry 33 is sent by the
 * viewer passing z 0x4a000 and, once beaten, changes the lighting and
 * leaves (flag 27).
 */
ADDRESS(0x801ed614, 0x328)
void map_update_events(void)
{
    Vis212 *e48 = &vis_table_a.entries[48];
    Vis88 *d6 = &vis_table_b.entries[6];
    Vis192 *r1 = &vis_table_a.unk3500[1];
    Vis212 *e33 = &vis_table_a.entries[33];
    Vis192 *r14 = &vis_table_a.unk3500[14];

    if (fdat_801edfd0 == 1) {
        map_slide_ground_item();
        creatures_restart_fade_in();
        fdat_801edfd0 = 0xff;
        DAT_801cba08.unk1440.word |= 0x4000000;
    }
    if (fdat_801edfd4 == 0) {
        if (DAT_801cba08.unk1448[r1->unk09] == 2) {
            creature_fade_out_despawn(e48);
            fdat_801edfd4 = 0xff;
        }
        if (e48->unk64 == 1) {
            fdat_801edfd4 = 0xfd;
            e48->unk1C.bytes[2] &= 0xea;
            e48->unk70 &= ~0x2000000;
            DAT_801cba08.unk1440.word |= 0x40000000;
        }
    }
    if (fdat_801edfd4 == 0xfd && e48->unk34 == -1) {
        DAT_801cba08.unk1448[r1->unk09] = 2;
        r1->unk60[12]->unk00 = 0x80;
        r1->unk60[13]->unk00 = 0x72;
        e48->unk70 |= 0x2000000;
        creature_force_talk(e48, 0, 0);
        fdat_801edfd4 = 0xfe;
        e48->unk1C.bytes[3] = 0;
        e48->unk70 &= ~0x2000000;
    }
    if (e48->unk2C == 0)
        DAT_801cba08.unk1440.word |= 0x20000000;
    if (DAT_801cba08.unk1448[r14->unk09] == 0 && viewer.position.vz > 0x4a000) {
        viewer.unk218 = 5;
        creature_force_talk(e33, 0, 0);
        e33->unk64 = 1;
        viewer.unk218 = 0;
        e33->unk70 &= ~0x2000000;
        r14->unk1C &= ~0x180;
    }
    if (fdat_801edfd8 == 0 && e33->unk2C == 0) {
        fdat_801edfd8 = 1;
        lighting.area_mode[2] = 0x11;
        lighting.area_mode[17] = 0x11;
        DAT_801cba08.unk1440.word |= 0x8000000;
    }
    if (fdat_801edfd8 == 1 && e33->unk05 == 3) {
        viewer.unk218 = 5;
        e33->unk70 |= 0x2000000;
        creature_force_talk(e33, 1, 0);
        fdat_801edfd8 = 0xff;
        d6->unk05 = 0xed;
        viewer.unk218 = 0;
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
    Vis212 *e = vis_table_a.unk6138;
    Vis192 *r = vis_table_a.unk6134;
    u16 wall;
    int d;
    short x;
    short z;
    short dx;
    short dz;

    if (e->unk1A == 0xf0 || e->unk1A == 0)
        return;
    creature_choose_action();
    if (e->unk02 == 14 && DAT_801cba08.unk1448[r->unk09] == 0)
        e->unk44 = 128;
    if (e->unk02 == 1 && (fdat_801edfd4 == 0 || fdat_801edfd4 == 253)) {
        creature_start_script(e, 9);
        return;
    }
    creature_pick_weighted_script();
    switch (e->unk44) {
    case 8:
        wall = e->unk5E;
        if ((short)wall == 0x7fff) {
            e->unk5A = e->unk5C = (e->unk56 + 2048) & 0xfff;
        } else {
            d = (((e->unk56 + 2048) & 0xfff) - wall + 1024) & 0xfff;
            if (d < 1024 || d > 3072)
                e->unk5A = e->unk5C = (wall + 3072) & 0xfff;
            else
                e->unk5A = e->unk5C = (wall + 1024) & 0xfff;
        }
        break;
    case 2:
    case 4:
    case 16:
    case 32:
        e->unk5A = e->unk5C = e->unk56;
        break;
    default:
        e->unk5A = e->unk5C = e->unk8E;
        break;
    }
    if (e->unk44 != 8)
        e->unk5E = 0x7fff;
    if (vis_records[e->unk26].unk05 != 0 || vis_records[e->unk26].unk06 != 0) {
        x = rcos(e->unk5C - 1024) >> 5;
        z = rsin(e->unk5C - 1024) >> 5;
        dx = x + vis_records[e->unk26].unk05;
        dz = z + vis_records[e->unk26].unk06;
        e->unk5C = (ratan2(dz, dx) + 1024) & 0xfff;
        x = rcos(e->unk5A) >> 5;
        z = rsin(e->unk5A) >> 5;
        dx = x + vis_records[e->unk26].unk05;
        dz = z + vis_records[e->unk26].unk06;
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
    Vis212 *e = vis_table_a.unk6138;

    if (fdat_801edfd0 != 0xff && e->unk02 != 1 && e < &vis_table_a.entries[56] && e->unk05 != 5)
        e->unk03 = 3;
    else
        e->unk03 = 0;
    e->unk2A += e->unkB0;
    if (creature_step_hurt_timer(e) == 1)
        return;
    if (e->unkB0 >= 0 ? (short)e->unk2A >= 4096 : (short)e->unk2A <= 0) {
        if (e->unk02 == 1 && (fdat_801edfd4 == 0 || fdat_801edfd4 == 0xfd)) {
            creature_start_script(e, 9);
        } else {
            e->unk1A = 0xff;
            map_pick_creature_move();
        }
    }
    if (e->unkA8 != NULL) {
        creature_run_script_command(e);
        e->unk1A = 0xf0;
    }
}

/* Item 0x12a used in front of table B entry 12 sets fdat_801edfd0; the
 * result is 1 then, else 2. */
ADDRESS(0x801edd38, 0xc8)
void map_use_item_on_object(short id)
{
    VECTOR p;
    VisRequest req;

    if (id == 0x12a) {
        p.vx = viewer.position.vx;
        p.vy = viewer.position.vy + 1600;
        p.vz = viewer.position.vz;
        req.unk0 = 0;
        req.unk4 = p;
        req.unk1C = 1056;
        req.unk14.vx = 0;
        req.unk14.vy = 0;
        req.unk14.vz = 0;
        req.unk22 = 0;
        req.unk1E = 1600;
        req.unk20 = 0;
        if (object_find_in_reach((VisLists *)&req) == 12) {
            DAT_801cd0e0 = 1;
            fdat_801edfd0 = 1;
        } else {
            DAT_801cd0e0 = 2;
        }
    }
}

/* Lifts table C item 28 by 0x200, 24 a frame while its model is loaded. */
ADDRESS(0x801ede00, 0xf8)
void map_slide_ground_item(void)
{
    int step = 24;
    Vis48 *item = &vis_table_c.entries[28];
    Vis44 *kind;
    int top;

    item->unk22 = 5;
    kind = &vis_table_c.unk0118[item->unk00];
    top = item->unk08.vy + 0x200;
    item->unk2D -= 2;
    for (;;) {
        StAnimFile *file;

        if (item->unk08.vy == top)
            break;
        anim_get_file(kind->unk4 + 588, &file);
        if (file != NULL)
            item->unk08.vy += step;
        if (item->unk08.vy > top)
            item->unk08.vy = top;
        world_run_frame(NULL, NULL, 0);
    }
    item->unk2D += 2;
}

/* Marks every active table A entry: unk36 0xfff and range bits 0xd. */
ADDRESS(0x801edef8, 0x50)
void creatures_restart_fade_in(void)
{
    Vis212 *e = vis_table_a.entries;
    int i = 64;

    while (i--) {
        if (e->unk05 == 1) {
            e->unk36 = 0xfff;
            e->unk1C.bytes[2] |= 0xd;
        }
        e++;
    }
}

/* In area 3, while flag 27 is clear, hands table A entry 33 to its exit once
 * its record's stream counter has started. */
ADDRESS(0x801edf48, 0x88)
void map_talk_on_player_death(void)
{
    Vis212 *e;
    Vis192 *r;

    e = &vis_table_a.entries[33];
    r = &vis_table_a.unk3500[14];

    if (DAT_801cba08.unk1448[r->unk09] != 0
        && !(DAT_801cba08.unk1440.word >> 27 & 1) && vis_table_a.unk6168 == 3) {
        DAT_801cba08.unk1448[r->unk09] = 2;
        e->unk70 |= 0x2000000;
        creature_force_talk(e, 1, 1);
    }
}
