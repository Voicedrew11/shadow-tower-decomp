/*
 * FDAT_182.BIN: its first three handlers and the two scene words after its
 * code (WIP module; a unit of its own because fdat_801ed4a0 after it is not
 * yet exact).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/vis.h>
#include <st/data.h>
#include <st/light.h>
#include <st/menu.h>
#include <st/fdat.h>

/* 1 or 2: the gate the event 0x131 opened (func_801ed604); 0 none. */
DATA(0x801edc50, 0x4, ".bss")
static int fdat_801edc50;

/* 1 while the area 3 exit scene is armed, 0xff once played. */
DATA(0x801edc54, 0x4, ".bss")
static int fdat_801edc54;

void func_801ed9cc(int gate);

/* Clears the scene statics. */
ADDRESS(0x801ed4a0, 0x14)
void func_801ed4a0(void)
{
    fdat_801edc50 = 0;
    fdat_801edc54 = 0;
}

/*
 * Runs the opened gate's scene; arms the exit scene (flag 18, area lighting)
 * once table A entry 32 is beaten, then hands the entry to its exit when it
 * dies.
 */
ADDRESS(0x801ed4b4, 0x150)
void func_801ed4b4(void)
{
    Vis212 *e = &vis_table_a.entries[32];

    if (fdat_801edc50 == 1)
        func_801ed9cc(0);
    else if (fdat_801edc50 == 2)
        func_801ed9cc(1);
    if (!(DAT_801cba08.unk1440.word >> 18 & 1) && e->unk2C == 0) {
        viewer.unk218 = 5;
        DAT_801cba08.unk1440.word |= 0x40000;
        lighting.area_mode[3] = 0x11;
        lighting.area_mode[20] = 0x11;
        lighting.area_mode[24] = 0x11;
        lighting.area_mode[18] = 0x11;
        fdat_801edc54 = 1;
    }
    if (fdat_801edc54 == 1) {
        func_800402ac(0);
        func_800402ac(1);
        if (fdat_801edc54 == 1 && e->unk05 == 3) {
            viewer.unk218 = 5;
            e->unk70 |= 0x2000000;
            fdat_801ed388(e, 1, 0);
            fdat_801edc54 = 0xff;
            viewer.unk218 = 0;
        }
    }
}

/* Event 0x131 opens a gate: 2 when the viewer stands in area 3, else 1. */
ADDRESS(0x801ed604, 0x58)
void func_801ed604(short id)
{
    if (id == 0x131 && fdat_801edc50 == 0) {
        if (((VisRecord *)viewer.unk174)->unk07 == 3)
            fdat_801edc50 = 2;
        else
            fdat_801edc50 = 1;
        DAT_801cd0e0 = 1;
    }
}
