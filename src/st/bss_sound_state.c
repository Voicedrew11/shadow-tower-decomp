/*
 * ST.EXE: a data-only module (WIP) defining game .bss: the sound state
 * block and the equipment screen state. PSYLINK allocates each object's
 * tentative definitions (COMMON) after the previous object's, starting
 * eight-aligned, in symbol-hash bucket order (docs/executable-linking.md),
 * so the definitions here are listed in address order and their spellings
 * ascend in bucket; every user declares them extern. The spelling
 * sound_state sorts below sound_buffers_equipment in that order (buckets
 * 180 and 451), so it starts an object of its own.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/data.h>
#include <st/equip_menu.h>

DATA(0x801dd1c0, 0x3b28, ".bss")
Data801dd1c0 sound_state;
DATA(0x801e0ce8, 0x20, ".bss")
Data801e0ce8 equip_menu_state;
