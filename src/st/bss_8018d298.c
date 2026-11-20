/*
 * ST.EXE: a data-only module (WIP) defining game .bss:
 * the effect pool and the item load buffers.
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
#include <st/effect.h>

/* Retail leaves 0x8018d23c..0x8018d298 unreferenced before this batch. */
DATA(0x8018d298, 0x3c8c, ".bss")
StEffects effects;
DATA(0x80190f28, 0x8000, ".bss")
u8 DAT_80190f28[2][0x4000];
