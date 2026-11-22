/*
 * ST.EXE: a data-only module (WIP) defining game .bss: the viewer block and
 * the equipment model buffers. PSYLINK allocates each object's tentative
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

DATA(0x80198f28, 0x2f8, ".bss")
Viewer viewer;
DATA(0x80199220, 0x1c000, ".bss")
u8 equipment_model_buffers[4][0x7000];
