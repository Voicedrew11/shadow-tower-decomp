/*
 * FDAT_332.BIN as one translation unit (WIP module): its handler table,
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
#include <LIBGPU.H>
#include <st/menu.h>
#include <st/light.h>

RODATA(0x801eb560, 0x24c)

void func_801ed4a0();
void func_801ed614();
void func_801ed904();

const FdatHeader fdat_header = {
    0x26,
    {
        fdat_801ed498, func_801ed4a0, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, func_801ed614, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed4a0, func_801ed904, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
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
void func_801ed4a0(void)
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
            fdat_801eb7b4(e4, 5);
            func_8004da2c(e4);
            func_8004db0c(e4);
            e4->unk34 = 0x1000;
            viewer.unk30++;
        }
        if (e5->unk05 == 1) {
            e5->unk05 = state;
            e5->unk2C = 0;
            e5->unk1C.bytes[2] |= 0x15;
            fdat_801eb7b4(e5, 5);
            func_8004da2c(e5);
            func_8004db0c(e5);
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
void func_801ed614(void)
{
    Vis88 *b = &vis_table_b.entries[3];
    Vis212 *e = &vis_table_a.entries[33];

    if (DAT_801cba08.unk1440.bit.f16) {
        if (e->unk05 != 3) {
            e->unk65 = 20;
            e->unk66 = 30;
            e->unk05 = 3;
            e->unk67 = 100;
            func_8004da2c(e);
            func_8004db0c(e);
        }
        fdat_801edb3c = 2;
    } else {
        fdat_801edb3c = 0;
        b->unk05 = 0xff;
        e->unk70 &= ~0x2000000;
    }
}

#include "../lib/fdat_801ed4a0.inc"
