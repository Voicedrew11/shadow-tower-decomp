/*
 * VS.EXE: a data-only module (WIP) defining game .bss: the two loaded card
 * saves. PSYLINK allocates each object's tentative definitions (COMMON)
 * after the previous object's, starting eight-aligned, in symbol-hash
 * bucket order (docs/executable-linking.md), and every user declares it
 * extern. The spelling saves sorts below save_file_names in that order
 * (buckets 39 and 48), so it starts an object of its own.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <vs/menu.h>
#include <vs/card.h>

DATA(0x801a0a80, 0x3b10, ".bss")
VsSave saves[2];
