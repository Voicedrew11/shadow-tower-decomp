/*
 * FDAT_112.BIN as one translation unit (WIP module): its handler table,
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

void map_restore_progress();
void map_step_creature();

const FdatHeader fdat_header = {
    0x10,
    {
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_restore_progress, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        creature_pick_move, map_step_creature, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
    },
    0x43,
};

#include "../lib/fdat_common_a.inc"
#include "../lib/fdat_common.inc"
#include "../lib/fdat_common_b.inc"

/* Sets up the area: table C item 7, and table A entry 48 by flags 14 and 7
 * and the count of item 303. */
ADDRESS(0x801ed4a0, 0x8c)
void map_restore_progress(void)
{
    Vis212 *e;

    vis_table_c.entries[7].unk22 = 4;
    e = &vis_table_a.entries[48];
    if (DAT_801cba08.unk1440.bit.f14 == 1 || DAT_801c7590.counts[303].count == 1)
        e->unk05 = 3;
    if (DAT_801cba08.unk1440.bit.f7 == 1) {
        e->unk1C.bytes[3] = 0;
        e->unk70 &= ~0x2000000;
    } else {
        e->unk1C.bytes[3] = 2;
    }
}

#include "../lib/fdat_creature_pick_move.inc"
