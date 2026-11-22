/*
 * ST.EXE: a data-only module (WIP) defining game .bss: sound bank buffers
 * 10..13. PSYLINK allocates each object's tentative definitions (COMMON)
 * after the previous object's, starting eight-aligned, in symbol-hash
 * bucket order (docs/executable-linking.md), and every user declares it
 * extern. The spelling sound_buffers_equipment sorts below
 * sound_buffer_common in that order (buckets 451 and 509), so it starts an
 * object of its own.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/data.h>
#include <st/equip_menu.h>

DATA(0x801d91c0, 0x4000, ".bss")
u8 sound_buffers_equipment[4][0x1000];
