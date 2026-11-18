/*
 * FDAT_272.BIN as one translation unit (WIP module): its handler table,
 * the 21 functions every overlay starts with (src/lib/fdat_common_a.inc,
 * fdat_common.inc, fdat_common_b.inc), then its map handlers through the
 * palette blend. One object: the blend reads func_801ed4d0's RECT
 * initializer (cc1 pools one copy of equal constants), the read-only run
 * is one pool, and the scene words are consecutive statics.
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

RODATA(0x801eb560, 0x27c)

/* A palette word: two 15-bit colours and their mask bits. */
typedef struct {
    u32 r0 : 5, g0 : 5, b0 : 5, s0 : 1;
    u32 r1 : 5, g1 : 5, b1 : 5, s1 : 1;
} ClutPair;

/* Table A entries that must all be beaten for flag 13 (func_801ed4d0). */
DATA(0x801ef504, 0x18, ".data")
static int fdat_801ef504[6] = {16, 17, 18, 19, 20, 21};

/* Steps of the blend left; at 32 the two rows are read first. */
DATA(0x801ef51c, 0x4, ".data")
static int fdat_801ef51c = 32;

DATA(0x801ef520, 0x200, ".bss")
static ClutPair fdat_801ef520[128];

DATA(0x801ef720, 0x200, ".bss")
static ClutPair fdat_801ef720[128];

DATA(0x801ef920, 0x4, ".bss")
static int fdat_801ef920;

DATA(0x801ef924, 0x4, ".bss")
static int fdat_801ef924;

DATA(0x801ef928, 0x4, ".bss")
static int fdat_801ef928;

DATA(0x801ef92c, 0x4, ".bss")
static int fdat_801ef92c;

DATA(0x801ef930, 0x4, ".bss")
static int fdat_801ef930;

DATA(0x801ef934, 0x4, ".bss")
static int fdat_801ef934;

DATA(0x801ef938, 0x4, ".bss")
static int fdat_801ef938;

DATA(0x801ef93c, 0x4, ".bss")
static int fdat_801ef93c;

void func_801ed4d0();
void func_801ed760();
void func_801edad8();
void func_801edd24();
void func_801ee43c();
void func_801ee544(short id);

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


void func_801edad8(void);
void func_801eead8(void);
void func_801eebb4(void);
void func_801eec74(void);
void func_801eeeb4(void);
void func_801eea14(void);
void func_801ef07c(void);

/*
 * The current entry's handler: kind 4 runs the item 0x12f scene once,
 * kind 0 steps the gate stages (story flag bits 10-12), kind 10 the scene
 * stages in fdat_801ef924, kind 11 holds its pose; then the animation
 * steps as fdat_801ed6e4 does.
 */
