/*
 * FDAT_022.BIN: its first two handlers and the scene word after its code
 * (WIP module; fdat_801ed4a0 after them is not yet exact).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/vis.h>
#include <st/data.h>
#include <st/menu.h>
#include <st/fdat.h>

/* The scene state: 0 pending, 1 ready, 0xff done (flag 25). */
DATA(0x801edad0, 0x4, ".bss")
int fdat_801edad0;

void func_801ed8f0(void);

/* Sets up the area: done when flag 25 is set, ready once record 6's stream
 * counter reached 2, else table A entries 48 and 49 set up for the scene. */
ADDRESS(0x801ed4a0, 0x74)
void func_801ed4a0(void)
{
    Vis212 *e48 = &vis_table_a.entries[48];
    Vis212 *e49 = &vis_table_a.entries[49];
    Vis192 *r = &vis_table_a.unk3500[6];

    if (DAT_801cba08.unk1440.bit.f25 == 1) {
        fdat_801edad0 = 0xff;
    } else if (DAT_801cba08.unk1448[r->unk09] == 2) {
        fdat_801edad0 = 1;
    } else {
        e49->unk1C.bytes[3] = 1;
        e48->unk00 = 0;
        fdat_801edad0 = 0;
    }
}

/* Once the viewer stands on record 0x27, entry 49 leaves and entry 48 is
 * marked; when record 2's stream counter reaches 3 the scene plays. */
ADDRESS(0x801ed514, 0xe8)
void func_801ed514(void)
{
    Vis212 *e48 = &vis_table_a.entries[48];
    Vis192 *r = &vis_table_a.unk3500[2];
    Vis212 *e49 = &vis_table_a.entries[49];

    if (fdat_801edad0 == 0 && viewer.unk178 == 0x27) {
        fdat_801ed388(e49, 0, 0);
        fdat_801ed388(e49, 0, 0);
        fdat_801ecf44(e49);
        e48->unk00 = 100;
        fdat_801edad0 = 1;
    }
    if (fdat_801edad0 == 1 && DAT_801cba08.unk1448[r->unk09] == 3) {
        viewer.unk218 = 5;
        func_801ed8f0();
        fdat_801edad0 = 0xff;
        viewer.unk218 = 0;
    }
}
