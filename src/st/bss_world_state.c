/*
 * ST.EXE: a data-only module (WIP) defining game .bss: the graphics state.
 * PSYLINK allocates each object's tentative definitions (COMMON) after the
 * previous object's, starting eight-aligned, in symbol-hash bucket order
 * (docs/executable-linking.md), and every user declares it extern.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/data.h>
#include <st/gfx.h>
#include <st/menu.h>

DATA(0x8015d090, 0x1ee1a, ".bss")
WorldState world_state;
