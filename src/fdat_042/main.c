/*
 * FDAT_042.BIN as one translation unit (WIP module): its handler table,
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

RODATA(0x801eb560, 0x24c)

void func_801ed4a0();
void func_801ed500();
void func_801ed568();
void func_801ed7b4();

const FdatHeader fdat_header = {
    0x9,
    {
        fdat_801ed498, func_801ed4a0, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, func_801ed500, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        func_801ed568, func_801ed7b4, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
    },
    0x3c,
};

#include "../lib/fdat_common_a.inc"
#include "../lib/fdat_common.inc"
#include "../lib/fdat_common_b.inc"

/* The scene state: -1 armed, 0 idle. */
DATA(0x801ed96c, 0x4, ".bss")
int fdat_801ed96c;

void func_801ed8d0(void);

/* Plays the scene once door 19 of table B is raised past 0xa00, unless
 * flag 1 is set. */
ADDRESS(0x801ed4a0, 0x60)
void func_801ed4a0(void)
{
    Vis88 *door = &vis_table_b.entries[19];

    if (!DAT_801cba08.unk1440.bit.f1 && door->unk10 > 0xa00 && fdat_801ed96c == -1)
        func_801ed8d0();
}

/* Sets up the area: table A entry 32 by flag 1, and arms the scene. */
ADDRESS(0x801ed500, 0x68)
void func_801ed500(void)
{
    Vis212 *e = &vis_table_a.entries[32];

    if (e->unk00 == 0xff) {
        fdat_801ed96c = 0;
        return;
    }
    if (!DAT_801cba08.unk1440.bit.f1) {
        e->unk00 = 0;
        fdat_801ed970 = 0;
    } else {
        e->unk00 = 100;
        fdat_801ed970 = 0xff;
    }
    fdat_801ed96c = -1;
}

/*
 * The entry's move picker, fdat_801ed4a0 with one difference: the headings
 * stay as they are when no script was picked.
 */
ADDRESS(0x801ed568, 0x24c)
void func_801ed568(void)
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
    if (!fdat_801ecbcc())
        return;
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
