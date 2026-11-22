/*
 * VS.EXE: a data-only module (WIP) defining game .bss:
 * the CD state and the effect pool.
 * PSYLINK allocates each object's tentative definitions (COMMON) after the
 * previous object's, in symbol-hash bucket order (docs/executable-linking.md),
 * so the definitions here are listed in address order and their spellings
 * ascend in bucket; every user declares them extern.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <vs/data.h>
#include <vs/effect.h>

DATA(0x801a4590, 0x204a4, ".bss")
VsCdState DAT_801a4590;
DATA(0x801c4a38, 0x1c, ".bss")
s32 DAT_801c4a38[7];
DATA(0x801c4a58, 0x9c40, ".bss")
VsRecord20 DAT_801c4a58[2000];
DATA(0x801ce698, 0x350c, ".bss")
VsEffects DAT_801ce698;
