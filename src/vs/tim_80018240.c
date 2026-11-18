/*
 * VS.EXE: a single TIM upload (WIP module), as one pass of func_80012c2c.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>

/* Uploads one TIM image (and its CLUT) to the given positions. */
ADDRESS(0x80018240, 0xac)
void func_80018240(u_long *tim, DVECTOR *pos, DVECTOR *clut_pos)
{
    u_long flags;
    u_long *cursor;
    u_long *clut;
    u_long *clut_data;
    u_long *image;
    u_long *image_data;

    cursor = tim;
    if (cursor[0] != 0x10)
        return;
    cursor += 2;
    flags = tim[1];
    if (flags & 8) {
        clut_data = tim + 5;
        clut = cursor;
        cursor = (u_long *)((u_char *)cursor + tim[2]);
    }
    image = cursor;
    image_data = image + 3;
    cursor = (u_long *)((u_char *)cursor + image[0]);
    if (flags & 8) {
        ((RECT *)(clut + 1))->x = clut_pos->vx;
        ((RECT *)(clut + 1))->y = clut_pos->vy;
        LoadImage((RECT *)(clut + 1), clut_data);
    }
    ((RECT *)(image + 1))->x = pos->vx;
    ((RECT *)(image + 1))->y = pos->vy;
    LoadImage((RECT *)(image + 1), image_data);
}
