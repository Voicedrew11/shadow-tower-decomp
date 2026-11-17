/*
 * FDAT_332.BIN: a handler before fdat_801ed4a0 and the scene word after its
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

/* The scene state: 0 idle, 1 armed, 2 done (flag 48). */
DATA(0x801edb3c, 0x4, ".bss")
int fdat_801edb3c;

/* Sets up the area by flag 48: table A entry 33 dead with its drop chances,
 * or table B entry 3 retired and entry 33 shown. */
ADDRESS(0x801ed614, 0xac)
void func_801ed614(void)
{
    Vis88 *b = &vis_table_b.entries[3];
    Vis212 *e = &vis_table_a.entries[33];

    if (DAT_801cba08.unk1440.bit.f16) {
        if (e->unk05 != 3) {
            e->unk65 = 20;
            e->unk66 = 30;
            e->unk05 = 3;
            e->unk67 = 100;
            func_8004da2c(e);
            func_8004db0c(e);
        }
        fdat_801edb3c = 2;
    } else {
        fdat_801edb3c = 0;
        b->unk05 = 0xff;
        e->unk70 &= ~0x2000000;
    }
}
