/*
 * VS.EXE: the team screen's item grid page (WIP module): the cursor box,
 * the scroll arrows, the name of the item under the cursor, three rows of
 * item pictures (dark when already in the team) and the team's three
 * slots.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <vs/data.h>
#include <vs/menu.h>
#include <vs/funcs.h>

/* The scroll arrows' initializers: 8 bytes each, so .sdata under -G8. */
SDATA(0x80063270, 0x10)

/* Number-width flag for func_8001d9cc (ST.EXE's DIGITS, src/st/ui.c). */
#define DIGITS(n) ((n) << 24)

/* Not exact (one attempt; 79.3%): first divergence at +0xd8, the cursor
 * box: retail keeps ~unk1D0 apart from the row (in $v1) and stores the box
 * x before its y on each path so the y tails cross-jump; register choices
 * then differ through the item lookup and the grid loops. */
ADDRESS(0x80020aa4, 0x69c)
void func_80020aa4(int player)
{
    RECT up = {392, 32, 32, 12};
    RECT down = {392, 224, 32, 12};
    CVECTOR black;
    CVECTOR frame;
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    int c;
    int row;
    int id;
    int idx;
    int n;
    int i, j, k;
    s16 x, y;

    func_8001d8b8(VS_MESSAGE(180), 44, 21, &sp->style[0]);
    func_8001f23c(16, 16, 138, 22);
    c = sp->unk1D0;
    row = c >> 2;
    if (c < 0) {
        row = ~c;
        if (sp->unk1D6) {
            sp->unk218.x = 26;
            sp->unk218.y = row * 56 + 44;
            goto box;
        }
        sp->unk218.x = 32;
    } else {
        sp->unk218.x = (s16)(c - row * 4) * 112 + 176;
    }
    sp->unk218.y = row * 56 + 48;
box:
    sp->unk218.w = 96;
    sp->unk218.h = 48;
    func_8001de8c(&sp->unk218, &sp->unk214, 513, 1);
    func_8001e890(1, 0, 0);
    if (sp->unk1D2 > 0)
        func_8001eef4(&up, &sp->unk214, 3072);
    if (sp->unk1D2 <= sp->unk0.team.rows - 3)
        func_8001eef4(&down, &sp->unk214, 1024);
    func_8001e890(1, 0, 0);
    c = sp->unk1D0;
    if (c < 0) {
        id = DAT_801624b8[player].slot[-c - 1];
        if (id != -1) {
            for (i = 0; i < sp->unk0.team.count; i++) {
                if (id == sp->unk0.team.kinds[i]) {
                    idx = i;
                    break;
                }
            }
        } else {
            idx = -1;
        }
    } else {
        idx = sp->unk1D2 * 4 + c;
        id = sp->unk0.team.kinds[idx];
        for (i = 0; i < 3; i++) {
            if (id == DAT_801624b8[player].slot[i])
                idx = -1;
        }
    }
    if (idx < sp->unk0.team.count && idx != -1) {
        sp->style[0].cd |= 0x80;
        func_8001d8b8(VS_ITEM_NAME(id), 428, 21, &sp->style[0]);
        sp->style[0].cd &= 0x7f;
    }
    func_8001f23c(420, 16, 200, 22);
    sp->unk238.w = 96;
    sp->unk238.h = 48;
    sp->unk238.y = 48;
    frame.r = 50;
    frame.g = 50;
    frame.b = 50;
    idx = sp->unk1D2 * 4;
    for (i = 0; i < 3; i++) {
        n = 4;
        if (sp->unk0.team.rows == sp->unk1D2 + i)
            n = sp->unk0.team.last;
        sp->unk238.x = 176;
        for (j = 0; j < n; j++, idx++) {
            if (idx < sp->unk0.team.count) {
                func_8001de8c(&sp->unk238, &frame, 513, 0);
                for (k = 0; k < 3; k++) {
                    if (DAT_801624b8[player].slot[k] == sp->unk0.team.kinds[idx]) {
                        black.r = 0;
                        black.g = 0;
                        black.b = 0;
                        func_8001e3d8(&sp->unk238, &black, 0);
                        break;
                    }
                }
                if (k == 3)
                    func_8001e530(&sp->unk238, &sp->unk0.team.pos[idx % 12], NULL, NULL, 16);
            }
            sp->unk238.x += 112;
        }
        sp->unk238.y += 56;
    }
    sp->unk238.x = 32;
    sp->unk238.y = 48;
    for (i = 0; i < 3; i++) {
        func_8001d9cc((i + 1) | DIGITS(3), 110, i * 56 + 85, &sp->style[3]);
        if (DAT_801624b8[player].slot[i] == -1) {
            black.r = 0;
            black.g = 0;
            black.b = 0;
            func_8001e3d8(&sp->unk238, &black, 0);
        } else {
            x = sp->unk238.x;
            y = sp->unk238.y;
            if (sp->unk1D6 && ~sp->unk1D0 == i) {
                sp->unk238.x -= 6;
                sp->unk238.y -= 4;
            }
            func_8001e530(&sp->unk238, &sp->unk0.team.pos[12 + i], NULL, NULL, 16);
            sp->unk238.x = x;
            sp->unk238.y = y;
        }
        func_8001de8c(&sp->unk238, &frame, 513, 0);
        sp->unk238.y += 56;
    }
    func_8001e890(0, 0, 0);
    func_8001f23c(160, 40, 464, 176);
    func_8001f23c(16, 40, 138, 176);
}
