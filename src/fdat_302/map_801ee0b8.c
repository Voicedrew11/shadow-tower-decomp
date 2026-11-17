/*
 * FDAT_302.BIN: entry 0's scene, the scene check and the word fdat_801ee474 (WIP module; a unit of its own until its neighbours in
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

/* Entry 0's dying scene: 0, 1 dying, 2 due, 0xff done. */
DATA(0x801ee474, 0x4, ".bss")
int fdat_801ee474;

/* Not exact (one attempt, retries off; 87.9%): first divergence retail `lui v0,0x0`, this source `lui v1,0x0`. */
/* Entry 0 finishes animation 10; entry 48 takes its place and goes, entry
 * 3 rises there (animation 10) and entry 49 replaces 48 until 3 is up. */
ADDRESS(0x801ee0b8, 0x2c4)
void func_801ee0b8(void)
{
    Vis212 *e0 = vis_table_a.entries;
    Vis212 *e48 = &vis_table_a.entries[48];
    Vis212 *e49 = &vis_table_a.entries[49];
    Vis212 *e3 = &vis_table_a.entries[3];
    Vis192 *r2 = &vis_table_a.unk3500[2];
    StAnimFile *file;

    viewer.unk218 = 5;
    fdat_801eb7b4(e0, 10);
    for (;;) {
        e0->unk2A += e0->unkB0;
        if ((short)e0->unk2A >= 0x1000)
            break;
        func_80026dd4(NULL, NULL, 0);
    }
    e48->unk74 = e0->unk74;
    *(SVECTOR *)&e48->unk8C = *(SVECTOR *)&e0->unk8C;
    e48->unk26 = e0->unk26;
    func_8004ae38(e0);
    viewer.unk30--;
    fdat_801ed410(e3);
    e3->unk74 = e48->unk74;
    *(SVECTOR *)&e3->unk8C = *(SVECTOR *)&e48->unk8C;
    e3->unk36 = -1;
    e3->unk26 = e48->unk26;
    e3->unk1C.bytes[2] &= 0xf2;
    e3->unk70 &= ~0x2000000;
    fdat_801eb7b4(e3, 10);
    do {
        e3->unk2A = 0;
        func_80026dd4(NULL, NULL, 0);
        anim_get_file(e3->unk0B + 128, &file);
    } while (file == NULL);
    e49->unk74 = e3->unk74;
    *(SVECTOR *)&e49->unk8C = *(SVECTOR *)&e3->unk8C;
    e49->unk26 = e3->unk26;
    r2->unk60[0]->unk0C = 0x90;
    fdat_801eb7b4(e49, 0);
    func_8004ae38(e48);
    for (;;) {
        e49->unk2A += e49->unkB0;
        e3->unk2A += e3->unkB0;
        if ((short)e49->unk2A >= 0x1000)
            e49->unk2A = 0xfff;
        if ((short)e3->unk2A >= 0x1000)
            break;
        fdat_801eba4c(e49);
        fdat_801eba4c(e3);
        func_80026dd4(NULL, NULL, 0);
    }
    func_8004ae38(e49);
    viewer.unk218 = 0;
    fdat_801ee474 = 0xff;
}

/* Not exact (one attempt, retries off; 43.7%): first divergence retail `addiu a3,v0,0`, this source `addiu a2,v0,0`. */
/* While record 0's stream counter has started: wakes entry 0 (counter 2)
 * until its scene ran, else entry 3 (record 3's counter 1) until it is
 * beaten, else entry 5 (record 8's counter 3). */
ADDRESS(0x801ee37c, 0xf0)
void func_801ee37c(void)
{
    Vis212 *e0 = vis_table_a.entries;
    Vis212 *e3 = &vis_table_a.entries[3];
    Vis192 *r3 = &vis_table_a.unk3500[3];
    Vis212 *e5 = &vis_table_a.entries[5];
    Vis192 *r8 = &vis_table_a.unk3500[8];
    Vis212 *e;

    if (DAT_801cba08.unk1448[vis_table_a.unk3500[0].unk09] == 0)
        return;
    if (fdat_801ee474 != 0xff) {
        DAT_801cba08.unk1448[vis_table_a.unk3500[0].unk09] = 2;
        e = e0;
    } else if (fdat_801ee478 != 1) {
        DAT_801cba08.unk1448[r3->unk09] = 1;
        e = e3;
    } else if (DAT_801cba08.unk1448[r8->unk09] != 0 && fdat_801ee480 != 1) {
        DAT_801cba08.unk1448[r8->unk09] = 3;
        e = e5;
    } else {
        return;
    }
    e->unk70 |= 0x2000000;
    fdat_801ed388(e, 1, 1);
}
