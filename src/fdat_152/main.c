/*
 * FDAT_152.BIN as one translation unit (WIP module): its handler table,
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
#include <st/light.h>
#include <st/menu.h>

RODATA(0x801eb560, 0x24c)

void map_restore_progress();
void map_update_events();
void map_object_hook_none();
void map_use_item_on_object(short id);
void map_step_creature();

const FdatHeader fdat_header = {
    0x14,
    {
        map_hook_none, map_update_events, map_hook_none, map_hook_none,
        map_hook_none, map_restore_progress, map_hook_none, map_hook_none,
        map_hook_none, map_object_hook_none, map_hook_none, map_hook_none,
        map_use_item_on_object, map_hook_none, map_hook_none, map_hook_none,
        creature_pick_move, map_step_creature, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
    },
    0x47,
};

#include "../lib/fdat_common_a.inc"
#include "../lib/fdat_common.inc"
#include "../lib/fdat_common_b.inc"

/* The scene state: 0 pending, 1 armed, 2 done (flag 15). */
DATA(0x801edb84, 0x4, ".bss")
int fdat_801edb84;

void map_slide_ground_item(void);

/* Sets up the area by flag 15: table B entries 5..9 shown and item 11 in
 * place, or all hidden and item 11 already moved. */
ADDRESS(0x801ed4a0, 0x90)
void map_restore_progress(void)
{
    AreaObject *d = &vis_table_b.entries[5];
    GroundItem *item = &vis_table_c.entries[11];

    if (!game_progress.event_flags.bit.f15) {
        item->state = GROUND_ITEM_HIDDEN;
        d[0].visibility = d[1].visibility = d[2].visibility = d[3].visibility = d[4].visibility = 2;
        fdat_801edb84 = 0;
    } else {
        item->state = GROUND_ITEM_SCRIPTED;
        d[0].visibility = d[1].visibility = d[2].visibility = d[3].visibility = d[4].visibility = 0;
        fdat_801edb84 = 2;
        item->position.vx += 0x1e0;
    }
}

/* Plays the armed scene: moves item 11, hides table B entries 5..9, sets
 * the area lighting unless flag 16 is set, and sets flag 15. */
ADDRESS(0x801ed530, 0xb0)
void map_update_events(void)
{
    AreaObject *d = &vis_table_b.entries[5];

    if (fdat_801edb84 == 1) {
        map_slide_ground_item();
        d[0].visibility = d[1].visibility = d[2].visibility = d[3].visibility = d[4].visibility = 0;
        if (!game_progress.event_flags.bit.f16) {
            lighting.area_mode[11] = 0x24;
            lighting.area_mode[27] = 0x24;
            lighting.area_mode[12] = 0x24;
            lighting.area_mode[28] = 0x24;
            lighting.area_mode[15] = 0x24;
            lighting.area_mode[33] = 0x24;
        }
        fdat_801edb84 = 2;
        game_progress.event_flags.bit.f15 = 1;
    }
}

/* An empty handler between the others. */
ADDRESS(0x801ed5e0, 0x8)
void map_object_hook_none(void)
{
}

/* Item 0x130 used in front of table B entry 4 sets fdat_801edb84; the
 * result is 1 then, else 2. */
ADDRESS(0x801ed5e8, 0xc8)
void map_use_item_on_object(short id)
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
    if (id == 0x130) {
        if (object_find_in_reach((CollisionState *)&req) == 4) {
            fdat_801edb84 = 1;
            DAT_801cd0e0 = 1;
        } else {
            DAT_801cd0e0 = 2;
        }
    }
}

#include "../lib/fdat_creature_pick_move.inc"
