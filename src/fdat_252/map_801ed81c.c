/*
 * FDAT_252.BIN's handler fdat_801ed6e4, which 19 overlays share (WIP module).
 * Its move picker is this overlay's own variant, func_801ed5b4.
 * A unit of its own until its neighbours in the run are exact.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <psyq/libc.h>
#include <st/vis.h>
#include <st/fdat.h>

void func_801ed5b4(void);
#define FDAT_PICK_MOVE func_801ed5b4

#include "../lib/fdat_801ed6e4.inc"
