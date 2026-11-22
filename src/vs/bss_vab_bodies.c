/*
 * VS.EXE: a data-only module (WIP) defining game .bss:
 * the shared buffer and the menu sound bank body.
 * PSYLINK allocates each object's tentative definitions (COMMON) after the
 * previous object's, in symbol-hash bucket order (docs/executable-linking.md),
 * so the definitions here are listed in address order and their spellings
 * ascend in bucket; every user declares them extern.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <vs/buffer.h>
#include <vs/sound.h>

DATA(0x800633b0, 0x9c1d0, ".bss")
VsBuffer shared_buffer;
DATA(0x800ff580, 0x17000, ".bss")
u_char menu_vab_body[0x17000];
