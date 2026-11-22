/*
 * VS.EXE: a data-only module (WIP) defining game .bss: the guard flash
 * model buffer. PSYLINK allocates each object's tentative definitions
 * (COMMON) after the previous object's, starting eight-aligned, in
 * symbol-hash bucket order (docs/executable-linking.md), and every user
 * declares it extern. The spelling guard_flash_model sorts below
 * arena_model_2 in that order (buckets 257 and 277), so it starts an object
 * of its own.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <vs/data.h>
#include <vs/main.h>
#include <vs/card.h>
#include <vs/pad.h>

DATA(0x8015ec38, 0x1388, ".bss")
u8 guard_flash_model[5000];
