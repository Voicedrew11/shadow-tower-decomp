/*
 * FDAT_242.BIN as one translation unit (WIP module): its handler table,
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

RODATA(0x801eb560, 0x28c)

/* Frames the texture animation has run (func_801edebc). */
DATA(0x801ee0b0, 0x4, ".bss")
static int fdat_801ee0b0;

/* The exit scene state: 3 once the animation ends, 4 played. */
DATA(0x801ee0b4, 0x4, ".bss")
int fdat_801ee0b4;

/* The gate scene state: 2 armed, 3 opened by event 0x10e. */
DATA(0x801ee0b8, 0x4, ".bss")
int fdat_801ee0b8;

/* The texture animation: 1 to start, 2 to move the finished pages, 0 done. */
DATA(0x801ee0bc, 0x4, ".bss")
int fdat_801ee0bc;

void func_801ed4e0();
void func_801ed71c();
void func_801edaa4(short id);
void func_801edd54();
void func_801ede3c();

const FdatHeader fdat_header = {
    0x1d,
    {
        fdat_801ed498, func_801ed71c, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, func_801ed4e0, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        func_801edaa4, func_801ede3c, fdat_801ed498, fdat_801ed498,
        fdat_801ed4a0, func_801edd54, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
    },
    0x50,
};

#include "../lib/fdat_common_a.inc"
#include "../lib/fdat_common.inc"
#include "../lib/fdat_common_b.inc"

void func_801edebc(void);
void func_801edf2c(void);

/* Sets up the area by flag 17 and the stream counters of records 12 and
 * 13: entries 48..50, the gate scene stage fdat_801ee0b8 and table B
 * entry 2. */
ADDRESS(0x801ed4e0, 0x23c)
void func_801ed4e0(void)
{
    Vis212 *e48 = &vis_table_a.entries[48];
    Vis212 *e50 = &vis_table_a.entries[50];
    Vis212 *e49 = &vis_table_a.entries[49];
    Vis88 *d2 = &vis_table_b.entries[2];
    int n;

    if (DAT_801cba08.unk1440.word >> 17 & 1) {
        fdat_801ee0bc = 2;
        e48->unk05 = 3;
        fdat_801ee0b4 = 4;
    } else {
        fdat_801ee0bc = 1;
        func_80016878();
        fdat_801ee0b4 = 0;
        e48->unk1C.bytes[3] = 2;
        vis_table_a.entries[33].unk00 = 0;
    }
    n = DAT_801cba08.unk1448[13];
    if (n == 0)
        fdat_801ee0b8 = 0;
    else if (n == 1) {
        if (DAT_801cba08.unk1448[12] == 0)
            fdat_801ee0b8 = n;
        else
            fdat_801ee0b8 = 2;
    } else if (n == 2)
        fdat_801ee0b8 = 3;
    else {
        fdat_801ee0b8 = 4;
        d2->unk4A.door.unk4F = 0xff;
    }
    if (DAT_801cba08.unk1448[13] >= 2)
        e50->unk70 &= ~0x2000000;
    if (DAT_801cba08.unk1448[13] == 3)
        e49->unk70 &= ~0x2000000;
    if (DAT_801cba08.unk1448[12] == 1)
        e48->unk70 &= ~0x2000000;
    if (fdat_801ee0b8 == 2) {
        fdat_801ee0b8 = 3;
        DAT_801cba08.unk1448[13]++;
    }
    switch (fdat_801ee0b8) {
    case 0:
        e50->unk00 = 100;
        e49->unk00 = 0;
        break;
    case 3:
        e50->unk00 = 0;
        e49->unk00 = 100;
        break;
    case 1:
        e50->unk68 = (e50->unk68 + 0x800) & 0xfff;
    case 4:
        e50->unk00 = 0;
        e49->unk00 = 0;
        break;
    }
    e49->unk1C.bytes[3] = 1;
    e50->unk1C.bytes[3] = 1;
}

/*
 * Runs the area: table B entries 4 then 3 open the exit scene
 * (func_801edebc, flag 17), which ends once the viewer leaves area 3; the
 * viewer standing on records 0x126, 0xa0, 0x124 and 0x146 steps the gate
 * scene through entries 50, 48, 50 and 49; and the texture animation
 * starts or its pages are moved.
 */
