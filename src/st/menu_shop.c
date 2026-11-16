/*
 * ST.EXE: the item list screens (WIP module): a cursor bar, three list
 * layouts chosen by the page bits (with carried counts, with prices, plain),
 * the selected item's detail panel and the screen frame that picks them.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/data.h>
#include <st/funcs.h>
#include <st/gfx.h>
#include <st/menu.h>
#include <st/vis.h>

/* The list rows shown at once. */
#define LIST_ROWS 5

#define LIST_TEXT(sp, id) \
    ((u16 *)((sp)->unk0.list.messages->text + (sp)->unk0.list.messages->offsets[id]))

ADDRESS(0x8002d574, 0x98)
void func_8002d574(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    int x, w;

    if (sp->unk2DC & 0x6) {
        x = 54;
        sp->unk330.w = 218;
    } else {
        x = 28;
        sp->unk330.w = 272;
    }
    sp->unk330.x = x - 4;
    sp->unk330.h = 34;
    sp->unk330.y = sp->unk2E0 * 36 + 44;
    func_80027d18(&sp->unk330, &sp->unk32C, 257, 1);
    func_800286a8(1, 0, 0);
}

ADDRESS(0x8002d60c, 0x2dc)
void func_8002d60c(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    int rows;
    int i;
    int id;
    CVECTOR *text, *figure;

    rows = LIST_ROWS;
    if (sp->unk0.list.count < LIST_ROWS + 1)
        rows = sp->unk0.list.count;
    func_80027440(DAT_801c7590.counts[266].count | 0x3000000, 244, 27, &sp->style[6]);
    func_8002732c(MESSAGE(100), 160, 27, &sp->style[2]);
    for (i = 0; i < rows; i++) {
        id = DAT_801c7590.unk13A[sp->unk0.list.ids[sp->unk2E2 + i]];
        if (DAT_801c7590.counts[266].count >= vis_table_c.unk0118[id].unk2) {
            text = &sp->style[0];
            figure = &sp->style[4];
        } else {
            text = &sp->style[3];
            figure = &sp->style[7];
        }
        sp->style[0].cd |= 0x80;
        sp->style[3].cd |= 0x80;
        func_8002732c(LIST_TEXT(sp, id), 28, i * 36 + 48, text);
        sp->style[0].cd &= 0x7f;
        sp->style[3].cd &= 0x7f;
        if (vis_table_c.unk0118[id].unk27) {
            func_8002732c(MESSAGE(109), 72, i * 36 + 62, text);
            func_80027440(vis_table_c.unk0118[id].unk28 | 0x3000000, 128, i * 36 + 62, figure);
            func_80026ecc(74, 160, i * 36 + 62, text);
            func_80027440(vis_table_c.unk0118[id].unk27 | 0x3000000, 172, i * 36 + 62, figure);
        }
        func_80027440(vis_table_c.unk0118[id].unk2 | 0x3000000, 244, i * 36 + 62, figure);
        func_8002732c(MESSAGE(102), 216, i * 36 + 62, text);
    }
}

ADDRESS(0x8002d8e8, 0x1b8)
void func_8002d8e8(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    int rows;
    int i;
    int id;

    rows = LIST_ROWS;
    if (sp->unk0.list.count < LIST_ROWS + 1)
        rows = sp->unk0.list.count;
    for (i = 0; i < rows; i++) {
        id = sp->unk0.list.ids[sp->unk2E2 + i];
        sp->style[0].cd |= 0x80;
        func_8002732c(LIST_TEXT(sp, id), 54, i * 36 + 48, &sp->style[0]);
        sp->style[0].cd &= 0x7f;
        if (vis_table_c.unk0118[id].unk27) {
            func_8002732c(MESSAGE(109), 131, i * 36 + 62, &sp->style[0]);
            func_80027440(vis_table_c.unk0118[id].unk28 | 0x3000000, 187, i * 36 + 62,
                          &sp->style[4]);
            func_80026ecc(74, 219, i * 36 + 62, &sp->style[0]);
            func_80027440(vis_table_c.unk0118[id].unk27 | 0x3000000, 231, i * 36 + 62,
                          &sp->style[4]);
        }
    }
}