ADDRESS(0x801edd24, 0x718)
void func_801edd24(void)
{
    VisTableA *a = &vis_table_a;
    Vis212 *e48 = &a->entries[48];
    Vis212 *e50 = &a->entries[50];
    Vis212 *e = a->unk6138;
    Vis192 *r = a->unk6134;
    VECTOR pos;
    SVECTOR rot;
    u8 part;

    if (e->unk02 == 4 && fdat_801ef93c == 1) {
        rot.vx = 0;
        rot.vy = -0x400;
        viewer.unk218 = 5;
        pos.vx = e48->unk74.vx - 0xa00;
        pos.vz = e48->unk74.vz;
        fdat_801ecfc4(&pos, &rot);
        func_801eead8();
        rot.vx = 0xc0;
        rot.vy = -0x500;
        fdat_801ecfc4(&pos, &rot);
        func_801eebb4();
        func_801eec74();
        func_80054368(e, 0, 0);
        func_801eeeb4();
        fdat_801ecf44(e);
        fdat_801ef93c = 2;
        viewer.unk218 = 0;
        DAT_801cba08.unk1440.word |= 0x4000;
    }
    if (e->unk02 == 0) {
        switch (DAT_801cba08.unk1440.word >> 10 & 7) {
        case 2:
            viewer.unk218 = 5;
            fdat_801ed388(e, 0, 0);
            fdat_801ed1e4(e);
            func_801ef07c();
            viewer.unk218 = 0;
            e->unk00 = 100;
            e->unk8E = 0x400;
            e->unk28 = 0x31;
            e->unk74.vx = 0x40800;
            e->unk74.vy = 0x41000;
            e->unk74.vz = 0x38800;
            part = vis_records[49].unk01;
            e->unk6A = 0x800;
            e->unk6C = 0;
            e->unk6E = -0x800;
            e->unk06 = part;
            DAT_801cba08.unk1440.word = (DAT_801cba08.unk1440.word & ~0x1c00) | 0xc00;
            break;
        case 3:
            if (viewer.unk178 == 0x2f) {
                fdat_801ed388(e, 0, 0);
                DAT_801cba08.unk1440.word = (DAT_801cba08.unk1440.word & ~0x1c00) | 0x1000;
            } else if (viewer.unk178 == 0x16) {
                DAT_801cba08.unk1448[r->unk09]++;
                DAT_801cba08.unk1440.word = (DAT_801cba08.unk1440.word & ~0x1c00) | 0x1400;
            }
            break;
        case 4:
            if (viewer.unk178 == 0x16)
                DAT_801cba08.unk1440.word = (DAT_801cba08.unk1440.word & ~0x1c00) | 0x1400;
            break;
        case 5:
            if (DAT_801cba08.unk1440.word >> 29 & 1) {
                DAT_801cba08.unk1448[r->unk09]++;
                DAT_801cba08.unk1440.word = (DAT_801cba08.unk1440.word & ~0x1c00) | 0x1800;
            } else {
                e->unk70 |= 0x2000000;
                fdat_801ed388(e, 0, 0);
                e->unk1C.bytes[3] = 2;
                DAT_801cba08.unk1440.word = (DAT_801cba08.unk1440.word & ~0x1c00) | 0x1800;
                e->unk70 &= ~0x2000000;
            }
            break;
        case 6:
            if (e->unk64 != 0) {
                e->unk70 |= 0x2000000;
                func_80054368(e, 0, 0);
                e->unk36 = -1;
                e->unk34 = -1;
                e->unk1C.bytes[3] = 0;
                e->unk1C.bytes[2] &= 0xe2;
                if (e->unk2C == 0)
                    e->unk2C = 1;
                DAT_801cba08.unk1440.word |= 0x1c00;
                e->unk70 &= ~0x2000000;
            }
            break;
        case 1:
        case 7:
            break;
        }
    }
    if (e->unk02 == 10) {
        if (e->unk64 != 0) {
            func_801eea14();
            fdat_801ef924 = 3;
        }
        switch (fdat_801ef924) {
        case 0:
            if (viewer.unk178 == 0xa4) {
                e50->unk36 = -1;
                e50->unk1C.bytes[2] &= 0xf2;
                fdat_801ed388(e, 0, 0);
                fdat_801ef924 = 1;
                e->unk70 &= ~0x2000000;
                DAT_801cba08.unk1440.word = (DAT_801cba08.unk1440.word & ~0x300) | 0x100;
            }
            break;
        case 1:
            if (fdat_801ef920 == 0) {
                func_801eea14();
                fdat_801ef924 = 3;
            } else {
                fdat_801ef920--;
            }
            break;
        case 3:
            if ((short)e->unk2A + e->unkB0 >= 0x1000) {
                func_8004ae38(e);
                fdat_801ef924 = 4;
            }
            break;
        }
    }
    if (e->unk02 == 11) {
        if (fdat_801ef924 == 3 && (short)e->unk2A + e->unkB0 >= 0x1000) {
            e->unk2A = 0xfff;
            e->unkB0 = 0;
        }
        if (fdat_801ef924 == 4) {
            if (fdat_801ef92c == 2)
                fdat_801eb7b4(e, 0);
            else
                fdat_801eb7b4(e, 1);
            e->unk2A = 0xfff;
            e->unkB0 = 0;
        }
    }
    e->unk2A += e->unkB0;
    if (fdat_801ece7c(e) == 1)
        return;
    if (e->unkB0 >= 0 ? (short)e->unk2A >= 4096 : (short)e->unk2A <= 0) {
        if (e->unk02 == 4 && fdat_801ef93c == 0) {
            fdat_801eb7b4(e, 0);
            e->unkB0 = 0;
        }
        if (e->unk02 == 0) {
            if ((DAT_801cba08.unk1440.word >> 10 & 7) == 1) {
                fdat_801eb7b4(e, 0);
            } else {
                e->unk1A = 0xff;
                func_801edad8();
            }
        } else if (e->unk02 == 10) {
            if (fdat_801ef924 < 2) {
                if (fdat_801ef924 < 0)
                    goto repeat_current_script;
                fdat_801eb7b4(e, 0);
            } else {
repeat_current_script:
                fdat_801eb7b4(e, e->unk18);
                return;
            }
        } else if (e->unk02 == 11) {
            if (fdat_801ef92c == 2)
                fdat_801eb7b4(e, 0);
            else
                fdat_801eb7b4(e, 1);
            e->unkB0 = 0;
        } else {
            e->unk1A = 0xff;
            func_801edad8();
        }
    }
    if (e->unkA8 != NULL) {
        fdat_801ebec4(e);
        e->unk1A = 0xf0;
    }
}

