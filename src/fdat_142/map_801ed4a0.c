/*
 * FDAT_142.BIN: the first of its own handlers and the four scene words after
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

DATA(0x801edaf4, 0x4, ".bss")
static int fdat_801edaf4;

DATA(0x801edaf8, 0x4, ".bss")
static int fdat_801edaf8;

/* 0xff once record 0's stream counter has started. */
DATA(0x801edafc, 0x4, ".bss")
static int fdat_801edafc;

/* 0xff once record 4's stream counter reached 2. */
DATA(0x801edb00, 0x4, ".bss")
static int fdat_801edb00;

/* Sets up the area: the scene words by records 0 and 4's stream counters;
 * until record 4's reaches 2, table A entries 5..7 are cleared and entry
 * 49's range bits set. */
ADDRESS(0x801ed4a0, 0xac)
void func_801ed4a0(void)
{
    Vis192 *r0 = &vis_table_a.unk3500[0];
    Vis212 *e49 = &vis_table_a.entries[49];
    Vis192 *r4 = &vis_table_a.unk3500[4];
    Vis212 *e5 = &vis_table_a.entries[5];
    Vis212 *e6 = &vis_table_a.entries[6];
    Vis212 *e7 = &vis_table_a.entries[7];

    fdat_801edaf4 = 0;
    fdat_801edaf8 = 0;
    if (DAT_801cba08.unk1448[r0->unk09] != 0)
        fdat_801edafc = 0xff;
    else
        fdat_801edafc = 0;
    if (DAT_801cba08.unk1448[r4->unk09] == 2) {
        fdat_801edb00 = 0xff;
    } else {
        e7->unk00 = 0;
        e6->unk00 = 0;
        e5->unk00 = 0;
        e49->unk1C.bytes[3] = 2;
        fdat_801edb00 = 0;
    }
}

/*
 * Not exact: the instructions agree, but equivalent .bss+8 HI16 pairs
 * order differently in the two ELF views (module offsets 0x160/0x19c/0x1b0).
 * Runs the area: the viewer standing on record 0x1f2 or 0x63 picks the
 * scene words; entry 48 asleep until it wakes (scene word 3 armed), then
 * leaves once the area fade is done, keeping the viewer's status 0x188 at
 * 30; record 0x10e with both words set sends entry 49 away and frees
 * entries 5..7.
 */
ADDRESS(0x801ed54c, 0x26c)
void func_801ed54c(void)
{
    Vis212 *e48 = &vis_table_a.entries[48];
    Vis212 *e49 = &vis_table_a.entries[49];
    Vis212 *e5 = &vis_table_a.entries[5];
    Vis212 *e6 = &vis_table_a.entries[6];
    Vis212 *e7 = &vis_table_a.entries[7];

    if (fdat_801edaf8 == 0) {
        if (viewer.unk178 == 0x1f2) {
            fdat_801edaf4 = 1;
            fdat_801edaf8 = 1;
        } else if (viewer.unk178 == 0x63) {
            fdat_801edaf4 = 0;
            fdat_801edaf8 = 1;
        }
    }
    if (fdat_801edaf4 == 1 && fdat_801edaf8 == 1)
        e48->unk00 = 0;
    if (fdat_801edaf4 == 0) {
        if (fdat_801edaf8 == 1)
            e49->unk00 = 0;
        if (fdat_801edaf4 == 0 && e48->unk05 == 1) {
            fdat_801edafc = 1;
            viewer.unk218 = 5;
        }
    }
    if (fdat_801edafc == 1 && area_state.unk18 == -1 && (short)area_state.unk2 >= 2)
        fdat_801edafc = 2;
    if (fdat_801edafc == 2) {
        fdat_801ed388(e48, 0, 0);
        fdat_801ecf44(e48);
        fdat_801edafc = 0xff;
        viewer.unk218 = 0;
    }
    if (fdat_801edafc == 0xff && !(viewer.unk2C & 0x800)) {
        viewer.unk188 = 30;
        viewer.unk182 = 0;
    }
    if (fdat_801edb00 == 0 && viewer.unk178 == 0x10e && fdat_801edaf4 == 1
        && fdat_801edaf8 == 1) {
        fdat_801ed388(e49, 0, 0);
        fdat_801ed388(e49, 0, 0);
        fdat_801ecf44(e49);
        fdat_801ed410(e5);
        fdat_801ed410(e6);
        fdat_801ed410(e7);
        fdat_801edb00 = 0xff;
    }
}
