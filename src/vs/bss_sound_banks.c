/*
 * VS.EXE: a data-only module (WIP) defining game .bss: the fighters' sound
 * banks. PSYLINK allocates each object's tentative definitions (COMMON)
 * after the previous object's, starting eight-aligned, in symbol-hash
 * bucket order (docs/executable-linking.md), and every user declares it
 * extern.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <vs/data.h>
#include <vs/sound.h>
#include <vs/menu.h>

DATA(0x801604b0, 0x2008, ".bss")
VsSoundBank sound_banks[2];
