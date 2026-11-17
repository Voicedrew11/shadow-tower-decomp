/*
 * FDAT_012.BIN: its last two handlers (WIP module; a unit of its own until
 * its neighbours in the run are exact).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/vis.h>
#include <st/menu.h>
#include <st/fdat.h>

extern int fdat_801edfac;

/* Arms the scene when the entry comes within 0xc00, then plays script 8
 * once it is defeated or beyond 0x1000. */
ADDRESS(0x801edf0c, 0x7c)
void func_801edf0c(Vis212 *e)
{
    if (fdat_801edfac == 0) {
        if (e->unk40 < 0xc00)
            fdat_801edfac = 1;
    } else if (fdat_801edfac == 1 && (e->unk04 == 0 || e->unk40 > 0x1000)) {
        fdat_801edfac = 2;
        fdat_801eb7b4(e, 8);
    }
}

/* Ends the scene: state 3 and flag 0. */
ADDRESS(0x801edf88, 0x24)
void func_801edf88(void)
{
    fdat_801edfac = 3;
    DAT_801cba08.unk1440.bit.f0 = 1;
}
