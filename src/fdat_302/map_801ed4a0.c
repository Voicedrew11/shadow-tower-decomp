/*
 * FDAT_302.BIN: its first two handlers and two scene words after its code (WIP module; a unit of its own until its neighbours in
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

/* 1 while table B entries 1..6 are all open (flags 16, 18, 20, 24, 27,
 * 28). */
DATA(0x801ee46c, 0x4, ".bss")
static int fdat_801ee46c;

/* The viewer's progress through the hall (0..3). */
DATA(0x801ee470, 0x4, ".bss")
static int fdat_801ee470;

void func_801edd9c(int which);
void func_801edea0(void);
void func_801ee090(int arg);
void func_801ee0b8(void);

/* Sets up the area: table B entries 1..6 closed unless their flags are
 * set (entry 9 off when all are open), entries 4 and 5 placed, the scene
 * words cleared. */
ADDRESS(0x801ed4a0, 0x110)
void func_801ed4a0(void)
{
    Vis88 *d1 = &vis_table_b.entries[1];
    Vis88 *d9 = &vis_table_b.entries[9];
    Vis212 *e4 = &vis_table_a.entries[4];
    Vis212 *e5 = &vis_table_a.entries[5];
    u32 w = DAT_801cba08.unk1440.word;

    fdat_801ee46c = 1;
    if (!(w >> 16 & 1)) {
        d1[0].unk00 = 2;
        fdat_801ee46c = 0;
    }
    if (!(w >> 18 & 1)) {
        d1[1].unk00 = 2;
        fdat_801ee46c = 0;
    }
    if (!(w >> 20 & 1)) {
        d1[2].unk00 = 2;
        fdat_801ee46c = 0;
    }
    if (!(w >> 24 & 1)) {
        d1[3].unk00 = 2;
        fdat_801ee46c = 0;
    }
    if (!(w >> 27 & 1)) {
        d1[4].unk00 = 2;
        fdat_801ee46c = 0;
    }
    if (!(w >> 28 & 1)) {
        d1[5].unk00 = 2;
        fdat_801ee46c = 0;
    }
    if (fdat_801ee46c)
        d9->unk05 = 0xff;
    e4->unk07 = 1;
    e5->unk07 = 2;
    fdat_801ee474 = 0;
    fdat_801ee478 = 0;
    fdat_801ee47c = 0;
    fdat_801ee480 = 0;
}

/*
 * Runs the area: with all doors open, table B entry 9's square starts the
 * hall scene; the viewer crossing z 0x46800 / 0x40800 moves the hall
 * along; entries 0, 3 and 5 are kept in range by their distance; entry 0
 * dying starts its scene, entries 3 and 5 leave once beaten.
 */
ADDRESS(0x801ed5b0, 0x46c)
void func_801ed5b0(void)
{
    Vis88 *d9 = &vis_table_b.entries[9];
    Vis212 *e0 = &vis_table_a.entries[0];
    Vis212 *e3 = &vis_table_a.entries[3];
    Vis212 *e5 = &vis_table_a.entries[5];

    if (fdat_801ee46c != 0
        && func_8004fd28(d9->unk24.vx, d9->unk24.vz, 0xc00, 0xc00, d9->unk24.vy)) {
        func_801edea0();
        fdat_801ee470 = 1;
    }
    if (viewer.z > 0x46800) {
        func_801edd9c(0);
        fdat_801ee470++;
    }
    if (viewer.z < 0x40800) {
        func_801edd9c(1);
        fdat_801ee470--;
    }
    if (e0->unk40 >= 0x6000)
        e0->unk03 = 2;
    if (e3->unk40 >= 0x6000)
        e3->unk03 = 2;
    if (e5->unk40 >= 0x6000)
        e5->unk03 = 2;
    if (e0->unk40 <= 0x6000)
        e0->unk03 = 0;
    if (e3->unk40 <= 0x6000)
        e3->unk03 = 0;
    if (e5->unk40 <= 0x6000)
        e5->unk03 = 0;
    if (fdat_801ee478 != 0xff && fdat_801ee470 >= 2)
        fdat_801ee470 = 1;
    if (e0->unk05 == 5) {
        e0->unk05 = 1;
        e0->unk1C.bytes[3] = 1;
        e0->unk34 = -1;
        e0->unk1C.bytes[2] &= 0xea;
        fdat_801eb7b4(e0, 6);
        fdat_801ee474 = 1;
    }
    if (fdat_801ee474 == 2)
        func_801ee0b8();
    if (fdat_801ee478 == 0 && e3->unk2C == 0)
        fdat_801ee478 = 1;
    if (fdat_801ee478 == 1 && e3->unk05 == 3) {
        e3->unk70 |= 0x2000000;
        fdat_801ed388(e3, 1, 0);
        fdat_801ee478 = 0xff;
    }
    if (fdat_801ee480 == 0 && e5->unk2C != (e5->unk02 + vis_table_a.unk3500)->unk32)
        fdat_801ee480 = 1;
    if (fdat_801ee480 == 2 && e5->unk2C == 0)
        fdat_801ee480 = 3;
    if (fdat_801ee480 == 1) {
        e5->unk70 |= 0x2000000;
        fdat_801ed388(e5, 1, 0);
        fdat_801ee480 = 2;
        e5->unk70 &= ~0x2000000;
    }
    if (fdat_801ee480 == 3 && e5->unk05 == 3) {
        e5->unk70 |= 0x2000000;
        fdat_801ed388(e5, 1, 0);
        fdat_801ee480 = 0xff;
        DAT_800881a4 = 3;
        DAT_801cba08.unk1444.word |= 0x80;
    }
    if (fdat_801ee47c == 1 && viewer.z > 0x41000) {
        viewer.unk218 = 5;
        fdat_801ed388(e5, 0, 0);
        e5->unk1C.bytes[3] = 0;
        fdat_801ee47c = 0xff;
        viewer.unk218 = 0;
        e5->unk70 &= ~0x2000000;
    }
    if (fdat_801ee47c == 0 && fdat_801ee470 == 3) {
        func_801ee090(2);
        e5->unk00 = 100;
        e5->unk1C.bytes[3] = 1;
        fdat_801ee47c = 1;
        e5->unk70 |= 0x2000000;
    }
}
