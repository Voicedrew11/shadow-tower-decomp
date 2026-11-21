/*
 * FDAT_472.BIN as one translation unit (WIP module): its handler table,
 * the 21 functions every overlay starts with (src/lib/fdat_common_a.inc,
 * fdat_common.inc, fdat_common_b.inc), the scene reset it shares with the
 * other three ending overlays (src/lib/fdat_801ed420.inc) and its ending
 * handlers. One object, so that its two scene words are consecutive
 * statics after the code.
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

RODATA(0x801eb560, 0x1cc)

/* A word no handler touches; it keeps the scene words four bytes off the code. */
DATA(0x801ed8c4, 0x4, ".bss")
static int fdat_801ed8c0;

/* The scene state: 0 until the ending is picked (func_801ed420 clears it). */
DATA(0x801ed8c8, 0x4, ".bss")
static int fdat_801ed8c4;

/* Which ending plays: 0..3 by the viewer's state (see func_801ed454). */
DATA(0x801ed8cc, 0x4, ".bss")
static int fdat_801ed8cc;

void func_801ed420();
void func_801ed454();
void func_801ed67c();

const FdatHeader fdat_header = {
    0x34,
    {
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, func_801ed420, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        func_801ed454, func_801ed67c, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
    },
    0x67,
};

#include "../lib/fdat_common_a.inc"
#include "../lib/fdat_common.inc"
#include "../lib/fdat_common_b.inc"

#include "../lib/fdat_801ed420.inc"

void SsSetSerialVol(short s_num, short voll, short volr);
void func_80042cec(void);
void func_8005f54c(void);

/*
 * Once: picks the ending by the items the viewer holds: the least
 * (two thirds of the base) x (wear) over the worn ones against the
 * viewer's figure, none worn, then the second flag word's bits 5-6 and a
 * random draw.
 */
ADDRESS(0x801ed454, 0x228)
void func_801ed454(void)
{
    Vis212 *e = vis_table_a.unk6138;
    Vis192 *r = &vis_table_a.unk3500[e->unk02];
    int least;
    int none;
    int i;
    int n;

    if (fdat_801ed8c4 != 0)
        return;
    fdat_801ed8c4 = 1;
    least = 0xffff;
    none = 1;
    for (i = 0; i < 266; i++) {
        if (DAT_801c7590.counts[i].count != 0
            && vis_table_c.unk0118[i].unk27 != vis_table_c.unk0118[i].unk28) {
            n = vis_table_c.unk0118[i].unk3 * 2 / 3
                * (vis_table_c.unk0118[i].unk27 - vis_table_c.unk0118[i].unk28);
            if (n < least)
                least = n;
            if (vis_table_c.unk0118[i].unk27 != vis_table_c.unk0118[i].unk28)
                none = 0;
        }
    }
    if (least != 0xffff && least >= viewer.unk2) {
        fdat_801ed8cc = 1;
        fdat_801eb7b4(e, 1);
    } else if (none) {
        fdat_801ed8cc = 2;
        fdat_801eb7b4(e, 2);
    } else if ((DAT_801cba08.unk1444.word >> 5 & 3) == 0) {
        fdat_801ed8cc = 0;
        fdat_801eb7b4(e, 0);
        DAT_801cba08.unk1444.word = (DAT_801cba08.unk1444.word & ~0x60) | 0x20;
    } else if ((short)rand() < 0x1000) {
        fdat_801ed8cc = 3;
        fdat_801eb7b4(e, 3);
    } else {
        fdat_801ed8cc = 4;
        fdat_801eb7b4(e, 4);
        fdat_801ed8c4 = 4;
    }
    if (fdat_801ed8cc != 4) {
        func_80015ad4();
        func_800542d0(r->unk09 * 6 + fdat_801ed8cc, 1);
    }
}

/*
 * The ending entry's handler: steps the ending stages in fdat_801ed8c4
 * (1 waiting for half the animation, 2 streaming, 3 stream done, 4 menu,
 * 5 done) by the XA stream state in the world block.
 */
ADDRESS(0x801ed67c, 0x248)
void func_801ed67c(void)
{
    Vis212 *e = vis_table_a.unk6138;
    Vis192 *r = vis_table_a.unk6134;
    MenuScratch *menu = MENU_SCRATCH;

    e->unk1C.bytes[2] &= 0xf2;
    e->unk36 = -1;
    if (e->unkA4 == 0)
        return;
    if (fdat_801ed8c4 == 2 && func_800541f4()) {
        if (world_state.unk1EE16 == 3) {
            func_80054278();
            world_state.unk1EE19 = 0;
            world_state.unk1EE16 = 4;
        }
    }
    if (world_state.unk1EE16 == 5) {
        fdat_801ed8c4 = 3;
        world_state.unk1EE16 = 0;
    }
    e->unk2A += e->unkB0;
    if ((short)e->unk2A >= 0x1000) {
        e->unk2A = 0;
        if (fdat_801ed8c4 == 2 && (fdat_801ed8cc == 0 || fdat_801ed8cc == 2))
            fdat_801eb7b4(e, 4);
    }
    if (fdat_801ed8c4 == 1) {
        int ending = fdat_801ed8cc;
        if (ending != 0 || (short)e->unk2A >= 0x600) {
            viewer.unk1E4 = 0;
            func_8005408c(r->unk09, ending);
            SsSetSerialVol(0, 0x40, 0x40);
            world_state.unk1EE16 = 2;
            world_state.unk1EE17 = 1;
            world_state.unk1EE19 = 0;
            fdat_801ed8c4 = 2;
            func_80042cec();
        }
    } else if (fdat_801ed8c4 == 3) {
        if (fdat_801ed8cc == 2)
            fdat_801ed8c4 = 5;
        else
            fdat_801ed8c4 = 4;
    } else if (fdat_801ed8c4 == 4) {
        DAT_801cd180.unk4 = 0;
        menu->unk2DC = 8;
        menu->unk2E5 = 0;
        func_8005f54c();
        fdat_801ed8c4 = 5;
    } else if (fdat_801ed8c4 == 5) {
        DAT_801cd0e4 = 1;
    }
}
