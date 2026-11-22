/*
 * ST.EXE: a data-only module (WIP) defining game .bss: the card file list's
 * per-row clip counts. PSYLINK allocates each object's tentative
 * definitions (COMMON) after the previous object's, starting eight-aligned,
 * in symbol-hash bucket order (docs/executable-linking.md), and every user
 * declares it extern. The spelling file_row_clipped sorts below world_state
 * in that order (buckets 167 and 179), so it starts an object of its own.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/data.h>
#include <st/gfx.h>
#include <st/menu.h>

DATA(0x8017beb0, 0x1c, ".bss")
s32 file_row_clipped[7];
