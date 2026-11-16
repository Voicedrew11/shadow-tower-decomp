/*
 * VS.EXE: the sound bank load and the TIM uploads (WIP module).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBSND.H>
#include <vs/sound.h>
#include <vs/buffer.h>

/* Opens sound bank `bank`'s VAB header and transfers its body to SPU
 * memory at 0x3e400 + bank * 0xa000. */
ADDRESS(0x80012a70, 0xe0)
void func_80012a70(int bank)
{
    VsSoundBank *b = &DAT_801604b0[bank];

    b->vab = SsVabOpenHeadSticky(b->head, -1, bank * 0xa000 + 0x3e400);
    if (b->vab != -1) {
        b->unk1002 = SsVabTransBodyPartly(DAT_800633b0.bank[bank], 0x12c00, b->vab);
        if (b->unk1002 == b->vab)
            SsVabTransCompleted(SS_WAIT_COMPLETED);
    }
}

/* Uploads a run of TIM images, shifting each image `page` texture pages
 * right and each CLUT `clut_y` lines down. */
/* Not exact: retail keeps the record start in $a0 and the CLUT flag test in
 * $v1 (this build swaps the two). */
ADDRESS(0x80012b50, 0xdc)
void func_80012b50(u_long *data, int page, int clut_y)
{
    u_long *tim = data;
    u_long flags;
    u_long *p;
    u_long *clut;
    u_long *clut_data;
    u_long *image;
    u_long *image_data;

    for (;;) {
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
        tim = (u_long *)((u_char *)tim + image[0]);
        if (flags & 8) {
            ((RECT *)(clut + 1))->y += clut_y;
            LoadImage((RECT *)(clut + 1), clut_data);
        }
        ((RECT *)(image + 1))->x += page * 64;
        LoadImage((RECT *)(image + 1), image_data);
    }
}

/* Uploads a run of TIM images to the given image and CLUT positions. */
/* Not exact: as func_80012b50, and retail copies the parameters to
 * $s1/$s5/$s6 (the record pointer in $s1). */
ADDRESS(0x80012c2c, 0xe4)
void func_80012c2c(u_long *data, DVECTOR *pos, DVECTOR *clut_pos)
{
    u_long *tim = data;
    u_long flags;
    u_long *p;
    u_long *clut;
    u_long *clut_data;
    u_long *image;
    u_long *image_data;

    for (;;) {
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
        tim = (u_long *)((u_char *)tim + image[0]);
        if (flags & 8) {
            ((RECT *)(clut + 1))->x = clut_pos->vx;
            ((RECT *)(clut + 1))->y = clut_pos->vy;
            LoadImage((RECT *)(clut + 1), clut_data);
        }
        ((RECT *)(image + 1))->x = pos->vx;
        ((RECT *)(image + 1))->y = pos->vy;
        LoadImage((RECT *)(image + 1), image_data);
    }
}
