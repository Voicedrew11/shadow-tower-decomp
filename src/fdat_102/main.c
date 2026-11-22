/*
 * FDAT_102.BIN as one translation unit (WIP module): its handler table,
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

/* 0xff once record 0's stream counter reached 2. */
DATA(0x801eda78, 0x4, ".bss")
static int fdat_801eda78;

DATA(0x801eda7c, 0x4, ".bss")
static int fdat_801eda7c;

void map_restore_progress();
void map_update_events();
void map_use_item_on_creature(short id);

const FdatHeader fdat_header = {
    0xf,
    {
        map_hook_none, map_update_events, map_hook_none, map_hook_none,
        map_hook_none, map_restore_progress, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_use_item_on_creature, map_hook_none, map_hook_none, map_hook_none,
        creature_pick_move, creature_step_script, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
    },
    0x42,
};

#include "../lib/fdat_common_a.inc"
#include "../lib/fdat_common.inc"
#include "../lib/fdat_common_b.inc"

/* Sets up the area: table A entry 48's range bits unless it is dying, the
 * scene words, and table B entries 39..42 unlinked until flag 19 is set. */
ADDRESS(0x801ed4a0, 0xb4)
void map_restore_progress(void)
{
    Creature *e = &vis_table_a.entries[48];
    CreatureType *r = &vis_table_a.types[0];
    Vis88 *d39 = &vis_table_b.entries[39];
    Vis88 *d40 = &vis_table_b.entries[40];
    Vis88 *d41 = &vis_table_b.entries[41];
    Vis88 *d42 = &vis_table_b.entries[42];

    if (e->state != 3)
        e->status.bytes[3] = 2;
    if (game_progress.unk1448[r->talk_id] == 2)
        fdat_801eda78 = 0xff;
    else
        fdat_801eda78 = 0;
    if (!game_progress.unk1440.bit.f19) {
        d39->unk0a = d40->unk0a = d41->unk0a = d42->unk0a = 0xffff;
        fdat_801eda7c = 0;
    } else {
        fdat_801eda7c = 0xff;
    }
}

/*
 * Runs the area: once record 0's stream counter reaches 2, entry 48 takes
 * item 0x127 and leaves; a hit does the same. Record 4's counter is held at
 * 1 or more. Item 0x12b used on entry 0x31 (map_use_item_on_creature) sends entry 49
 * away, links table B entries 39..42 and sets flag 19.
 */
ADDRESS(0x801ed554, 0x164)
void map_update_events(void)
{
    Creature *e48 = &vis_table_a.entries[48];
    Vis88 *d39 = &vis_table_b.entries[39];
    CreatureType *r0 = &vis_table_a.types[0];
    Creature *e49 = &vis_table_a.entries[49];
    int state = fdat_801eda78;
    CreatureType *r4 = &vis_table_a.types[4];
    Vis88 *d40 = &vis_table_b.entries[40];
    Vis88 *d41 = &vis_table_b.entries[41];
    Vis88 *d42 = &vis_table_b.entries[42];

    if (state != 0xff && game_progress.unk1448[r0->talk_id] == 2) {
        creature_drop_item(e48, 0x127);
        creature_fade_out_despawn(e48);
        fdat_801eda78 = 0xff;
    }
    if (e48->hit != 0) {
        e48->hit = 0;
        creature_fade_out_despawn(e48);
        fdat_801eda78 = 0xff;
    }
    if (game_progress.unk1448[r4->talk_id] >= 2)
        game_progress.unk1448[r4->talk_id] = 1;
    if (fdat_801eda7c == 1) {
        creature_fade_out_despawn(e49);
        d42->unk0a = 250;
        d41->unk0a = 250;
        d40->unk0a = 250;
        d39->unk0a = 250;
        fdat_801eda7c = 0xff;
        game_progress.unk1440.word |= 0x80000;
    }
}

/* Item 0x12b used in front of table A entry 0x31 sets fdat_801eda7c; the
 * result is 1 then, else 2. */
ADDRESS(0x801ed6b8, 0xcc)
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
    if (id == 0x12b) {
        if (creature_find_in_reach((VisLists *)&req, 1) == 0x31) {
            fdat_801eda7c = 1;
            DAT_801cd0e0 = 1;
        } else {
            DAT_801cd0e0 = 2;
        }
    }
}

#include "../lib/fdat_creature_pick_move.inc"
#include "../lib/fdat_creature_step_script.inc"
