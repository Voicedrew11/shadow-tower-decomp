/*
 * ST.EXE: a data-only module (WIP) defining game .bss:
 * the settings bytes, the pad buffers and state, and sound bank buffers 0, 1 and 14.
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
#include <st/pad.h>

DATA(0x801cd120, 0x9, ".bss")
Data801cd120 DAT_801cd120;
DATA(0x801cd130, 0x4c, ".bss")
volatile StPadData DAT_801cd130;
DATA(0x801cd180, 0x3c, ".bss")
Data801cd180 DAT_801cd180;
DATA(0x801cd1c0, 0x2000, ".bss")
u8 DAT_801cd1c0[2][0x1000];
DATA(0x801cf1c0, 0x1000, ".bss")
u8 DAT_801cf1c0[0x1000];
