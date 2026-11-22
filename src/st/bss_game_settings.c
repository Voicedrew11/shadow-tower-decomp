/*
 * ST.EXE: a data-only module (WIP) defining game .bss: the settings bytes.
 * PSYLINK allocates each object's tentative definitions (COMMON) after the
 * previous object's, starting eight-aligned, in symbol-hash bucket order
 * (docs/executable-linking.md), and every user declares it extern.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/data.h>
#include <st/pad.h>

DATA(0x801cd120, 0x9, ".bss")
GameSettings game_settings;
