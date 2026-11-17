/*
 * FDAT_272.BIN: table B entry 15's rise (WIP module; a unit of its own until its neighbours in
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

/* Raises table B entry 15 until its state is 0xfff, then marks it open. */
ADDRESS(0x801eebb4, 0xc0)
void func_801eebb4(void)
{
    Vis88 *d = &vis_table_b.entries[15];
    int step;

    d->unk05 = 50;
    d->unk02 = 0;
    d->unk10 = 0;
    step = d->unk48 * 4;
    do {
        if (d->unk10 == 0)
            func_8004ed54(d, d->unk0a * 3 + 0xd0);
        d->unk10 += step;
        if (d->unk10 >= 0x1000)
            d->unk10 = 0xfff;
        func_80026dd4(NULL, NULL, 0);
    } while (d->unk10 != 0xfff);
    d->unk0C = 0xff;
}
