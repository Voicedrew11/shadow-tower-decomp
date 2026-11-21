/*
 * FDAT_122.BIN: the per-entry handler (WIP module; a unit of its own until its neighbours in
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

void func_801ed8f8(Vis212 *e);

/* The current entry's handler: kind 0 dying sets fdat_801ed938 and the
 * second flag word's bit 15; then the animation steps as fdat_801ed6e4
 * does, kind 2 running func_801ed8f8 after its next move. */
ADDRESS(0x801ed77c, 0x104)
void func_801ed77c(void)
{
    Vis212 *e = vis_table_a.unk6138;
    u32 flags;

    if (e->unk02 == 0 && e->unk05 == 5) {
        Data801cba08 *story = &DAT_801cba08;

        flags = story->unk1444.word;
        fdat_801ed938 = 1;
        story->unk1444.word = flags | 0x8000;
    }
    e->unk2A += e->unkB0;
    if (fdat_801ece7c(e) == 1)
        return;
    if (e->unkB0 >= 0 ? (short)e->unk2A >= 4096 : (short)e->unk2A <= 0) {
        e->unk1A = 0xff;
        fdat_801ed4a0();
        if (e->unk02 == 2)
            func_801ed8f8(e);
    }
    if (e->unkA8 != NULL) {
        fdat_801ebec4(e);
        e->unk1A = 0xf0;
    }
}
