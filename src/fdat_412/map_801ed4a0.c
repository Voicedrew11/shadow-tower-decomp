/*
 * FDAT_412.BIN: the first of its own handlers (WIP module; a unit of its own
 * because fdat_801ed4a0 after it is not yet exact).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/vis.h>
#include <st/menu.h>
#include <st/fdat.h>

/* Marks table B entry 21 once flag 39 (word 1, bit 7) is set. */
ADDRESS(0x801ed4a0, 0x2c)
void func_801ed4a0(void)
{
    Vis88 *b = &vis_table_b.entries[21];

    if (DAT_801cba08.unk1444.bit.f7)
        b->unk05 = 0xff;
}
