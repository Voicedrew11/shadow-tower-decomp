/*
 * ST.EXE: the status figure page, the cell grid drawer and the item and
 * cell pages (WIP module).
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

SDATA(0x80087f70, 0x14)

/* Attribute rows. */
#define STATUS_ROWS 14

/* Unreferenced: eight zero bytes before this unit's constants. */
DATA(0x80087f68, 0x8, ".sdata")
static s32 DAT_80087f68[2] = {0, 0};

/*
 * The figure page: two current/maximum pairs coloured by how low the
 * current value is, the other figures and two weights in tenths.
 *
 * Not exact: another saved-register assignment from the first loop on
 * (retail: s1 y, s2 row, s3 message text).
 */
ADDRESS(0x8002ed98, 0x460)
void func_8002ed98(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    s16 *p;
    int y, i;

    sp->style[6].r = 96;
    sp->style[7].r = 96;
    sp->style[7].g = 96;
    sp->style[7].b = 32;
    sp->style[6].g = 48;
    sp->style[6].b = 48;
    sp->style[7].cd = 49;
    sp->style[6].cd = 49;
    for (i = 0, y = 16; i < STATUS_ROWS; y += 15, i++)
        func_8002732c(MESSAGE(i), 316, y, &sp->style[0]);
    p = (s16 *)&viewer;
    for (i = 0, y = 16; i < 2; y += 15, i++) {
        if (p[1] < p[0] / 2)
            sp->unk320 = &sp->style[7];
        else if (p[1] < (short)(p[0] / 5))
            sp->unk320 = &sp->style[6];
        else
            sp->unk320 = &sp->style[4];
        func_80027440((u16)*p++ | 0x5000000, 566, y, sp->unk320);
        func_80026ecc(14, 554, y, sp->unk320);
        func_80027440((u16)*p++ | 0x5000000, 494, y, sp->unk320);
    }
    p++;
    func_80027440((u16)*p++ | 0x3000000, 542, 46, &sp->style[4]);
    func_80026ecc(15, 578, 46, &sp->style[4]);
    func_80027440((u16)*p++ | 0x3000000, 590, 46, &sp->style[4]);
    for (i = 3, y = 61; i < 13; y += 15, i++)
        func_80027440((u16)*p++ | 0x3000000, 590, y, &sp->style[4]);
    if ((short)(p[1] / 10) < (short)(p[0] / 10))
        sp->unk320 = &sp->style[6];
    else
        sp->unk320 = &sp->style[4];
    DAT_8008819e = *p * 100 / 454;
    DAT_80088188 = *p++ * 1000 / 454;
    func_80027440(DAT_8008819e | 0x3000000, 494, 211, sp->unk320);
    func_80026ecc(18, 534, 211, sp->unk320);
    func_80027440((u16)(DAT_80088188 - DAT_8008819e * 10) | 0x1000000, 542, 211, sp->unk320);
    func_80026ecc(14, 554, 211, sp->unk320);
    DAT_8008819e = *p * 100 / 454;
    DAT_80088188 = *p * 1000 / 454;
    func_80027440(DAT_8008819e | 0x3000000, 566, 211, sp->unk320);
    func_80026ecc(18, 606, 211, sp->unk320);
    func_80027440((u16)(DAT_80088188 - DAT_8008819e * 10) | 0x1000000, 614, 211, sp->unk320);
}

/*
 * Draws a 25 x 5 grid of 10 x 10 texture cells from tex at pos, 11 pixels
 * apart horizontally and 12 vertically, semi-transparent in color.
 *
 * Not exact: retail keeps pos and tex in t0/t1 with caller-save
 * spills and hoists v and v + 10 out of the inner loop; this build keeps
 * pos in s8 and hoists the addPrim mask instead.
 */
ADDRESS(0x8002f1f8, 0x2a0)
void func_8002f1f8(DVECTOR *pos, DVECTOR *tex, DVECTOR *clut, CVECTOR *color, int arg4)
{
    int row;
    int u0;
    int cols, rows;
    int col, x, y;
    int u, v;
    StPacketBuffer *buf;
    POLY_FT4 *p;
    CVECTOR c;

    cols = 25;
    rows = 5;
    u0 = (short)(tex->vx % 64) * 4;
    v = (short)(tex->vy % 256);
    for (row = 0, y = 0; row < rows; row++, y += 12, v += 10) {
        for (col = 0, x = 0, u = u0; col < cols; col++, x += 11, u += 10) {
            buf = world_state.packet;
            p = (POLY_FT4 *)buf->cur;
            if (buf->end < (u_long *)(p + 1))
                return;
            buf->cur = (u_long *)(p + 1);
            world_state.unk8328++;
            setUV4(p, u, v, u + 10, v, u, v + 10, u + 10, v + 10);
            p->x0 = p->x2 = pos->vx + x;
            p->x1 = p->x3 = pos->vx + x + 10;
            p->y0 = p->y1 = pos->vy + y;
            p->y2 = p->y3 = pos->vy + y + 10;
            c = *color;
            setRGB0(p, c.r, c.g, c.b);
            p->tpage = GetTPage(0, color->cd & 3, tex->vx, tex->vy);
            p->clut = GetClut(clut->vx, clut->vy);
            setPolyFT4(p);
            setSemiTrans(p, 1);
            addPrim(world_state.ot_cur, p);
        }
    }
}

