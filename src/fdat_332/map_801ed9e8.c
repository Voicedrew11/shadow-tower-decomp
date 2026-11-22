/*
 * FDAT_332.BIN: its last handler (WIP module; a unit of its own until its neighbours in
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

/* Not exact: all body instructions agree, but retail reserves 40 bytes and
 * saves the four registers from sp+24; this source reserves 32 and saves
 * them from sp+16. The eight bytes at sp+16 are never accessed: either an
 * unused 8-byte local (the sibling scenes keep an SVECTOR there) or a third
 * word pair of outgoing arguments from a five-argument call that was
 * optimized away; neither is evidenced by an instruction. */
/* Entry 33's end: it faces the viewer, finishes its animation, leaves its
 * drops (20/30/100 percent), dies (animation 12) and table B entry 3
 * opens; the area's history is reset (scene word 2). */
ADDRESS(0x801ed9e8, 0x154)
void func_801ed9e8(void)
{
    Vis212 *e;
    Vis88 *d3;

    viewer.unk218 = 5;
    e = &vis_table_a.entries[33];
    d3 = &vis_table_b.entries[3];
    player_turn_to_creature(e);
    for (;;) {
        e->unk2A += e->unkB0;
        if ((short)e->unk2A >= 0x1000)
            goto animation_done;
        world_run_frame(NULL, NULL, 0);
    }
animation_done:
    creature_force_talk(e, 0, 0);
    creature_force_talk(e, 0, 0);
    vis_table_a.unk6158[3] = 0xff;
    vis_table_a.unk6158[2] = 0xff;
    vis_table_a.unk6158[1] = 0xff;
    vis_table_a.unk6158[0] = 0xff;
    e->unk65 = 20;
    e->unk66 = 30;
    e->unk67 = 100;
    creature_roll_drops(e);
    creature_award_kill_bonuses(e);
    creature_start_script(e, 12);
    e->unk05 = 5;
    e->unk34 = 0xfff;
    e->unk1A = 0xf0;
    e->unk2A = 0;
    e->unk2C = 0;
    d3->unk05 = 0xed;
    viewer.unk218 = 0;
    fdat_801edb3c = 2;
    vis_table_a.unk612C = 0;
    vis_table_a.unk6128 = 10;
    e->unk1C.bytes[2] |= 0x15;
}
