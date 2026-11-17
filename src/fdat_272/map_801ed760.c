/*
 * FDAT_272.BIN: its second handler (WIP module; a unit of its own between
 * the not-yet-exact func_801ed4d0 and fdat_801ed4a0's variant). The
 * story flags' bits 8-9 and 10-12 are two small fields here (the area's
 * scene stage and the gate's stage).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/vis.h>
#include <st/data.h>
#include <st/menu.h>
#include <st/fdat.h>

DATA(0x801ef928, 0x4, ".bss")
int fdat_801ef928;

/*
 * Sets up the area by the story flags: flag 14 (item 0x12f used) or not,
 * flag 29 swaps two records' parts, gate stage 3 and up places entry 32,
 * and the scene stage (bits 8-9) poses entries 50/51 and table C item 28.
 */
ADDRESS(0x801ed760, 0x378)
void func_801ed760(void)
{
    Vis212 *e49 = &vis_table_a.entries[49];
    Vis212 *e48 = &vis_table_a.entries[48];
    Vis212 *e32 = &vis_table_a.entries[32];
    Vis212 *e51 = &vis_table_a.entries[51];
    Vis212 *e50 = &vis_table_a.entries[50];
    Vis192 *r11 = &vis_table_a.unk3500[11];
    Vis48 *c28 = &vis_table_c.entries[28];
    Vis48 *c29 = &vis_table_c.entries[29];
    Vis48 *c10 = &vis_table_c.entries[10];
    Vis48 *c11 = &vis_table_c.entries[11];
    Vis88 *d15 = &vis_table_b.entries[15];
    Vis88 *d16 = &vis_table_b.entries[16];
    int stage;
    u8 part;

    if (DAT_801cba08.unk1440.bit.f14 == 1) {
        c29->unk22 = 5;
        fdat_801ef93c = 2;
        d15->unk05 = 50;
        d15->unk02 = 0;
        d15->unk10 = 0xfff;
        d15->unk0C = 0xff;
        c29->unk08.vx += 0x1ea;
    } else {
        fdat_801ef93c = 0;
        e49->unk1C.bytes[3] = 1;
        c29->unk22 = 4;
        c11->unk22 = 5;
        c10->unk22 = 5;
        fdat_801ef93c = 0;
        e49->unk70 &= ~0x2000000;
    }
    e48->unk1C.bytes[3] = 1;
    if (DAT_801cba08.unk1440.word >> 29 & 1) {
        part = vis_records[38].part;
        vis_records[38].unk03 = 3;
        vis_records[38].part = vis_records[330].part;
        vis_records[330].part = part;
    }
    if ((int)(DAT_801cba08.unk1440.word >> 10 & 7) >= 3) {
        e32->unk8E = 0x400;
        e32->unk28 = 0x31;
        e32->unk6A = 0x800;
        e32->unk6E = -0x800;
        e32->unk6C = 0;
        e32->unk00 = 100;
        e32->unk06 = vis_records[49].unk01;
        e32->unk70 &= ~0x2000000;
    } else {
        e32->unk00 = 0;
    }
    if (DAT_801cba08.unk1440.word >> 29 & 1)
        e32->unk65 = 0;
    stage = DAT_801cba08.unk1440.word >> 8 & 3;
    fdat_801ef928 = 0;
    if (stage == 1) {
        fdat_801eb7b4(e50, 1);
        e50->unk1C.bytes[3] = 2;
        r11->unk16 = 0x200;
        r11->unk18 = 0x800;
        e51->unk05 = 3;
        fdat_801ef92c = stage;
        c28->unk22 = 4;
        fdat_801ef924 = 4;
    } else if (stage == 2) {
        e50->unk05 = 3;
        e51->unk05 = 3;
        fdat_801ef92c = 2;
        c28->unk22 = 5;
        c28->unk08.vz = 0x41b88;
        d16->unk05 = 50;
        d16->unk10 = 0xfff;
        d16->unk02 = 0;
        d16->unk0C = 0xff;
        fdat_801ef924 = 4;
        c28->unk2D += 3;
    } else {
        vis_table_a.unk3500[10].unk60[0]->unk00 = 0x70;
        vis_table_a.unk3500[10].unk60[1]->unk00 = 0x71;
        vis_table_a.unk3500[10].unk60[2]->unk00 = 0x72;
        e51->unk1C.bytes[3] = 2;
        fdat_801eb7b4(e50, 1);
        e50->unkB0 = 0;
        e50->unk1C.bytes[3] = 2;
        fdat_801eb7b4(e51, 0);
        c28->unk22 = 4;
        fdat_801ef924 = 0;
        fdat_801ef920 = 400;
        fdat_801ef92c = 0;
    }
    if (DAT_801cba08.unk1440.bit.f13 == 1) {
        fdat_801ef934 = 1;
        fdat_801ef930 = -1;
        fdat_801ef938 = 0xff;
    } else {
        fdat_801ef934 = 0;
        fdat_801ef930 = 600;
        fdat_801ef938 = 0;
    }
}
