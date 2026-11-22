/*
 * ST.EXE: a data-only module (WIP) defining game .bss:
 * the GPU packet buffers and the animated objects.
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
#include <st/gfx.h>

DATA(0x8011c488, 0x40000, ".bss")
u_long DAT_8011c488[0x10000];
DATA(0x8015c488, 0xc04, ".bss")
StAnimObjects DAT_8015c488;
