/*
 * FDAT_252.BIN: the first of its own handlers and the scene word after its
 * code (WIP module; a unit of its own until its neighbours in the run are
 * exact).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/vis.h>
#include <st/menu.h>
#include <st/fdat.h>

/* The scene state: 1 once record 8's stream counter reached 1. */
DATA(0x801ed8cc, 0x4, ".bss")
int fdat_801ed8cc;

/* Sets up the area: arms the scene and clears record 8's flags 0x180 once
 * its stream counter has reached 1. */
ADDRESS(0x801ed4a0, 0x4c)
void func_801ed4a0(void)
{
    Vis192 *r = &vis_table_a.unk3500[8];

    if (DAT_801cba08.unk1448[r->unk09] == 1) {
        fdat_801ed8cc = 1;
        r->unk1C &= ~0x180;
    } else {
        fdat_801ed8cc = 0;
    }
}
