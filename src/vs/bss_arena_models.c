/*
 * VS.EXE: a data-only module (WIP) defining game .bss: arena model buffers
 * 1 and 2. PSYLINK allocates each object's tentative definitions (COMMON)
 * after the previous object's, starting eight-aligned, in symbol-hash
 * bucket order (docs/executable-linking.md), so the definitions here are
 * listed in address order and their spellings ascend in bucket; every user
 * declares them extern. The spelling arena_model_1 sorts below
 * packet_buffers in that order (buckets 276 and 466), so it starts an
 * object of its own.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <vs/data.h>
#include <vs/main.h>
#include <vs/card.h>
#include <vs/pad.h>

DATA(0x8015c528, 0x1388, ".bss")
u8 arena_model_1[5000];
DATA(0x8015d8b0, 0x1388, ".bss")
u8 arena_model_2[5000];
