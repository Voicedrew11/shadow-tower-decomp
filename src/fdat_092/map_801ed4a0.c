/*
 * FDAT_092.BIN: its first two handlers and the scene words after its code (WIP module; a unit of its own until its neighbours in
 * the run are exact).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <psyq/libc.h>
#include <st/vis.h>
#include <st/data.h>
#include <st/anim.h>
#include <st/hud.h>
#include <st/menu.h>
#include <st/light.h>
#include <st/xa.h>
#include <st/fdat.h>

/* 0xff once the gate scene (flag 44) ran. */
DATA(0x801edf54, 0x4, ".bss")
static int fdat_801edf54;

/* 0xff once table B entries 5/6 are open (flag 46). */
DATA(0x801edf58, 0x4, ".bss")
static int fdat_801edf58;

/* 0xff once table B entries 7/8 are open (flag 45). */
DATA(0x801edf5c, 0x4, ".bss")
static int fdat_801edf5c;

/* 1 once both pairs are open, 0xff after the lighting change. */
DATA(0x801edf60, 0x4, ".bss")
static int fdat_801edf60;

/* Table A entry 39's stage: 0 asleep, 1 rising, 2 sinking, 0xff done. */
DATA(0x801edf64, 0x4, ".bss")
static int fdat_801edf64;

/* Table A entry 38's stage, as fdat_801edf64. */
DATA(0x801edf68, 0x4, ".bss")
static int fdat_801edf68;

/* Entry 39's resting height. */
DATA(0x801edf6c, 0x4, ".bss")
static int fdat_801edf6c;

/* Entry 38's resting height. */
DATA(0x801edf70, 0x4, ".bss")
static int fdat_801edf70;

void func_801ede14(void);
void func_801edea4(void);

/* Sets up the area by the second flag word's bits 12..17: the gate scene,
 * table B entry pairs 5/6 and 7/8, and table A entries 38 and 39 (raised
 * out of reach until their flags are set). */
ADDRESS(0x801ed4a0, 0x1e0)
void func_801ed4a0(void)
{
    Vis212 *e48 = &vis_table_a.entries[48];
    Vis88 *d5 = &vis_table_b.entries[5];
    Vis88 *d6 = &vis_table_b.entries[6];
    Vis88 *d7 = &vis_table_b.entries[7];
    Vis88 *d8 = &vis_table_b.entries[8];
    Vis48 *c5 = &vis_table_c.entries[5];
    Vis212 *e39 = &vis_table_a.entries[39];
    Vis212 *e38 = &vis_table_a.entries[38];

    if (DAT_801cba08.unk1444.bit.f12 == 1) {
        vis_table_a.entries[40].unk00 = 100;
        c5->unk22 = 4;
        fdat_801edf54 = 0xff;
    } else {
        fdat_801edf54 = 0;
        c5->unk22 = 4;
        e48->unk1C.bytes[3] = 1;
    }
    if (DAT_801cba08.unk1444.bit.f13 == 1) {
        d8->unk00 = 2;
        d7->unk00 = 2;
        fdat_801edf5c = 0xff;
    } else {
        fdat_801edf5c = 0;
    }
    if (DAT_801cba08.unk1444.bit.f14 == 1) {
        d6->unk00 = 2;
        d5->unk00 = 2;
        fdat_801edf58 = 0xff;
    } else {
        fdat_801edf58 = 0;
    }
    if (fdat_801edf58 == 0xff && fdat_801edf5c == 0xff)
        fdat_801edf60 = 0xff;
    else
        fdat_801edf60 = 0;
    if (e39->unk05 != 3) {
        if (DAT_801cba08.unk1444.bit.f16 == 1) {
            e39->unk00 = 100;
            fdat_801edf64 = 0xff;
        } else {
            fdat_801edf64 = 0;
            fdat_801edf6c = e39->unk74.vy;
            e39->unk74.vy += 0xc00;
        }
    }
    if (e38->unk05 != 3) {
        if (DAT_801cba08.unk1444.bit.f17 == 1) {
            e38->unk00 = 100;
            fdat_801edf68 = 0xff;
        } else {
            fdat_801edf68 = 0;
            fdat_801edf70 = e38->unk74.vy;
            e38->unk74.vy += 0xa00;
        }
    }
}

/* Not exact (one attempt, retries off; 62.2%): first divergence retail `lui v0,0x0`, this source `sw s1,20(sp)`. */
/*
 * Runs the area: the gate scene once record 14's stream counter is 1;
 * opens table B entry pairs 7/8 and 5/6 once table A entries 32 and 35 are
 * beaten (lighting change when both are); raises entries 39 and 38 when
 * table B entries 14 and 17 are triggered and lowers them back.
 */
