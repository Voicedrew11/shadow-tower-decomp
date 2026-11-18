/*
 * ST.EXE: using an item (table C ids 266..313): status cures and boosts,
 * keys tried on the table B entries in reach, repairs, and the area's own
 * item handler (WIP module).
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/data.h>
#include <st/funcs.h>
#include <st/sound.h>
#include <st/card.h>
#include <st/menu.h>
#include <st/player.h>
#include <st/item.h>
#include <st/vis.h>

RODATA(0x8001274c, 0x48)
JUMPTABLES(0x80012794, 0xc0)

/* The use result: whether the item was used (1) or not (2). The word is
 * DAT_801cba08.unk16D8, which the relocation census names DAT_801cd0e0.
 * Retail's two loops reach it from the DAT_801cba08 base (base + 0x16d8),
 * and the 0x10e arm keeps the callback load after the store, which the
 * scheduler only does for a store to an aggregate member; those places use
 * the member spelling, the rest the census name. */
#define USE_RESULT DAT_801cd0e0
#define USE_RESULT_MEMBER DAT_801cba08.unk16D8

/* Uses item `id` and leaves the use result.
 * Not exact: every instruction matches. Only the relocation of the 0x10e
 * store differs (DAT_801cba08 + 0x16d8 against the census's DAT_801cd0e0).
 * Renaming relocs.tsv row 0x80054b84 to DAT_801cba08 makes it exact but
 * raises access-crosses-owner, because that identity's extent stops at
 * +0x14a8.
 */
