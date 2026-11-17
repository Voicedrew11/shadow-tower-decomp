/*
 * FDAT_272.BIN: the scene's last part (WIP module; a unit of its own until its neighbours in
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

DATA(0x801ef938, 0x4, ".bss")
int fdat_801ef938;

/* Bumps record 10's stream counter, sends entry 51 away (its scripts 0..2
 * become 0x80, 5..7 0x70..0x72) and restarts entry 50. */
ADDRESS(0x801ee948, 0xcc)
void func_801ee948(void)
{
    Vis212 *e = &vis_table_a.entries[51];
    Vis192 *r = &vis_table_a.unk3500[10];

    DAT_801cba08.unk1448[r->unk09]++;
    e->unk70 |= 0x2000000;
    r->unk60[0]->unk00 = 0x80;
    r->unk60[1]->unk00 = 0x80;
    r->unk60[2]->unk00 = 0x80;
    r->unk60[5]->unk00 = 0x70;
    r->unk60[6]->unk00 = 0x71;
    r->unk60[7]->unk00 = 0x72;
    func_80054368(e, 0, 0);
    fdat_801ecf44(e);
    e--;
    fdat_801eb7b4(e, 0);
    func_8004ae38(e);
}
