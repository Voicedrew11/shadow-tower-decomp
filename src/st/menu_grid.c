/*
 * ST.EXE: the id grid's detail panels (WIP module): the name and figures
 * of the id under the grid cursor, and its table C counts.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/funcs.h>
#include <st/menu.h>
#include <st/vis.h>

ADDRESS(0x80031378, 0x2fc)
void func_80031378(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    int id;
    short column;

    column = sp->unk2E0 % 8;
    if (column < 4)
        id = sp->unk0.grid.ids0[(sp->unk0.grid.row0 + sp->unk2E0 / 8) * 4 + column];
    else
        id = sp->unk0.grid.ids1[(sp->unk0.grid.row1 + sp->unk2E0 / 8) * 4 + column - 4];
    sp->style[0].cd |= 0x80;
    if (id != -1)
        func_8002732c((u16 *)(sp->unk0.grid.messages->text + sp->unk0.grid.messages->offsets[id]),
                      18, 202, &sp->style[0]);
    sp->style[0].cd &= 0x7f;
    if (id != -1) {
        if (vis_table_c.unk0118[id].unk27) {
            func_8002732c(MESSAGE(29), 74, 215, &sp->style[0]);
            func_80027440(vis_table_c.unk0118[id].unk28 | 0x3000000, 138, 215, &sp->style[4]);
            func_80026ecc(14, 174, 215, &sp->style[0]);
            func_80027440(vis_table_c.unk0118[id].unk27 | 0x3000000, 190, 215, &sp->style[4]);
        }
        if (vis_table_c.unk0118[id].unk24) {
            func_8002732c(MESSAGE(28), 250, 215, &sp->style[0]);
            DAT_8008819e = vis_table_c.unk0118[id].unk24 * 100 / 454;
            DAT_80088188 = vis_table_c.unk0118[id].unk24 * 1000 / 454;
            func_80027440(DAT_8008819e | 0x3000000, 300, 215, &sp->style[4]);
            func_80026ecc(18, 339, 215, &sp->style[0]);
            func_80027440((u16)(DAT_80088188 - DAT_8008819e * 10) | 0x1000000, 346, 215,
                          &sp->style[4]);
            func_80026ecc(53, 362, 215, &sp->style[0]);
        }
    }
    func_80029548(10, 198, 620, 34);
}

/*
 * Not exact: the same calls, branches and loop shape, but retail spills the
 * row counter, the column x and the scratchpad pointer to a 104-byte frame
 * (this build: 96) and allocates the saved registers in another order;
 * first divergence is the frame size.
 */
ADDRESS(0x80031674, 0x334)
void func_80031674(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    int text_y;
    int i;
    short x, y;
    int name_x;
    int count_x;
    short column, index;
    short id;
    int none;
    u8 *count;
    int n, k, j;

    if ((short)(sp->unk2E0 % 8) < 4) {
        x = 334;
        y = 9;
        text_y = 135;
        count_x = 342;
    } else {
        x = 10;
        y = 9;
        text_y = 135;
        count_x = 18;
    }
    sp->unk350.x = x;
    sp->unk350.w = 192;
    sp->unk350.h = 124;
    sp->unk350.y = y;
    none = 0;
    column = sp->unk2E0 % 8;
    if (column < 4) {
        index = (sp->unk0.grid.row0 + sp->unk2E0 / 8) * 4 + column;
        id = sp->unk0.grid.ids0[index];
    } else {
        index = (sp->unk0.grid.row1 + sp->unk2E0 / 8 - 1) * 4 + column;
        id = sp->unk0.grid.ids1[index];
    }
    if (id == -1)
        none = 1;
    if (!none) {
        sp->unk358.vx = 640;
        sp->unk358.vy = 0;
        func_80028364(&sp->unk350, &sp->unk358, 0, 0, 16);
        n = 0;
        k = 2;
        name_x = count_x;
        sp->unk350.w = 100;
        sp->unk358.vx = 832;
        sp->unk350.x += 192;
        func_80028364(&sp->unk350, &sp->unk358, 0, 0, 16);
        count = vis_table_c.unk0118[id].unk10;
        for (i = 0; i < 4; i++) {
            for (j = 0; j < 4; j++) {
                if (n == 14)
                    goto done;
                n++;
                if (*count) {
                    func_8002732c(MESSAGE(200 + k), name_x, text_y + j * 15, &sp->style[0]);
                    func_80027440(*count | 0x13000000, name_x + 42, text_y + j * 15, &sp->style[4]);
                }
                count++;
                k++;
            }
            if (n == 14)
                break;
            name_x += 70;
        }
    }
done:
    func_80029548(x, y, 296, 188);
}