void func_801ee610(void);
void func_801ee7d4(void);
void func_801ee948(void);

/* Standing at table B entry 16 with item slot 3 below 50 and the scene not
 * yet run: walks up to it and plays the scene (scene stage 2). */
ADDRESS(0x801ee43c, 0x108)
void func_801ee43c(void)
{
    Vis88 *d = vis_table_b.unk71B0;
    Vis212 *e51 = &vis_table_a.entries[51];
    VECTOR pos;
    SVECTOR rot;

    if (d == &vis_table_b.entries[16] && viewer.unk38[3] < 50 && d->unk0C == 1
        && fdat_801ef92c == 0) {
        pos.vx = d->unk24.vx;
        pos.vz = d->unk24.vz - 0x600;
        rot.vx = 0;
        rot.vy = 0;
        viewer.unk218 = 5;
        fdat_801ecfc4(&pos, &rot);
        func_801ee610();
        fdat_801ed1e4(e51);
        func_801ee7d4();
        func_801ee948();
        viewer.unk218 = 0;
        fdat_801ef924 = 2;
        fdat_801ef92c = 2;
        DAT_801cba08.unk1440.word = (DAT_801cba08.unk1440.word & ~0x300) | 0x200;
    }
}

/* Item 0x12f used in front of table A entry 0x30 sets fdat_801ef93c; the
 * result is 1 then, else 2. */
ADDRESS(0x801ee544, 0xcc)
void func_801ee544(short id)
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
    if (id == 0x12f) {
        if (func_8004bb88((VisLists *)&req, 1) == 0x30) {
            fdat_801ef93c = 1;
            DAT_801cd0e0 = 1;
        } else {
            DAT_801cd0e0 = 2;
        }
    }
}

/* Takes the item of slot 3, recomputes the figures and slots, raises table
 * B entry 16 and slides table C item 28 along. */
ADDRESS(0x801ee610, 0x1c4)
void func_801ee610(void)
{
    Vis88 *d16 = &vis_table_b.entries[16];
    u8 *speed = &d16->unk48;
    Vis48 *c28 = &vis_table_c.entries[28];
    Data801c7590 *items = &DAT_801c7590;
    Vis44 *kind;
    int step;

    kind = &vis_table_c.unk0118[c28->unk00];
    *(u8 *)&items->counts[viewer.unk38[3]] = 0;
    viewer.unk38[3] = 0xffff;
    func_8005a3a8(0);
    func_8003d030();
    func_8003f12c(viewer.unk38[3], 0, 0);
    func_8003f12c(viewer.unk38[16], 1, 0);
    d16->unk05 = 50;
    d16->unk02 = 0;
    d16->unk10 = 0;
    c28->unk22 = 5;
    c28->unk2D += 3;
    for (;;) {
        StAnimFile *file;

        if (c28->unk08.vz > d16->unk24.vz - 0x300)
            goto item_placed;
        anim_get_file(kind->unk4 + 588, &file);
        if (file != NULL)
            c28->unk08.vz += 36;
        func_80026dd4(NULL, NULL, 0);
    }
item_placed:
    step = *speed * 4;
    for (;;) {
        int v;
        int next;

        func_8004ed54(d16, d16->unk0a * 3 + 0xd0);
        v = d16->unk10;
        next = v + step;
        if (v == 0xfff) {
            d16->unk0C = 0xff;
            break;
        }
        if (next >= 0x1000)
            d16->unk10 = 0xfff;
        else
            d16->unk10 = next;
        c28->unk08.vz += 12;
        func_80026dd4(NULL, NULL, 0);
    }
}

