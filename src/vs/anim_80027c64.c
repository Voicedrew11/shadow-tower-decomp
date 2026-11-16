/*
 * VS.EXE: keyframe vertex arithmetic (WIP module). The sum, the vertex
 * list copy and the list additions are ST.EXE's (src/lib/anim_vertices.inc
 * and anim_vertex_list.inc, shared with src/st/anim.c); the difference is
 * VS's copy of ST's vertices_sub, the indexed list lookup VS's own.
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

#include "../lib/anim_vertex_list.inc"

/*
 * The entry `index` of a count-prefixed list of three-halfword entries,
 * where an entry -32768 followed by a halfword skips that many indices;
 * NULL when the index is skipped or past the end.
 */
ADDRESS(0x80027eb4, 0x7c)
short *func_80027eb4(short *list, int index)
{
    int i = 0;
    int count = *list++;

    while (count--) {
        if (*list == -32768) {
            list++;
            i += *list++;
            if (index < i)
                return NULL;
        } else {
            if (i == index)
                return list;
            i++;
            list += 3;
        }
    }
    return NULL;
}
