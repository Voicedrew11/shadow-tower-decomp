/*
 * ST.EXE: a data-only module (WIP) defining game .bss: the common animation
 * pack buffer. PSYLINK allocates each object's tentative definitions
 * (COMMON) after the previous object's, starting eight-aligned, in
 * symbol-hash bucket order (docs/executable-linking.md), and every user
 * declares it extern.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/data.h>
#include <st/area.h>
#include <st/memory.h>

DATA(0x8009b088, 0x1800, ".bss")
u8 common_anim_pack[0x1800];
