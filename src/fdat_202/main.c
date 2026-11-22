/*
 * FDAT_202.BIN as one translation unit (WIP module): its handler table,
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
#include <LIBGPU.H>

RODATA(0x801eb560, 0x24c)

/* A word no handler touches; it keeps the scene word four bytes off the code. */
DATA(0x801eda04, 0x4, ".bss")
static int fdat_801eda04;

/* 0xff once door 33 of table B is open. */
DATA(0x801eda08, 0x4, ".bss")
static int fdat_801eda08;

void map_restore_progress();
void map_update_switch_objects();

const FdatHeader fdat_header = {
    0x19,
    {
        map_hook_none, map_update_switch_objects, map_hook_none, map_hook_none,
        map_hook_none, map_restore_progress, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        creature_pick_move, creature_step_script, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
    },
    0x4c,
};

#include "../lib/fdat_common_a.inc"
#include "../lib/fdat_common.inc"
#include "../lib/fdat_common_b.inc"

/* Sets up the area by door 33: once it is open, retires every table B entry
 * of records 0x29, 0x2b and 0x94; the lighting of area 20 unless flag 18 is
 * set. */
ADDRESS(0x801ed4a0, 0xc0)
void map_restore_progress(void)
{
    AreaObject *door = &vis_table_b.entries[33];
    AreaObject *b;
    int i;

    if (door->state == 0xff) {
        b = vis_table_b.entries;
        i = 240;
        while (i--) {
            if (b->type == 0x29 || b->type == 0x2b || b->type == 0x94)
                b->kind = OBJECT_KIND_NONE;
            b++;
        }
        fdat_801eda08 = 0xff;
        if (!game_progress.event_flags.bit.f18)
            lighting.area_mode[20] = 0x41;
    } else {
        fdat_801eda08 = 0;
        if (!game_progress.event_flags.bit.f18)
            lighting.area_mode[20] = 0x22;
    }
}

/*
 * Table B entry 33 (a switch) state 0xff turns the area's table B entries
 * off: kinds 0x29, 0x2b and 0x94 stop (0x94 also loses its scale), 0xaf is
 * unlinked; state 0 turns them back on. Without flag 18 the lighting of
 * area 20 follows.
 *
 * The kind-0x94 arm of the second loop stores its state twice. The repeat
 * leaves no instruction (reload's cse drops a store of a register into a
 * byte that already holds it), but it is a second use of the hoisted 2,
 * which is what puts 2, 5 and -1 in $a1, $a2 and $a3. Any place between
 * the first store and the flags gives the same bytes.
 */
ADDRESS(0x801ed560, 0x1b0)
void map_update_switch_objects(void)
{
    AreaObject *sw = &vis_table_b.entries[33];
    int i;

    if (fdat_801eda08 == 0 && sw->state == 0xff) {
        AreaObject *b;
        for (i = 239, b = vis_table_b.entries; i != -1; i--, b++) {
            if (b->type == 0x29 || b->type == 0x2b || b->type == 0x94) {
                b->kind = OBJECT_KIND_NONE;
                if (b->type == 0x94) {
                    b->scale.vx = 0;
                    b->scale.vy = 0;
                    b->scale.vz = 0;
                }
            } else if (b->type == 0xaf) {
                b->kind = OBJECT_KIND_NONE;
                b->type = 0xffff;
            }
        }
        fdat_801eda08 = 0xff;
        if (!(game_progress.event_flags.word >> 18 & 1))
            lighting.area_mode[20] = 0x41;
    } else if (fdat_801eda08 == 0xff && sw->state == 0) {
        AreaObject *b;
        for (i = 239, b = vis_table_b.entries; i != -1; i--, b++) {
            if (b->type == 0x29)
                b->kind = 3;
            if (b->type == 0x2b)
                b->kind = 5;
            if (b->type == 0x94) {
                b->kind = 2;
                b->state = 0;
                b->scale.vx = 0;
                b->scale.vy = 0;
                b->scale.vz = 0;
                b->kind = 2;
                b->draw_flags = 5;
                b->flags |= 0x40;
                *(u8 *)&b->unk50 = 0;
            }
        }
        fdat_801eda08 = 0;
        if (!(game_progress.event_flags.word >> 18 & 1))
            lighting.area_mode[20] = 0x22;
    }
}

#include "../lib/fdat_creature_pick_move.inc"
#include "../lib/fdat_creature_step_script.inc"
