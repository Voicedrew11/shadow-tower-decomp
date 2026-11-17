/*
 * VS.EXE: drawing a model part with its floor reflection (WIP module).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <INLINE_C.H>
#include <st/mesh.h>
#include <vs/data.h>
#include <vs/scratch.h>
#include <vs/funcs.h>

/* Projects the current part's vertices into the scratchpad's vertex buffer
 * (RTPT in threes, then RTPS), negating each one's y first when `flip`.
 * Start the first group ahead of the loop, then store each finished group
 * before starting the next. Each projection phase owns its cursors. */
#define MIRROR_PROJECT(sp, flip)                                              \
    {                                                                         \
        int n = (sp)->header->n_vertices / 3;                                 \
        MeshVertex *out = (MeshVertex *)(sp)->vertices;                       \
        SVECTOR *in = (SVECTOR *)(sp)->local_vertices;                        \
        if (n--) {                                                            \
            if (flip) {                                                       \
                in[0].vy = -in[0].vy;                                         \
                in[1].vy = -in[1].vy;                                         \
                in[2].vy = -in[2].vy;                                         \
            }                                                                 \
            gte_ldv3c(in);                                                    \
            gte_rtpt();                                                       \
            in += 3;                                                          \
            for (;;) {                                                        \
                gte_stsxy3(&out[0].sxy, &out[1].sxy, &out[2].sxy);            \
                gte_stsz3(&out[0].z, &out[1].z, &out[2].z);                   \
                out += 3;                                                     \
                if (!n--)                                                     \
                    break;                                                    \
                if (flip) {                                                   \
                    in[0].vy = -in[0].vy;                                     \
                    in[1].vy = -in[1].vy;                                     \
                    in[2].vy = -in[2].vy;                                     \
                }                                                             \
                gte_ldv3c(in);                                                \
                in += 3;                                                      \
                gte_rtpt();                                                   \
            }                                                                 \
        }                                                                     \
        n = (sp)->header->n_vertices % 3;                                     \
        while (n--) {                                                         \
            if (flip)                                                         \
                in->vy = -in->vy;                                             \
            gte_ldv0(in);                                                     \
            gte_rtps();                                                       \
            in++;                                                             \
            gte_stsxy(&out->sxy);                                             \
            gte_stsz(&out->z);                                                \
            out++;                                                            \
        }                                                                     \
    }

/*
 * Draws model part 0. With the mirror switch on, draws it as is (both
 * renderer switches off), then flips it in y and draws it again mirrored
 * and semi-transparent, leaving the vertices flipped and the switches on.
 */
/* Not exact (same size and CFG): seven Y negations load with LHU rather
 * than retail's LH (first divergence at +0x19c). */
ADDRESS(0x80028474, 0x4a0)
void func_80028474(void)
{
    VsScratchPad *sp = VS_SCRATCHPAD;

    if (DAT_80063360) {
        MIRROR_PROJECT(sp, 0);
        DAT_80063360 = 0;
        DAT_80063350 = 0;
        func_80027ba4(0);
        MIRROR_PROJECT(sp, 1);
        DAT_80063360 = 1;
        DAT_80063350 = 1;
        func_80027ba4(0);
    } else {
        MIRROR_PROJECT(sp, 0);
        func_80027ba4(0);
    }
}
