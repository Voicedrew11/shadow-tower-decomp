/*
 * FDAT_272.BIN as one translation unit (WIP module): its handler table,
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

RODATA(0x801eb560, 0x254)

void func_801ed4d0();
void func_801ed760();
void func_801edad8();
void func_801edd24();
void func_801ee43c();
void func_801ee544();

const FdatHeader fdat_header = {
    0x20,
    {
        fdat_801ed498, func_801ed4d0, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, func_801ed760, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, func_801ee43c, fdat_801ed498, fdat_801ed498,
        func_801ee544, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        func_801edad8, func_801edd24, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
    },
    0x53,
};

#include "../lib/fdat_common_a.inc"
#include "../lib/fdat_common.inc"
#include "../lib/fdat_common_b.inc"

DATA(0x801ef504, 0x18, ".bss")
int fdat_801ef504[6];


void func_801ef1c0(void);

/*
 * Runs the gate stages (story flag bits 10-12) by table B entry 5's state;
 * sets flag 13 once the six entries of fdat_801ef504 are all beaten, then
 * counts down to func_801ef1c0's scene; moves the palette row 487 to 489
 * when fdat_801ef934 asks.
 */
ADDRESS(0x801ed4d0, 0x290)
void func_801ed4d0(void)
{
    Vis88 *d5 = &vis_table_b.entries[5];
    Vis212 *e32 = &vis_table_a.entries[32];
    int i;
    u32 flags;

    if (DAT_801cba08.unk1440.field.gate == 0 && d5->unk10 != 0) {
        DAT_801cba08.unk1440.field.gate = 1;
        fdat_801ed410(e32);
        e32->unk36 = -1;
        e32->unk03 = 1;
        e32->unk1C.bytes[2] &= 0xf2;
    } else if (DAT_801cba08.unk1440.field.gate == 1 && d5->unk10 > 0x800) {
        DAT_801cba08.unk1440.field.gate = 2;
    }
    if (fdat_801ef938 == 0 && area_state.unk0 == 0)
        fdat_801ef938 = 1;
    if (!DAT_801cba08.unk1440.bit.f13 && fdat_801ef938 == 1) {
        DAT_801cba08.unk1440.bit.f13 = 1;
        for (i = 0; i < 6; i++) {
            u8 state = vis_table_a.entries[fdat_801ef504[i]].unk05;

            if (state != 3 && state != 5) {
                DAT_801cba08.unk1440.bit.f13 = 0;
                break;
            }
        }
    }
    if (DAT_801cba08.unk1440.bit.f13 == 1 && fdat_801ef930 >= 0) {
        if (fdat_801ef930 == 0) {
            func_801ef1c0();
            fdat_801ef938 = 0xff;
        } else {
            fdat_801ef930--;
        }
    }
    if (fdat_801ef934 == 1 && area_state.unk0 == 0) {
        RECT rect = {0, 487, 256, 1};

        MoveImage(&rect, 0, 489);
        flags = vis_lists.unk47E0;
        fdat_801ef930 = -1;
        fdat_801ef934 = 0;
        vis_lists.unk47E0 = flags | 0x10;
    }
}

/*
 * Sets up the area by the story flags: flag 14 (item 0x12f used) or not,
 * flag 29 swaps two records' parts, gate stage 3 and up places entry 32,
 * and the scene stage (bits 8-9) poses entries 50/51 and table C item 28.
 */
