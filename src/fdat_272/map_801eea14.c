/*
 * FDAT_272.BIN: the scene's stage 1 setup (WIP module; a unit of its own until its neighbours in
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

DATA(0x801ef920, 0x4, ".bss")
int fdat_801ef920;

/* Once: entry 51 plays animation 8 and entry 50 animation 1, record 11's
 * ranges are set and the scene stage becomes 1. */
ADDRESS(0x801eea14, 0xc4)
void func_801eea14(void)
{
    Vis212 *e51 = &vis_table_a.entries[51];
    Vis212 *e50 = e51 - 1;
    Vis192 *r = &vis_table_a.unk3500[11];

    if (fdat_801ef928 == 0) {
        e51->unk1C.bytes[3] = 1;
        fdat_801ef920 = -1;
        e51->unk70 &= ~0x2000000;
        fdat_801eb7b4(e51, 8);
        fdat_801eb7b4(e50, 1);
        fdat_801ef92c = 1;
        r->unk16 = 0x200;
        r->unk18 = 0x800;
        fdat_801ef928 = 1;
        DAT_801cba08.unk1440.word = (DAT_801cba08.unk1440.word & ~0x300) | 0x100;
    }
}
