/*
 * FDAT_332.BIN as one translation unit (WIP module): its handler table,
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

RODATA(0x801eb560, 0x24c)

void map_update_events();
void map_restore_progress();
void map_step_creature();

const FdatHeader fdat_header = {
    0x26,
    {
        map_hook_none, map_update_events, map_hook_none, map_hook_none,
        map_hook_none, map_restore_progress, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        creature_pick_move, map_step_creature, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
    },
    0x59,
};

#include "../lib/fdat_common_a.inc"
#include "../lib/fdat_common.inc"
#include "../lib/fdat_common_b.inc"

/* Once table A entry 33 dies (scene word 0): the lighting of six areas
 * changes, the entry comes back (flag 16) and entries 4 and 5, if active,
 * die with it. */
ADDRESS(0x801ed4a0, 0x174)
void map_update_events(void)
{
    Vis212 *e33 = &vis_table_a.entries[33];
    Vis212 *e4 = &vis_table_a.entries[4];
    Vis212 *e5 = &vis_table_a.entries[5];
    int state = e33->unk05;

    if (state == 5 && fdat_801edb3c == 0) {
        lighting.area_mode[11] = 0x11;
        lighting.area_mode[27] = 0x11;
        lighting.area_mode[12] = 0x11;
        lighting.area_mode[28] = 0x11;
        lighting.area_mode[15] = 0x11;
        lighting.area_mode[33] = 0x11;
        e33->unk05 = 1;
        e33->unk1C.bytes[3] = 1;
        e33->unk34 = -1;
        fdat_801edb3c = 1;
        DAT_801cba08.unk1440.word |= 0x10000;
        e33->unk1C.bytes[2] &= 0xea;
        if (e4->unk05 == 1) {
            e4->unk05 = state;
            e4->unk2C = 0;
            e4->unk1C.bytes[2] |= 0x15;
            creature_start_script(e4, 5);
            creature_roll_drops(e4);
            creature_award_kill_bonuses(e4);
            e4->unk34 = 0x1000;
            viewer.unk30++;
        }
        if (e5->unk05 == 1) {
            e5->unk05 = state;
            e5->unk2C = 0;
            e5->unk1C.bytes[2] |= 0x15;
            creature_start_script(e5, 5);
            creature_roll_drops(e5);
            creature_award_kill_bonuses(e5);
            e5->unk34 = 0x1000;
            viewer.unk30++;
        }
    }
}

/* The scene state: 0 idle, 1 armed, 2 done (flag 48). */
DATA(0x801edb3c, 0x4, ".bss")
int fdat_801edb3c;

/* Sets up the area by flag 48: table A entry 33 dead with its drop chances,
 * or table B entry 3 retired and entry 33 shown. */
ADDRESS(0x801ed614, 0xac)
void map_restore_progress(void)
{
    Vis88 *b = &vis_table_b.entries[3];
    Vis212 *e = &vis_table_a.entries[33];

    if (DAT_801cba08.unk1440.bit.f16) {
        if (e->unk05 != 3) {
            e->unk65 = 20;
            e->unk66 = 30;
            e->unk05 = 3;
            e->unk67 = 100;
            creature_roll_drops(e);
            creature_award_kill_bonuses(e);
        }
        fdat_801edb3c = 2;
    } else {
        fdat_801edb3c = 0;
        b->unk05 = 0xff;
        e->unk70 &= ~0x2000000;
    }
}

#include "../lib/fdat_801ed4a0.inc"
