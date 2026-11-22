/*
 * VS.EXE: a data-only module (WIP) defining game .bss: the file names the
 * saves were loaded from. PSYLINK allocates each object's tentative
 * definitions (COMMON) after the previous object's, starting eight-aligned,
 * in symbol-hash bucket order (docs/executable-linking.md), and every user
 * declares it extern. The spelling save_file_names sorts below
 * vs_menu_messages in that order (buckets 48 and 196), so it starts an
 * object of its own.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <vs/menu.h>
#include <vs/card.h>

DATA(0x801a0a30, 0x50, ".bss")
char save_file_names[2][40];
