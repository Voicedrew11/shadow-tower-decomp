/*
 * ST.EXE: a data-only module (WIP) owning an unreferenced .bss hole between
 * texanim_clut_buffer's end and common_anim_pack (0x8009b088). Nothing
 * references it, but PSYLINK leaves no hole between objects' COMMON beyond
 * each request's alignment, so some object allocated these bytes; its
 * identity is admitted from the layout alone and its unit stays unresolved
 * until the linked .SYM confirms the placement. The spelling DAT_8009aca8
 * sorts below texanim_clut_buffer in that order (buckets 370 and 505), so
 * it starts an object of its own.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/data.h>
#include <st/gfx.h>

/* Unreferenced (layout evidence only): the 0x3e0 bytes before common_anim_pack. */
DATA(0x8009aca8, 0x3e0, ".bss")
u8 DAT_8009aca8[0x3e0];
