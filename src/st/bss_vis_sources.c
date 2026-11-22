/*
 * ST.EXE: a data-only module (WIP) defining game .bss: the visibility
 * records, source masks and axis masks. PSYLINK allocates each object's
 * tentative definitions (COMMON) after the previous object's, starting
 * eight-aligned, in symbol-hash bucket order (docs/executable-linking.md),
 * so the definitions here are listed in address order and their spellings
 * ascend in bucket; every user declares them extern.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/data.h>
#include <st/light.h>
#include <st/vis.h>

DATA(0x8017bed0, 0x1800, ".bss")
VisRecord vis_records[512];
DATA(0x8017d6d0, 0x8000, ".bss")
VisSourceMask vis_source_masks[512];
DATA(0x801856d0, 0x6000, ".bss")
VisSourceMask vis_axis_masks[3][128];
