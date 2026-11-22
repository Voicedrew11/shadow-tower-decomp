/*
 * FDAT_082.BIN as one translation unit (WIP module): its handler table,
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
void func_801ed4f8();

const FdatHeader fdat_header = {
    0xd,
    {
        fdat_801ed498, func_801ed4f8, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, func_801ed4a0, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed4a0, fdat_801ed6e4, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
    },
    0x40,
};

#include "../lib/fdat_common_a.inc"
#include "../lib/fdat_common.inc"
#include "../lib/fdat_common_b.inc"

/* The scene state: 0 pending, 0xff done. */
DATA(0x801ed87c, 0x4, ".bss")
int fdat_801ed87c;

/* Sets up the area: the scene is pending unless table A entry 48 is dying
 * or record 0's stream counter has started. */
ADDRESS(0x801ed4a0, 0x58)
void func_801ed4a0(void)
{
    Vis212 *e = &vis_table_a.entries[48];
    Vis192 *r = &vis_table_a.unk3500[0];

    if (e->unk05 != 3 && DAT_801cba08.unk1448[r->unk09] == 0)
        fdat_801ed87c = 0;
    else
        fdat_801ed87c = 0xff;
}

/* While pending: ends the scene once entry 48 is beaten, and turns it into
 * item 0x116 once record 0's stream counter reaches 1. */
ADDRESS(0x801ed4f8, 0x90)
void func_801ed4f8(void)
{
    Vis212 *e = &vis_table_a.entries[48];
    Vis192 *r = &vis_table_a.unk3500[0];

    if (fdat_801ed87c == 0) {
        if (e->unk2C == 0)
            fdat_801ed87c = 0xff;
        if (DAT_801cba08.unk1448[r->unk09] == 1) {
            creature_drop_item(e, 0x116);
            fdat_801ecf44(e);
            fdat_801ed87c = 0xff;
        }
    }
}

#include "../lib/fdat_801ed4a0.inc"
#include "../lib/fdat_801ed6e4.inc"
