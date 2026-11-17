/*
 * FDAT_012.BIN: the entry's drop (WIP module; a unit of its own until its neighbours in
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

/* Not exact (one attempt, retries off; 63.6%): first divergence retail `lui a1,0xfdff`, this source `lui a0,0xfdff`. */
/*
 * Starts the entry's fall (animation 9) and, once its record's stream
 * counter is 4, adds table C item 0x10a at its place to the drop slots
 * 48..63 (the oldest drops out when all 16 are used).
 */
ADDRESS(0x801eda38, 0x1b4)
void func_801eda38(Vis212 *e)
{
    Vis48 *c;
    int i;

    e->unk0F = 0x80;
    e->unk08 = 9;
    e->unkB0 = 0x40;
    e->unk0E = 8;
    e->unk1C.bytes[0] = 1;
    e->unk1C.bytes[3] = 1;
    e->unk2A = 0;
    e->unk16 = 0;
    e->unk70 &= ~0x2000000;
    e->unk50 &= ~0x80;
    if (DAT_801cba08.unk1448[vis_table_a.unk3500[e->unk02].unk09] != 4)
        return;
    if (vis_table_c.unk0000[3] == 16) {
        c = &vis_table_c.entries[48];
        i = 15;
        while (i--) {
            c[0] = c[1];
            c++;
        }
    } else {
        c = &vis_table_c.entries[48 + vis_table_c.unk0000[3]++];
    }
    c->unk00 = 0x10a;
    c->unk02 = e->unk26;
    *(u32 *)c->unk04 = 0;
    c->unk08 = e->unk74;
    *(SVECTOR *)&c->unk18 = *(SVECTOR *)&e->unk8C;
    c->unk22 = 2;
    c->unk23 = 2;
    c->unk2c = 1;
    c->unk24 = 0;
    c->unk20 = vis_table_c.unk0118[266].unk1E;
    c->unk26 = 0;
    c->unk25 = 0;
    c->unk28 = e->unk40;
}
