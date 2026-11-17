/*
 * FDAT_212.BIN: its first three handlers and the four scene words after its code (WIP module; a unit of its own until its neighbours in
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

#include <st/bank.h>

/* Set by event 0x2e: table B entry 46 carries table C item 28. */
DATA(0x801edcd4, 0x4, ".bss")
static int fdat_801edcd4;

/* Set by event 2: table B entry 2 carries table C item 29. */
DATA(0x801edcd8, 0x4, ".bss")
static int fdat_801edcd8;

/* The bridge (table B kind 0x97): 1 rising, 2 risen, 0xff down. */
DATA(0x801edcdc, 0x4, ".bss")
static int fdat_801edcdc;

/* Frames the bridge has risen. */
DATA(0x801edce0, 0x4, ".bss")
static int fdat_801edce0;

void func_801edbd0(int which);

/* Not exact (one attempt, retries off; 83.7%): first divergence retail `bne v0,v1,<func_801ed4a0+0xc8>`, this source `bne v0,v1,<func_801ed4a0+0xc4>`. */
/* Sets up the area by the second flag word's bits 8 and 9: table C items
 * 28/29, the scene words and the bridge pieces (table B kind 0x97). */
ADDRESS(0x801ed4a0, 0xf4)
void func_801ed4a0(void)
{
    Vis48 *c28 = &vis_table_c.entries[28];
    Vis88 *b;
    int i;

    c28[1].unk22 = 4;
    c28->unk22 = 4;
    if (DAT_801cba08.unk1444.bit.f8 == 1)
        fdat_801edcd4 = 0xff;
    else
        fdat_801edcd4 = 0;
    if (DAT_801cba08.unk1444.bit.f9 == 1) {
        fdat_801edcd8 = 0xff;
        b = vis_table_b.entries;
        for (i = 239; i != -1; i--) {
            if (b->unk0a == 0x97)
                b->unk00 = 2;
            b++;
        }
        fdat_801edcdc = 0xff;
        vis_lists.unk47E0 |= 0x20;
    } else {
        fdat_801edcd8 = 0;
        fdat_801edcdc = 0;
        vis_lists.unk47E0 &= ~0x20;
    }
}

/* Not exact (one attempt, retries off; 97.9%): first divergence retail `lui v0,0x0`, this source `lui v0,0x0`. */
/*
 * Runs the area: events 0x2e and 2 start table B entries 46 and 2 (each
 * carrying its table C item on node 0, vertex 0x23); once entry 2 and then
 * entry 3 are done the bridge rises 0x18 a frame for 0x97 frames.
 */
ADDRESS(0x801ed594, 0x2fc)
void func_801ed594(void)
{
    Vis48 *c28 = &vis_table_c.entries[28];
    Vis48 *c29 = &vis_table_c.entries[29];
    Vis88 *d46 = &vis_table_b.entries[46];
    Vis88 *d2 = &vis_table_b.entries[2];
    Vis88 *d3 = &vis_table_b.entries[3];
    Vis88 *b;
    VECTOR v1;
    VECTOR v2;
    int i;

    if (fdat_801edcd4 == 1) {
        func_801edbd0(0);
        fdat_801edcd4 = 2;
        d46->unk0C = 1;
        d46->unk02 = 0;
        DAT_801cba08.unk1444.word |= 0x100;
    } else if (fdat_801edcd4 == 2) {
        anim_set_node_vertex(*(StAnimSet **)d46->unk44, func_80023208(d46->unk0a), 0, 0x23, &v1);
        c28->unk08.vx = d46->unk24.vx + v1.vx;
        c28->unk08.vy = d46->unk24.vy + v1.vy;
        c28->unk08.vz = d46->unk24.vz + v1.vz - 0x12;
        if (d46->unk0C == 0xff)
            fdat_801edcd4 = 0xff;
    }
    if (fdat_801edcd8 == 1) {
        func_801edbd0(1);
        fdat_801edcd8 = 2;
        d2->unk0C = 1;
        d2->unk02 = 0;
        DAT_801cba08.unk1444.word |= 0x200;
    } else if (fdat_801edcd8 == 2) {
        anim_set_node_vertex(*(StAnimSet **)d2->unk44, func_80023208(d2->unk0a), 0, 0x23, &v2);
        c29->unk08.vx = d2->unk24.vx + v2.vx;
        c29->unk08.vy = d2->unk24.vy + v2.vy;
        c29->unk08.vz = d2->unk24.vz + v2.vz - 0x12;
        if (d2->unk0C == 0xff)
            fdat_801edcd8 = 0xfe;
    } else if (fdat_801edcd8 == 0xfe && d3->unk0C == 0xff) {
        fdat_801edcdc = 1;
        fdat_801edce0 = 0;
        fdat_801edcd8 = 0xff;
    }
    if (fdat_801edcdc == 1) {
        b = vis_table_b.entries;
        for (i = 239; i != -1; i--) {
            if (b->unk0a == 0x97)
                b->unk24.vy += 0x18;
            b++;
        }
        if (++fdat_801edce0 >= 0x97)
            fdat_801edcdc = 2;
    } else if (fdat_801edcdc == 2) {
        b = vis_table_b.entries;
        for (i = 239; i != -1; i--) {
            if (b->unk0a == 0x97)
                b->unk00 = 2;
            b++;
        }
        fdat_801edcdc = 0xff;
        vis_lists.unk47E0 |= 0x20;
    }
}

/* Records events 0x2e and 2 when they come with flag 0x4000. */
ADDRESS(0x801ed890, 0x4c)
void func_801ed890(int event)
{
    short id = event & ~0x4000;

    if (event & 0x4000) {
        if (id == 0x2e)
            fdat_801edcd4 = 1;
        else if (id == 2)
            fdat_801edcd8 = 1;
    }
}