ADDRESS(0x80054a54, 0x848)
void func_80054a54(int id)
{
    VECTOR pos;
    int i;
    int points;

    pos = viewer.position;
    USE_RESULT = 2;
    pos.vy += 1600;
    switch (id) {
    case 0x10a:
    case 0x10b:
        break;
    case 0x10c:
        if (!(viewer.unk2C & 0x30000)) {
            USE_RESULT = 1;
            viewer.unk190[5][1] = 3000;
            viewer.unk190[5][0] = 0;
            viewer.unk2C |= 0x10000;
        }
        break;
    case 0x10d:
        if (!(viewer.unk2C & 0x30000)) {
            USE_RESULT = 1;
            viewer.unk190[5][1] = 18000;
            viewer.unk190[5][0] = 0;
            viewer.unk2C |= 0x20000;
        }
        break;
    case 0x10e:
        if (area_state.area != 30 && viewer.unk22C != 0xff) {
            viewer.unk2B = 1;
            viewer.unk2F0 = 0;
            viewer.unk2EC = 0;
            USE_RESULT_MEMBER = 1;
            area_state.unkC->unk30(id);
        }
        break;
    case 0x110:
    case 0x111:
    case 0x112:
    case 0x113:
    case 0x114:
    case 0x115:
    case 0x116:
    case 0x117:
    case 0x118:
        i = 0;
        vis_lists.unk4 = pos;
        vis_lists.unk1C = 1056;
        vis_lists.unk1E = 1600;
        vis_lists.unk14.vx = 0;
        vis_lists.unk14.vy = 0;
        vis_lists.unk14.vz = 0;
        vis_lists.unk22 = 0;
        vis_lists.unk20 = 0;
        vis_lists.unk0 = 0;
        while ((i = func_8004f718(i, &vis_lists)) != -1) {
            switch (func_8004fb88(&vis_table_b.entries[i], id)) {
            case 0:
                USE_RESULT_MEMBER = 2;
                goto done;
            case 1:
                if (id == 0x114)
                    area_state.unkC->unk30(i | 0x4000);
                else
                    func_80057d58();
                USE_RESULT_MEMBER = 1;
                if (id == 0x114 || id == 0x116)
                    goto used;
                goto done;
            case 3:
                func_80057cec();
                USE_RESULT_MEMBER = 1;
                goto done;
            }
            i++;
        }
        break;
    case 0x11c:
        if (viewer.unk2 < viewer.unk0) {
            viewer.unk2 = viewer.unk0;
            USE_RESULT = 1;
            func_800582f0(8, 128);
        }
        break;
    case 0x11d:
        if (viewer.unk6 < viewer.unk4) {
            viewer.unk6 = viewer.unk4;
            USE_RESULT = 1;
            func_800582f0(8, 128);
        }
        break;
    case 0x11e:
        if (viewer.unk2C & 7) {
            viewer.unk190[0][1] = 0;
            viewer.unk190[0][0] = 0;
            viewer.unk2C &= ~7;
            USE_RESULT = 1;
            func_800582f0(8, 128);
        }
        break;
    case 0x11f:
        if (viewer.unk2C & 0x18) {
            viewer.unk190[1][1] = 0;
            viewer.unk190[1][0] = 0;
            viewer.unk2C &= ~0x18;
            USE_RESULT = 1;
            func_800582f0(8, 128);
        }
        break;
    case 0x120:
        if (viewer.unk2C & 0x100) {
            viewer.unk190[4][1] = 0;
            viewer.unk190[4][0] = 0;
            viewer.unk2C &= ~0x100;
            USE_RESULT = 1;
            func_800582f0(8, 128);
        }
        break;
    case 0x122:
        if (!(viewer.unk2C & 0x40000)) {
            viewer.unk190[6][1] = 300;
            viewer.unk190[6][0] = 0;
            viewer.unk2C |= 0x40000;
            USE_RESULT = 1;
        }
        break;
    case 0x123:
        if (!(viewer.unk2C & 0x80000)) {
            viewer.unk190[7][1] = 3000;
            viewer.unk190[7][0] = 0;
            viewer.unk2C |= 0x80000;
            USE_RESULT = 1;
        }
        break;
    case 0x124:
        if (!(viewer.unk2C & 0x100000)) {
            viewer.unk190[8][1] = 600;
            viewer.unk190[8][0] = 0;
            viewer.unk2C |= 0x100000;
            USE_RESULT = 1;
        }
        break;
    case 0x125:
        if (!(viewer.unk2C & 0x200000)) {
            viewer.unk190[9][1] = 300;
            viewer.unk190[9][0] = 0;
            viewer.unk2C |= 0x200000;
            USE_RESULT = 1;
        }
        break;
    case 0x126:
        if (!(viewer.unk2C & 0x400000)) {
            viewer.unk190[10][1] = 6000;
            viewer.unk190[10][0] = 0;
            viewer.unk2C |= 0x400000;
            USE_RESULT = 1;
        }
        break;
    case 0x127:
        if (!(viewer.unk2C & 0x800000)) {
            viewer.unk190[11][1] = 3000;
            viewer.unk190[11][0] = 0;
            viewer.unk2C |= 0x800000;
            USE_RESULT = 1;
        }
        break;
    case 0x128:
        if (!(viewer.unk2C & 0x800)) {
            viewer.unk190[12][1] = 6000;
            viewer.unk190[12][0] = 0;
            viewer.unk2C |= 0x800;
            USE_RESULT = 1;
        }
        break;
    case 0x12e: {
        u16 *ids[12] = {
            &viewer.unk38[0], &viewer.unk38[2], &viewer.unk38[3], &viewer.unk38[4],
            &viewer.unk38[8], &viewer.unk38[9], &viewer.unk38[10], &viewer.unk38[13],
            &viewer.unk38[15], &viewer.unk38[16], &viewer.unk38[17], NULL,
        };
        u16 **p;
        Vis44 *c;
        int n;
        int base;

        /* 10 to 30 percent of each item's maximum, by how full both bars are */
        base = viewer.unk6 * 10 / viewer.unk4 + 10;
        points = viewer.unk2 * 10 / viewer.unk0 + base;
        for (p = ids; *p != NULL; p++) {
            if (**p != 0xffff) {
                c = &vis_table_c.unk0118[**p];
                n = c->unk27 * points / 100;
                if (n == 0)
                    n = 1;
                if (c->unk28 + n > c->unk27)
                    c->unk28 = c->unk27;
                else
                    c->unk28 += n;
                USE_RESULT_MEMBER = 1;
            }
        }
        func_8003d030();
        break;
    }
    case 0x129:
    case 0x12a:
    case 0x12b:
    case 0x12f:
    case 0x130:
        area_state.unkC->unk30(id);
        break;
    case 0x131:
        area_state.unkC->unk30(id);
        goto done;
    case 0x136:
        func_80057cc8();
        USE_RESULT = func_8005e924(5);
        break;
    case 0x137:
        func_80057cc8();
        USE_RESULT = func_8005e924(53);
        break;
    case 0x138:
        func_80057cc8();
        USE_RESULT = func_8005e924(29);
        break;
    case 0x139:
        func_80057cc8();
        USE_RESULT = func_8005e924(14);
        break;
    }
used:
    if (USE_RESULT == 1)
        func_800552bc(id);
done:
    func_8003ead0();
    {
        /* the quick-use slots drop an item used up */
        u16 *slots[5] = {
            &viewer.unk38[5], &viewer.unk38[19], &viewer.unk38[11], &viewer.unk38[20],
            &viewer.unk38[18],
        };
        int slot;

        for (slot = 0; slot < 5; slot++) {
            if (*slots[slot] == id && !DAT_801c7590.counts[*slots[slot]].count)
                *slots[slot] = 0xffff;
        }
    }
}
