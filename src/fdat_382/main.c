/*
 * FDAT_382.BIN as one translation unit (WIP module): its handler table,
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
#include <st/fdat_382.h>

RODATA(0x801eb560, 0x24c)

void func_801ed4a0();
void func_801ed588();
void func_801ed74c();
void func_801ed9b4();
void func_801eda74();

const FdatHeader fdat_header = {
    0x2b,
    {
        fdat_801ed498, func_801ed588, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, func_801ed4a0, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, func_801eda74, fdat_801ed498, fdat_801ed498,
        func_801ed74c, func_801ed9b4, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
    },
    0x5e,
};

#include "../lib/fdat_common_a.inc"
#include "../lib/fdat_common.inc"
#include "../lib/fdat_common_b.inc"

/* Table A entry 32 beaten: 0, 1 this visit, 0xff gone (flag 24). */
DATA(0x801edb04, 0x4, ".bss")
int fdat_801edb04;

/* Sets up the area: with flags 21..23 all set entries 0..2 and 32 stand
 * (scene word 1), else they wait; flag 24 and record 1's stream counter
 * set the other words. */
ADDRESS(0x801ed4a0, 0xe8)
void func_801ed4a0(void)
{
    VisTableA *a = &vis_table_a;
    Vis212 *e32 = &a->entries[32];
    Vis192 *r1 = &a->unk3500[1];

    if ((DAT_801cba08.unk1440.word & 0xe00000) == 0xe00000) {
        vis_table_a.entries[32].unk00 = 100;
        vis_table_a.entries[2].unk00 = 100;
        vis_table_a.entries[1].unk00 = 100;
        vis_table_a.entries[0].unk00 = 100;
        e32->unk1C.bytes[3] = 1;
        fdat_801edafc = 1;
        e32->unk70 &= ~0x2000000;
    } else {
        vis_table_a.entries[32].unk00 = 0;
        vis_table_a.entries[2].unk00 = 0;
        vis_table_a.entries[1].unk00 = 0;
        vis_table_a.entries[0].unk00 = 0;
        fdat_801edafc = 0;
    }
    if (!DAT_801cba08.unk1440.bit.f24)
        fdat_801edb04 = 0;
    else
        fdat_801edb04 = 0xff;
    if (DAT_801cba08.unk1448[r1->unk09] >= 2)
        fdat_801edb00 = 0xff;
    else
        fdat_801edb00 = 0;
}

/* Runs the area: the viewer reaching z 0x3a800 wakes entry 32 (or sends
 * it, with entries 0 and 32 out of range); beaten, it changes the lighting
 * of four areas (flag 24) and leaves. */
ADDRESS(0x801ed588, 0x1c4)
void func_801ed588(void)
{
    Vis212 *e0 = vis_table_a.entries;
    Vis212 *e32 = &vis_table_a.entries[32];
    Vis192 *r1 = &vis_table_a.unk3500[1];

    if (fdat_801edb00 == 0 && viewer.position.vz < 0x3a800) {
        if (fdat_801edafc == 0) {
            DAT_801cba08.unk1448[r1->unk09] = 0;
            fdat_801ed388(e32, 1, 0);
            e32->unk70 &= ~0x2000000;
        } else {
            DAT_801cba08.unk1448[r1->unk09] = 1;
            fdat_801ed388(e32, 0, 0);
            e32->unk70 &= ~0x2000000;
            e32->unk1C.bytes[3] = 0;
            e0->unk1C.bytes[3] = 0;
        }
        fdat_801edb00 = 0xff;
    }
    if (fdat_801edb04 == 0 && e32->unk2C == 0) {
        fdat_801edb04 = 1;
        lighting.area_mode[9] = 0x11;
        lighting.area_mode[8] = 0x11;
        lighting.area_mode[13] = 0x11;
        lighting.area_mode[38] = 0x11;
        DAT_801cba08.unk1440.word |= 0x1000000;
    }
    if (fdat_801edb04 == 1 && e32->unk05 == 3) {
        viewer.unk218 = 5;
        e32->unk70 |= 0x2000000;
        DAT_801cba08.unk1448[r1->unk09] = 2;
        fdat_801ed388(e32, 1, 0);
        fdat_801edb04 = 0xff;
        viewer.unk218 = 0;
    }
}

/*
 * The entry's move picker, fdat_801ed4a0 with one addition: kind 1 waits
 * (action 128) while fdat_801edb00 is 0.
 */
ADDRESS(0x801ed74c, 0x268)
void func_801ed74c(void)
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
    if (e->unk02 == 1 && fdat_801edb00 == 0)
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
