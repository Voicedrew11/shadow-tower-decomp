/*
 * ST.EXE: the visible-set sources around up to three positions (the
 * intersection of the three axis masks of each position's cell, united),
 * and the sources' cell positions (WIP module).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/data.h>
#include <st/gfx.h>
#include <st/vis.h>

/* Adds every source whose x, y and z cell masks all hold a position. */
ADDRESS(0x80046270, 0x2fc)
void vis_collect_sources(VECTOR *a, VECTOR *b, VECTOR *c)
{
    u_long mask[16];
    u_long *x;
    u_long *y;
    u_long *z;
    int i;
    int j;
    s16 id;

    x = vis_axis_masks[0][a->vx / 4096].bits;
    y = vis_axis_masks[1][a->vy / 4096].bits;
    z = vis_axis_masks[2][a->vz / 4096].bits;
    for (i = 0; i < 16; i++)
        mask[i] = x[i] & z[i] & y[i];
    if (b) {
        u_long *x = vis_axis_masks[0][b->vx / 4096].bits;
        u_long *y = vis_axis_masks[1][b->vy / 4096].bits;
        u_long *z = vis_axis_masks[2][b->vz / 4096].bits;

        for (i = 0; i < 16; i++)
            mask[i] |= x[i] & z[i] & y[i];
    }
    if (c) {
        u_long *x = vis_axis_masks[0][c->vx / 4096].bits;
        u_long *y = vis_axis_masks[1][c->vy / 4096].bits;
        u_long *z = vis_axis_masks[2][c->vz / 4096].bits;

        for (i = 0; i < 16; i++)
            mask[i] |= x[i] & z[i] & y[i];
    }
    for (id = i = 0; i < 16; i++) {
        for (j = 0; j < 32; j++) {
            if ((mask[i] & 1) && (area_state.loading != 1 || !area_state.loading_world ||
                             vis_records[id].part < ((u_long *)world_state.area_geometry[1])[1] >> 1)) {
                vis_lists.sources[vis_lists.n_sources] = id;
                vis_lists.n_sources++;
            }
            mask[i] >>= 1;
            id++;
        }
    }
}

/* The cell position of each collected source. */
ADDRESS(0x8004656c, 0x98)
void collision_source_positions(VECTOR *out)
{
    MapBlock *rec;
    int i;

    i = vis_lists.n_sources;
    while (i--) {
        rec = &vis_records[vis_lists.sources[i]];
        out[i].vx = rec->cell_x << 12;
        out[i].vy = rec->cell_y * 4096 + (rec->y_offset - 128) * 128;
        out[i].vz = rec->cell_z << 12;
    }
}