/* Entries 51 and 50 play animations 3 and 0 to the end (51 once 50 is half
 * through), table B entry 19 steps from state 11 to 12, and the view
 * pitches up 16 a frame for 16 frames. */
ADDRESS(0x801ee7d4, 0x174)
void func_801ee7d4(void)
{
    Vis212 *e51 = &vis_table_a.entries[51];
    Vis88 *d19 = &vis_table_b.entries[19];
    Vis212 *e50;
    int n;

    fdat_801eb7b4(e51, 3);
    e50 = e51 - 1;
    fdat_801eb7b4(e50, 0);
    e51->unk2A = 0;
    e50->unk2A = 0;
    do {
        fdat_801eba4c(e50);
        fdat_801eba4c(e51);
        e50->unk2A += e50->unkB0;
        if ((short)e50->unk2A > 0x800)
            e51->unk2A += e51->unkB0;
        if ((short)e51->unk2A >= 0x1000)
            e51->unk2A = 0xfff;
        if ((short)e50->unk2A >= 0x1000)
            e50->unk2A = 0xfff;
        func_80026dd4(NULL, NULL, 0);
    } while ((short)e51->unk2A != 0xfff);
    if (d19->unk0C == 11)
        d19->unk0C = 12;
    n = 16;
    while (n--) {
        viewer.unk278.vx += 16;
        viewer.unk268 = viewer.unk278.vx + viewer.unk280.halves[0] + viewer.unk288 + viewer.unk290;
        func_80026dd4(NULL, (SVECTOR *)&viewer.unk268, 0);
    }
}

/* Bumps record 10's stream counter, sends entry 51 away (its scripts 0..2
 * become 0x80, 5..7 0x70..0x72) and restarts entry 50. */
ADDRESS(0x801ee948, 0xcc)
void func_801ee948(void)
{
    Vis212 *e = &vis_table_a.entries[51];
    Vis192 *r = &vis_table_a.unk3500[10];

    DAT_801cba08.unk1448[r->unk09]++;
    e->unk70 |= 0x2000000;
    r->unk60[0]->unk00 = 0x80;
    r->unk60[1]->unk00 = 0x80;
    r->unk60[2]->unk00 = 0x80;
    r->unk60[5]->unk00 = 0x70;
    r->unk60[6]->unk00 = 0x71;
    r->unk60[7]->unk00 = 0x72;
    func_80054368(e, 0, 0);
    fdat_801ecf44(e);
    e--;
    fdat_801eb7b4(e, 0);
    func_8004ae38(e);
}

/* Once: entry 51 plays animation 8 and entry 50 animation 1, record 11's
 * ranges are set and the scene stage becomes 1. */
ADDRESS(0x801eea14, 0xc4)
void func_801eea14(void)
{
    Vis212 *e51 = &vis_table_a.entries[51];
    Vis212 *e50 = e51 - 1;
    Vis192 *r = &vis_table_a.unk3500[11];

    if (fdat_801ef928 == 0) {
        e51->unk1C.bytes[3] = 1;
        fdat_801ef920 = -1;
        e51->unk70 &= ~0x2000000;
        fdat_801eb7b4(e51, 8);
        fdat_801eb7b4(e50, 1);
        fdat_801ef92c = 1;
        r->unk16 = 0x200;
        r->unk18 = 0x800;
        fdat_801ef928 = 1;
        DAT_801cba08.unk1440.word = (DAT_801cba08.unk1440.word & ~0x300) | 0x100;
    }
}