ADDRESS(0x801ed680, 0x4a0)
void func_801ed680(void)
{
    Vis212 *e48 = &vis_table_a.entries[48];
    Vis88 *d5 = &vis_table_b.entries[5];
    Vis192 *r14 = &vis_table_a.unk3500[14];
    Vis212 *e40 = &vis_table_a.entries[40];
    Vis212 *e39 = &vis_table_a.entries[39];
    Vis212 *e38 = &vis_table_a.entries[38];

    if (fdat_801edf54 == 0 && DAT_801cba08.unk1448[r14->unk09] == 1) {
        viewer.unk218 = 5;
        func_801ede14();
        if (!(vis_table_c.unk0070[1] & 0x10000))
            func_801edea4();
        fdat_801ecf44(e48);
        fdat_801ed410(e40);
        e40->unk36 = -1;
        e40->unk03 = 1;
        e40->unk1C.bytes[2] &= 0xf2;
        e40->unk70 |= 0x2000000;
        fdat_801ed388(e40, 1, 0);
        fdat_801ed1e4(e40);
        fdat_801ed388(e40, 0, 0);
        e48->unk00 = 100;
        viewer.unk218 = 0;
        fdat_801edf54 = 0xff;
        e40->unk70 &= ~0x2000000;
        DAT_801cba08.unk1444.word |= 0x1000;
    }
    if (fdat_801edf5c == 0 && vis_table_a.entries[32].unk2C == 0) {
        vis_table_b.entries[8].unk00 = 2;
        vis_table_b.entries[7].unk00 = 2;
        fdat_801edf5c = 0xff;
        DAT_801cba08.unk1444.word |= 0x2000;
        if (fdat_801edf58 == 0xff)
            fdat_801edf60 = 1;
    }
    if (fdat_801edf58 == 0 && vis_table_a.entries[35].unk2C == 0) {
        vis_table_b.entries[6].unk00 = 2;
        d5->unk00 = 2;
        fdat_801edf58 = 0xff;
        DAT_801cba08.unk1444.word |= 0x4000;
        if (fdat_801edf5c == 0xff)
            fdat_801edf60 = 1;
    }
    if (fdat_801edf60 == 1) {
        if (!DAT_801cba08.unk1440.bit.f24)
            lighting.area_mode[9] = 0x41;
        fdat_801edf60 = 0xff;
    }
    if (fdat_801edf64 == 0 && vis_table_b.entries[14].unk0C == 1) {
        fdat_801ed410(e39);
        e39->unk36 = -1;
        e39->unk1C.bytes[2] &= 0xf2;
        fdat_801eb7b4(e39, 0);
        e39->unk00 = 100;
        e39->unk1C.bytes[3] = 1;
        e39->unk07 = vis_table_a.unk6103;
        fdat_801edf64 = 1;
        e39->unk74.vy += 0xa00;
        DAT_801cba08.unk1444.word |= 0x10000;
    }
    if (fdat_801edf68 == 0 && vis_table_b.entries[17].unk0C == 1) {
        fdat_801ed410(e38);
        e38->unk36 = -1;
        e38->unk1C.bytes[2] &= 0xf2;
        fdat_801eb7b4(e38, 0);
        e38->unk00 = 100;
        e38->unk1C.bytes[3] = 1;
        e38->unk07 = vis_table_a.unk6103;
        fdat_801edf68 = 1;
        e38->unk74.vy += 0xa00;
        DAT_801cba08.unk1444.word |= 0x20000;
    }
    if (fdat_801edf64 == 1 && vis_table_b.entries[14].unk10 > 0xc00)
        fdat_801edf64 = 2;
    if (fdat_801edf68 == 1 && vis_table_b.entries[17].unk10 > 0xc00)
        fdat_801edf68 = 2;
    if (fdat_801edf64 == 2) {
        if (fdat_801edf6c < e39->unk74.vy) {
            e39->unk74.vy -= 0x100;
        } else {
            e39->unk1C.bytes[3] = 0;
            fdat_801edf64 = 0xff;
        }
    }
    if (fdat_801edf68 == 2) {
        if (fdat_801edf70 < e38->unk74.vy) {
            e38->unk74.vy -= 0x100;
        } else {
            e38->unk1C.bytes[3] = 0;
            fdat_801edf68 = 0xff;
        }
    }
}
