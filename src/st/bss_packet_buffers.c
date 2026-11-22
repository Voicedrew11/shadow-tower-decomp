/*
 * ST.EXE: a data-only module (WIP) defining game .bss: the GPU packet
 * buffers. PSYLINK allocates each object's tentative definitions (COMMON)
 * after the previous object's, starting eight-aligned, in symbol-hash
 * bucket order (docs/executable-linking.md), and every user declares it
 * extern.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/data.h>
#include <st/gfx.h>

DATA(0x8011c488, 0x40000, ".bss")
u_long packet_buffers[0x10000];
