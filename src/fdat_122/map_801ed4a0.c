/*
 * FDAT_122.BIN: its first two handlers and the scene word after its code
 * (WIP module; a unit of its own because fdat_801ed4a0 after it is not yet
 * exact).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/vis.h>
#include <st/data.h>
#include <st/menu.h>
#include <st/fdat.h>

/* The scene state: 1 armed by flag 47, 2 played. */
DATA(0x801ed938, 0x4, ".bss")
int fdat_801ed938;

void func_801ed880(void);

/* Sets up the area: clears table A entries 32..34 and arms the scene. */
ADDRESS(0x801ed4a0, 0x44)
void func_801ed4a0(void)
{
    vis_table_a.entries[32].unk00 = vis_table_a.entries[33].unk00 = vis_table_a.entries[34].unk00 = 0;
    if (DAT_801cba08.unk1444.bit.f15 == 1)
        fdat_801ed938 = 1;
    else
        fdat_801ed938 = 0;
}

/* Plays the scene once the viewer stands on record 0x35. */
ADDRESS(0x801ed4e4, 0x54)
void func_801ed4e4(void)
{
    if (fdat_801ed938 == 1 && viewer.unk178 == 0x35) {
        func_801ed880();
        fdat_801ed938 = 2;
    }
}
