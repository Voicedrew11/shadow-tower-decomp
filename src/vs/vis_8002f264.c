/*
 * VS.EXE: jittered placement (WIP module). VS's copy of ST.EXE's
 * func_8004cba0 (src/st/vis_entry.c).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <psyq/libc.h>

/* Jitters off and places out at base plus it. */
ADDRESS(0x8002f264, 0xd0)
void effect_jitter_offset(SVECTOR *off, VECTOR *out, VECTOR *base)
{
    off->vx += 8 - rand() / 2048;
    off->vy += -16 - rand() / 1024;
    off->vz += 8 - rand() / 2048;
    out->vx = base->vx + off->vx;
    out->vy = base->vy;
    out->vz = base->vz + off->vz;
}
