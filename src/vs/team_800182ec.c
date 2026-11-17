/*
 * VS.EXE: the team screen's input loop (WIP module): moves the cursor over
 * the team slots and the item grid, picks items into the team, swaps
 * slots, scrolls the grid (reloading the pictures that scroll in) and
 * opens the detail page, until the cancel button leaves.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <vs/data.h>
#include <vs/menu.h>
#include <vs/pad.h>
#include <vs/sound.h>
#include <vs/funcs.h>

int func_8002ee90(int archive, int member, u_long *buf);
void func_80017bc4(void);
void func_80017c84(VsMenuScratch *sp, int player, VsTeamEdit *edit);
void func_80017da4(VsMenuScratch *sp, int player, VsTeamEdit *edit);
void func_80017fe8(VsMenuScratch *sp, int player, VsTeamEdit *edit);
void func_80018240(u_long *tim, DVECTOR *pos, DVECTOR *clut_pos);
void func_80021c14(int page);

/* The detail page toggle: the attribute figures when nonzero (bss, unowned). */

#define PRESSED (DAT_801603c0.pads[0].unk8 | DAT_801603c0.pads[1].unk8)
#define REPEAT (DAT_801603c0.pads[0].unkC | DAT_801603c0.pads[1].unkC)
#define HELD (DAT_801603c0.pads[0].unk0 | DAT_801603c0.pads[1].unk0)

/* Not exact: player/pad/index lifetimes and some button-tail edges differ;
 * retail spills player to the stack instead of keeping it in s7. */
