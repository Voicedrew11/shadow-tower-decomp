/*
 * ST.EXE visible-id culling (WIP module): vis_measure_position measures one
 * position (in the scratchpad) against the viewer, with an optional
 * side-plane rejection; vis_cull_by_range drops visible ids that are out of
 * range and flags the near ones.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/mesh.h>
#include <st/vis.h>

/*
 * Not exact: same instructions and size; GCC allocates the viewer pointer
 * and the two offsets to t1/t3/t2 where retail has t3/t2/t1, and orders
 * the final squares' temporaries differently.
 */
ADDRESS(0x8001c4c8, 0x144)
int vis_measure_position(void)
{
    VisScratch *sp = VIS_SCRATCH;
    Viewer *cam = &viewer;
    int x;
    int z;
    int y;
    int a;
    int c;
    int ox;
    int oz;

    x = (sp->x - cam->x) / 2048 + (ox = sp->offset_x);
    z = (sp->z - cam->z) / 2048 + (oz = sp->offset_z);
    if (!sp->no_plane_test) {
        a = sp->plane[0] * x - sp->plane[2] * z + sp->plane[4];
        c = sp->plane[1] * x - sp->plane[3] * z + sp->plane[5];
        if (a > 0 || c < 0) {
            return 256;
        }
    }
    sp->unk1ec = -(sp->unk208 * 8) >> 12;
    y = (sp->y - cam->y) / 2048;
    return (x - ox) * (x - ox) + (z - oz) * (z - oz) + y * y;
}

/*
 * Not exact: retail loads the middle range before the far-range pointer and
 * keeps the far depth and its biased copy in separate registers; the rest
 * agrees.
 */
ADDRESS(0x8001c60c, 0x1f4)
void vis_cull_by_range(void)
{
    VisScratch *sp = VIS_SCRATCH;
    VisRecord *rec;
    int range_near;
    int range_mid;
    int range_far;
    int d;
    int i;
    int k;

    range_near = (world_state.unk1ee00 >> 11) * (world_state.unk1ee00 >> 11);
    range_mid = (world_state.unk1ee04 >> 11) * (world_state.unk1ee04 >> 11);
    i = vis_lists.n_visible;
    range_far = *(short *)(world_state.unk1ee08 + 0x6a) / 2048 + 2;
    range_far = range_far * range_far;
    while (i--) {
        rec = &vis_records[vis_lists.visible[i]];
        sp->x = rec->unk08 << 12;
        sp->y = (rec->unk09 << 12) + ((rec->unk0b - 128) << 7);
        sp->z = rec->unk0a << 12;
        sp->no_plane_test = world_state.unk1ee15;
        rec->unk04 &= 1;
        d = vis_measure_position();
        if (d > range_far) {
            vis_lists.n_visible--;
            for (k = i; k < vis_lists.n_visible; k++) {
                vis_lists.visible[k] = vis_lists.visible[k + 1];
            }
        } else if (d < range_near || d <= range_mid) {
            rec->unk04 |= 2;
        }
    }
}