/* Slides table C item 29 by 0x1ea, 24 a frame while its model is loaded. */
ADDRESS(0x801eead8, 0xdc)
void func_801eead8(void)
{
    Vis48 *item = &vis_table_c.entries[29];
    Vis44 *kind = &vis_table_c.unk0118[303];
    int step = 24;
    int top;

    item->unk22 = 5;
    top = item->unk08.vx + 0x1ea;
    item->unk2D -= 2;
    for (;;) {
        StAnimFile *file;

        if (item->unk08.vx == top)
            break;
        anim_get_file(kind->unk4 + 588, &file);
        if (file != NULL)
            item->unk08.vx += step;
        if (item->unk08.vx > top)
            item->unk08.vx = top;
        func_80026dd4(NULL, NULL, 0);
    }
    item->unk2D += 2;
}

/* Raises table B entry 15 until its state is 0xfff, then marks it open. */
ADDRESS(0x801eebb4, 0xc0)
void func_801eebb4(void)
{
    Vis88 *d = &vis_table_b.entries[15];
    int step;

    d->unk05 = 50;
    d->unk02 = 0;
    d->unk10 = 0;
    step = d->unk48 * 4;
    do {
        if (d->unk10 == 0)
            func_8004ed54(d, d->unk0a * 3 + 0xd0);
        d->unk10 += step;
        if (d->unk10 >= 0x1000)
            d->unk10 = 0xfff;
        func_80026dd4(NULL, NULL, 0);
    } while (d->unk10 != 0xfff);
    d->unk0C = 0xff;
}

/* Not exact (registers): retail takes the two to-products in a3 and v1, this
 * source in v0, and loads unk280 before unk288. Here global-alloc puts both
 * to-products in LO, reload spills LO for the from-product's output reload
 * and the retry gives them v0 (free at that point); retail's retry skipped
 * v0 for both and a1/a2 for the first, which fits a retry with v0 already
 * taken as a reload register. Operand order, a total - n local and x/y
 * locals were ruled out. */
/* Entry 49 plays animation 0 to its end while the view turns from its
 * pitch and yaw to (0, -0x400). */
ADDRESS(0x801eec74, 0x240)
void func_801eec74(void)
{
    SVECTOR from;
    SVECTOR to;
    Vis212 *e;
    int n;
    int total;

    from = viewer.unk278;
    e = &vis_table_a.entries[49];
    to.vy = -0x400;
    to.vx = 0;
    if (from.vx > 2048)
        from.vx -= 4096;
    if (from.vy > 2048)
        from.vy -= 4096;
    fdat_801eb7b4(e, 0);
    n = 0x1000 / e->unkB0;
    total = n;
    do {
        fdat_801eba4c(e);
        e->unk2A += e->unkB0;
        if ((short)e->unk2A >= 0x1000)
            e->unk2A = 0xfff;
        viewer.unk278.vx = (from.vx * n + to.vx * (total - n)) / total;
        viewer.unk268 = viewer.unk278.vx + viewer.unk280.halves[0] + viewer.unk288 + viewer.unk290;
        viewer.unk278.vy = (from.vy * n + to.vy * (total - n)) / total;
        viewer.unk26A = viewer.unk278.vy + viewer.unk280.halves[1] + viewer.unk28A + viewer.unk292;
        func_80026dd4(NULL, (SVECTOR *)&viewer.unk268, 0);
        n--;
    } while ((short)e->unk2A != 0xfff);
    e->unk70 |= 0x2000000;
}

/*
 * Entry 49 plays animation 4 and lifts its arm a frame at a time; table C
 * item 10 (11 once 10 is taken) rides on vertex 0x160 until the arm is up.
 * Then the view settles and the item drops.
 */