/* The item page: its arrows, panels, name, cell grid, weight and figures. */
ADDRESS(0x8002f498, 0x4e4)
void func_8002f498(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    DVECTOR pos = {20, 152};
    RECT up = {16, 72, 16, 8};
    RECT down = {300, 72, 16, 8};

    sp->style[2].r = sp->style[6].r = 96;
    sp->style[2].b = sp->style[6].b = 48;
    sp->style[2].g = sp->style[6].g = 48;
    sp->style[2].cd = 73;
    sp->style[6].cd = 49;
    func_8002916c(&down, &sp->unk32C, 0);
    func_8002916c(&up, &sp->unk32C, 2048);
    func_800286a8(1, 0, 0);
    sp->unk350.x = 12;
    sp->unk350.y = 10;
    sp->unk350.w = 192;
    sp->unk350.h = 124;
    sp->unk358.vx = 640;
    sp->unk358.vy = 0;
    func_80028364(&sp->unk350, &sp->unk358, 0, 0, 16);
    sp->unk350.x = 204;
    sp->unk350.w = 100;
    sp->unk358.vx = 832;
    func_80028364(&sp->unk350, &sp->unk358, 0, 0, 16);
    if (sp->unk2BE == -1)
        return;
    sp->style[0].cd |= 0x80;
    func_8002732c((u16 *)(sp->unk2CC->text + sp->unk2CC->offsets[sp->unk2C0]), 14, 136,
                  &sp->style[0]);
    sp->style[0].cd &= 0x7f;
    func_8002f1f8(&pos, &sp->unk0.vram[0], &sp->unk0.vram[1], &sp->style[0], 0xf0004);
    if (vis_table_c.unk0118[sp->unk2C0].unk24) {
        DAT_8008819e = vis_table_c.unk0118[sp->unk2C0].unk24 * 100 / 454;
        DAT_80088188 = vis_table_c.unk0118[sp->unk2C0].unk24 * 1000 / 454;
        func_8002732c(MESSAGE(28), 188, 217, &sp->style[0]);
        func_80027440(DAT_8008819e | 0x3000000, 224, 217, &sp->style[4]);
        func_80026ecc(18, 264, 217, &sp->style[0]);
        func_80027440((u16)(DAT_80088188 - DAT_8008819e * 10) | 0x1000000, 272, 217,
                      &sp->style[4]);
        func_80026ecc(53, 287, 217, &sp->style[0]);
    }
    if (vis_table_c.unk0118[sp->unk2C0].unk27) {
        func_8002732c(MESSAGE(29), 16, 216, &sp->style[0]);
        if (vis_table_c.unk0118[sp->unk2C0].unk28 < 4) {
            func_80027440(vis_table_c.unk0118[sp->unk2C0].unk28 | 0x3000000, 80, 217,
                          &sp->style[6]);
            func_80026ecc(14, 120, 217, &sp->style[2]);
            func_80027440(vis_table_c.unk0118[sp->unk2C0].unk27 | 0x3000000, 136, 217,
                          &sp->style[6]);
        } else {
            func_80027440(vis_table_c.unk0118[sp->unk2C0].unk28 | 0x3000000, 80, 217,
                          &sp->style[4]);
            func_80026ecc(14, 120, 217, &sp->style[0]);
            func_80027440(vis_table_c.unk0118[sp->unk2C0].unk27 | 0x3000000, 136, 217,
                          &sp->style[4]);
        }
    }
}

/* The cell page: its name and grid, the cursor and the cells not yet set. */
ADDRESS(0x8002f97c, 0x2d0)
void func_8002f97c(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    CVECTOR grey = {64, 64, 64, 0};
    DVECTOR pos = {20, 152};
    int row, col, bit;

    sp->style[0].cd |= 0x80;
    func_8002732c((u16 *)(sp->unk2CC->text + sp->unk2CC->offsets[512 + sp->unk2BE]), 14, 136,
                  &sp->style[0]);
    sp->style[0].cd &= 0x7f;
    func_8002f1f8(&pos, &sp->unk0.vram[0], &sp->unk0.vram[1], &sp->style[0], 0xf0004);
    sp->unk330.w = 48;
    sp->unk330.h = 24;
    sp->unk330.x = (short)(sp->unk2BE / 3) * 49 + 11;
    sp->unk330.y = (short)(sp->unk2BE % 3) * 25 + 12;
    func_80027d18(&sp->unk330, &grey, 513, 0);
    sp->unk330.x += 2;
    sp->unk330.y += 1;
    sp->unk330.w -= 4;
    sp->unk330.h -= 2;
    func_80028220(&sp->unk330, &grey, 1);
    func_800286a8(1, 0, 0);
    sp->unk350.w = 48;
    sp->unk350.h = 24;
    sp->unk350.y = 12;
    sp->unk358.vy = 0;
    for (row = 0; row < 2; row++) {
        CVECTOR dim = {64, 64, 64, 0};

        sp->unk350.x = 11;
        sp->unk358.vx = 640;
        for (col = 0, bit = row; col < 6; bit += 3, col++) {
            if (!(*sp->unk2D4 & (1 << bit))) {
                func_80028220(&sp->unk350, &dim, 1);
                func_800286a8(2, 0, 0);
            }
            func_80028364(&sp->unk350, &sp->unk358, 0, 0, 16);
            sp->unk350.x += 49;
            sp->unk358.vx += 48;
        }
        sp->unk350.y += 25;
        sp->unk358.vy += 24;
    }
    func_800286a8(1, 0, 0);
}
