/*
 * FDAT_012.BIN: the first two of its own handlers and the scene word after
 * its code (WIP module; a unit of its own because fdat_801ed4a0 after it is
 * not yet exact).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/vis.h>
#include <st/menu.h>
#include <st/fdat.h>

/* The scene state: 0 idle, 1 armed, 2 played, 3 done (flag 0 set). */
DATA(0x801edfac, 0x4, ".bss")
int fdat_801edfac;

/* Places table A entry 48 again when it is in use but not active. */
ADDRESS(0x801ed4a0, 0x48)
void func_801ed4a0(void)
{
    Vis212 *e = &vis_table_a.entries[48];

    if (e->unk1C.bytes[0] != 0 && e->unk05 == 0)
        func_8004ae38(e);
}

/* Sets up the area: the scene state from flag 0, entry 48's range bits. */
ADDRESS(0x801ed4e8, 0x40)
void func_801ed4e8(void)
{
    Vis212 *e = &vis_table_a.entries[48];

    if (DAT_801cba08.unk1440.bit.f0)
        fdat_801edfac = 3;
    else
        fdat_801edfac = 0;
    e->unk1C.bytes[3] = 2;
}