ADDRESS(0x801eeeb4, 0x1c8)
void func_801eeeb4(void)
{
    Vis212 *e = &vis_table_a.entries[49];
    Vis48 *c10 = &vis_table_c.entries[10];
    Vis48 *c11 = &vis_table_c.entries[11];
    Vis48 *c;
    SVECTOR v;
    SVECTOR unused;

    fdat_801eb7b4(e, 4);
    c = c10;
    if (vis_table_c.unk0070[c10->unk00 / 32] & (1 << (c10->unk00 % 32)))
        c = c11;
    do {
        fdat_801eba4c(e);
        e->unk2A += e->unkB0;
        if ((s16)e->unk2A >= 0x1000)
            e->unk2A = 0xfff;
        anim_get_vertex(e->unk0B + 128, e->unk08, (s16)e->unk2A, 0x160, &v);
        c->unk08.vx = e->unk74.vx - v.vx;
        c->unk08.vy = e->unk74.vy + v.vy;
        c->unk08.vz = e->unk74.vz - v.vz;
        func_80026dd4(NULL, &viewer.unk278, 0);
    } while ((s16)e->unk2A != 0xfff);
    c->unk22 = 2;
    viewer.unk268 = viewer.unk278.vx + viewer.unk280.halves[0] + viewer.unk288 + viewer.unk290;
    viewer.unk26A = viewer.unk278.vy + viewer.unk280.halves[1] + viewer.unk28A + viewer.unk292;
    viewer.unk26C = viewer.unk278.vz + viewer.unk284 + viewer.unk28C + viewer.unk294;
    viewer.unk270.vx = 4096 - viewer.unk268;
    viewer.unk270.vy = 4096 - viewer.unk26A;
    viewer.unk270.vz = 4096 - viewer.unk26C;
    RotMatrixZYX(&viewer.unk270, &viewer.unk298);
    func_8005335c(c, 1);
}

/* Entry 32 plays animation 9 (the ground shakes through its middle part
 * and it starts moving near the end), then walks off along x to 0x3f000. */
ADDRESS(0x801ef07c, 0x144)
void func_801ef07c(void)
{
    Vis212 *e = &vis_table_a.entries[32];

    fdat_801eb7b4(e, 9);
    do {
        fdat_801eba4c(e);
        e->unk2A += e->unkB0;
        if ((short)e->unk2A >= 0x1000)
            e->unk2A = 0xfff;
        if (e->unk2A > 0xa00 && e->unk2A < 0xdfc)
            fdat_801eb814(0x400, 0x80);
        if ((short)e->unk2A > 0xdfc)
            e->unk74.vx += 0x40;
        func_80026dd4(NULL, NULL, 0);
    } while ((short)e->unk2A != 0xfff);
    fdat_801eb7b4(e, 1);
    do {
        fdat_801eba4c(e);
        e->unk2A += e->unkB0;
        if ((short)e->unk2A > 0x1000)
            e->unk2A = 0;
        e->unk74.vx += 0x40;
        func_80026dd4(NULL, NULL, 0);
    } while (e->unk74.vx <= 0x3f000);
}

/*
 * Blends palette row 489 from row 487 by fdat_801ef51c / 32, a step a
 * call; at 0 loads row 487's copy and ends.
 */
ADDRESS(0x801ef1c0, 0x344)
void func_801ef1c0(void)
{
    RECT dst = {0, 489, 256, 1};
    RECT src = {0, 487, 256, 1};
    ClutPair buf[128];
    int i;
    int t;
    int u;
    u32 flags;

    if (fdat_801ef51c == 32) {
        StoreImage(&dst, (u_long *)fdat_801ef520);
        StoreImage(&src, (u_long *)fdat_801ef720);
    }
    t = fdat_801ef51c;
    u = 32 - t;
    for (i = 0; i < 128; i++) {
        buf[i].b0 = (fdat_801ef520[i].b0 * t + fdat_801ef720[i].b0 * u) >> 5;
        buf[i].g0 = (fdat_801ef520[i].g0 * t + fdat_801ef720[i].g0 * u) >> 5;
        buf[i].r0 = (fdat_801ef520[i].r0 * t + fdat_801ef720[i].r0 * u) >> 5;
        buf[i].b1 = (fdat_801ef520[i].b1 * t + fdat_801ef720[i].b1 * u) >> 5;
        buf[i].g1 = (fdat_801ef520[i].g1 * t + fdat_801ef720[i].g1 * u) >> 5;
        buf[i].r1 = (fdat_801ef520[i].r1 * t + fdat_801ef720[i].r1 * u) >> 5;
        buf[i].s0 = fdat_801ef520[i].s0;
        buf[i].s1 = fdat_801ef520[i].s1;
    }
    LoadImage(&dst, (u_long *)buf);
    if (fdat_801ef51c == 0) {
        LoadImage(&dst, (u_long *)fdat_801ef720);
        flags = vis_lists.unk47E0;
        fdat_801ef930 = -1;
        vis_lists.unk47E0 = flags | 0x10;
    } else {
        fdat_801ef51c--;
    }
}
