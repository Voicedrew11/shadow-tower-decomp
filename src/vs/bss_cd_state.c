/*
 * VS.EXE: a data-only module (WIP) defining game .bss: the CD state and the
 * card file list's per-row clip counts. PSYLINK allocates each object's
 * tentative definitions (COMMON) after the previous object's, starting
 * eight-aligned, in symbol-hash bucket order (docs/executable-linking.md),
 * so the definitions here are listed in address order and their spellings
 * ascend in bucket; every user declares them extern.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <vs/data.h>
#include <vs/effect.h>

DATA(0x801a4590, 0x204a4, ".bss")
VsCdState vs_cd_state;
DATA(0x801c4a38, 0x1c, ".bss")
s32 file_row_clipped[7];
