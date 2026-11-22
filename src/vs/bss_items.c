/*
 * VS.EXE: a data-only module (WIP) defining game .bss:
 * the VS copy of the item visibility table.
 * PSYLINK allocates each object's tentative definitions (COMMON) after the
 * previous object's, in symbol-hash bucket order (docs/executable-linking.md),
 * so the definitions here are listed in address order and their spellings
 * ascend in bucket; every user declares them extern.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <vs/data.h>

DATA(0x80191d00, 0x4310, ".bss")
ItemTable vis_table_c;
