/*
 * ST.EXE: new-game defaults and the per-area snapshots of the entry tables
 * in DAT_801cba08 (WIP module, 0x800559bc..0x800562d4).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBETC.H>
#include <psyq/libc.h>
#include <st/memory.h>
#include <st/data.h>
#include <st/gfx.h>
#include <st/light.h>
#include <st/vis.h>
#include <st/menu.h>
#include <st/save.h>

#define BIT(i) (1 << ((i) % 32))

/*
 * New-game defaults.
 *
 * The three flags whose masks take two instructions (bits 17, 16 and 15
 * of unk1444) are each cleared twice. The repeat leaves no instruction
 * (cse cannot fold it because the merged mask is not a valid operand,
 * combine can), but flow has already counted the mask's second use, and
 * that is what ranks those three masks ahead of 0x7fffffff in local-alloc
 * (retail: 0x7fffffff in t4, the others in t3, t2, t1).
 */
ADDRESS(0x800559bc, 0x338)
void save_reset_new_game(void)
{
    u8 unused[128];

    fill_words((u32 *)DAT_801cba08.unk1448, 0, 12);
    fill_words((u32 *)&DAT_801c7590, 0, 89);
    fill_words(&DAT_801cba08.unk14A8[10], 0, 79);
    DAT_801cba08.unk14A8[89] = -1;
    DAT_801cba08.unk14A8[90] = -1;
    DAT_801cba08.unk14A8[91] = -1;
    lighting.area_mode[0] = 0x11;
    lighting.area_mode[40] = 0x11;
    lighting.area_mode[41] = 0x11;
    lighting.area_mode[4] = 0x11;
    lighting.area_mode[5] = 0x41;
    lighting.area_mode[1] = 0x41;
    lighting.area_mode[6] = 0x41;
    lighting.area_mode[11] = 0x41;
    lighting.area_mode[27] = 0x41;
    lighting.area_mode[12] = 0x41;
    lighting.area_mode[28] = 0x41;
    lighting.area_mode[15] = 0x41;
    lighting.area_mode[33] = 0x41;
    lighting.area_mode[3] = 0x22;
    lighting.area_mode[20] = 0x22;
    lighting.area_mode[24] = 0x22;
    lighting.area_mode[18] = 0x22;
    lighting.area_mode[10] = 0x41;
    lighting.area_mode[21] = 0x41;
    lighting.area_mode[32] = 0x41;
    lighting.area_mode[37] = 0x11;
    lighting.area_mode[9] = 0x22;
    lighting.area_mode[8] = 0x41;
    lighting.area_mode[13] = 0x41;
    lighting.area_mode[38] = 0x41;
    lighting.area_mode[2] = 0x41;
    lighting.area_mode[17] = 0x41;
    lighting.area_mode[25] = 0x41;
    lighting.area_mode[26] = 0x41;
    lighting.area_mode[14] = 0x41;
    lighting.area_mode[23] = 0x41;
    lighting.area_mode[30] = 0x11;
    lighting.area_mode[44] = 0x11;
    lighting.area_mode[45] = 0x11;
    lighting.area_mode[46] = 0x11;
    lighting.area_mode[47] = 0x11;
    DAT_801cba08.unk16DC = 0;
    DAT_801c7590.unk13A[0] = 268;
    DAT_801c7590.unk156[0] = 3;
    DAT_801c7590.unk13A[1] = 270;
    DAT_801c7590.unk156[1] = 5;
    DAT_801c7590.unk13A[2] = 313;
    DAT_801c7590.unk156[2] = 1;
    DAT_801c7590.unk13A[3] = 312;
    DAT_801c7590.unk156[3] = 1;
    DAT_801c7590.unk13A[4] = 275;
    DAT_801c7590.unk156[4] = 1;
    DAT_801c7590.unk13A[5] = 284;
    DAT_801c7590.unk156[5] = 30;
    DAT_801c7590.unk13A[6] = 285;
    DAT_801c7590.unk156[6] = 20;
    DAT_801c7590.unk13A[7] = 286;
    DAT_801c7590.unk156[7] = 10;
    DAT_801c7590.unk13A[8] = 302;
    DAT_801c7590.unk156[8] = 2;
    DAT_801c7590.unk13A[9] = 18;
    DAT_801c7590.unk156[9] = 1;
    DAT_801cba08.unk1444.word &= ~0x60;
    DAT_801cba08.unk1444.word &= ~0x18;
    DAT_801cba08.unk1444.word &= ~0x6;
    DAT_801cba08.unk1440.word &= ~0x80000000;
    DAT_801cba08.unk1444.word &= ~0x1;
    DAT_801cba08.unk1440.word &= 0x80000000;
    DAT_801cba08.unk1444.word &= ~0x20000;
    DAT_801cba08.unk1444.word &= ~0x20000;
    DAT_801cba08.unk1444.word &= ~0x10000;
    DAT_801cba08.unk1444.word &= ~0x10000;
    DAT_801cba08.unk1444.word &= ~0x8000;
    DAT_801cba08.unk1444.word &= ~0x8000;
    DAT_801cba08.unk1444.word &= ~0x4000;
    DAT_801cba08.unk1444.word &= ~0x2000;
    DAT_801cba08.unk1444.word &= ~0x1000;
    DAT_801cba08.unk1444.word &= ~0x800;
    DAT_801cba08.unk1444.word &= ~0x400;
    DAT_801cba08.unk1444.word &= ~0x200;
    DAT_801cba08.unk1444.word &= ~0x100;
    DAT_801cba08.unk1444.word &= ~0x80;
    DAT_801c7590.unk13A[10] = 28;
    DAT_801c7590.unk13A[11] = 88;
    DAT_801c7590.unk13A[12] = 243;
    DAT_801c7590.unk156[10] = 1;
    DAT_801c7590.unk156[11] = 1;
    DAT_801c7590.unk156[12] = 1;
    DAT_801c7590.unk13A[13] = 133;
    DAT_801c7590.unk156[13] = 1;
    fill_words(DAT_801cba08.unk180[0], 0, 48);
    fill_words(DAT_801cba08.unk0[0], 0, 96);
    fill_words(DAT_801cba08.unk240[0], 0, 384);
    fill_words(DAT_801cba08.unk840[0], 0, 384);
    fill_words(DAT_801cba08.unkE40[0], 0, 384);
    fill_words((u32 *)DAT_801cba08.unk1478, 0, 12);
    fill_words(DAT_801cba08.unk14A8, 0, 5);
    fill_words(&DAT_801cba08.unk14A8[5], 0, 5);
    fill_words((u32 *)DAT_801cba08.unk16E4, 0, 12);
    DAT_801cba08.unk16E0 = -1;
}

