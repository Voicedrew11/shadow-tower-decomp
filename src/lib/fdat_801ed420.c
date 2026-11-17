/*
 * The first own handler of FDAT_442.BIN, FDAT_452.BIN, FDAT_462.BIN and
 * FDAT_472.BIN, one body at the same address in all four (WIP module), with
 * the scene state it clears. The state sits 4 bytes after each overlay's
 * code and carries FDAT_442.BIN's address as its shared name; it is defined
 * here and the overlay's later units declare it extern (an overlay's data
 * keeps no linkage evidence, so the WIP split uses globals).
 * One source for the four overlays: FDAT_442.BIN binds by address, the
 * others by name.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/vis.h>
#include <st/data.h>
#include <st/menu.h>
#include <st/fdat.h>

DATA(0x801ed8c4, 0x4, ".bss")
int fdat_801ed8c4;

/* Resets the map scene: the viewer's slots, pitch and the scene state. */
ADDRESS(0x801ed420, 0x34)
void func_801ed420(void)
{
    DAT_801cd0e4 = 0;
    viewer.unk218 = 6;
    viewer.unk278.vx = 0;
    viewer.unk130[0] = -1;
    viewer.unk130[1] = -1;
    fdat_801ed8c4 = 0;
}
