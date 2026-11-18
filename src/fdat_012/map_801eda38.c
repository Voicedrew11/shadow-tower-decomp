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

/* Not exact (size-exact, schedule): retail's listing is this statement order
 * untouched, with the two bytes[] stores and the record index sharing $a0
 * and the range mask in $a1; here sched1 lifts the constant 1 and the index
 * load to the top of the block (their variable is set twice, so they are not
 * held back next to their uses) and the mask takes $a0. With a literal 1 and
 * a single-set index the index load sinks to its use instead. The record
 * lookup, drop queue and final slot writes agree. Ruled out: every position
 * of the two masked updates as compound assignments, the index in i,
 * distance or a u8 local, r computed before or after the stores. */
/*
 * Starts the entry's fall (animation 9) and, once its record's stream
 * counter is 4, adds table C item 0x10a at its place to the drop slots
 * 48..63 (the oldest drops out when all 16 are used).
 */
ADDRESS(0x801eda38, 0x1b4)
void func_801eda38(Vis212 *e)
{
    Vis48 *c;
    Vis192 *r;
    u16 info;
    u32 range;
    u32 collision;
    int distance;
    int record;
    int i;

    range = e->unk70;
    e->unk0F = 0x80;
    e->unk08 = 9;
    e->unkB0 = 0x40;
    e->unk0E = 8;
    collision = e->unk50;
    record = 1;
    e->unk1C.bytes[0] = record;
    e->unk1C.bytes[3] = record;
    record = e->unk02;
    r = &vis_table_a.unk3500[record];
    e->unk2A = 0;
    e->unk16 = 0;
    e->unk70 = range & ~0x2000000;
    e->unk50 = collision & ~0x80;
    if (DAT_801cba08.unk1448[r->unk09] != 4)
        return;
    if (vis_table_c.unk0000[3] == 16) {
        c = &vis_table_c.entries[48];
        i = 15;
        while (i--) {
            c[0] = c[1];
            c++;
        }
    } else {
        c = &vis_table_c.entries[48 + vis_table_c.unk0000[3]];
        vis_table_c.unk0000[3]++;
    }
    c->unk00 = 0x10a;
    c->unk02 = e->unk26;
    *(u32 *)c->unk04 = 0;
    c->unk08 = e->unk74;
    *(SVECTOR *)&c->unk18 = *(SVECTOR *)&e->unk8C;
    info = vis_table_c.unk0118[266].unk1E;
    c->unk22 = 2;
    c->unk23 = 2;
    c->unk2c = 1;
    c->unk24 = 0;
    c->unk20 = info;
    distance = e->unk40;
    c->unk26 = 0;
    c->unk25 = 0;
    c->unk28 = distance;
}
