/*
 * FDAT_122.BIN as one translation unit (WIP module): its handler table,
 * the 21 functions every overlay starts with (src/lib/fdat_common_a.inc,
 * fdat_common.inc, fdat_common_b.inc), then its map handlers through the
 * shared fdat_801ed4a0. One object, because fdat_801ed4a0's switch table
 * is 8-aligned after fdat_801ebec4's in the same read-only run.
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

void func_801ed4a0();
void func_801ed4e4();
void func_801ed77c();

const FdatHeader fdat_header = {
    0x11,
    {
        fdat_801ed498, func_801ed4e4, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, func_801ed4a0, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed4a0, func_801ed77c, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
    },
    0x44,
};

#include "../lib/fdat_common_a.inc"
#include "../lib/fdat_common.inc"
#include "../lib/fdat_common_b.inc"

/* The scene state: 1 armed by flag 47, 2 played. */
DATA(0x801ed938, 0x4, ".bss")
int fdat_801ed938;

void func_801ed880(void);

/* Sets up the area: clears table A entries 32..34 and arms the scene. */
ADDRESS(0x801ed4a0, 0x44)
void func_801ed4a0(void)
{
    vis_table_a.entries[32].unk00 = vis_table_a.entries[33].unk00 = vis_table_a.entries[34].unk00 = 0;
    if (DAT_801cba08.unk1444.bit.f15 == 1)
        fdat_801ed938 = 1;
    else
        fdat_801ed938 = 0;
}

/* Plays the scene once the viewer stands on record 0x35. */
ADDRESS(0x801ed4e4, 0x54)
void func_801ed4e4(void)
{
    if (fdat_801ed938 == 1 && viewer.unk178 == 0x35) {
        func_801ed880();
        fdat_801ed938 = 2;
    }
}

#include "../lib/fdat_801ed4a0.inc"
