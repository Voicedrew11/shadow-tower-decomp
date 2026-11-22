/*
 * VS.EXE: a data-only module (WIP) defining game .bss: the menu sound bank
 * header. PSYLINK allocates each object's tentative definitions (COMMON)
 * after the previous object's, starting eight-aligned, in symbol-hash
 * bucket order (docs/executable-linking.md), and every user declares it
 * extern.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <vs/data.h>
#include <vs/sound.h>

DATA(0x80116580, 0x1000, ".bss")
u_char menu_vab_head[0x1000];
