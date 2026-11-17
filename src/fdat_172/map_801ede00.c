/*
 * FDAT_172.BIN: its last three handlers (WIP module).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <psyq/libc.h>
#include <st/anim.h>
#include <st/vis.h>
#include <st/data.h>
#include <st/hud.h>
#include <st/menu.h>
#include <st/fdat.h>

/* Lifts table C item 28 by 0x200, 24 a frame while its model is loaded. */
ADDRESS(0x801ede00, 0xf8)
void func_801ede00(void)
{
    int step = 24;
    Vis48 *item = &vis_table_c.entries[28];
    Vis44 *kind;
    int top;

    item->unk22 = 5;
    kind = &vis_table_c.unk0118[item->unk00];
    top = item->unk08.vy + 0x200;
    item->unk2D -= 2;
    for (;;) {
        StAnimFile *file;

        if (item->unk08.vy == top)
            break;
        anim_get_file(kind->unk4 + 588, &file);
        if (file != NULL)
            item->unk08.vy += step;
        if (item->unk08.vy > top)
            item->unk08.vy = top;
        func_80026dd4(NULL, NULL, 0);
    }
    item->unk2D += 2;
}

/* Marks every active table A entry: unk36 0xfff and range bits 0xd. */
ADDRESS(0x801edef8, 0x50)
void func_801edef8(void)
{
    Vis212 *e = vis_table_a.entries;
    int i = 64;

    while (i--) {
        if (e->unk05 == 1) {
            e->unk36 = 0xfff;
            e->unk1C.bytes[2] |= 0xd;
        }
        e++;
    }
}

/* In area 3, while flag 27 is clear, hands table A entry 33 to its exit once
 * its record's stream counter has started. */
ADDRESS(0x801edf48, 0x88)
void func_801edf48(void)
{
    Vis212 *e;
    Vis192 *r;

    e = &vis_table_a.entries[33];
    r = &vis_table_a.unk3500[14];

    if (DAT_801cba08.unk1448[r->unk09] != 0
        && !(DAT_801cba08.unk1440 >> 27 & 1) && vis_table_a.unk6168 == 3) {
        DAT_801cba08.unk1448[r->unk09] = 2;
        e->unk70 |= 0x2000000;
        fdat_801ed388(e, 1, 1);
    }
}
