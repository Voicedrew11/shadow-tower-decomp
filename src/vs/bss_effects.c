/*
 * VS.EXE: a data-only module (WIP) defining game .bss: the effect pool.
 * PSYLINK allocates each object's tentative definitions (COMMON) after the
 * previous object's, starting eight-aligned, in symbol-hash bucket order
 * (docs/executable-linking.md), and every user declares it extern. The
 * spelling vs_effects sorts below sparks in that order (buckets 50 and
 * 154), so it starts an object of its own.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <vs/data.h>
#include <vs/effect.h>

DATA(0x801ce698, 0x350c, ".bss")
VsEffects vs_effects;
