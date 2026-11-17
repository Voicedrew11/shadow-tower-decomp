/*
 * FDAT_262.BIN: its first three handlers and five scene words after its code (WIP module; a unit of its own until its neighbours in
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

/* 0xff once record 13's stream counter reached 1 (or entry 50 was hit). */
DATA(0x801ee374, 0x4, ".bss")
static int fdat_801ee374;

/* Table A entry 33 sent off: 0 not yet, 0xff done. */
DATA(0x801ee378, 0x4, ".bss")
static int fdat_801ee378;

/* 0xff once table A entry 49 is beaten. */
DATA(0x801ee37c, 0x4, ".bss")
static int fdat_801ee37c;

/* Item 0x129 on table B entries 0x16 and 0x17: 1 due, 0xff done (second
 * flag word bits 10 and 11). */
DATA(0x801ee380, 0x4, ".bss")
static int fdat_801ee380;

DATA(0x801ee384, 0x4, ".bss")
static int fdat_801ee384;

void func_801edfb4(void);
void func_801ee140(int which);
void func_801ee278(int which);

/* Not exact (one attempt, retries off; 74.3%): first divergence retail `addiu sp,sp,-16`, this source `addiu sp,sp,-8`. */
/* Sets up the area by flags 14, 29, 30 and the second word's bits 10/11:
 * entry 48 (asleep, awake or gone), entries 50, 33 and 49, table B entry
 * 0 and table C items 7..10, 28 and 29. */
ADDRESS(0x801ed4a0, 0x2a0)
void func_801ed4a0(void)
{
    Vis212 *e48 = &vis_table_a.entries[48];
    Vis192 *r1 = &vis_table_a.unk3500[1];
    Vis212 *e50 = &vis_table_a.entries[50];
    Vis192 *r13 = &vis_table_a.unk3500[13];
    Vis212 *e33 = &vis_table_a.entries[33];
    Vis192 *r4 = &vis_table_a.unk3500[4];
    Vis212 *e49 = &vis_table_a.entries[49];
    Vis48 *c28 = &vis_table_c.entries[28];
    Vis48 *c29 = &vis_table_c.entries[29];
    Vis48 *c7 = &vis_table_c.entries[7];
    Vis48 *c8 = &vis_table_c.entries[8];
    Vis48 *c9 = &vis_table_c.entries[9];
    Vis88 *d0 = &vis_table_b.entries[0];
    Vis48 *c10 = &vis_table_c.entries[10];

    if (e48->unk05 != 3 && DAT_801cba08.unk1440.bit.f29 != 1 && DAT_801cba08.unk1440.bit.f14) {
        if (DAT_801cba08.unk1440.bit.f30 == 1) {
            fdat_801ee370 = 0xfe;
            e48->unk70 &= ~0x2000000;
        } else {
            fdat_801ee370 = 0;
            e48->unk1C.bytes[3] = 3;
            r1->unk60[13]->unk00 = 0x72;
            r1->unk60[14]->unk00 = 0x80;
        }
    } else {
        vis_table_a.entries[48].unk00 = 0;
        fdat_801ee370 = 0xff;
    }
    if (e50->unk05 != 3)
        e50->unk1C.bytes[3] = 2;
    if (DAT_801cba08.unk1448[r13->unk09] == 1)
        fdat_801ee374 = 0xff;
    else
        fdat_801ee374 = 0;
    if (e33->unk05 != 3) {
        e33->unk1C.bytes[3] = 3;
        d0->unk0C = 11;
    }
    if (DAT_801cba08.unk1448[r4->unk09] == 1) {
        fdat_801ee378 = 0xff;
        e33->unk70 &= ~0x2000000;
        r4->unk1C &= ~0x180;
    } else {
        fdat_801ee378 = 0;
    }
    if (e49->unk05 == 3)
        fdat_801ee37c = 0xff;
    else
        fdat_801ee37c = 0;
    if (!(DAT_801cba08.unk1444.word >> 10 & 1)) {
        c9->unk22 = 4;
        c7->unk22 = 4;
        c28->unk22 = 4;
        fdat_801ee380 = 0;
    } else {
        c28->unk22 = 5;
        fdat_801ee380 = 0xff;
        c28->unk08.vx -= 0x200;
        c28->unk2D -= 1;
    }
    if (!(DAT_801cba08.unk1444.word >> 11 & 1)) {
        c10->unk22 = 4;
        c8->unk22 = 4;
        c29->unk22 = 4;
        fdat_801ee384 = 0;
    } else {
        c29->unk22 = 5;
        fdat_801ee384 = 0xff;
        c29->unk08.vx += 0x200;
        c29->unk2D -= 1;
    }
}

/* Not exact: the dead prologue LUI for vis_table_b has no curated retail
 * relocation. The remaining instruction and relocation listings agree. */
