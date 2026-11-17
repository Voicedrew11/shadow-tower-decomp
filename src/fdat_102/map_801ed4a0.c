/*
 * FDAT_102.BIN: its first two handlers and the two scene words after its
 * code (WIP module; ends at func_801ed554, which is not yet exact).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/vis.h>
#include <st/data.h>
#include <st/menu.h>
#include <st/fdat.h>

/* 0xff once record 0's stream counter reached 2. */
DATA(0x801eda78, 0x4, ".bss")
static int fdat_801eda78;

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

/* Not exact (one attempt, retries off; 94.5%): first divergence retail `sw s7,44(sp)`, this source `sw s6,40(sp)`. */
/*
 * Not exact (registers): retail gives entry 49's pointer s7 and table B
 * entry 41's s6; this source swaps them (3 retries spent).
 *
 * Runs the area: once record 0's stream counter reaches 2, entry 48 takes
 * item 0x127 and leaves; a hit does the same. Record 4's counter is held at
 * 1 or more. Item 0x12b used on entry 0x31 (func_801ed6b8) sends entry 49
 * away, links table B entries 39..42 and sets flag 19.
 */
ADDRESS(0x801ed554, 0x164)
void func_801ed554(void)
{
    Vis212 *e48 = &vis_table_a.entries[48];
    Vis88 *d39 = &vis_table_b.entries[39];
    Vis192 *r0 = &vis_table_a.unk3500[0];
    Vis212 *e49 = &vis_table_a.entries[49];
    Vis192 *r4 = &vis_table_a.unk3500[4];
    Vis88 *d40 = &vis_table_b.entries[40];
    Vis88 *d41 = &vis_table_b.entries[41];
    Vis88 *d42 = &vis_table_b.entries[42];

    if (fdat_801eda78 != 0xff && DAT_801cba08.unk1448[r0->unk09] == 2) {
        func_8004d804(e48, 0x127);
        fdat_801ecf44(e48);
        fdat_801eda78 = 0xff;
    }
    if (e48->unk64 != 0) {
        e48->unk64 = 0;
        fdat_801ecf44(e48);
        fdat_801eda78 = 0xff;
    }
    if (DAT_801cba08.unk1448[r4->unk09] >= 2)
        DAT_801cba08.unk1448[r4->unk09] = 1;
    if (fdat_801eda7c == 1) {
        fdat_801ecf44(e49);
        d42->unk0a = 250;
        d41->unk0a = 250;
        d40->unk0a = 250;
        d39->unk0a = 250;
        fdat_801eda7c = 0xff;
        DAT_801cba08.unk1440.word |= 0x80000;
    }
}
