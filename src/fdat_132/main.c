/*
 * FDAT_132.BIN as one translation unit (WIP module): its handler table,
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
void func_801ed5bc();

const FdatHeader fdat_header = {
    0x12,
    {
        fdat_801ed498, func_801ed5bc, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, func_801ed4a0, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed4a0, fdat_801ed6e4, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
    },
    0x45,
};

#include "../lib/fdat_common_a.inc"
#include "../lib/fdat_common.inc"
#include "../lib/fdat_common_b.inc"

/* 0xff once record 2's stream counter reached 1 (or entry 48 was hit). */
DATA(0x801eda30, 0x4, ".bss")
static int fdat_801eda30;

/* Table A entries 34, 33 and 35 beaten (flags 21, 22, 23): 0 not yet,
 * 1 this visit, 0xff before. */
DATA(0x801eda34, 0x4, ".bss")
static int fdat_801eda34;

DATA(0x801eda38, 0x4, ".bss")
static int fdat_801eda38;

DATA(0x801eda3c, 0x4, ".bss")
static int fdat_801eda3c;

/* Sets up the area: entry 48's range bits unless it is beaten, the scene
 * words by record 2's stream counter and flags 21..23, and records 1, 4
 * and 8 with their bits 7-8 clear. */
ADDRESS(0x801ed4a0, 0x11c)
void func_801ed4a0(void)
{
    Vis212 *e48 = &vis_table_a.entries[48];
    Vis192 *r2 = &vis_table_a.unk3500[2];
    Vis192 *r1 = &vis_table_a.unk3500[1];
    Vis192 *r4 = &vis_table_a.unk3500[4];
    Vis192 *r8 = &vis_table_a.unk3500[8];

    if (e48->unk05 != 3)
        e48->unk1C.bytes[3] = 2;
    if (DAT_801cba08.unk1448[r2->unk09] == 1)
        fdat_801eda30 = 0xff;
    else
        fdat_801eda30 = 0;
    if (!(DAT_801cba08.unk1440.word >> 21 & 1))
        fdat_801eda34 = 0;
    else
        fdat_801eda34 = 0xff;
    if (!(DAT_801cba08.unk1440.word >> 22 & 1))
        fdat_801eda38 = 0;
    else
        fdat_801eda38 = 0xff;
    if (!(DAT_801cba08.unk1440.word >> 23 & 1))
        fdat_801eda3c = 0;
    else
        fdat_801eda3c = 0xff;
    r1->unk1C &= ~0x180;
    r4->unk1C &= ~0x180;
    r8->unk1C &= ~0x180;
}

/* Runs the area: entry 48 leaves once record 2's stream counter is 1 or
 * when hit; entries 34, 33 and 35 beaten set flags 21, 22 and 23. */
ADDRESS(0x801ed5bc, 0x180)
void func_801ed5bc(void)
{
    Vis212 *e48 = &vis_table_a.entries[48];
    Vis192 *r2 = &vis_table_a.unk3500[2];
    Vis212 *e34 = &vis_table_a.entries[34];
    Vis212 *e33 = &vis_table_a.entries[33];
    Vis212 *e35 = &vis_table_a.entries[35];

    if (fdat_801eda30 != 0xff && DAT_801cba08.unk1448[r2->unk09] == 1) {
        fdat_801ecf44(e48);
        fdat_801eda30 = 0xff;
    }
    if (e48->unk64 != 0) {
        e48->unk64 = 0;
        fdat_801ecf44(e48);
        fdat_801eda30 = 0xff;
    }
    if (fdat_801eda34 == 0 && e34->unk2C == 0) {
        fdat_801eda34 = 1;
        DAT_801cba08.unk1440.word |= 0x200000;
    }
    if (fdat_801eda38 == 0 && e33->unk2C == 0) {
        fdat_801eda38 = 1;
        DAT_801cba08.unk1440.word |= 0x400000;
    }
    if (fdat_801eda3c == 0 && e35->unk2C == 0) {
        fdat_801eda3c = 1;
        DAT_801cba08.unk1440.word |= 0x800000;
    }
}

#include "../lib/fdat_801ed4a0.inc"
#include "../lib/fdat_801ed6e4.inc"