ADDRESS(0x8002daa0, 0x2d0)
void func_8002daa0(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    int rows;
    int i;
    int id;
    int price;
    CVECTOR *text, *figure;

    func_80027440(viewer.unk2 | 0x4000000, 244, 27, &sp->style[6]);
    func_8002732c(MESSAGE(101), 176, 27, &sp->style[2]);
    rows = LIST_ROWS;
    if (sp->unk0.list.count < LIST_ROWS + 1)
        rows = sp->unk0.list.count;
    for (i = 0; i < rows; i++) {
        id = sp->unk0.list.ids[sp->unk2E2 + i];
        price = vis_table_c.unk0118[id].unk3 * 2 / 3 *
                (vis_table_c.unk0118[id].unk27 - vis_table_c.unk0118[id].unk28);
        if (vis_table_c.unk0118[id].unk28 == 0)
            price *= 2;
        if (price < viewer.unk2) {
            text = &sp->style[0];
            figure = &sp->style[4];
        } else {
            text = &sp->style[3];
            figure = &sp->style[7];
        }
        sp->style[0].cd |= 0x80;
        sp->style[3].cd |= 0x80;
        func_8002732c(LIST_TEXT(sp, id), 28, i * 36 + 48, text);
        sp->style[0].cd &= 0x7f;
        sp->style[3].cd &= 0x7f;
        if (vis_table_c.unk0118[id].unk27) {
            func_8002732c(MESSAGE(109), 77, i * 36 + 62, text);
            func_80027440(vis_table_c.unk0118[id].unk28 | 0x3000000, 133, i * 36 + 62, figure);
            func_80026ecc(74, 165, i * 36 + 62, text);
            func_80027440(vis_table_c.unk0118[id].unk27 | 0x3000000, 177, i * 36 + 62, figure);
        }
        func_80027440((u16)price | 0x4000000, 244, i * 36 + 62, figure);
    }
}

/*
 * Not exact: retail keeps the scratchpad base in s4 from the first
 * load; this build rematerializes lui 0x1f80 for the first load and the
 * first three argument addresses.
 */
