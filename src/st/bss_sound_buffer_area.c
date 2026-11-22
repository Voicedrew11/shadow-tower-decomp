/*
 * ST.EXE: a data-only module (WIP) defining game .bss: sound bank buffer
 * 14. PSYLINK allocates each object's tentative definitions (COMMON) after
 * the previous object's, starting eight-aligned, in symbol-hash bucket
 * order (docs/executable-linking.md), and every user declares it extern.
 * The spelling sound_buffer_area sorts below sound_buffers_magic in that
 * order (buckets 267 and 488), so it starts an object of its own.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/data.h>
#include <st/pad.h>

DATA(0x801cf1c0, 0x1000, ".bss")
u8 sound_buffer_area[0x1000];