ADDRESS(0x801ed71c, 0x388)
void func_801ed71c(void)
{
    Vis88 *d4 = &vis_table_b.entries[4];
    Vis212 *e48 = &vis_table_a.entries[48];
    Vis88 *d3 = &vis_table_b.entries[3];
    Vis212 *e33 = &vis_table_a.entries[33];
    Vis212 *e50 = &vis_table_a.entries[50];
    Vis212 *e49 = &vis_table_a.entries[49];
    Vis88 *d2 = &vis_table_b.entries[2];

    if (fdat_801ee0b4 == 0 && d4->unk0C == 0xff) {
        d3->unk0C = 12;
        fdat_801ee0b4 = 1;
        DAT_801cba08.unk1440.word |= 0x20000;
    }
    if (fdat_801ee0b4 == 1 && d3->unk0C == 0xff)
        fdat_801ee0b4 = 2;
    if (fdat_801ee0b4 == 2)
        func_801edebc();
    if (fdat_801ee0b4 == 3 && ((VisRecord *)viewer.unk174)->unk07 != 3) {
        func_801edf2c();
        func_8004ae38(e48);
        e33->unk00 = 100;
    }
    if (fdat_801ee0b8 == 0 && viewer.unk178 == 0x126) {
        fdat_801ed388(e50, 0, 0);
        fdat_801eb7b4(e50, 0);
        fdat_801ecf44(e50);
        e50->unk05 = 0;
        e50->unk00 = 0;
        fdat_801ee0b8 = 1;
        e50->unk68 = (e50->unk68 + 0x800) & 0xfff;
    }
    if (fdat_801ee0b8 == 1 && viewer.unk178 == 0xa0) {
        fdat_801ed388(e48, 0, 0);
        fdat_801ee0b8 = 2;
        e50->unk00 = 100;
        e48->unk70 &= ~0x2000000;
    }
    if (fdat_801ee0b8 == 2 && viewer.unk178 == 0x124) {
        fdat_801ed388(e50, 0, 0);
        fdat_801eb7b4(e50, 0);
        fdat_801ecf44(e50);
        fdat_801ee0b8 = 3;
        e49->unk00 = 100;
        e50->unk70 &= ~0x2000000;
    }
    if (fdat_801ee0b8 == 3 && viewer.unk178 == 0x146) {
        fdat_801ed388(e49, 0, 0);
        fdat_801eb7b4(e49, 0);
        func_80057d58();
        d2->unk4A.door.unk4F = 0xff;
        fdat_801ecf44(e49);
        fdat_801ee0b8 = 4;
        e49->unk70 &= ~0x2000000;
    }
    if (fdat_801ee0bc == 1) {
        RECT a = {480, 64, 64, 96};
        RECT b = {512, 483, 16, 1};

        func_800168e0(&a, 0, 4, 2, 24);
        func_800168e0(&b, 2, 1, 0x31, 24);
        fdat_801ee0bc = 0;
    } else if (fdat_801ee0bc == 2 && area_state.unk0 == 0) {
        func_801edf2c();
        fdat_801ee0bc = 0;
    }
}

/* Event 0x10e opens the armed gate: table A entry 50 dies, 49 wakes, and
 * record stream counter 13 advances. */
ADDRESS(0x801edaa4, 0x6c)
void func_801edaa4(short id)
{
    Vis212 *e = &vis_table_a.entries[50];

    if (id == 0x10e && fdat_801ee0b8 == 2) {
        fdat_801ee0b8 = 3;
        e->unk05 = 3;
        vis_table_a.entries[49].unk00 = 100;
        e->unk70 &= ~0x2000000;
        DAT_801cba08.unk1448[13]++;
    }
}

#include "../lib/fdat_801ed4a0.inc"

/*
 * The step of the overlay's entries, like fdat_801ed6e4: entry record 0
 * restarts script 0 at the end of a run until stream counter 12 starts.
 */
ADDRESS(0x801edd54, 0xe8)
void func_801edd54(void)
{
    Vis212 *e = vis_table_a.unk6138;

    e->unk2A += e->unkB0;
    if (fdat_801ece7c(e) == 1)
        return;
    if (e->unkB0 >= 0 ? (short)e->unk2A >= 4096 : (short)e->unk2A <= 0) {
        if (e->unk02 == 0 && DAT_801cba08.unk1448[12] == 0) {
            fdat_801eb7b4(e, 0);
        } else {
            e->unk1A = 0xff;
            fdat_801ed4a0();
        }
    }
    if (e->unkA8 != NULL) {
        fdat_801ebec4(e);
        e->unk1A = 0xf0;
    }
}

/* In area 0, unless the scene was played, hands table A entry 48 to its
 * exit once its record's stream counter has started. */
ADDRESS(0x801ede3c, 0x80)
void func_801ede3c(void)
{
    Vis212 *e;
    Vis192 *r;

    e = &vis_table_a.entries[48];
    r = &vis_table_a.unk3500[0];

    if (DAT_801cba08.unk1448[r->unk09] != 0 && fdat_801ee0b4 != 4
        && vis_table_a.unk6168 == 0) {
        DAT_801cba08.unk1448[r->unk09] = 1;
        e->unk70 |= 0x2000000;
        fdat_801ed388(e, 1, 1);
    }
}

/* Slows texture animations 0 and 1 every 4th frame for 32 frames, then
 * stops them and ends the scene. */
ADDRESS(0x801edebc, 0x70)
void func_801edebc(void)
{
    StTexAnim *t0 = &world_state.texanim[0];
    StTexAnim *t1 = &world_state.texanim[1];

    if (fdat_801ee0b0 < 32) {
        if ((fdat_801ee0b0 & 3) == 0) {
            t0->period++;
            t1->period++;
        }
        fdat_801ee0b0++;
    } else {
        t0->area = 0xfe;
        t1->area = 0xfe;
        fdat_801ee0b4 = 3;
    }
}

/* Moves the finished texture animation's pages and palette rows into
 * place and ends the exit scene. */
ADDRESS(0x801edf2c, 0x184)
void func_801edf2c(void)
{
    RECT a = {496, 0, 16, 64};
    RECT b = {480, 160, 16, 96};
    RECT c = {800, 224, 32, 32};
    RECT d = {512, 485, 16, 1};
    RECT e = {512, 486, 16, 1};
    RECT f = {0, 490, 256, 1};
    u32 flags;

    MoveImage(&a, 480, 0);
    MoveImage(&b, 480, 64);
    MoveImage(&c, 768, 224);
    MoveImage(&d, 512, 483);
    MoveImage(&e, 512, 484);
    MoveImage(&f, 0, 490);
    flags = vis_lists.unk47E0;
    fdat_801ee0b4 = 4;
    vis_lists.unk47E0 = flags | 8;
}
