/*
 * FDAT_372.BIN as one translation unit (WIP module): its handler table,
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
#include <st/fdat_372.h>

RODATA(0x801eb560, 0x24c)

void func_801ed4a0();
void func_801ed510();
void func_801ed688();
void func_801ed9b4();

const FdatHeader fdat_header = {
    0x2a,
    {
        fdat_801ed498, func_801ed510, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, func_801ed4a0, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, func_801ed9b4, fdat_801ed498, fdat_801ed498,
        func_801ed688, fdat_801ed6e4, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
    },
    0x5d,
};

#include "../lib/fdat_common_a.inc"
#include "../lib/fdat_common.inc"
#include "../lib/fdat_common_b.inc"

/* The scene state: 0 pending, 0xff done (flag 20). */
DATA(0x801eda3c, 0x4, ".bss")
int fdat_801eda3c;

/* Sets up the area: the scene by flag 20, and record 5's flags 0x180 cleared
 * once its stream counter has started. */
ADDRESS(0x801ed4a0, 0x70)
void func_801ed4a0(void)
{
    Vis192 *r = &vis_table_a.unk3500[5];

    if (!DAT_801cba08.unk1440.bit.f20)
        fdat_801eda3c = 0;
    else
        fdat_801eda3c = 0xff;
    if (DAT_801cba08.unk1448[r->unk09] != 0)
        r->unk1C &= ~0x180;
}

/* Until record 5's stream counter starts, the viewer passing z 0x4c000
 * sends table A entry 32; beaten, it changes the lighting of four areas
 * (flag 20) and leaves. */
ADDRESS(0x801ed510, 0x178)
void func_801ed510(void)
{
    Vis212 *e = &vis_table_a.entries[32];
    Vis192 *r = &vis_table_a.unk3500[5];

    if (DAT_801cba08.unk1448[r->unk09] == 0 && viewer.position.vz > 0x4c000) {
        viewer.unk218 = 5;
        fdat_801ed388(e, 0, 0);
        e->unk64 = 1;
        viewer.unk218 = 0;
        e->unk70 &= ~0x2000000;
        r->unk1C &= ~0x180;
    }
    if (fdat_801eda3c == 0 && e->unk2C == 0) {
        fdat_801eda3c = 1;
        lighting.area_mode[10] = 0x11;
        lighting.area_mode[21] = 0x11;
        lighting.area_mode[32] = 0x11;
        lighting.area_mode[37] = 0x11;
        DAT_801cba08.unk1440.word |= 0x100000;
    }
    if (fdat_801eda3c == 1 && e->unk05 == 3) {
        viewer.unk218 = 5;
        e->unk70 |= 0x2000000;
        fdat_801ed388(e, 1, 0);
        fdat_801eda3c = 0xff;
        viewer.unk218 = 0;
    }
}

/*
 * The entry's move picker, fdat_801ed4a0 with one addition: kind 5 waits
 * (action 128) while its record's stream counter is 0.
 */
ADDRESS(0x801ed688, 0x27c)
void func_801ed688(void)
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
    if (e->unk02 == 5 && DAT_801cba08.unk1448[r->unk09] == 0)
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

void func_801ed688(void);
#define FDAT_PICK_MOVE func_801ed688

#include "../lib/fdat_801ed6e4.inc"

/* In area 1, while flag 20 is clear, hands table A entry 32 to its exit once
 * its record's stream counter has started. */
ADDRESS(0x801ed9b4, 0x88)
void func_801ed9b4(void)
{
    Vis212 *e;
    Vis192 *r;

    e = &vis_table_a.entries[32];
    r = &vis_table_a.unk3500[5];

    if (DAT_801cba08.unk1448[r->unk09] != 0
        && !(DAT_801cba08.unk1440.word >> 20 & 1) && vis_table_a.unk6168 == 1) {
        DAT_801cba08.unk1448[r->unk09] = 2;
        e->unk70 |= 0x2000000;
        fdat_801ed388(e, 1, 1);
    }
}
