/*
 * VS.EXE: a data-only module (WIP) defining game .bss: the first arena
 * model buffer (also a file buffer) and the GPU packet buffers. PSYLINK
 * allocates each object's tentative definitions (COMMON) after the previous
 * object's, starting eight-aligned, in symbol-hash bucket order
 * (docs/executable-linking.md), so the definitions here are listed in
 * address order and their spellings ascend in bucket; every user declares
 * them extern.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <vs/data.h>
#include <vs/main.h>
#include <vs/card.h>
#include <vs/pad.h>

DATA(0x80117708, 0x4e20, ".bss")
u_long arena_model_0[0x4e20 / 4];
DATA(0x8011c528, 0x40000, ".bss")
u_long packet_buffers[0x10000];