ADDRESS(0x801ed760, 0x378)
void func_801ed760(void)
{
    Vis212 *e49 = &vis_table_a.entries[49];
    Vis212 *e48 = &vis_table_a.entries[48];
    Vis212 *e32 = &vis_table_a.entries[32];
    Vis212 *e51 = &vis_table_a.entries[51];
    Vis212 *e50 = &vis_table_a.entries[50];
    Vis192 *r11 = &vis_table_a.unk3500[11];
    Vis48 *c28 = &vis_table_c.entries[28];
    Vis48 *c29 = &vis_table_c.entries[29];
    Vis48 *c10 = &vis_table_c.entries[10];
    Vis48 *c11 = &vis_table_c.entries[11];
    Vis88 *d15 = &vis_table_b.entries[15];
    Vis88 *d16 = &vis_table_b.entries[16];
    int stage;
    u8 part;

    if (DAT_801cba08.unk1440.bit.f14 == 1) {
        c29->unk22 = 5;
        fdat_801ef93c = 2;
        d15->unk05 = 50;
        d15->unk02 = 0;
        d15->unk10 = 0xfff;
        d15->unk0C = 0xff;
        c29->unk08.vx += 0x1ea;
    } else {
        fdat_801ef93c = 0;
        e49->unk1C.bytes[3] = 1;
        c29->unk22 = 4;
        c11->unk22 = 5;
        c10->unk22 = 5;
        fdat_801ef93c = 0;
        e49->unk70 &= ~0x2000000;
    }
    e48->unk1C.bytes[3] = 1;
    if (DAT_801cba08.unk1440.word >> 29 & 1) {
        part = vis_records[38].part;
        vis_records[38].unk03 = 3;
        vis_records[38].part = vis_records[330].part;
        vis_records[330].part = part;
    }
    if ((int)(DAT_801cba08.unk1440.word >> 10 & 7) >= 3) {
        e32->unk8E = 0x400;
        e32->unk28 = 0x31;
        e32->unk6A = 0x800;
        e32->unk6E = -0x800;
        e32->unk6C = 0;
        e32->unk00 = 100;
        e32->unk06 = vis_records[49].unk01;
        e32->unk70 &= ~0x2000000;
    } else {
        e32->unk00 = 0;
    }
    if (DAT_801cba08.unk1440.word >> 29 & 1)
        e32->unk65 = 0;
    stage = DAT_801cba08.unk1440.word >> 8 & 3;
    fdat_801ef928 = 0;
    if (stage == 1) {
        fdat_801eb7b4(e50, 1);
        e50->unk1C.bytes[3] = 2;
        r11->unk16 = 0x200;
        r11->unk18 = 0x800;
        e51->unk05 = 3;
        fdat_801ef92c = stage;
        c28->unk22 = 4;
        fdat_801ef924 = 4;
    } else if (stage == 2) {
        e50->unk05 = 3;
        e51->unk05 = 3;
        fdat_801ef92c = 2;
        c28->unk22 = 5;
        c28->unk08.vz = 0x41b88;
        d16->unk05 = 50;
        d16->unk10 = 0xfff;
        d16->unk02 = 0;
        d16->unk0C = 0xff;
        fdat_801ef924 = 4;
        c28->unk2D += 3;
    } else {
        vis_table_a.unk3500[10].unk60[0]->unk00 = 0x70;
        vis_table_a.unk3500[10].unk60[1]->unk00 = 0x71;
        vis_table_a.unk3500[10].unk60[2]->unk00 = 0x72;
        e51->unk1C.bytes[3] = 2;
        fdat_801eb7b4(e50, 1);
        e50->unkB0 = 0;
        e50->unk1C.bytes[3] = 2;
        fdat_801eb7b4(e51, 0);
        c28->unk22 = 4;
        fdat_801ef924 = 0;
        fdat_801ef920 = 400;
        fdat_801ef92c = 0;
    }
    if (DAT_801cba08.unk1440.bit.f13 == 1) {
        fdat_801ef934 = 1;
        fdat_801ef930 = -1;
        fdat_801ef938 = 0xff;
    } else {
        fdat_801ef934 = 0;
        fdat_801ef930 = 600;
        fdat_801ef938 = 0;
    }
}

/*
 * The entry's move picker, fdat_801ed4a0 with one difference: the headings
 * stay as they are when no script was picked.
 */
ADDRESS(0x801edad8, 0x24c)
void func_801edad8(void)
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
