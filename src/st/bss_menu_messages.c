/*
 * ST.EXE: a data-only module (WIP) defining game .bss: the message table.
 * PSYLINK allocates each object's tentative definitions (COMMON) after the
 * previous object's, starting eight-aligned, in symbol-hash bucket order
 * (docs/executable-linking.md), and every user declares it extern. The
 * spelling menu_messages sorts below arrange_list_state in that order
 * (buckets 377 and 397), so it starts an object of its own.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/data.h>
#include <st/equip_menu.h>
#include <st/idgrid.h>
#include <st/menu.h>

DATA(0x801e0d50, 0x5400, ".bss")
MenuMessages menu_messages;
