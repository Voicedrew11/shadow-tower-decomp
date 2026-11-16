/*
 * VS.EXE: a single TIM upload (WIP module), as one pass of func_80012c2c.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>

/* Uploads one TIM image (and its CLUT) to the given positions. */
/* Not exact: retail keeps the record start in $t0 and steps the pointer in
 * $a0 before copying it to $s0; this build holds the pointer in $s0. */
ADDRESS(0x80018240, 0xac)
void func_80018240(u_long *tim, DVECTOR *pos, DVECTOR *clut_pos)
{
    u_long flags;
    u_long *p;
    u_long *clut;
    u_long *clut_data;
    u_long *image;
    u_long *image_data;

    if (tim[0] != 0x10)
        return;
    p = tim;
    flags = p[1];
    tim += 2;
    if (flags & 8) {
        clut_data = p + 5;
        clut = tim;
        tim = (u_long *)((u_char *)tim + p[2]);
    }
    image = tim;
    image_data = image + 3;
    if (flags & 8) {
        ((RECT *)(clut + 1))->x = clut_pos->vx;
        ((RECT *)(clut + 1))->y = clut_pos->vy;
        LoadImage((RECT *)(clut + 1), clut_data);
    }
    ((RECT *)(image + 1))->x = pos->vx;
    ((RECT *)(image + 1))->y = pos->vy;
    LoadImage((RECT *)(image + 1), image_data);
}
