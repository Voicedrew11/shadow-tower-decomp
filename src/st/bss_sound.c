/*
 * ST.EXE: a data-only module (WIP) defining game .bss: sound bank buffers
 * 2..9 and 15. PSYLINK allocates each object's tentative definitions
 * (COMMON) after the previous object's, starting eight-aligned, in
 * symbol-hash bucket order (docs/executable-linking.md), so the definitions
 * here are listed in address order and their spellings ascend in bucket;
 * every user declares them extern.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/data.h>
#include <st/equip_menu.h>

DATA(0x801d01c0, 0x4000, ".bss")
u8 sound_buffers_object[4][0x1000];
DATA(0x801d41c0, 0x4000, ".bss")
u8 sound_buffers_creature[4][0x1000];
DATA(0x801d81c0, 0x1000, ".bss")
u8 sound_buffer_common[0x1000];
