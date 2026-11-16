/*
 * ST.EXE: the status screens (WIP module): the two window frames, the
 * equipment slot grid and the attribute table with its bars.
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

RODATA(0x8001128c, 0xfc)
SDATA(0x80087f64, 0x4)

/* The slots of the equipment grid. */
#define EQUIP_SLOTS 21
/* Attribute rows. */
#define STATUS_ROWS 14

ADDRESS(0x8002e490, 0x40)
void func_8002e490(void)
{
    func_80029548(8, 8, 300, 224);
    func_80029548(308, 8, 324, 224);
}

/*
 * Draws the equipment slots from their VRAM images, highlights the slot
 * under the cursor and, while +0x2c6 is 1, prints each equipped item's
 * +0x28 figure with a shadow.
 *
 * Not exact: the same instructions with another saved-register
 * assignment (retail: s2 slot pointer, s3 draw mode, s4 scratchpad base).
 */
ADDRESS(0x8002e4d0, 0x47c)
void func_8002e4d0(void)
{
    MenuScratch *sp;
    RECT rects[EQUIP_SLOTS] = {
        {14, 11, 56, 28},   {14, 40, 56, 28},   {14, 69, 56, 28},   {14, 98, 92, 73},
        {14, 172, 56, 28},  {14, 201, 56, 28},  {72, 15, 48, 24},   {130, 11, 56, 28},
        {130, 40, 56, 28},  {108, 69, 100, 65}, {108, 135, 100, 65}, {130, 201, 56, 28},
        {196, 15, 48, 24},  {246, 11, 56, 28},  {246, 40, 56, 28},  {246, 69, 56, 28},
        {210, 98, 92, 73},  {246, 172, 56, 28}, {246, 201, 56, 28}, {72, 201, 56, 28},
        {188, 201, 56, 28},
    };
    DVECTOR tex[EQUIP_SLOTS] = {
        {740, 152}, {796, 152}, {852, 152}, {932, 0},   {908, 152}, {964, 152}, {740, 124},
        {740, 180}, {796, 180}, {640, 124}, {640, 189}, {852, 180}, {788, 124}, {908, 180},
        {964, 180}, {852, 152}, {932, 73},  {796, 208}, {852, 208}, {908, 208}, {964, 208},
    };
    DVECTOR clut = {0, 511};
    CVECTOR unused = {0};
    RECT unused2;
    int i, slot, mode;
    u16 *equip;

    sp = MENU_SCRATCH;
    if (sp->unk2E0 < 10)
        slot = sp->unk2E0;
    else
        slot = sp->unk2E0 - 1;
    sp->unk330.x = rects[slot].x - 2;
    sp->unk330.y = rects[slot].y - 1;
    sp->unk330.w = rects[slot].w + 4;
    sp->unk330.h = rects[slot].h + 2;
    func_80027d18(&sp->unk330, &sp->unk32C, 513, 1);
    func_800286a8(1, 0, 0);
    sp->style[7].r = 192;
    sp->style[7].g = 158;
    sp->style[7].b = 96;
    sp->style[7].cd = 48;
    equip = viewer.unk38;
    for (i = 0; i < EQUIP_SLOTS; i++) {
        if (i == 2 || (i == 3 && vis_table_c.unk0118[viewer.unk38[16]].unk26 == 9) ||
            (i == 4 && vis_table_c.unk0118[viewer.unk38[17]].unk26 == 9))
            mode = 48;
        else
            mode = 16;
        if (i != 6 && i != 12 && sp->unk2C6 == 1 && equip[i] != 0xffff &&
            vis_table_c.unk0118[equip[i]].unk27) {
            sp->style[7].r = 200;
            sp->style[7].g = 164;
            sp->style[7].b = 96;
            func_80027440(vis_table_c.unk0118[equip[i]].unk28 | 0x2000000, rects[i].x + 8,
                          rects[i].y + 4, &sp->style[7]);
            sp->style[7].g = sp->style[7].r = 8;
            sp->style[7].b = 0;
            func_80027440(vis_table_c.unk0118[equip[i]].unk28 | 0x2000000, rects[i].x + 9,
                          rects[i].y + 5, &sp->style[7]);
        }
        func_80028364(&rects[i], &tex[i], &clut, 0, mode);
    }
}

