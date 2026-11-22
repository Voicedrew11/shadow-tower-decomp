/*
 * VS.EXE: a data-only module (WIP) defining game .bss: the lineups and the
 * graphics state. PSYLINK allocates each object's tentative definitions
 * (COMMON) after the previous object's, starting eight-aligned, in
 * symbol-hash bucket order (docs/executable-linking.md), so the definitions
 * here are listed in address order and their spellings ascend in bucket;
 * every user declares them extern (lineups bucket 264, vs_gfx_state 281).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <vs/data.h>
#include <vs/sound.h>
#include <vs/menu.h>

DATA(0x801624b8, 0x18, ".bss")
VsLineup lineups[2];
DATA(0x801624d0, 0x2ca90, ".bss")
VsWorldState vs_gfx_state;
