/*
 * ST.EXE: the stored-item grid page (WIP module): five cells per row,
 * three rows shown, the selected entry's number and name, and the cell
 * images.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/funcs.h>
#include <st/menu.h>

SDATA(0x80087f98, 0x10)

/* The cells per grid row and the rows shown at once. */
#define STORE_COLUMNS 5
#define STORE_ROWS 3

/*
 * Not exact: the same instructions with the scratchpad base and the
 * constant 48 in swapped saved registers (retail: s3 and s2).
 */
ADDRESS(0x8002fe4c, 0x410)
void func_8002fe4c(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    RECT up = {320, 16, 32, 12};
    RECT down = {320, 208, 32, 12};
    int index, kind;
    int row, col, cols;

    DAT_800881aa = 0;
    sp->unk330.w = 96;
    sp->unk330.h = 48;
    sp->unk330.x = (short)(sp->unk2E0 % STORE_COLUMNS) * 112 + 48;
    sp->unk330.y = (short)(sp->unk2E0 / STORE_COLUMNS) * 56 + 32;
    func_80027d18(&sp->unk330, &sp->unk32C, 513, 1);
    func_800286a8(1, 0, 0);
    if (sp->unk2E2 > 0)
        func_8002916c(&up, &sp->unk32C, 3072);
    if (sp->unk2E2 <= sp->unk0.store.rows - 3)
        func_8002916c(&down, &sp->unk32C, 1024);
    func_800286a8(1, 0, 0);
    index = sp->unk2E2 * STORE_COLUMNS + sp->unk2E0;
    kind = sp->unk0.store.kinds[index];
    func_80027440((u16)(index + 1) | 0x3000000, 486, 214, &sp->style[4]);
    func_80026ecc(14, 531, 214, &sp->style[0]);
    func_80027440(150 | 0x3000000, 551, 214, &sp->style[4]);
    if (index < sp->unk0.store.count) {
        sp->style[0].cd |= 0x80;
        func_8002732c((u16 *)(sp->unk0.store.messages->text +
                              sp->unk0.store.messages->offsets[kind]),
                      40, 214, &sp->style[0]);
        sp->style[0].cd &= 0x7f;
    }
    func_80029548(32, 210, 576, 22);
    sp->unk350.w = 96;
    sp->unk350.h = 48;
    sp->unk350.y = 32;
    index = sp->unk2E2 * STORE_COLUMNS;
    for (row = 0; row < STORE_ROWS; row++) {
        cols = STORE_COLUMNS;
        if (sp->unk2E2 + row == sp->unk0.store.rows)
            cols = sp->unk0.store.last;
        sp->unk350.x = 48;
        for (col = 0; col < cols; col++, index++) {
            if (index < sp->unk0.store.count) {
                func_80027d18(&sp->unk350, &sp->style[0], 513, 1);
                func_80028364(&sp->unk350, &sp->unk0.store.tex[index % 15], 0, 0, 16);
            }
            sp->unk350.x += 112;
        }
        sp->unk350.y += 56;
    }
    func_800286a8(0, 0, 0);
    func_80029548(32, 24, 576, 176);
}
