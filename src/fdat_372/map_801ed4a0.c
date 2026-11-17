/*
 * FDAT_372.BIN: the first of its own handlers and the scene word after its
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

/* The scene state: 0 pending, 0xff done (flag 20). */
DATA(0x801eda3c, 0x4, ".bss")
int fdat_801eda3c;

/* Sets up the area: the scene by flag 20, and record 5's flags 0x180 cleared
 * once its stream counter has started. */
ADDRESS(0x801ed4a0, 0x70)
void func_801ed4a0(void)
{
    Vis192 *r = &vis_table_a.unk3500[5];

    if (!DAT_801cba08.unk1440.bit.f20)
        fdat_801eda3c = 0;
    else
        fdat_801eda3c = 0xff;
    if (DAT_801cba08.unk1448[r->unk09] != 0)
        r->unk1C &= ~0x180;
}