/*
 * The attribute table: names, current values (with the change a candidate
 * item would make on page 2) and bars scaled to +0x2c4.
 *
 * Not exact: retail schedules the colour stores in another order and
 * keeps the scratchpad base in s2 (this build: s3).
 */
ADDRESS(0x8002e94c, 0x44c)
void func_8002e94c(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    CVECTOR bright = {128, 128, 32, 0};
    CVECTOR dark = {0};
    RECT unused;
    RECT bar;
    short xoff[STATUS_ROWS] = {0};
    int i, y;
    u8 *count;
    s16 *figure;

    sp->style[3].r = sp->style[2].g = 120;
    sp->style[2].r = sp->style[2].b = sp->style[3].b = 40;
    sp->style[3].g = 60;
    sp->style[3].cd = sp->style[2].cd = 73;
    sp->style[7].r = sp->style[6].g = 120;
    sp->style[6].r = sp->style[6].b = sp->style[7].b = 40;
    sp->style[7].g = 60;
    sp->style[7].cd = sp->style[6].cd = 49;
    if (sp->unk2DC == 1) {
        sp->unk320 = sp->unk324 = sp->unk328 = &sp->style[0];
    } else {
        sp->unk320 = &sp->style[0];
        sp->unk324 = &sp->style[2];
        sp->unk328 = &sp->style[3];
    }
    for (i = 0, y = 16; i < STATUS_ROWS; i++, y += 15)
        func_8002732c(MESSAGE(14 + i), xoff[i] + 316, y, sp->unk320);
    if (sp->unk2DC == 1) {
        sp->unk320 = sp->unk324 = sp->unk328 = &sp->style[4];
    } else {
        sp->unk320 = &sp->style[4];
        sp->unk324 = &sp->style[6];
        sp->unk328 = &sp->style[7];
    }
    if (sp->unk2DC == 2 && sp->unk2C6) {
        if (sp->unk2BE != -1) {
            count = vis_table_c.unk0118[sp->unk2C0].unk10;
            for (i = 0, y = 16; i < STATUS_ROWS; i++, y += 15)
                func_80027440(*count++ | 0x13000000, 590, y, sp->unk320);
        }
        return;
    }
    figure = viewer.unk62;
    for (i = 0, y = 16; i < STATUS_ROWS; i++, y += 15, figure++) {
        if (sp->unk2DC == 2) {
            if (sp->unk25C[i] > 0) {
                func_80027440(sp->unk1B4[i] | 0x3000000, 590, y, sp->unk320);
                func_80026ecc(19, 544, y, sp->unk324);
                func_80027440(sp->unk25C[i] | 0x3000000, 508, y, sp->unk324);
            } else if (sp->unk25C[i] < 0) {
                func_80027440(sp->unk1B4[i] | 0x3000000, 590, y, sp->unk320);
                func_80026ecc(20, 544, y, sp->unk328);
                func_80027440(-sp->unk25C[i] | 0x3000000, 508, y, sp->unk328);
            } else {
                func_80027440(sp->unk1B4[i] | 0x3000000, 590, y, sp->unk320);
            }
        } else if (*figure < 0) {
            func_80027440(0x3000000, 590, y, sp->unk320);
        } else {
            func_80027440((u16)*figure | 0x3000000, 590, y, sp->unk320);
        }
    }
    bar.h = 4;
    for (i = 0, y = 27; i < STATUS_ROWS; i++, y += 15) {
        bar.w = sp->unk1B4[i] * 50 / sp->unk2C4;
        if (bar.w > 100)
            bar.w = 100;
        bar.y = y;
        bar.x = 626 - bar.w;
        func_80028db8(&bar, &bright, &dark, 1);
    }
    func_800286a8(1, 832, 256);
}
