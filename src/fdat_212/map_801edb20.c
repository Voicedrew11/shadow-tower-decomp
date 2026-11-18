/*
 * FDAT_212.BIN's shared handlers fdat_801ed4a0 (23 overlays; its switch
 * table is the unit's read-only claim) and fdat_801ed6e4 (19 overlays;
 * WIP module).
 * Then the overlay's last handler, func_801edbd0.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <psyq/libc.h>
#include <st/anim.h>
#include <st/vis.h>
#include <st/hud.h>
#include <st/sound.h>
#include <st/fdat.h>

RODATA(0x801eb730, 0x7c)

#include "../lib/fdat_801ed4a0.inc"
#include "../lib/fdat_801ed6e4.inc"

/* Lifts table C item 28 (`which` 0) or 29 by 0x200 along z, 24 a frame
 * while its model is loaded. */
ADDRESS(0x801edbd0, 0x104)
void func_801edbd0(int which)
{
    int step = 24;
    Vis48 *item;
    Vis44 *kind;
    int end;

    if (which == 0)
        item = &vis_table_c.entries[28];
    else
        item = &vis_table_c.entries[29];
    item->unk22 = 5;
    kind = &vis_table_c.unk0118[item->unk00];
    end = item->unk08.vz + 0x200;
    item->unk2D -= 12;
    for (;;) {
        StAnimFile *file;

        if (item->unk08.vz == end)
            break;
        anim_get_file(kind->unk4 + 588, &file);
        if (file != NULL)
            item->unk08.vz += step;
        if (item->unk08.vz > end)
            item->unk08.vz = end;
        func_80026dd4(NULL, NULL, 0);
    }
    func_80057d58();
}