ADDRESS(0x800182ec, 0x998)
void func_800182ec(VsMenuScratch *sp, int player)
{
    VsTeamEdit edit;
    int shown = -1;
    s32 *team = DAT_801624b8[player].slot;
    u_long *buf;
    int top;
    int col;
    int row;
    int idx;
    int id;
    int n;
    int size;
    int i, j;

    edit.unk0 = 0;
    for (;;) {
        if (player) {
            DAT_801603c0.pads[0].unk8 = 0;
            DAT_801603c0.pads[0].unkC = 0;
            DAT_801603c0.pads[0].unk0 = 0;
        } else {
            DAT_801603c0.pads[1].unk8 = 0;
            DAT_801603c0.pads[1].unkC = 0;
            DAT_801603c0.pads[1].unk0 = 0;
        }
        top = sp->unk1D2;
        if (sp->unk1CC == 1) {
            int cursor = sp->unk1D0;

            if (cursor < 0) {
                col = -1;
                row = ~cursor;
            } else {
                row = cursor >> 2;
                col = (s16)(cursor - row * 4);
            }
            if (PRESSED & 0x2000) {
                if (!edit.unk0) {
                    func_8001c8f8();
                    return;
                }
                func_8001c8f8();
                func_80017fe8(sp, player, &edit);
                edit.unk0 = 0;
                sp->unk1D6 = 0;
                func_80017bc4();
            }
            if (!edit.unk0) {
                if (REPEAT & 0x20) {
                    func_8001c8b8();
                    if (col < 0 && !edit.unk0)
                        sp->unk1D0 = row << 2;
                    else if (col < 3)
                        sp->unk1D0++;
                }
                if (!edit.unk0 && (REPEAT & 0x80)) {
                    func_8001c8b8();
                    if (col > 0)
                        sp->unk1D0--;
                    else if (col == 0)
                        sp->unk1D0 = ~row;
                }
            }
            if (REPEAT & 0x10) {
                func_8001c8b8();
                if (edit.unk0) {
                    if (row > 0) {
                        sp->unk1D0++;
                        func_80017da4(sp, player, &edit);
                    }
                } else if (HELD & 0x8000) {
                    sp->unk1D2 -= 3;
                    if (sp->unk1D2 < 0)
                        sp->unk1D2 = 0;
                } else if (sp->unk1D0 < 0) {
                    if (row > 0)
                        sp->unk1D0++;
                } else if (row > 0) {
                    sp->unk1D0 -= 4;
                } else if (sp->unk1D2 > 0) {
                    sp->unk1D2--;
                }
            } else if (REPEAT & 0x40) {
                func_8001c8b8();
                if (edit.unk0) {
                    if (row < 2 && team[row + 1] != -1) {
                        sp->unk1D0--;
                        func_80017da4(sp, player, &edit);
                    }
                } else if (HELD & 0x8000) {
                    if (sp->unk0.team.rows > 2) {
                        sp->unk1D2 += 3;
                        if (sp->unk1D2 > sp->unk0.team.rows - 2)
                            sp->unk1D2 = sp->unk0.team.rows - 2;
                    }
                } else if (sp->unk1D0 < 0) {
                    if (row < 2)
                        sp->unk1D0--;
                } else if (sp->unk1D2 + row < sp->unk0.team.rows) {
                    if (row < 2)
                        sp->unk1D0 += 4;
                    else if (sp->unk1D2 <= sp->unk0.team.rows - 3)
                        sp->unk1D2++;
                }
            } else if (REPEAT & 0x800) {
                func_8001c8b8();
                sp->unk1D2 -= 3;
                if (sp->unk1D2 < 0)
                    sp->unk1D2 = 0;
            } else if (REPEAT & 0x200) {
                func_8001c8b8();
                if (sp->unk0.team.rows > 2) {
                    sp->unk1D2 += 3;
                    if (sp->unk1D2 > sp->unk0.team.rows - 2)
                        sp->unk1D2 = sp->unk0.team.rows - 2;
                }
            } else if (PRESSED & 0x4000) {
                if (edit.unk0) {
                    func_8001c978();
                    edit.unk0 = 0;
                    sp->unk1D6 = 0;
                    func_80017bc4();
                } else {
                    idx = sp->unk1D2 * 4 + sp->unk1D0;
                    edit.unk0 = 1;
                    sp->unk1D6 = 1;
                    if (sp->unk1D0 < 0) {
                        edit.unk4 = ~sp->unk1D0;
                        if (team[edit.unk4] != -1) {
                            func_8001c978();
                            func_80017c84(sp, player, &edit);
                            goto scrolled;
                        }
                    } else if (DAT_801624b8[player].slot[2] == -1 &&
                               sp->unk0.team.kinds[idx] != 0xff) {
                        for (i = 0; i < 3; i++) {
                            if (team[i] == sp->unk0.team.kinds[idx])
                                break;
                        }
                        if (i == 3) {
                            for (j = 0; j < 3; j++) {
                                if (team[j] == -1) {
                                    RECT r;

                                    team[j] = sp->unk0.team.kinds[idx];
                                    r.x = sp->unk0.team.pos[idx % 12].vx;
                                    r.y = sp->unk0.team.pos[idx % 12].vy;
                                    r.w = 96;
                                    r.h = 48;
                                    MoveImage2(&r, sp->unk0.team.pos[12 + j].vx,
                                               sp->unk0.team.pos[12 + j].vy);
                                    DrawSync(0);
                                    break;
                                }
                            }
                            if (j != 3) {
                                func_8001c978();
                                sp->unk1D0 = ~j;
                                edit.unk4 = j;
                                func_80017c84(sp, player, &edit);
                                goto scrolled;
                            }
                        }
                    }
                    func_8001c938();
                    edit.unk0 = 0;
                    sp->unk1D6 = 0;
                }
            } else if ((PRESSED & 0x1000) && !edit.unk0) {
                DAT_8006335c = 0;
                func_8001c978();
                if (sp->unk1D0 < 0) {
                    id = team[-1 - sp->unk1D0];
                    if (id == -1) {
                        idx = -1;
                    } else {
                        /* Retail only updates idx when a kind matches. */
                        for (i = 0; i < sp->unk0.team.count; i++) {
                            if (id == sp->unk0.team.kinds[i]) {
                                idx = i;
                                break;
                            }
                        }
                    }
                } else {
                    idx = sp->unk1D2 * 4 + sp->unk1D0;
                    for (i = 0; i < 3; i++) {
                        if (sp->unk0.team.kinds[idx] == team[i]) {
                            idx = -1;
                            break;
                        }
                    }
                }
                if (idx < sp->unk0.team.count && idx != -1) {
                    if (idx != shown) {
                        DVECTOR tex;
                        DVECTOR clut;

                        shown = idx;
                        buf = DAT_801a4590.unk4A4;
                        func_8002ee90(10, sp->unk0.team.kinds[idx] + 200, buf);
                        func_80012c2c(&buf[1], &sp->unk0.team.pos[15], NULL);
                        tex.vx = 960;
                        tex.vy = 446;
                        clut.vx = 560;
                        clut.vy = 497;
                        buf = (u_long *)((buf[0] + sizeof(*buf)) + (u8 *)buf);
                        func_80018240(buf + 1, &tex, &clut);
                    }
                    sp->unk1CC = 2;
                }
            }
        scrolled:
            if (sp->unk1D2 != top) {
                if (top < sp->unk1D2) {
                    n = sp->unk1D2 - top;
                    idx = (top + 3) * 4;
                } else {
                    n = top - sp->unk1D2;
                    idx = sp->unk1D2 * 4;
                }
                DAT_801a4590.unk4A0 = DAT_801a4590.unk4A4;
                for (i = 0; i < n * 4; i++, idx++) {
                    if (idx < sp->unk0.team.count) {
                        size = func_8002ee90(10, sp->unk0.team.kinds[idx],
                                             DAT_801a4590.unk4A0) * 2048;
                        func_80012c2c(DAT_801a4590.unk4A0, &sp->unk0.team.pos[idx % 12], NULL);
                        DAT_801a4590.unk4A0 = (u_long *)((u8 *)DAT_801a4590.unk4A0 + size);
                    }
                }
                DrawSync(0);
            }
        } else if (PRESSED & 0x2000) {
            func_8001c8f8();
            sp->unk1CC = 1;
        } else if (PRESSED & 0x800) {
            DAT_8006335c = 1;
        } else if (PRESSED & 0x400) {
            DAT_8006335c = 0;
        }
        DAT_801603c0.pads[0].unk8 = 0;
        DAT_801603c0.pads[1].unk8 = 0;
        DAT_801603c0.pads[0].unkC = 0;
        DAT_801603c0.pads[1].unkC = 0;
        func_80021c14(player);
        sp->unk1D4 = (sp->unk1D4 + 1) % 32;
    }
}
