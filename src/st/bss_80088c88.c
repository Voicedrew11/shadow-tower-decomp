/*
 * ST.EXE: a data-only module (WIP) defining game .bss:
 * the CD file buffer, the CLUT image buffer and an unreferenced block.
 * PSYLINK allocates each object's tentative definitions (COMMON) after the
 * previous object's, in symbol-hash bucket order (docs/executable-linking.md),
 * so the definitions here are listed in address order and their spellings
 * ascend in bucket; every user declares them extern.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/data.h>
#include <st/gfx.h>

DATA(0x80088c88, 0x12000, ".bss")
u_long DAT_80088c88[0x12000 / 4];
DATA(0x8009ac88, 0x20, ".bss")
u16 DAT_8009ac88[16];

/* Unreferenced (layout evidence only): the 0x3e0 bytes before DAT_8009b088. */
DATA(0x8009aca8, 0x3e0, ".bss")
u8 DAT_8009aca8[0x3e0];
