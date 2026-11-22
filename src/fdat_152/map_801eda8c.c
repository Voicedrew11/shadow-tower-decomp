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
void func_801eda8c(void)
{
    int step = 24;
    Vis48 *item = &vis_table_c.entries[11];
    Vis44 *kind;
    int end;

    item->unk22 = 5;
    kind = &vis_table_c.unk0118[item->unk00];
    end = item->unk08.vx + 0x1e0;
    item->unk2D -= 2;
    for (;;) {
        StAnimFile *file;

        if (item->unk08.vx == end)
            break;
        anim_get_file(kind->unk4 + 588, &file);
        if (file != NULL)
            item->unk08.vx += step;
        if (item->unk08.vx > end)
            item->unk08.vx = end;
        world_run_frame(NULL, NULL, 0);
    }
    item->unk2D += 2;
}
