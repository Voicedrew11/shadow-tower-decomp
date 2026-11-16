/*
 * VS.EXE: keyframe vertex arithmetic (WIP module). The sum and the vertex
 * list copy are ST.EXE's (src/lib/anim_vertices.inc, shared with
 * src/st/anim.c); the difference is VS's copy of ST's vertices_sub.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/memory.h>
#include <st/gfx.h>
#include <st/mesh.h>
#include <st/scratch.h>
#include <st/anim.h>
#include <vs/data.h>
#include <vs/funcs.h>

/* dst = to - from, per vertex. */
ADDRESS(0x80027c64, 0x7c)
void func_80027c64(SVECTOR *dst, SVECTOR *from, SVECTOR *to, int count)
{
    while (count--) {
        dst->vx = to->vx - from->vx;
        dst->vy = to->vy - from->vy;
        dst->vz = to->vz - from->vz;
        dst++;
        to++;
        from++;
    }
}

#include "../lib/anim_vertices.inc"
