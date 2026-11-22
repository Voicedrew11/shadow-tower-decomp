/*
 * FDAT_032.BIN as one translation unit (WIP module): its handler table,
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

RODATA(0x801eb560, 0x254)

void map_start_texanim();

const FdatHeader fdat_header = {
    0x8,
    {
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_start_texanim, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        creature_pick_move, creature_step_script, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
    },
    0x3b,
};

#include "../lib/fdat_common_a.inc"
#include "../lib/fdat_common.inc"
#include "../lib/fdat_common_b.inc"

/* Starts the area's texture animation on the 64x64 page at (768, 0). */
ADDRESS(0x801ed4a8, 0x64)
void map_start_texanim(void)
{
    RECT rect = {768, 0, 64, 64};

    texanim_clear_all();
    texanim_start(&rect, 0, 4, 2, 3);
}

#include "../lib/fdat_creature_pick_move.inc"
#include "../lib/fdat_creature_step_script.inc"
