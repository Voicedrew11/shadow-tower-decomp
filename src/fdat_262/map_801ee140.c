/*
 * FDAT_262.BIN: its last two handlers (WIP module; a unit of its own until
 * its neighbours in the run are exact).
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

/* Slides table C item 28 (`which` 0, by -0x200) or 29 (by 0x200) along x,
 * 24 a frame while its model is loaded. */
ADDRESS(0x801ee140, 0x138)
void func_801ee140(int which)
{
    Vis48 *item;
    int end;
    int step;
    Vis44 *kind;

    if (which == 0) {
        item = &vis_table_c.entries[28];
        step = -24;
        end = item->unk08.vx - 0x200;
    } else {
        item = &vis_table_c.entries[29];
        step = 24;
        end = item->unk08.vx + 0x200;
    }
    item->unk22 = 5;
    kind = &vis_table_c.unk0118[item->unk00];
    item->unk2D -= 1;
    for (;;) {
        StAnimFile *file;

        if (item->unk08.vx == end)
            break;
        anim_get_file(kind->unk4 + 588, &file);
        if (file != NULL)
            item->unk08.vx += step;
        if ((step > 0 && item->unk08.vx > end) || (step < 0 && item->unk08.vx < end))
            item->unk08.vx = end;
        func_80026dd4(NULL, NULL, 0);
    }
    func_80057d58();
}

/*
 * Turns one of two table C items into its table A form: items 7 and 9
 * (`which` 0) or 8 and 10, the one whose id bit is clear first. Both items
 * are cleared.
 */
ADDRESS(0x801ee278, 0xf8)
void func_801ee278(int which)
{
    Vis212 e;
    Vis48 *a;
    Vis48 *b;

    if (which == 0) {
        if (vis_table_c.unk0070[1] & 0x20) {
            a = &vis_table_c.entries[9];
            b = &vis_table_c.entries[7];
        } else {
            a = &vis_table_c.entries[7];
            b = &vis_table_c.entries[9];
        }
    } else {
        if (vis_table_c.unk0070[1] & 0x800) {
            a = &vis_table_c.entries[10];
            b = &vis_table_c.entries[8];
        } else {
            a = &vis_table_c.entries[8];
            b = &vis_table_c.entries[10];
        }
    }
    e.unk26 = a->unk02;
    e.unk74 = a->unk08;
    e.unk30 = 0;
    *(SVECTOR *)&e.unk8C = *(SVECTOR *)&a->unk18;
    e.unk40 = a->unk28;
    func_8004d804(&e, a->unk00);
    b->unk22 = 0;
    a->unk22 = 0;
}
