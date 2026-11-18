/*
 * FDAT_102.BIN's shared handlers fdat_801ed4a0 (23 overlays; its switch
 * table is the unit's read-only claim) and fdat_801ed6e4 (19 overlays;
 * WIP module).
 * A unit of its own until its neighbours in the run are exact.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <psyq/libc.h>
#include <st/vis.h>
#include <st/fdat.h>

RODATA(0x801eb730, 0x7c)

#include "../lib/fdat_801ed4a0.inc"
#include "../lib/fdat_801ed6e4.inc"
