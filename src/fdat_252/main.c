/*
 * FDAT_252.BIN as one translation unit (WIP module): its handler table,
 * the 21 functions every overlay starts with (src/lib/fdat_common_a.inc,
 * fdat_common.inc, fdat_common_b.inc), then its map handlers through its
 * own move picker. One object, because the picker's switch table is
 * 8-aligned after fdat_801ebec4's in the same read-only run.
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
#include <LIBGPU.H>
#include <st/light.h>

RODATA(0x801eb560, 0x24c)

void func_801ed4a0();
void func_801ed4ec();
void func_801ed5b4();

const FdatHeader fdat_header = {
    0x1e,
    {
        fdat_801ed498, func_801ed4ec, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, func_801ed4a0, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        func_801ed5b4, fdat_801ed6e4, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
    },
    0x51,
};

#include "../lib/fdat_common_a.inc"
#include "../lib/fdat_common.inc"
#include "../lib/fdat_common_b.inc"

/* The scene state: 1 once record 8's stream counter reached 1. */
DATA(0x801ed8cc, 0x4, ".bss")
int fdat_801ed8cc;

/* Sets up the area: arms the scene and clears record 8's flags 0x180 once
 * its stream counter has reached 1. */
ADDRESS(0x801ed4a0, 0x4c)
void func_801ed4a0(void)
{
    Vis192 *r = &vis_table_a.unk3500[8];

    if (DAT_801cba08.unk1448[r->unk09] == 1) {
        fdat_801ed8cc = 1;
        r->unk1C &= ~0x180;
    } else {
        fdat_801ed8cc = 0;
    }
}

/* Until the scene ran, the viewer standing on record 0x33 or 0x36 sends
 * table A entry 32 off (record 8 loses its range bits); otherwise the entry
 * keeps state 0x80. */
ADDRESS(0x801ed4ec, 0xc8)
void func_801ed4ec(void)
{
    Vis212 *e = &vis_table_a.entries[32];
    Vis192 *r = &vis_table_a.unk3500[8];

    if (fdat_801ed8cc == 0) {
        if (viewer.unk178 == 0x33 || viewer.unk178 == 0x36) {
            viewer.unk218 = 5;
            fdat_801ed388(e, 0, 0);
            e->unk64 = 1;
            fdat_801ed8cc = 1;
            viewer.unk218 = 0;
            e->unk70 &= ~0x2000000;
            r->unk1C &= ~0x180;
        } else {
            e->unk44 = 0x80;
        }
    }
}

/*
 * The entry's move picker, fdat_801ed4a0 with one addition: kind 8 waits
 * (action 128) while fdat_801ed8cc is 0.
 */
ADDRESS(0x801ed5b4, 0x268)
void func_801ed5b4(void)
{
    Vis212 *e = vis_table_a.unk6138;
    u16 wall;
    int d;
    short x;
    short z;
    short dx;
    short dz;

    if (e->unk1A == 0xf0 || e->unk1A == 0)
        return;
    fdat_801ec6a4();
    if (e->unk02 == 8 && fdat_801ed8cc == 0)
        e->unk44 = 128;
    fdat_801ecbcc();
    switch (e->unk44) {
    case 8:
        wall = e->unk5E;
        if ((short)wall == 0x7fff) {
            e->unk5A = e->unk5C = (e->unk56 + 2048) & 0xfff;
        } else {
            d = (((e->unk56 + 2048) & 0xfff) - wall + 1024) & 0xfff;
            if (d < 1024 || d > 3072)
                e->unk5A = e->unk5C = (wall + 3072) & 0xfff;
            else
                e->unk5A = e->unk5C = (wall + 1024) & 0xfff;
        }
        break;
    case 2:
    case 4:
    case 16:
    case 32:
        e->unk5A = e->unk5C = e->unk56;
        break;
    default:
        e->unk5A = e->unk5C = e->unk8E;
        break;
    }
    if (e->unk44 != 8)
        e->unk5E = 0x7fff;
    if (vis_records[e->unk26].unk05 != 0 || vis_records[e->unk26].unk06 != 0) {
        x = rcos(e->unk5C - 1024) >> 5;
        z = rsin(e->unk5C - 1024) >> 5;
        dx = x + vis_records[e->unk26].unk05;
        dz = z + vis_records[e->unk26].unk06;
        e->unk5C = (ratan2(dz, dx) + 1024) & 0xfff;
        x = rcos(e->unk5A) >> 5;
        z = rsin(e->unk5A) >> 5;
        dx = x + vis_records[e->unk26].unk05;
        dz = z + vis_records[e->unk26].unk06;
        e->unk5A = ratan2(dz, dx);
    }
}

void func_801ed5b4(void);
#define FDAT_PICK_MOVE func_801ed5b4

#include "../lib/fdat_801ed6e4.inc"
