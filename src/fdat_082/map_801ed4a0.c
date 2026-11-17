/*
 * FDAT_082.BIN: its first two handlers and the scene word after its code
 * (WIP module; a unit of its own because fdat_801ed4a0 after it is not yet
 * exact).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/vis.h>
#include <st/menu.h>
#include <st/fdat.h>

/* The scene state: 0 pending, 0xff done. */
DATA(0x801ed87c, 0x4, ".bss")
int fdat_801ed87c;

/* Sets up the area: the scene is pending unless table A entry 48 is dying
 * or record 0's stream counter has started. */
ADDRESS(0x801ed4a0, 0x58)
void func_801ed4a0(void)
{
    Vis212 *e = &vis_table_a.entries[48];
    Vis192 *r = &vis_table_a.unk3500[0];

    if (e->unk05 != 3 && DAT_801cba08.unk1448[r->unk09] == 0)
        fdat_801ed87c = 0;
    else
        fdat_801ed87c = 0xff;
}

/* While pending: ends the scene once entry 48 is beaten, and turns it into
 * item 0x116 once record 0's stream counter reaches 1. */
ADDRESS(0x801ed4f8, 0x90)
void func_801ed4f8(void)
{
    Vis212 *e = &vis_table_a.entries[48];
    Vis192 *r = &vis_table_a.unk3500[0];

    if (fdat_801ed87c == 0) {
        if (e->unk2C == 0)
            fdat_801ed87c = 0xff;
        if (DAT_801cba08.unk1448[r->unk09] == 1) {
            func_8004d804(e, 0x116);
            fdat_801ecf44(e);
            fdat_801ed87c = 0xff;
        }
    }
}
