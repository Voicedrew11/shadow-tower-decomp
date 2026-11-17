/*
 * FDAT_272.BIN: the scene's first part (WIP module; a unit of its own until its neighbours in
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

DATA(0x801ef930, 0x4, ".bss")
int fdat_801ef930;

/* Takes the item of slot 3, recomputes the figures and slots, raises table
 * B entry 16 and slides table C item 28 along. */
ADDRESS(0x801ee610, 0x1c4)
void func_801ee610(void)
{
    Vis88 *d16 = &vis_table_b.entries[16];
    Vis48 *c28 = &vis_table_c.entries[28];
    Vis44 *kind;
    int step;

    ((u8 *)&DAT_801c7590)[viewer.unk38[3]] = 0;
    viewer.unk38[3] = 0xffff;
    kind = &vis_table_c.unk0118[c28->unk00];
    func_8005a3a8(0);
    func_8003d030();
    func_8003f12c(viewer.unk38[3], 0, 0);
    func_8003f12c(viewer.unk38[16], 1, 0);
    d16->unk05 = 50;
    d16->unk02 = 0;
    d16->unk10 = 0;
    c28->unk22 = 5;
    c28->unk2D += 3;
    while (c28->unk08.vz <= d16->unk24.vz - 0x300) {
        StAnimFile *file;

        anim_get_file(kind->unk4 + 588, &file);
        if (file != NULL)
            c28->unk08.vz += 36;
        func_80026dd4(NULL, NULL, 0);
    }
    step = d16->unk48 * 4;
    for (;;) {
        int v;

        func_8004ed54(d16, d16->unk0a * 3 + 0xd0);
        v = d16->unk10;
        if (v == 0xfff) {
            d16->unk0C = 0xff;
            break;
        }
        v += step;
        if (v >= 0x1000)
            d16->unk10 = 0xfff;
        else
            d16->unk10 = v;
        c28->unk08.vz += 12;
        func_80026dd4(NULL, NULL, 0);
    }
}
