/*
 * FDAT_332.BIN: its first handler (WIP module; a unit of its own until its neighbours in
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

/* Once table A entry 33 dies (scene word 0): the lighting of six areas
 * changes, the entry comes back (flag 16) and entries 4 and 5, if active,
 * die with it. */
ADDRESS(0x801ed4a0, 0x174)
void func_801ed4a0(void)
{
    Vis212 *e33 = &vis_table_a.entries[33];
    Vis212 *e4 = &vis_table_a.entries[4];
    Vis212 *e5 = &vis_table_a.entries[5];
    int state = e33->unk05;

    if (state == 5 && fdat_801edb3c == 0) {
        lighting.area_mode[11] = 0x11;
        lighting.area_mode[27] = 0x11;
        lighting.area_mode[12] = 0x11;
        lighting.area_mode[28] = 0x11;
        lighting.area_mode[15] = 0x11;
        lighting.area_mode[33] = 0x11;
        e33->unk05 = 1;
        e33->unk1C.bytes[3] = 1;
        e33->unk34 = -1;
        fdat_801edb3c = 1;
        DAT_801cba08.unk1440.word |= 0x10000;
        e33->unk1C.bytes[2] &= 0xea;
        if (e4->unk05 == 1) {
            e4->unk05 = state;
            e4->unk2C = 0;
            e4->unk1C.bytes[2] |= 0x15;
            fdat_801eb7b4(e4, 5);
            func_8004da2c(e4);
            func_8004db0c(e4);
            e4->unk34 = 0x1000;
            viewer.unk30++;
        }
        if (e5->unk05 == 1) {
            e5->unk05 = state;
            e5->unk2C = 0;
            e5->unk1C.bytes[2] |= 0x15;
            fdat_801eb7b4(e5, 5);
            func_8004da2c(e5);
            func_8004db0c(e5);
            e5->unk34 = 0x1000;
            viewer.unk30++;
        }
    }
}
