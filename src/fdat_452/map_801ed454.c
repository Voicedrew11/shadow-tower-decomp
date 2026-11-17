/*
 * FDAT_452.BIN: its ending handlers and the ending word fdat_801ed988 (WIP module; a unit of its own until its neighbours in
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
#include <st/cd.h>

/* Which ending plays: 0..3 by the viewer's state (see func_801ed454). */
DATA(0x801ed988, 0x4, ".bss")
int fdat_801ed988;

void SsSetSerialVol(short s_num, short voll, short volr);
void func_80042cec(void);
void func_8005f54c(void);

/*
 * Once: picks the ending. Every item the viewer holds outside its slots
 * (ids below 266) makes ending 0 or later; none makes ending 1; then the
 * second flag word's bits 1-2 and the viewer's figures choose 0, 2, 3 or
 * 4 (no stream). The entry plays the ending's animation and its XA
 * stream starts.
 */
ADDRESS(0x801ed454, 0x2ec)
void func_801ed454(void)
{
    Vis212 *e = vis_table_a.unk6138;
    Vis192 *r = &vis_table_a.unk3500[e->unk02];
    int none;
    int i;

    if (fdat_801ed8c4 != 0)
        return;
    fdat_801ed8c4 = 1;
    none = 1;
    for (i = 0; i < 266; i++) {
        if (DAT_801c7590.counts[i].count == 0)
            continue;
        if (i == viewer.unk38[0] || i == viewer.unk38[1] || i == viewer.unk38[2]
            || i == viewer.unk38[3] || i == viewer.unk38[4] || i == viewer.unk38[5]
            || i == viewer.unk38[7] || i == viewer.unk38[8] || i == viewer.unk38[9]
            || i == viewer.unk38[10] || i == viewer.unk38[11] || i == viewer.unk38[13]
            || i == viewer.unk38[14] || i == viewer.unk38[15] || i == viewer.unk38[16]
            || i == viewer.unk38[17] || i == viewer.unk38[18] || i == viewer.unk38[19]
            || i == viewer.unk38[20])
            continue;
        none = 0;
        break;
    }
    if (none) {
        fdat_801ed988 = 1;
        fdat_801eb7b4(e, 1);
    } else if ((DAT_801cba08.unk1444.word >> 1 & 3) == 0) {
        fdat_801ed988 = 0;
        fdat_801eb7b4(e, 0);
        DAT_801cba08.unk1444.word = (DAT_801cba08.unk1444.word & ~6) | 2;
    } else if (viewer.unk2 < viewer.unk0 >> 3) {
        fdat_801ed988 = 2;
        fdat_801eb7b4(e, 2);
    } else if (viewer.unk2 == viewer.unk0) {
        fdat_801ed988 = 3;
        fdat_801eb7b4(e, 2);
    } else {
        fdat_801ed988 = 4;
        fdat_801eb7b4(e, 3);
        fdat_801ed8c4 = 4;
    }
    if (fdat_801ed988 != 4) {
        func_80015ad4();
        func_800542d0(r->unk09 * 6 + fdat_801ed988, 1);
    }
}

/*
 * The ending entry's handler: steps the ending stages in fdat_801ed8c4
 * (1 waiting for half the animation, 2 streaming, 3 stream done, 4 menu,
 * 5 done) by the XA stream state in the world block.
 */
ADDRESS(0x801ed740, 0x240)
void func_801ed740(void)
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
        if (fdat_801ed8c4 == 2 && fdat_801ed988 == 0)
            fdat_801eb7b4(e, 3);
    }
    if (fdat_801ed8c4 == 1) {
        int ending = fdat_801ed988;
        if (ending != 0 || (short)e->unk2A >= 0x800) {
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
        if (fdat_801ed988 == 1)
            fdat_801ed8c4 = 5;
        else
            fdat_801ed8c4 = 4;
    } else if (fdat_801ed8c4 == 4) {
        DAT_801cd180.unk4 = 0;
        menu->unk2DC = 2;
        menu->unk2E5 = 0;
        func_8005f54c();
        fdat_801ed8c4 = 5;
    } else if (fdat_801ed8c4 == 5) {
        DAT_801cd0e4 = 1;
    }
}
