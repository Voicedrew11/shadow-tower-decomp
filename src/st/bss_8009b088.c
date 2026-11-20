/*
 * ST.EXE: a data-only module (WIP) defining game .bss:
 * the animation pack buffer, the block heap arena and the two area buffers.
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
#include <st/area.h>
#include <st/memory.h>

/* Retail leaves 0x8009aca8..0x8009b088 unreferenced before this batch. */
DATA(0x8009b088, 0x1800, ".bss")
u8 DAT_8009b088[0x1800];
DATA(0x8009c888, 0x56400, ".bss")
StBlock DAT_8009c888[0x56400 / sizeof(StBlock)];
DATA(0x800f2c88, 0x9800, ".bss")
u_long DAT_800f2c88[0x9800 / 4];
DATA(0x800fc488, 0x20000, ".bss")
u_long DAT_800fc488[0x20000 / 4];
