/*
 * VS.EXE: a data-only module (WIP) defining game .bss:
 * file and packet buffers, the card headers and the pads.
 * PSYLINK allocates each object's tentative definitions (COMMON) after the
 * previous object's, in symbol-hash bucket order (docs/executable-linking.md),
 * so the definitions here are listed in address order and their spellings
 * ascend in bucket; every user declares them extern.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <vs/data.h>
#include <vs/main.h>
#include <vs/card.h>
#include <vs/pad.h>

DATA(0x80117708, 0x4e20, ".bss")
u_long DAT_80117708[0x4e20 / 4];
DATA(0x8011c528, 0x40000, ".bss")
u_long DAT_8011c528[0x10000];
DATA(0x8015c528, 0x1388, ".bss")
u8 DAT_8015c528[5000];
DATA(0x8015d8b0, 0x1388, ".bss")
u8 DAT_8015d8b0[5000];
DATA(0x8015ec38, 0x1388, ".bss")
u8 DAT_8015ec38[5000];
DATA(0x8015ffc0, 0x400, ".bss")
VsCardHeader DAT_8015ffc0[2];
DATA(0x801603c0, 0x48, ".bss")
VsPadState pad_state;
