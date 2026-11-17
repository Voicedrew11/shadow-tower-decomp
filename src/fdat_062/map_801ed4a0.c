/*
 * FDAT_062.BIN: the first of its own handlers (WIP module; a unit of its own
 * until its neighbours in the run are exact).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/vis.h>
#include <st/menu.h>
#include <st/fdat.h>

/*
 * Arms the live table A entries 0..4 whose flags 2..5 are set.
 * Not exact: retail reaches entry 3 by its absolute address and entry 4
 * through entry 3's pointer, and forms entry 0's pointer last; this build
 * reaches entries 3 and 4 through entry 0's pointer, formed first.
 */
ADDRESS(0x801ed4a0, 0xec)
void func_801ed4a0(void)
{
    Vis212 *e3 = &vis_table_a.entries[3];
    Vis212 *e2 = &vis_table_a.entries[2];
    Vis212 *e1 = &vis_table_a.entries[1];
    Vis212 *e0 = &vis_table_a.entries[0];

    if (DAT_801cba08.unk1440.bit.f2 == 1) {
        if (e3->unk00 != 0xff)
            e3->unk00 = 100;
        if (vis_table_a.entries[4].unk00 != 0xff)
            vis_table_a.entries[4].unk00 = 100;
    }
    if (DAT_801cba08.unk1440.bit.f3 == 1 && e2->unk00 != 0xff)
        e2->unk00 = 100;
    if (DAT_801cba08.unk1440.bit.f4 == 1 && e1->unk00 != 0xff)
        e1->unk00 = 100;
    if (DAT_801cba08.unk1440.bit.f5 == 1 && e0->unk00 != 0xff)
        e0->unk00 = 100;
}
