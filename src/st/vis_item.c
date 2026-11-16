/*
 * ST.EXE: table A entry drops and checks (WIP module, 0x8004d75c..0x8004def8;
 * its caller func_8004def8 opens the next band and may share its file).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <psyq/libc.h>
#include <st/vis.h>
#include <st/funcs.h>
#include <st/data.h>

ADDRESS(0x8004d75c, 0x8)
int func_8004d75c(Vis212 *e)
{
    return 0;
}

/* Plays the entry's sound from its current list entry at its position. */
ADDRESS(0x8004d764, 0x98)
void func_8004d764(Vis212 *e)
{
    u8 *p = e->unkA8;

    if (e->unk70 & 0x800000)
        func_80058254(p[5] + 48, &e->unk74, 127, 24576, 30720, func_8004d75c(e));
    else
        func_80058220(p[5] + 48, &e->unk74, 127, func_8004d75c(e));
}

ADDRESS(0x8004d7fc, 0x8)
void func_8004d7fc(void)
{
}

/* Drops item `id` where the entry stands: a slot of table C's group 48
 * (kind 30) or group 32, the oldest slot giving way when the group is full. */
/*
 * Not exact: in both copy loops retail counts in t2 and ends at t1; this
 * source swaps the two registers.
 */
ADDRESS(0x8004d804, 0x228)
void func_8004d804(Vis212 *e, int id)
{
    Vis44 *rec = &vis_table_c.unk0118[id];
    Vis48 *slot;
    int n;

    if (rec->unk26 == 30) {
        if (vis_table_c.unk0000[3] == 16) {
            slot = &vis_table_c.entries[48];
            n = 15;
            while (n--) {
                slot[0] = slot[1];
                slot++;
            }
        } else {
            slot = &vis_table_c.entries[48 + vis_table_c.unk0000[3]];
            vis_table_c.unk0000[3]++;
        }
    } else if (vis_table_c.unk0000[2] == 16) {
        slot = &vis_table_c.entries[32];
        n = 15;
        while (n--) {
            slot[0] = slot[1];
            slot++;
        }
    } else {
        slot = &vis_table_c.entries[32 + vis_table_c.unk0000[2]];
        vis_table_c.unk0000[2]++;
    }
    slot->unk00 = id;
    slot->unk02 = e->unk26;
    *(u32 *)slot->unk04 = 0;
    slot->unk08 = e->unk74;
    slot->unk08.vy -= e->unk30 >> 1;
    /* One 8-byte copy of the rotation (unk8C..unk92) over unk18..unk1E. */
    *(SVECTOR *)&slot->unk18 = *(SVECTOR *)&e->unk8C;
    slot->unk20 = rec->unk1E;
    slot->unk22 = 1;
    slot->unk23 = 1;
    slot->unk24 = 0;
    slot->unk28 = e->unk40;
    slot->unk26 = 0;
    slot->unk25 = 0;
    slot->unk2c = area_state.area;
    vis_table_c.unk0000[0]++;
    vis_table_c.unk0048[id / 32] |= 1 << (id % 32);
}

/* Drops one of the entry's three items, each by its percent chance when
 * func_80052d40 allows it. */
ADDRESS(0x8004da2c, 0xe0)
void func_8004da2c(Vis212 *e)
{
    int r = rand() * 100 >> 15;

    if (func_80052d40(e->unk10) && r < e->unk65)
        func_8004d804(e, e->unk10);
    else if (func_80052d40(e->unk12) && r < e->unk66)
        func_8004d804(e, e->unk12);
    else if (func_80052d40(e->unk14) && r < e->unk67)
        func_8004d804(e, e->unk14);
}

/* Adds the entry's record bonuses to the viewer's 14 figures and total. */
ADDRESS(0x8004db0c, 0x90)
void func_8004db0c(Vis212 *e)
{
    Vis192 *r = &vis_table_a.unk3500[e->unk02];
    int i;

    for (i = 0; i < 14; i++) {
        viewer.unk9A[i] += r->unk24[i];
        viewer.unk8 += r->unk24[i];
    }
    func_8005a3a8(0);
}

/* Whether the viewer looks at the entry within reach: 0 no, 1 yes, 2 dead
 * centre and close. */
ADDRESS(0x8004db9c, 0x288)
int func_8004db9c(Vis212 *e, short reach)
{
    short yaw;
    short pitch;
    short dy;
    short dy2;
    short lo;
    short hi;
    short half;
    int ay;

    yaw = (viewer.unk26A - e->unk56) & 0xfff;
    pitch = viewer.unk268 & 0xfff;
    if (reach + e->unk2E < e->unk40)
        return 0;
    dy = e->unk74.vy - viewer.y;
    dy2 = e->unk74.vy - e->unk30 - viewer.y;
    if (dy > 0 && dy2 > 0) {
        lo = ratan2(dy, e->unk40 - e->unk2E);
        hi = ratan2(dy2, e->unk40 + e->unk2E);
    } else if (dy < 0 && dy2 < 0) {
        lo = ratan2(dy, e->unk40 + e->unk2E);
        hi = ratan2(dy2, e->unk40 - e->unk2E);
    } else {
        lo = ratan2(dy, e->unk40);
        hi = ratan2(dy2, e->unk40);
    }
    half = ratan2(e->unk2E, e->unk40);
    if (yaw > 2048)
        yaw -= 4096;
    if (pitch > 2048)
        pitch -= 4096;
    if (lo > 2048)
        lo = 4096 - lo;
    if (hi > 2048)
        hi = 4096 - hi;
    if (pitch < hi || lo < pitch)
        return 0;
    ay = abs(yaw);
    if (ay < half && e->unk40 < e->unk2E + reach) {
        if (pitch < hi >> 1 || lo >> 1 < pitch)
            return 1;
        if (ay < half >> 1 && e->unk40 < (e->unk2E >> 1) + reach)
            return 2;
        return 1;
    }
    return 0;
}

/* Sums the record's weights over the 9 levels (arg 0 only). */
/*
 * Not exact: retail counts in a3 and walks the weights in t0; this source
 * swaps the two registers.
 */
ADDRESS(0x8004de24, 0xd4)
int func_8004de24(Vis212 *e, int arg, u16 *levels)
{
    Vis192 *r = &vis_table_a.unk3500[e->unk02];
    u16 *w;
    int l;
    int m;
    int sum;
    int i;

    sum = 0;
    if (arg == 0) {
        w = r->unk4A;
        for (i = 0; i < 9; i++, levels++, w++) {
            l = *levels;
            if (l != 0) {
                m = *w ? *w : 1;
                sum += l * 50000 / (m * (1000 - l));
            }
        }
    }
    return sum;
}
