/*
 * FDAT_272.BIN: table C item 29's slide (WIP module; a unit of its own until its neighbours in
 * the run are exact).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <psyq/libc.h>
#include <st/vis.h>
#include <st/data.h>
#include <st/anim.h>
#include <st/hud.h>
#include <st/menu.h>
#include <st/light.h>
#include <st/xa.h>
#include <st/fdat.h>

/* Not exact (one attempt, retries off; 91.7%): first divergence retail `sw s3,36(sp)`, this source `sw s3,36(sp)`. */
/* Slides table C item 29 by 0x1ea, 24 a frame while its model is loaded. */
ADDRESS(0x801eead8, 0xdc)
void func_801eead8(void)
{
    int step = 24;
    Vis48 *item = &vis_table_c.entries[29];
    Vis44 *kind = &vis_table_c.unk0118[303];
    int top;

    item->unk22 = 5;
    top = item->unk08.vx + 0x1ea;
    item->unk2D -= 2;
    for (;;) {
        StAnimFile *file;

        if (item->unk08.vx == top)
            break;
        anim_get_file(kind->unk4 + 588, &file);
        if (file != NULL)
            item->unk08.vx += step;
        if (item->unk08.vx > top)
            item->unk08.vx = top;
        func_80026dd4(NULL, NULL, 0);
    }
    item->unk2D += 2;
}
