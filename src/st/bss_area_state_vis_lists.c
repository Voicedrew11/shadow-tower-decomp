/*
 * ST.EXE: a data-only module (WIP) defining game .bss:
 * the area state and the visibility lists.
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
#include <st/vis.h>

DATA(0x801b5920, 0x28, ".bss")
AreaState area_state;
DATA(0x801b5948, 0x47f0, ".bss")
CollisionState vis_lists;
