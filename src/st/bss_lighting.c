/*
 * ST.EXE: a data-only module (WIP) defining game .bss: the lighting table.
 * PSYLINK allocates each object's tentative definitions (COMMON) after the
 * previous object's, starting eight-aligned, in symbol-hash bucket order
 * (docs/executable-linking.md), and every user declares it extern. The
 * spelling lighting sorts below vis_axis_masks in that order (buckets 350
 * and 498), so it starts an object of its own.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/data.h>
#include <st/light.h>
#include <st/vis.h>

DATA(0x8018b6d0, 0x1b6c, ".bss")
StLighting lighting;
