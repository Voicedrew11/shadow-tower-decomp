/*
 * VS.EXE: a data-only module (WIP) defining game .bss:
 * the pad receive buffers.
 * PSYLINK allocates each object's tentative definitions (COMMON) after the
 * previous object's, in symbol-hash bucket order (docs/executable-linking.md),
 * so the definitions here are listed in address order and their spellings
 * ascend in bucket; every user declares them extern.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <vs/pad.h>

DATA(0x801961e0, 0x44, ".bss")
VsPadBuffer DAT_801961e0[2];
