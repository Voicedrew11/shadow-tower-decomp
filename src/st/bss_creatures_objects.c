/*
 * ST.EXE: a data-only module (WIP) defining game .bss:
 * visibility tables A and B and the table C counts.
 * PSYLINK allocates each object's tentative definitions (COMMON) after the
 * previous object's, in symbol-hash bucket order (docs/executable-linking.md),
 * so the definitions here are listed in address order and their spellings
 * ascend in bucket; every user declares them extern.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/data.h>
#include <st/vis.h>

DATA(0x801ba138, 0x61d8, ".bss")
CreatureTable vis_table_a;
DATA(0x801c0310, 0x727c, ".bss")
AreaObjectTable vis_table_b;
DATA(0x801c7590, 0x168, ".bss")
Data801c7590 inventory;
