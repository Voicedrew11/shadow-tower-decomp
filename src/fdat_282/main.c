/*
 * FDAT_282.BIN as one translation unit (WIP module): its handler table,
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

void map_update_events();
void map_restore_progress();
void func_801ed7a4();
void func_801eda18();

const FdatHeader fdat_header = {
    0x21,
    {
        map_hook_none, map_update_events, map_hook_none, map_hook_none,
        map_hook_none, map_restore_progress, map_hook_none, map_hook_none,
        map_hook_none, func_801eda18, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        creature_pick_move, func_801ed7a4, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
    },
    0x54,
};

#include "../lib/fdat_common_a.inc"
#include "../lib/fdat_common.inc"
#include "../lib/fdat_common_b.inc"

void func_801eda20(void);

/* Opens door 26 of table B unless it is already open. */
ADDRESS(0x801ed4a0, 0x48)
void map_update_events(void)
{
    Vis88 *door = &vis_table_b.entries[26];

    if (door->unk0E != 0 && door->unk0C != 0xff)
        func_801eda20();
}

/* Sets up the area: table C item 2 and table A entries 48..50. */
ADDRESS(0x801ed4e8, 0x78)
void map_restore_progress(void)
{
    Vis212 *a48 = &vis_table_a.entries[48];
    Vis212 *a50 = &vis_table_a.entries[50];
    Vis48 *item;
    Vis192 *r;
    Vis212 *a49;

    r = &vis_table_a.unk3500[vis_table_a.entries[19].unk02];
    item = &vis_table_c.entries[2];
    a49 = &vis_table_a.entries[49];

    if (vis_table_b.entries[26].unk0C != 0xff)
        item->unk22 = 4;
    a49->unk1C.bytes[3] = 2;
    a50->unk1C.bytes[3] = 2;
    a48->unk1C.bytes[3] = 2;
    DAT_801cba08.unk1448[r->unk09] = 3;
}

#include "../lib/fdat_801ed4a0.inc"