/*
 * Runs the area: entry 48 wakes when hit or once record 1's counter is 2
 * (func_801edfb4's scene) and changes its scripts after its flash; entry
 * 50 leaves; the viewer on records 0x9c, 0xe9, 0xa0, 0xa2 or 0xa4 sends
 * entry 33 until entry 49 is beaten; item 0x129's two scenes run.
 */
ADDRESS(0x801ed740, 0x3cc)
void func_801ed740(void)
{
    Vis212 *e48 = &vis_table_a.entries[48];
    Vis192 *r1 = &vis_table_a.unk3500[1];
    Vis212 *e50 = &vis_table_a.entries[50];
    Vis192 *r13 = &vis_table_a.unk3500[13];
    Vis212 *e33 = &vis_table_a.entries[33];
    Vis192 *r4 = &vis_table_a.unk3500[4];
    Vis212 *e49 = &vis_table_a.entries[49];
    Vis88 *d0 = &vis_table_b.entries[0];

    if (fdat_801ee370 == 0) {
        if (DAT_801cba08.unk1448[r1->unk09] == 2) {
            func_801edfb4();
            fdat_801ecf44(e48);
            viewer.unk218 = 0;
            fdat_801ee370 = 0xff;
        }
        if (e48->unk64 == 1) {
            fdat_801ee370 = 0xfd;
            e48->unk1C.bytes[2] &= 0xea;
            e48->unk70 &= ~0x2000000;
            DAT_801cba08.unk1440.word |= 0x40000000;
        }
    }
    if (fdat_801ee370 == 0xfd && e48->unk34 == -1) {
        DAT_801cba08.unk1448[r1->unk09] = 2;
        r1->unk60[13]->unk00 = 0x80;
        r1->unk60[14]->unk00 = 0x72;
        e48->unk70 |= 0x2000000;
        fdat_801ed388(e48, 0, 0);
        fdat_801ee370 = 0xfe;
        e48->unk1C.bytes[3] = 0;
        e48->unk70 &= ~0x2000000;
    }
    if (e48->unk2C == 0)
        DAT_801cba08.unk1440.word |= 0x20000000;
    if (fdat_801ee374 != 0xff && DAT_801cba08.unk1448[r13->unk09] == 1) {
        fdat_801ecf44(e50);
        fdat_801ee374 = 0xff;
    }
    if (e50->unk64 != 0) {
        e50->unk64 = 0;
        fdat_801ecf44(e50);
        fdat_801ee374 = 0xff;
    }
    if (fdat_801ee378 == 0) {
        if (viewer.unk178 == 0x9c || viewer.unk178 == 0xe9 || viewer.unk178 == 0xa0
            || viewer.unk178 == 0xa2 || viewer.unk178 == 0xa4) {
            viewer.unk218 = 5;
            fdat_801ed388(e33, 0, 0);
            e33->unk64 = 1;
            fdat_801ee378 = 0xff;
            viewer.unk218 = 0;
            e33->unk70 &= ~0x2000000;
            r4->unk1C &= ~0x180;
        } else {
            e33->unk44 = 0x80;
        }
    }
    if (fdat_801ee37c == 0 && e49->unk2C == 0) {
        func_8004ae38(e33);
        fdat_801ee37c = 0xff;
        DAT_801cba08.unk1448[r4->unk09] = 1;
        fdat_801ee378 = 0xff;
        d0->unk0C = 0;
    } else if (fdat_801ee37c != 0xff) {
        e33->unk1C.bytes[3] = 3;
    }
    if (fdat_801ee380 == 1) {
        func_801ee140(0);
        func_801ee278(0);
        fdat_801ee380 = 0xff;
        DAT_801cba08.unk1444.word |= 0x400;
    }
    if (fdat_801ee384 == 1) {
        func_801ee140(1);
        func_801ee278(1);
        fdat_801ee384 = 0xff;
        DAT_801cba08.unk1444.word |= 0x800;
    }
}

/* Item 0x129 used in front of table B entry 0x16 or 0x17 starts its
 * scene; the result is 1 then, else 2. */
ADDRESS(0x801edb0c, 0x10c)
void func_801edb0c(short id)
{
    VECTOR p;
    VisRequest req;
    int n;

    if (id == 0x129) {
        p.vx = viewer.position.vx;
        p.vy = viewer.position.vy + 1600;
        p.vz = viewer.position.vz;
        req.unk0 = 0;
        req.unk4 = p;
        req.unk1C = 1056;
        req.unk14.vx = 0;
        req.unk14.vy = 0;
        req.unk14.vz = 0;
        req.unk22 = 0;
        req.unk1E = 1600;
        req.unk20 = 0;
        n = func_8004f718(0, (VisLists *)&req);
        if (fdat_801ee380 == 0 && n == 0x16) {
            fdat_801ee380 = 1;
            DAT_801cd0e0 = 1;
        } else if (fdat_801ee384 == 0 && n == 0x17) {
            fdat_801ee384 = 1;
            DAT_801cd0e0 = 1;
        } else {
            DAT_801cd0e0 = 2;
        }
    }
}
