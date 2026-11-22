/*
 * ST.EXE: a data-only module (WIP) defining game .bss: the animated
 * placement objects. PSYLINK allocates each object's tentative definitions
 * (COMMON) after the previous object's, starting eight-aligned, in
 * symbol-hash bucket order (docs/executable-linking.md), and every user
 * declares it extern. The spelling placement_objects sorts below
 * packet_buffers in that order (buckets 275 and 466), so it starts an
 * object of its own.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/data.h>
#include <st/gfx.h>

DATA(0x8015c488, 0xc04, ".bss")
PlacementObjects placement_objects;
