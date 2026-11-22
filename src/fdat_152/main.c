/*
 * FDAT_152.BIN as one translation unit (WIP module): its handler table,
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
#include <st/light.h>
#include <st/menu.h>

RODATA(0x801eb560, 0x24c)

void func_801ed4a0();
void func_801ed530();
void func_801ed5e0();
void func_801ed5e8(short id);
void func_801ed8f4();

const FdatHeader fdat_header = {
    0x14,
    {
        fdat_801ed498, func_801ed530, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, func_801ed4a0, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, func_801ed5e0, fdat_801ed498, fdat_801ed498,
        func_801ed5e8, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed4a0, func_801ed8f4, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
    },
    0x47,
};

#include "../lib/fdat_common_a.inc"
#include "../lib/fdat_common.inc"
#include "../lib/fdat_common_b.inc"

/* The scene state: 0 pending, 1 armed, 2 done (flag 15). */
DATA(0x801edb84, 0x4, ".bss")
int fdat_801edb84;

void func_801eda8c(void);

/* Sets up the area by flag 15: table B entries 5..9 shown and item 11 in
 * place, or all hidden and item 11 already moved. */
ADDRESS(0x801ed4a0, 0x90)
void func_801ed4a0(void)
{
    Vis88 *d = &vis_table_b.entries[5];
    Vis48 *item = &vis_table_c.entries[11];

    if (!DAT_801cba08.unk1440.bit.f15) {
        item->unk22 = 4;
        d[0].unk00 = d[1].unk00 = d[2].unk00 = d[3].unk00 = d[4].unk00 = 2;
        fdat_801edb84 = 0;
    } else {
        item->unk22 = 5;
        d[0].unk00 = d[1].unk00 = d[2].unk00 = d[3].unk00 = d[4].unk00 = 0;
        fdat_801edb84 = 2;
        item->unk08.vx += 0x1e0;
    }
}

/* Plays the armed scene: moves item 11, hides table B entries 5..9, sets
 * the area lighting unless flag 16 is set, and sets flag 15. */
ADDRESS(0x801ed530, 0xb0)
void func_801ed530(void)
{
    Vis88 *d = &vis_table_b.entries[5];

    if (fdat_801edb84 == 1) {
        func_801eda8c();
        d[0].unk00 = d[1].unk00 = d[2].unk00 = d[3].unk00 = d[4].unk00 = 0;
        if (!DAT_801cba08.unk1440.bit.f16) {
            lighting.area_mode[11] = 0x24;
            lighting.area_mode[27] = 0x24;
            lighting.area_mode[12] = 0x24;
            lighting.area_mode[28] = 0x24;
            lighting.area_mode[15] = 0x24;
            lighting.area_mode[33] = 0x24;
        }
        fdat_801edb84 = 2;
        DAT_801cba08.unk1440.bit.f15 = 1;
    }
}

/* An empty handler between the others. */
ADDRESS(0x801ed5e0, 0x8)
void func_801ed5e0(void)
{
}

/* Item 0x130 used in front of table B entry 4 sets fdat_801edb84; the
 * result is 1 then, else 2. */
ADDRESS(0x801ed5e8, 0xc8)
void func_801ed5e8(short id)
{
    VECTOR p;
    VisRequest req;

    p.vx = viewer.position.vx;
    p.vy = viewer.position.vy + 1600;
    p.vz = viewer.position.vz;
    req.unk0 = 0;
    req.unk4 = p;
    req.unk1C = 1056;
    req.unk1E = 1600;
    req.unk14.vx = 0;
    req.unk14.vy = 0;
    req.unk14.vz = 0;
    req.unk22 = 0;
    req.unk20 = 0;
    if (id == 0x130) {
        if (object_find_in_reach((VisLists *)&req) == 4) {
            fdat_801edb84 = 1;
            DAT_801cd0e0 = 1;
        } else {
            DAT_801cd0e0 = 2;
        }
    }
}

#include "../lib/fdat_801ed4a0.inc"
