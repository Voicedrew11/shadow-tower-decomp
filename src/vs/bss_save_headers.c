/*
 * VS.EXE: a data-only module (WIP) defining game .bss: the card save
 * headers and the pad state. PSYLINK allocates each object's tentative
 * definitions (COMMON) after the previous object's, starting eight-aligned,
 * in symbol-hash bucket order (docs/executable-linking.md), so the
 * definitions here are listed in address order and their spellings ascend
 * in bucket; every user declares them extern. The spelling save_headers
 * sorts below guard_flash_model in that order (buckets 246 and 257), so it
 * starts an object of its own.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <vs/data.h>
#include <vs/main.h>
#include <vs/card.h>
#include <vs/pad.h>

DATA(0x8015ffc0, 0x400, ".bss")
VsCardHeader save_headers[2];
DATA(0x801603c0, 0x48, ".bss")
VsPadState vs_pad_state;
