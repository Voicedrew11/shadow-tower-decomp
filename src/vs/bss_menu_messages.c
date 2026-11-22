/*
 * VS.EXE: a data-only module (WIP) defining game .bss: the two message
 * tables. PSYLINK allocates each object's tentative definitions (COMMON)
 * after the previous object's, starting eight-aligned, in symbol-hash
 * bucket order (docs/executable-linking.md), and every user declares it
 * extern.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <vs/menu.h>
#include <vs/card.h>

DATA(0x80196230, 0xa800, ".bss")
VsMessages vs_menu_messages[2];
