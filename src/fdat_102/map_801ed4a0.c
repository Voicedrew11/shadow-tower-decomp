/*
 * FDAT_102.BIN: the first of its own handlers and the two scene words after
 * its code (WIP module; a unit of its own until its neighbours in the run
 * are exact).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/vis.h>
#include <st/menu.h>
#include <st/fdat.h>

/* 0xff once record 0's stream counter reached 2. */
DATA(0x801eda78, 0x4, ".bss")
static int fdat_801eda78;

/* 0xff once flag 19 is set. */
DATA(0x801eda7c, 0x4, ".bss")
static int fdat_801eda7c;

/* Sets up the area: table A entry 48's range bits unless it is dying, the
 * scene words, and table B entries 39..42 unlinked until flag 19 is set. */
ADDRESS(0x801ed4a0, 0xb4)
void func_801ed4a0(void)
{
    Vis212 *e = &vis_table_a.entries[48];
    Vis192 *r = &vis_table_a.unk3500[0];
    Vis88 *d39 = &vis_table_b.entries[39];
    Vis88 *d40 = &vis_table_b.entries[40];
    Vis88 *d41 = &vis_table_b.entries[41];
    Vis88 *d42 = &vis_table_b.entries[42];

    if (e->unk05 != 3)
        e->unk1C.bytes[3] = 2;
    if (DAT_801cba08.unk1448[r->unk09] == 2)
        fdat_801eda78 = 0xff;
    else
        fdat_801eda78 = 0;
    if (!DAT_801cba08.unk1440.bit.f19) {
        d39->unk0a = d40->unk0a = d41->unk0a = d42->unk0a = 0xffff;
        fdat_801eda7c = 0;
    } else {
        fdat_801eda7c = 0xff;
    }
}
