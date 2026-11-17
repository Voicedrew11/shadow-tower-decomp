/*
 * FDAT_382.BIN: its per-entry handler and the word fdat_801edb00 (WIP module; a unit of its own until its neighbours in
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

/* 0xff once the scene ran (record 1's stream counter 2). */
DATA(0x801edb00, 0x4, ".bss")
int fdat_801edb00;

void func_801ed74c(void);

/* The current entry's handler: the animation steps as fdat_801ed6e4 does;
 * the next move is picked only once the scene ran. */
ADDRESS(0x801ed9b4, 0xc0)
void func_801ed9b4(void)
{
    Vis212 *e = vis_table_a.unk6138;

    e->unk2A += e->unkB0;
    if (fdat_801ece7c(e) == 1)
        return;
    if ((e->unkB0 >= 0 ? (short)e->unk2A >= 4096 : (short)e->unk2A <= 0) && fdat_801edb00 == 0xff) {
        e->unk1A = 0xff;
        func_801ed74c();
    }
    if (e->unkA8 != NULL) {
        fdat_801ebec4(e);
        e->unk1A = 0xf0;
    }
}