ADDRESS(0x8002dd70, 0x5a0)
void func_8002dd70(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    short id;
    int price;
    int i;
    int y;
    int x;

    x = 105;
    if (sp->unk0.list.unkA == 0)
        x = 167;
    sp->unk33C.x = x;
    sp->unk33C.y = 206;
    sp->unk33C.w = 50;
    sp->unk33C.h = 16;
    func_80027d18(&sp->unk33C, &sp->unk32C, 257, 1);
    func_800286a8(1, 0, 0);
    if (sp->unk2DC & 0x1) {
        id = DAT_801c7590.unk13A[sp->unk0.list.ids[sp->unk2E2 + sp->unk2E0]];
        func_8002732c(MESSAGE(105), 120, 208, &sp->style[0]);
    } else if (sp->unk2DC & 0x8) {
        id = sp->unk0.list.ids[sp->unk2E2 + sp->unk2E0];
        func_8002732c(MESSAGE(106), 110, 208, &sp->style[0]);
    } else {
        id = sp->unk0.list.ids[sp->unk2E2 + sp->unk2E0];
        func_8002732c(MESSAGE(107), 113, 208, &sp->style[0]);
    }
    func_8002732c(MESSAGE(108), 172, 208, &sp->style[0]);
    sp->style[0].cd |= 0x80;
    func_8002732c(LIST_TEXT(sp, id), 24, 176, &sp->style[0]);
    sp->style[0].cd &= 0x7f;
    if (vis_table_c.unk0118[id].unk27) {
        func_8002732c(MESSAGE(109), 48, 190, &sp->style[0]);
        func_80027440(vis_table_c.unk0118[id].unk28 | 0x3000000, 104, 190, &sp->style[4]);
        func_80026ecc(74, 136, 190, &sp->style[0]);
        func_80027440(vis_table_c.unk0118[id].unk27 | 0x3000000, 148, 190, &sp->style[4]);
    }
    if (sp->unk2DC & 0x1) {
        func_80027440(vis_table_c.unk0118[id].unk2 | 0x3000000, 244, 190, &sp->style[4]);
        func_8002732c(MESSAGE(102), 216, 190, &sp->style[0]);
    } else if (sp->unk2DC & 0x8) {
        price = vis_table_c.unk0118[id].unk3 * 2 / 3 *
                (vis_table_c.unk0118[id].unk27 - vis_table_c.unk0118[id].unk28);
        if (vis_table_c.unk0118[id].unk28 == 0)
            price *= 2;
        func_80027440((u16)price | 0x4000000, 244, 190, &sp->style[4]);
    }
    func_8002732c(MESSAGE(103), 165, 46, &sp->style[0]);
    y = 46;
    func_8002732c(MESSAGE(103), 165, y, &sp->style[0]);
    for (i = 0; i < 7; i++) {
        func_800276bc(vis_table_c.unk0118[id].unk10[i] | 0x13000000, 207, y, &sp->style[4]);
        func_800276bc(vis_table_c.unk0118[id].unk10[i + 7] | 0x13000000, 273, y, &sp->style[4]);
        y += 14;
    }
    if (vis_table_c.unk0118[id].unk24) {
        DAT_8008819e = vis_table_c.unk0118[id].unk24 * 100 / 454;
        DAT_80088188 = vis_table_c.unk0118[id].unk24 * 1000 / 454;
        func_8002732c(MESSAGE(104), 220, 156, &sp->style[0]);
        func_800276bc(DAT_8008819e | 0x3000000, 248, 156, &sp->style[4]);
        func_80026ecc(78, 269, 156, &sp->style[0]);
        func_800276bc((u16)(DAT_80088188 - DAT_8008819e * 10) | 0x1000000, 275, 156,
                      &sp->style[4]);
        func_80026ecc(108, 281, 156, &sp->style[0]);
    }
    sp->unk350.x = 20;
    sp->unk350.y = 40;
    sp->unk350.w = 128;
    sp->unk350.h = 128;
    sp->unk358.vx = 448;
    sp->unk358.vy = 256;
    func_80028364(&sp->unk350, &sp->unk358, 0, 0, 16);
}

/*
 * Not exact: retail loads the constant 48 before reading +0x2e4;
 * this build hoists the +0x2e4 load, swapping v0 and v1 in the first
 * colour stores.
 */
ADDRESS(0x8002e310, 0x180)
void func_8002e310(void)
{
    MenuScratch *sp = MENU_SCRATCH;

    func_800174c4();
    sp->style[2].b = sp->style[6].b = sp->style[3].g = sp->style[7].g =
        sp->style[3].b = sp->style[7].b = 48;
    sp->style[0].b = sp->style[4].b = sp->style[0].g = sp->style[4].g =
        sp->style[0].r = sp->style[4].r = sp->style[3].r = sp->style[7].r = 96;
    sp->style[2].r = sp->style[6].r = 98;
    sp->style[2].g = sp->style[6].g = 66;
    sp->style[0].cd = sp->style[2].cd = sp->style[3].cd = 57;
    sp->style[4].cd = sp->style[6].cd = sp->style[7].cd = 41;
    if (sp->unk2E4 < 16)
        sp->unk360 = 8 - sp->unk2E4;
    else
        sp->unk360 = sp->unk2E4 - 24;
    sp->unk32C.b = 0;
    sp->unk32C.r = (u8)sp->unk360 * 4 + 96;
    sp->unk32C.g = (u8)sp->unk360 * 2 + 48;
    if (!(sp->unk2DC & 0x8000))
        func_8002d574();
    if (sp->unk2DC & 0x8000)
        func_8002dd70();
    else if (sp->unk2DC & 0x1)
        func_8002d60c();
    else if (sp->unk2DC & 0x8)
        func_8002daa0();
    else
        func_8002d8e8();
    func_8002d1dc();
    func_800175a4();
    func_8002948c();
    func_80017634();
}
