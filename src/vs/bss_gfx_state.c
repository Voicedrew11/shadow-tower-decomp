/*
 * VS.EXE: a data-only module (WIP) defining game .bss: the teams and the
 * graphics state. PSYLINK allocates each object's tentative definitions
 * (COMMON) after the previous object's, starting eight-aligned, in
 * symbol-hash bucket order (docs/executable-linking.md), so the definitions
 * here are listed in address order and their spellings ascend in bucket;
 * every user declares them extern. The spelling teams sorts below
 * sound_banks in that order (buckets 31 and 162), so it starts an object of
 * its own.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <vs/data.h>
#include <vs/sound.h>
#include <vs/menu.h>

DATA(0x801624b8, 0x18, ".bss")
VsTeam teams[2];
DATA(0x801624d0, 0x2ca90, ".bss")
VsWorldState vs_gfx_state;
