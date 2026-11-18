/*
 * ST.EXE visible-id culling (WIP module): vis_measure_position measures one
 * position (in the scratchpad) against the viewer, with an optional
 * side-plane rejection; vis_cull_by_range drops visible ids that are out of
 * range and flags the configured distance bands.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/gfx.h>
#include <st/vis.h>

/*
 * Builds the viewer's two side planes for vis_measure_position: the view
 * apex sits behind the viewer along its yaw, and the half-angle widens with
 * the pitch (folded to 0..2048).
 */
ADDRESS(0x8001c264, 0x264)
void vis_build_planes(void)
{
    ScratchPad *sp = SCRATCHPAD;
    Viewer *cam = &viewer;
    int yaw;
    int pitch;
    int half;
    int c;
    int s;
    int cl;
    int sl;
    int cr;
    int sr;
    int cp;
    int back;
    int len;
    int near;
    short ox;
    short oz;
    short lx;
    short lz;
    short rx;
    short rz;

    yaw = cam->unk26A + 1024;
    pitch = cam->unk268;
    if (pitch > 2048) {
        half = 4096 - pitch;
    } else {
        half = pitch;
    }
    half = half * 584 / 1024 + 512;
    c = rcos(yaw);
    s = rsin(yaw);
    cl = rcos(yaw - half);
    sl = rsin(yaw - half);
    cr = rcos(yaw + half);
    sr = rsin(yaw + half);
    cp = rcos(pitch);
    sp->unk208 = rsin(pitch);
    back = cp * -11 >> 12;
    len = cp * 16 >> 12;
    near = cp * -8 >> 12;
    ox = back * c >> 12;
    oz = back * s >> 12;
    lx = (len * cl + back * c) >> 12;
    lz = (len * sl + back * s) >> 12;
    rx = (len * cr + back * c) >> 12;
    rz = (len * sr + back * s) >> 12;
    sp->plane[0] = lz - oz;
    sp->plane[2] = lx - ox;
    sp->plane[1] = rz - oz;
    sp->plane[3] = rx - ox;
    sp->offset_x = near * c >> 12;
    sp->offset_z = near * s >> 12;
    sp->plane[4] = -sp->plane[0] * ox + sp->plane[2] * oz;
    sp->plane[5] = -sp->plane[1] * ox + sp->plane[3] * oz;
}

/*
 * Not exact: same instructions and size. Retail has z, x, the two cell
 * offsets and the viewer pointer in a3, t0, t1, t2, t3; this build gives the
 * pointer t1 (its priority must fall below the offsets': 3 refs, or 5 refs
 * for x/z and 4 for the offsets), and the final squares use a1/v1 for
 * retail's v1/v0. A permuter run and reordering the reads did not get there.
 */
ADDRESS(0x8001c4c8, 0x144)
int vis_measure_position(void)
{
    ScratchPad *sp = SCRATCHPAD;
    Viewer *cam = &viewer;
    int x;
    int z;
    int y;
    int a;
    int c;
    int ox;
    int oz;

    x = (sp->x - cam->position.vx) / 2048 + (ox = sp->offset_x);
    z = (sp->z - cam->position.vz) / 2048 + (oz = sp->offset_z);
    if (!sp->no_plane_test) {
        a = sp->plane[0] * x - sp->plane[2] * z + sp->plane[4];
        c = sp->plane[1] * x - sp->plane[3] * z + sp->plane[5];
        if (a > 0 || c < 0) {
            return 256;
        }
    }
    sp->unk1EC = -(sp->unk208 * 8) >> 12;
    y = (sp->y - cam->position.vy) / 2048;
    return (x - ox) * (x - ox) + (z - oz) * (z - oz) + y * y;
}

ADDRESS(0x8001c60c, 0x1f4)
void vis_cull_by_range(void)
{
    ScratchPad *sp = SCRATCHPAD;
    VisRecord *rec;
    int range_near;
    int range_mid;
    int range_far;
    int d;
    int i;
    int k;

    range_near = (world_state.unk1EE00 >> 11) * (world_state.unk1EE00 >> 11);
    range_mid = (world_state.unk1EE04 >> 11) * (world_state.unk1EE04 >> 11);
    i = vis_lists.n_visible;
    range_far = (world_state.light->unk6A / 2048 + 2) *
                (world_state.light->unk6A / 2048 + 2);
    while (i--) {
        rec = &vis_records[vis_lists.visible[i]];
        sp->x = rec->unk08 << 12;
        sp->y = (rec->unk09 << 12) + ((rec->unk0b - 128) << 7);
        sp->z = rec->unk0a << 12;
        sp->no_plane_test = world_state.unk1EE15;
        rec->unk04 &= 1;
        d = vis_measure_position();
        if (d > range_far) {
            vis_lists.n_visible--;
            for (k = i; k < vis_lists.n_visible; k++) {
                vis_lists.visible[k] = vis_lists.visible[k + 1];
            }
        } else if (d >= range_near || d <= range_mid) {
            rec->unk04 |= 2;
        }
    }
}
