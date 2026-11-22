/*
 * FDAT_152.BIN: its last handler (WIP module; a unit of its own until its
 * neighbours in the run are exact).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <psyq/libc.h>
#include <st/anim.h>
#include <st/vis.h>
#include <st/hud.h>
#include <st/fdat.h>

/* Slides table C item 11 by 0x1e0 along x, 24 a frame while its model is
 * loaded. */
ADDRESS(0x801eda8c, 0xf8)
void map_slide_ground_item(void)
{
    int step = 24;
    GroundItem *item = &vis_table_c.entries[11];
    ItemType *kind;
    int end;

    item->state = 5;
    kind = &vis_table_c.types[item->item];
    end = item->position.vx + 0x1e0;
    item->depth_bias -= 2;
    for (;;) {
        StAnimFile *file;

        if (item->position.vx == end)
            break;
        anim_get_file(kind->model + 588, &file);
        if (file != NULL)
            item->position.vx += step;
        if (item->position.vx > end)
            item->position.vx = end;
        world_run_frame(NULL, NULL, 0);
    }
    item->depth_bias += 2;
}
