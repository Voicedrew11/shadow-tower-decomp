/*
 * FDAT_302.BIN: its per-entry handler and the word fdat_801ee478 (WIP module; a unit of its own until its neighbours in
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

/* Table A entry 3 beaten: 0, 1 this visit, 0xff gone; defined here only
 * because each map unit places one COMMON word. */
DATA(0x801ee478, 0x4, ".bss")
int fdat_801ee478;

/* The current entry's handler: the animation steps as fdat_801ed6e4 does;
 * kind 0 at the end of entry 0's dying stage turns to its scripts 7 and 9
 * and leaves (scene word 2). */
ADDRESS(0x801edc60, 0x13c)
void func_801edc60(void)
{
    Vis212 *e = vis_table_a.unk6138;
    Vis192 *r = vis_table_a.unk6134;

    e->unk2A += e->unkB0;
    if (fdat_801ece7c(e) == 1)
        return;
    if (e->unkB0 >= 0 ? (short)e->unk2A >= 4096 : (short)e->unk2A <= 0) {
        if (e->unk02 == 0 && fdat_801ee474 == 1) {
            e->unk70 |= 0x2000000;
            r->unk60[7]->unk00 = 0x72;
            r->unk60[9]->unk00 = 0x70;
            *(s16 *)&r->unk60[9]->unk0C = -0xc0;
            fdat_801ed388(e, 0, 0);
            fdat_801ee474 = 2;
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
