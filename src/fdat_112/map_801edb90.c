/*
 * FDAT_112.BIN: the entry's exit (WIP module; a unit of its own until its neighbours in
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

/* The leaving entry: animation 7 (turning to 0x800 at half its speed),
 * then 1 walking along z until 0x4f000. */
ADDRESS(0x801edb90, 0x114)
void func_801edb90(Vis212 *e)
{
    e->unk2A += e->unkB0;
    if (e->unk18 == 7) {
        if ((short)e->unk2A >= 0x1000)
            fdat_801eb7b4(e, 1);
        else
            fdat_801eb814(0x800, (short)e->unkB0 / 2);
    }
    if (e->unk18 == 1) {
        if ((short)e->unk2A >= 0x1000)
            e->unk2A = 0;
        e->unk0E = 1;
        e->unk50 &= ~0x82;
        fdat_801eb814(0x800, 0x40);
        fdat_801eb880(e->unk8E - 0x400, 0x40);
        if (e->unk74.vz > 0x4f000) {
            creature_despawn(e);
            viewer.unk218 = 0;
        }
    }
}
