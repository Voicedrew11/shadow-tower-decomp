/*
 * ST.EXE: a data-only module (WIP) defining game .bss: the pad buffers and
 * state, and sound bank buffers 0 and 1. PSYLINK allocates each object's
 * tentative definitions (COMMON) after the previous object's, starting
 * eight-aligned, in symbol-hash bucket order (docs/executable-linking.md),
 * so the definitions here are listed in address order and their spellings
 * ascend in bucket; every user declares them extern. The spelling
 * pad_buffers sorts below game_settings in that order (buckets 140 and
 * 375), so it starts an object of its own.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/data.h>
#include <st/pad.h>

DATA(0x801cd130, 0x4c, ".bss")
volatile StPadData pad_buffers;
DATA(0x801cd180, 0x3c, ".bss")
Data801cd180 pad_state;
DATA(0x801cd1c0, 0x2000, ".bss")
u8 sound_buffers_magic[2][0x1000];
