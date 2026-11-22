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
void func_801ed560();

const FdatHeader fdat_header = {
    0x19,
    {
        map_hook_none, func_801ed560, map_hook_none, map_hook_none,
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
    Vis88 *door = &vis_table_b.entries[33];
    Vis88 *b;
    int i;

    if (door->unk0C == 0xff) {
        b = vis_table_b.entries;
        i = 240;
        while (i--) {
            if (b->unk0a == 0x29 || b->unk0a == 0x2b || b->unk0a == 0x94)
                b->unk05 = 0xff;
            b++;
        }
        fdat_801eda08 = 0xff;
        if (!DAT_801cba08.unk1440.bit.f18)
            lighting.area_mode[20] = 0x41;
    } else {
        fdat_801eda08 = 0;
        if (!DAT_801cba08.unk1440.bit.f18)
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
void func_801ed560(void)
{
    Vis88 *sw = &vis_table_b.entries[33];
    int i;

    if (fdat_801eda08 == 0 && sw->unk0C == 0xff) {
        Vis88 *b;
        for (i = 239, b = vis_table_b.entries; i != -1; i--, b++) {
            if (b->unk0a == 0x29 || b->unk0a == 0x2b || b->unk0a == 0x94) {
                b->unk05 = 0xff;
                if (b->unk0a == 0x94) {
                    b->unk3C.vx = 0;
                    b->unk3C.vy = 0;
                    b->unk3C.vz = 0;
                }
            } else if (b->unk0a == 0xaf) {
                b->unk05 = 0xff;
                b->unk0a = 0xffff;
            }
        }
        fdat_801eda08 = 0xff;
        if (!(DAT_801cba08.unk1440.word >> 18 & 1))
            lighting.area_mode[20] = 0x41;
    } else if (fdat_801eda08 == 0xff && sw->unk0C == 0) {
        Vis88 *b;
        for (i = 239, b = vis_table_b.entries; i != -1; i--, b++) {
            if (b->unk0a == 0x29)
                b->unk05 = 3;
            if (b->unk0a == 0x2b)
                b->unk05 = 5;
            if (b->unk0a == 0x94) {
                b->unk05 = 2;
                b->unk0C = 0;
                b->unk3C.vx = 0;
                b->unk3C.vy = 0;
                b->unk3C.vz = 0;
                b->unk05 = 2;
                b->unk03 = 5;
                b->unk04 |= 0x40;
                *(u8 *)&b->unk50 = 0;
            }
        }
        fdat_801eda08 = 0;
        if (!(DAT_801cba08.unk1440.word >> 18 & 1))
            lighting.area_mode[20] = 0x22;
    }
}

#include "../lib/fdat_801ed4a0.inc"
#include "../lib/fdat_801ed6e4.inc"
