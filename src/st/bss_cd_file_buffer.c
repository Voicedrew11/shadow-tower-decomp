/*
 * ST.EXE: a data-only module (WIP) defining game .bss: the CD file buffer
 * and the CLUT image buffer. PSYLINK allocates each object's tentative
 * definitions (COMMON) after the previous object's, starting eight-aligned,
 * in symbol-hash bucket order (docs/executable-linking.md), so the
 * definitions here are listed in address order and their spellings ascend
 * in bucket; every user declares them extern.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/data.h>
#include <st/gfx.h>

DATA(0x80088c88, 0x12000, ".bss")
u_long cd_file_buffer[0x12000 / 4];
DATA(0x8009ac88, 0x20, ".bss")
u16 texanim_clut_buffer[16];
