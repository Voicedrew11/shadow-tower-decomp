/*
 * FDAT_332.BIN: a handler after fdat_801ed4a0 (WIP module; a unit of its own
 * until its neighbours in the run are exact).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <psyq/libc.h>
#include <st/vis.h>
#include <st/fdat.h>

void func_801ed9e8(void);

/*
 * The step of the overlay's entries, like fdat_801ed6e4: an entry of record
 * 4 plays the scene at the end of a run while it is armed.
 */
ADDRESS(0x801ed904, 0xe4)
void func_801ed904(void)
{
    Vis212 *e = vis_table_a.unk6138;

    e->unk2A += e->unkB0;
    if (fdat_801ece7c(e) == 1)
        return;
    if (e->unkB0 >= 0 ? (short)e->unk2A >= 4096 : (short)e->unk2A <= 0) {
        if (e->unk02 == 4 && fdat_801edb3c == 1) {
            func_801ed9e8();
            return;
        }
        e->unk1A = 0xff;
        fdat_801ed4a0();
    }
    if (e->unkA8 != NULL) {
        fdat_801ebec4(e);
        e->unk1A = 0xf0;
    }
}
