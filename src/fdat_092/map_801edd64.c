/*
 * FDAT_092.BIN's handler fdat_801ed6e4, which 19 overlays share (WIP module).
 * Then the overlay's last two handlers.
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
#include <st/fdat.h>

#include "../lib/fdat_801ed6e4.inc"

/* Plays script 1 of table A entry 48 to its end. */
ADDRESS(0x801ede14, 0x90)
void func_801ede14(void)
{
    Vis212 *e = &vis_table_a.entries[48];

    fdat_801eb7b4(e, 1);
    do {
        fdat_801eba4c(e);
        e->unk2A += e->unkB0;
        if ((short)e->unk2A >= 0x1000)
            e->unk2A = 0xfff;
        func_80026dd4(NULL, NULL, 0);
    } while ((short)e->unk2A != 0xfff);
}

/*
 * Drops table C item 5 (kind 48) at the viewer once its model is loaded.
 */
ADDRESS(0x801edea4, 0xb0)
void func_801edea4(void)
{
    StAnimFile *file;
    Vis48 *item = &vis_table_c.entries[5];
    Vis44 *kind = &vis_table_c.unk0118[48];

    item->unk08 = viewer.position;
    item->unk22 = 2;
    item->unk2D -= 3;
    for (;;) {
        anim_get_file(kind->unk4 + 588, &file);
        if (file != NULL)
            break;
        func_80026dd4(NULL, NULL, 0);
    }
    func_8005335c(item, 1);
}
