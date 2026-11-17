/*
 * FDAT_232.BIN's handler fdat_801ed6e4, which 19 overlays share (WIP module).
 * Its move picker is this overlay's own variant, func_801ed7d8.
 * Then the overlay's last handler, func_801edb08.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <psyq/libc.h>
#include <st/vis.h>
#include <st/menu.h>
#include <st/fdat.h>

void func_801ed7d8(void);
#define FDAT_PICK_MOVE func_801ed7d8

#include "../lib/fdat_801ed6e4.inc"

/* In area 3, while flag 28 is clear, hands table A entry 32 to its exit once
 * its record's stream counter has reached 2. */
ADDRESS(0x801edb08, 0x8c)
void func_801edb08(void)
{
    Vis212 *e;
    Vis192 *r;

    e = &vis_table_a.entries[32];
    r = &vis_table_a.unk3500[12];

    if (DAT_801cba08.unk1448[r->unk09] >= 2 && !(DAT_801cba08.unk1440 >> 28 & 1)
        && vis_table_a.unk6168 == 3) {
        DAT_801cba08.unk1448[r->unk09] = 3;
        e->unk70 |= 0x2000000;
        fdat_801ed388(e, 1, 1);
    }
}
