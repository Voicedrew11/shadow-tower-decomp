/*
 * FDAT_232.BIN as one translation unit (WIP module): its handler table,
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
#include <LIBGPU.H>
#include <st/menu.h>
#include <st/light.h>

RODATA(0x801eb560, 0x24c)

void func_801ed4a0();
void func_801ed59c();
void func_801ed7d8();
void func_801edb08();

const FdatHeader fdat_header = {
    0x1c,
    {
        fdat_801ed498, func_801ed59c, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, func_801ed4a0, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, func_801edb08, fdat_801ed498, fdat_801ed498,
        func_801ed7d8, fdat_801ed6e4, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
    },
    0x4f,
};

#include "../lib/fdat_common_a.inc"
#include "../lib/fdat_common.inc"
#include "../lib/fdat_common_b.inc"

/* Table A entry 32 beaten: 0 not yet, 1 this visit, 0xff (flag 28). */
DATA(0x801edb94, 0x4, ".bss")
static int fdat_801edb94;

/* 1 while the viewer's status 0x188 is held at 30. */
DATA(0x801edb98, 0x4, ".bss")
static int fdat_801edb98;

/* The entry scene: 0 waiting for the fade, 1 due, 0xff done. */
DATA(0x801edb9c, 0x4, ".bss")
static int fdat_801edb9c;

/* Sets up the area by flag 28 and record 12's stream counter. */
ADDRESS(0x801ed4a0, 0xfc)
void func_801ed4a0(void)
{
    Vis192 *r = &vis_table_a.unk3500[12];

    if (!(DAT_801cba08.unk1440.word >> 28 & 1))
        fdat_801edb94 = 0;
    else
        fdat_801edb94 = 0xff;
    if (DAT_801cba08.unk1448[r->unk09] >= 2)
        r->unk1C &= ~0x180;
    if (DAT_801cba08.unk1448[r->unk09] == 0) {
        viewer.unk218 = 5;
        fdat_801edb9c = 0;
    } else {
        fdat_801edb9c = 0xff;
    }
    if (DAT_801cba08.unk1448[r->unk09] != 0 && !(DAT_801cba08.unk1440.word >> 28 & 1))
        fdat_801edb98 = 1;
    else
        fdat_801edb98 = 0;
}

/*
 * Runs the area: once the fade is done table A entry 32 wakes; the
 * viewer's status 0x188 is held at 30 meanwhile; the viewer passing (x
 * 0x4c000, z 0x55000) while record 12's counter is 1 sends it; beaten, it
 * leaves (flag 28).
 */
ADDRESS(0x801ed59c, 0x23c)
void func_801ed59c(void)
{
    Vis212 *e = &vis_table_a.entries[32];
    Vis192 *r = &vis_table_a.unk3500[12];
    int n;

    if (fdat_801edb9c == 0 && area_state.unk18 == -1 && area_state.unk2 != 0)
        fdat_801edb9c = 1;
    if (fdat_801edb9c == 1) {
        e->unk70 |= 0x2000000;
        fdat_801ed388(e, 1, 0);
        fdat_801edb98 = 1;
        fdat_801edb9c = 0xff;
        viewer.unk218 = 0;
    }
    if (fdat_801edb98 == 1 && !(viewer.unk2C & 0x800)) {
        viewer.unk188 = 30;
        viewer.unk182 = 0;
    }
    n = DAT_801cba08.unk1448[r->unk09];
    if (n == 1 && viewer.position.vx > 0x4c000 && viewer.position.vz > 0x55000) {
        viewer.unk218 = 5;
        fdat_801ed388(e, 0, 0);
        e->unk64 = n;
        viewer.unk218 = 0;
        e->unk70 &= ~0x2000000;
        r->unk1C &= ~0x180;
    }
    if (fdat_801edb94 == 0 && e->unk2C == 0) {
        fdat_801edb94 = 1;
        DAT_801cba08.unk1440.word |= 0x10000000;
    }
    if (fdat_801edb94 == 1 && e->unk05 == 3) {
        viewer.unk218 = 5;
        e->unk70 |= 0x2000000;
        fdat_801ed388(e, 1, 0);
        fdat_801edb94 = 0xff;
        fdat_801edb98 = 0;
        viewer.unk218 = 0;
    }
}

/*
 * The entry's move picker, fdat_801ed4a0 with one addition: kind 12 waits
 * (action 128) until its record's stream counter reaches 2.
 */
ADDRESS(0x801ed7d8, 0x280)
void func_801ed7d8(void)
{
    Vis212 *e = vis_table_a.unk6138;
    Vis192 *r = vis_table_a.unk6134;
    u16 wall;
    int d;
    short x;
    short z;
    short dx;
    short dz;

    if (e->unk1A == 0xf0 || e->unk1A == 0)
        return;
    fdat_801ec6a4();
    if (e->unk02 == 12 && DAT_801cba08.unk1448[r->unk09] < 2)
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

void func_801ed7d8(void);
#define FDAT_PICK_MOVE func_801ed7d8

#include "../lib/fdat_801ed6e4.inc"

/* In area 3, while flag 28 is clear, hands table A entry 32 to its exit once
 * its record's stream counter has reached 2. */
ADDRESS(0x801edb08, 0x8c)
void func_801edb08(void)
{
    Vis212 *e;
    Vis192 *r;

    e = &vis_table_a.entries[32];
    r = &vis_table_a.unk3500[12];

    if (DAT_801cba08.unk1448[r->unk09] >= 2 && !(DAT_801cba08.unk1440.word >> 28 & 1)
        && vis_table_a.unk6168 == 3) {
        DAT_801cba08.unk1448[r->unk09] = 3;
        e->unk70 |= 0x2000000;
        fdat_801ed388(e, 1, 1);
    }
}