/* Records area `area`'s entry states: live table A entries in state 3 or 5,
 * empty table C slots 32..63, and per table B entry: free, unk4F 0xff,
 * unk0C 0xff. */
ADDRESS(0x80055cf4, 0x2a8)
void area_save_snapshot(int area)
{
    Vis212 *a = vis_table_a.entries;
    Vis48 *c = vis_table_c.entries;
    Vis88 *b = vis_table_b.entries;
    int i;
    int n;

    n = 2;
    for (i = 0; i < n; i++)
        DAT_801cba08.unk0[area][i] = 0;
    for (i = 0; i < 64; i++, a++) {
        if (a->unk00 != 0xff && (a->unk05 == 3 || a->unk05 == 5))
            DAT_801cba08.unk0[area][i / 32] |= BIT(i);
    }
    n = 1;
    for (i = 0; i < n; i++)
        DAT_801cba08.unk180[area][i] = 0;
    for (i = 0; i < 32; i++, c++) {
        if (c->unk22 == 0)
            DAT_801cba08.unk180[area][i / 32] |= BIT(i);
    }
    n = 8;
    for (i = 0; i < n; i++) {
        DAT_801cba08.unk240[area][i] = 0;
        DAT_801cba08.unk840[area][i] = 0;
        DAT_801cba08.unkE40[area][i] = 0;
    }
    for (i = 0; i < 240; i++, b++) {
        if (b->unk0a == 0xffff) {
            DAT_801cba08.unk240[area][i / 32] |= BIT(i);
        } else {
            if (b->unk4A.door.unk4F == 0xff)
                DAT_801cba08.unk840[area][i / 32] |= BIT(i);
            if (b->unk0C == 0xff)
                DAT_801cba08.unkE40[area][i / 32] |= BIT(i);
        }
    }
}

/* Restores what area_save_snapshot recorded for area `area`. */
ADDRESS(0x80055f9c, 0x338)
void area_restore_snapshot(int area)
{
    Vis212 *a = vis_table_a.entries;
    Vis48 *c = vis_table_c.entries;
    Vis88 *b = vis_table_b.entries;
    int i;
    u16 n;
    u8 t;

    for (i = 0; i < 64; i++, a++) {
        if (DAT_801cba08.unk0[area][i / 32] & BIT(i))
            a->unk05 = 3;
    }
    for (i = 0; i < 32; i++, c++) {
        if (DAT_801cba08.unk180[area][i / 32] & BIT(i))
            c->unk22 = 0;
    }
    for (i = 0; i < 240; b++, i++) {
        if (DAT_801cba08.unk240[area][i / 32] & BIT(i)) {
            b->unk0a = 0xffff;
        } else {
            switch (b->unk05) {
            default:
                if (DAT_801cba08.unk840[area][i / 32] & BIT(i))
                    b->unk4A.door.unk4F = 255;
                if (DAT_801cba08.unkE40[area][i / 32] & BIT(i))
                    b->unk0C = 255;
                break;
            case 0:
            case 1:
                if (DAT_801cba08.unk840[area][i / 32] & BIT(i))
                    b->unk4A.door.unk4F = 255;
                if (DAT_801cba08.unkE40[area][i / 32] & BIT(i)) {
                    n = b->unk4A.door.unk4E;
                    while (n--) {
                        t = vis_records[b->unk4A.door.unk4C + n].part;
                        vis_records[b->unk4A.door.unk4C + n].part =
                            vis_records[b->unk4A.door.unk4A + n].part;
                        vis_records[b->unk4A.door.unk4A + n].part = t;
                    }
                    if (b->unk0a == 58)
                        b->unk00 = 2;
                    else if (b->unk05 == 1)
                        b->unk00 = 1;
                    b->unk0C = 255;
                    b->unk10 = 4095;
                }
                break;
            case 52:
                if (DAT_801cba08.unk840[area][i / 32] & BIT(i))
                    b->unk4A.door.unk4F = 255;
                if (DAT_801cba08.unkE40[area][i / 32] & BIT(i)) {
                    b->unk0C = 255;
                    b->unk10 = 0;
                    b->unk02 = 1;
                }
                break;
            }
        }
    }
    vis_lists.unk47E0 &= ~0x20;
    vis_lists.unk47E0 &= ~0x10;
    vis_lists.unk47E0 &= ~0x8;
    vis_lists.unk47E0 &= ~0x4;
    vis_lists.unk47E0 &= ~0x2;
    vis_lists.unk47E0 &= ~0x1;
}
