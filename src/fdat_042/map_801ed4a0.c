/*
 * FDAT_042.BIN: its first two handlers and the two scene words after its
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

/* The scene state: -1 armed, 0 idle. */
DATA(0x801ed96c, 0x4, ".bss")
static int fdat_801ed96c;

/* 0 or 0xff: table A entry 32 set up asleep or awake. */
DATA(0x801ed970, 0x4, ".bss")
static int fdat_801ed970;

void func_801ed8d0(void);

/* Plays the scene once door 19 of table B is raised past 0xa00, unless
 * flag 1 is set. */
ADDRESS(0x801ed4a0, 0x60)
void func_801ed4a0(void)
{
    Vis88 *door = &vis_table_b.entries[19];

    if (!DAT_801cba08.unk1440.bit.f1 && door->unk10 > 0xa00 && fdat_801ed96c == -1)
        func_801ed8d0();
}

/* Sets up the area: table A entry 32 by flag 1, and arms the scene. */
ADDRESS(0x801ed500, 0x68)
void func_801ed500(void)
{
    Vis212 *e = &vis_table_a.entries[32];

    if (e->unk00 == 0xff) {
        fdat_801ed96c = 0;
        return;
    }
    if (!DAT_801cba08.unk1440.bit.f1) {
        e->unk00 = 0;
        fdat_801ed970 = 0;
    } else {
        e->unk00 = 100;
        fdat_801ed970 = 0xff;
    }
    fdat_801ed96c = -1;
}
