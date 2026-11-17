/*
 * FDAT_242.BIN: its first two handlers and the scene word fdat_801ee0bc (WIP module; a unit of its own until its neighbours in
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

#include <st/gfx.h>
#include <st/sound.h>

RODATA(0x801eb72c, 0x10)

/* The texture animation: 1 to start, 2 to move the finished pages, 0 done. */
DATA(0x801ee0bc, 0x4, ".bss")
int fdat_801ee0bc;

void func_801edebc(void);
void func_801edf2c(void);

/* Not exact (one attempt, retries off; 93.3%): first divergence retail `beq v1,a3,<func_801ed4e0+0x1f4>`, this source `beq v1,a3,<func_801ed4e0+0x1e8>`. */
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
    case 1:
        e50->unk68 = (e50->unk68 + 0x800) & 0xfff;
    case 4:
        e50->unk00 = 0;
        e49->unk00 = 0;
        break;
    case 3:
        e50->unk00 = 0;
        e49->unk00 = 100;
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
