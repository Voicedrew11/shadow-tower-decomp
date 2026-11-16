/*
 * ST.EXE: an id grid screen (WIP module): two four-column grids of table C
 * ids the player moves between, its frame and its CD loads. It shares the
 * id grid layout and panels of src/st/ui.c (MenuGrid).
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/kernel.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBETC.H>
#include <LIBSN.H>
#include <st/memory.h>
#include <st/cd.h>
#include <st/data.h>
#include <st/gfx.h>
#include <st/text.h>
#include <st/menu.h>
#include <psyq/libc.h>
#include <st/vis.h>
#include <st/idgrid.h>
#include <st/funcs.h>

RODATA(0x800138a0, 0x38)
SDATA(0x8008812c, 0x5c)

/* Nonzero while the CD request ring still holds a request. */
ADDRESS(0x8006616c, 0x5c)
s16 func_8006616c(void)
{
    s16 busy;

    EnterCriticalSection();
    busy = 1;
    if (cd_state.unk9D8 == cd_state.unk9D4)
        busy = cd_state.unk9D8->unk0 != 0;
    ExitCriticalSection();
    return busy;
}

/* The load callback: copies the loaded image to VRAM (640, 0). */
/* Not exact: retail saves $ra before the argument setup; this build
 * schedules the store after it (one word moved). */
ADDRESS(0x800661c8, 0x44)
void func_800661c8(void)
{
    DVECTOR pos;

    pos.vx = 640;
    pos.vy = 0;
    func_8003ec5c(DAT_80088c88, &pos, NULL, 1);
    DrawSync(0);
}

ADDRESS(0x8006620c, 0x34)
void func_8006620c(u16 archive, u16 member)
{
    DVECTOR unused;

    func_80015d54(archive, member, (u8 *)DAT_80088c88, func_800661c8);
}

/*
 * Opens the screen: loads its message tables and images, then lists every
 * held table C id (except 266 and the ids in viewer.unk38) into the grid of
 * its flag bit.
 */
/* Not exact: retail re-forms &DAT_80088c88 from its kept %hi register and
 * builds each image pointer in that register; this build keeps a copy (one
 * more saved register, 96-byte frame vs 88). */
ADDRESS(0x80066240, 0x470)
void func_80066240(void)
{
    DVECTOR tex = {896, 448};
    DVECTOR clut = {544, 502};
    RECT unused;
    char name[] = "\\st\\itemcg\\icon1\\0000.tim";
    u8 *tim;
    int i;

    DAT_801e0d38.unk0 = 104;
    DAT_801e0d38.unk4 = 0;
    DAT_801e0d38.unk10 = DAT_801e0d38.unkC = DAT_801e0d38.unk8 = -1;
    func_80058cc8();
    MENU_SCRATCH->unk0.grid.messages = func_800152d0(21504);
    func_80016030(3, 1, DAT_80088c88);
    func_80014cfc((u32 *)&menu_messages, (u32 *)DAT_80088c88 + 1, 5376);
    tim = (u8 *)DAT_80088c88 + (DAT_80088c88[0] + 4);
    func_8003ec5c(tim + 4, &MENU_SCRATCH->unk2E6, &MENU_SCRATCH->unk2EA, 1);
    func_80016030(3, 4, DAT_80088c88);
    func_80014cfc((u32 *)MENU_SCRATCH->unk0.grid.messages, (u32 *)DAT_80088c88 + 1, 5376);
    tim = (u8 *)DAT_80088c88 + (DAT_80088c88[0] + 4);
    func_8003ec5c(tim + 4, &MENU_SCRATCH->unk2EE, &MENU_SCRATCH->unk2F2, 1);
    func_80016030(3, 5, DAT_80088c88);
    func_8003ec5c(DAT_80088c88, &MENU_SCRATCH->unk2F6, NULL, 1);
    DrawSync(0);
    func_80016030(3, 7, DAT_80088c88);
    func_8003ec5c(DAT_80088c88, &tex, &clut, 1);
    DrawSync(0);
    MENU_SCRATCH->unk0.grid.count0 = 0;
    MENU_SCRATCH->unk0.grid.count1 = 0;
    MENU_SCRATCH->unk0.grid.ids0 = func_800152d0(628);
    MENU_SCRATCH->unk0.grid.ids1 = func_800152d0(628);
    if (MENU_SCRATCH->unk0.grid.ids0 == NULL || MENU_SCRATCH->unk0.grid.ids1 == NULL)
        exit(1);
    fill_words((u32 *)MENU_SCRATCH->unk0.grid.ids0, -1, 157);
    fill_words((u32 *)MENU_SCRATCH->unk0.grid.ids1, -1, 157);
    MENU_SCRATCH->unk0.grid.count0 = 0;
    MENU_SCRATCH->unk0.grid.count1 = 0;
    for (i = 0; i < 314; i++) {
        if (vis_table_c.unk0118[i].unk5[0] != 255 && i < 314 && i != 266 &&
            DAT_801c7590.counts[i].count && i != viewer.unk38[0] && i != viewer.unk38[1] &&
            i != viewer.unk38[2] && i != viewer.unk38[3] && i != viewer.unk38[4] &&
            i != viewer.unk38[5] && i != viewer.unk38[7] && i != viewer.unk38[8] &&
            i != viewer.unk38[9] && i != viewer.unk38[10] && i != viewer.unk38[11] &&
            i != viewer.unk38[12] && i != viewer.unk38[13] && i != viewer.unk38[14] &&
            i != viewer.unk38[15] && i != viewer.unk38[16] && i != viewer.unk38[17] &&
            i != viewer.unk38[18] && i != viewer.unk38[19] && i != viewer.unk38[20]) {
            if (!DAT_801c7590.counts[i].flag)
                MENU_SCRATCH->unk0.grid.ids0[MENU_SCRATCH->unk0.grid.count0++] = i;
            else
                MENU_SCRATCH->unk0.grid.ids1[MENU_SCRATCH->unk0.grid.count1++] = i;
        }
    }
    MENU_SCRATCH->unk2DC = 1;
    MENU_SCRATCH->unk2E0 = 0;
    MENU_SCRATCH->unk0.grid.row0 = 0;
    MENU_SCRATCH->unk0.grid.row1 = 0;
    DAT_801e0d38.unk2 = -1;
    func_80031cfc();
    DrawSync(0);
    func_800172bc();
}

#define GRID (&sp->unk0.grid)
#define ST DAT_801e0d38
#define PAD DAT_801cd180

/* Resets the held range. */
#define RANGE_CLEAR() \
    ST.unk4 = 0;      \
    ST.unk8 = -1;     \
    ST.unkC = -1;     \
    ST.unk10 = -1

/*
 * The screen loop: moves ids between the grids, compacts and scrolls them
 * and draws the frame, until cancel; then writes each listed id's grid back
 * to its flag bit.
 */
/* Not exact: the same 192 blocks and branches; retail saves $s0 first and
 * keeps the range index in $a3 and the state block in $t1 (this build
 * $t0/$a3), and the two closing flag loops fill their delay slots
 * differently. */
ADDRESS(0x800666b0, 0xed0)
void func_800666b0(void)
{
    DVECTOR unused0;
    DVECTOR pos = {640, 0};
    char name1[] = "\\st\\itemcg\\icon1\\0000.tim";
    char name5[] = "\\st\\itemcg\\icon5\\0000.tim";
    DVECTOR unused1;
    MenuScratch *sp = MENU_SCRATCH;
    s16 id;
    s16 n;
    s16 k;
    s16 idx;
    int i, j;
    int found;
    int cur;
    int anchor;

    func_80057a3c();
    for (;;) {
        if (PAD.unk4 & 0x2000) {
            func_80057cec();
            PAD.unk4 = 0;
            if (sp->unk2DC & 0x8000)
                sp->unk2DC &= 0x7fff;
            else if (sp->unk2DC == 2)
                sp->unk2DC = 1;
            else
                break;
        }
        if (PAD.unk4 & 0x4000) {
            func_80057cc8();
            PAD.unk4 = 0;
            if (sp->unk2DC & 0x8000) {
                if (ST.unk0 == 104) {
                    if (!ST.unk4)
                        ST.unk8 = ST.unkC = GRID->row0 + sp->unk2E0;
                    for (j = ST.unk8; j <= ST.unkC; j++) {
                        idx = j;
                        if (GRID->ids0[idx] != -1) {
                            k = GRID->count1;
                            for (i = 0; i < GRID->count1; i++) {
                                if (GRID->ids1[i] == -1) {
                                    k = i;
                                    break;
                                }
                            }
                            GRID->ids1[k] = GRID->ids0[idx];
                            GRID->ids0[idx] = -1;
                            GRID->count1++;
                            GRID->count0--;
                        }
                    }
                } else {
                    if (!ST.unk4)
                        ST.unk8 = ST.unkC = GRID->row1 + sp->unk2E0;
                    for (j = ST.unk8; j <= ST.unkC; j++) {
                        idx = j;
                        if (GRID->ids1[idx] != -1) {
                            k = GRID->count0;
                            for (i = 0; i < GRID->count0; i++) {
                                if (GRID->ids0[i] == -1) {
                                    k = i;
                                    break;
                                }
                            }
                            GRID->ids0[k] = GRID->ids1[idx];
                            GRID->ids1[idx] = -1;
                            GRID->count0++;
                            GRID->count1--;
                        }
                    }
                }
                RANGE_CLEAR();
                sp->unk2DC &= 0x7fff;
            } else {
                found = 0;
                if (!ST.unk4) {
                    if (ST.unk0 == 104)
                        found = GRID->ids0[GRID->row0 + sp->unk2E0] == -1;
                    else if (GRID->ids1[GRID->row1 + sp->unk2E0] == -1)
                        found = 1;
                } else {
                    found = 1;
                    if (ST.unk0 == 104) {
                        for (j = ST.unk8; j <= ST.unkC; j++) {
                            if (GRID->ids0[j] != -1) {
                                found = 0;
                                break;
                            }
                        }
                    } else {
                        for (j = ST.unk8; j <= ST.unkC; j++) {
                            if (GRID->ids1[j] != -1) {
                                found = 0;
                                break;
                            }
                        }
                    }
                }
                if (!found)
                    sp->unk2DC |= 0x8000;
            }
        }
        if (!(sp->unk2DC & 0x8000)) {
            if (PAD.unk4 & 0x1) {
                func_80057cc8();
                PAD.unk4 = 0;
                if (ST.unk0 == 104) {
                    for (i = 0; i < 314; i++) {
                        if (GRID->ids0[i] == -1) {
                            for (j = i; j < 314; j++) {
                                if (GRID->ids0[j] != -1) {
                                    GRID->ids0[i] = GRID->ids0[j];
                                    GRID->ids0[j] = -1;
                                    break;
                                }
                            }
                        }
                    }
                } else {
                    for (i = 0; i < 314; i++) {
                        if (GRID->ids1[i] == -1) {
                            for (j = i; j < 314; j++) {
                                if (GRID->ids1[j] != -1) {
                                    GRID->ids1[i] = GRID->ids1[j];
                                    GRID->ids1[j] = -1;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            if (PAD.unk4 & 0x8000) {
                func_80057cc8();
                PAD.unk4 = 0;
                if (ST.unk4) {
                    RANGE_CLEAR();
                } else {
                    ST.unk4 = 1;
                    if (ST.unk0 == 104)
                        cur = GRID->row0 + sp->unk2E0;
                    else
                        cur = GRID->row1 + sp->unk2E0;
                    ST.unk10 = ST.unk8 = ST.unkC = cur;
                }
            }
            if (PAD.unk4 & 0x1000) {
                func_80057cc8();
                PAD.unk4 = 0;
                if (sp->unk2DC == 1)
                    sp->unk2DC = 2;
                else if (sp->unk2DC == 2)
                    sp->unk2DC = 1;
            }
            if (PAD.unkC & 0x20) {
                func_80057d10();
                PAD.unkC = 0;
                if (ST.unk0 == 104) {
                    ST.unk0 = 98;
                    RANGE_CLEAR();
                }
            }
            if (PAD.unkC & 0x80) {
                func_80057d10();
                PAD.unkC = 0;
                if (ST.unk0 == 98) {
                    ST.unk0 = 104;
                    RANGE_CLEAR();
                }
            }
            if (PAD.unkC & 0x10) {
                func_80057d10();
                if (PAD.unk2 & 0x8000) {
                    if (ST.unk0 == 104) {
                        if (GRID->row0 == 0)
                            sp->unk2E0 = 0;
                        GRID->row0 -= 11;
                        if (GRID->row0 < 0)
                            GRID->row0 = 0;
                    } else {
                        if (GRID->row1 == 0)
                            sp->unk2E0 = 0;
                        GRID->row1 -= 11;
                        if (GRID->row1 < 0)
                            GRID->row1 = 0;
                    }
                } else if (sp->unk2E0 > 0) {
                    sp->unk2E0--;
                } else if (ST.unk0 == 104) {
                    if (GRID->row0 > 0)
                        GRID->row0--;
                } else {
                    if (GRID->row1 > 0)
                        GRID->row1--;
                }
                PAD.unkC = 0;
            }
            if (PAD.unkC & 0x40) {
                func_80057d10();
                if (PAD.unk2 & 0x8000) {
                    if (ST.unk0 == 104) {
                        if (GRID->row0 == 303)
                            sp->unk2E0 = 10;
                        GRID->row0 += 11;
                        if (GRID->row0 >= 304)
                            GRID->row0 = 303;
                    } else {
                        if (GRID->row1 == 303)
                            sp->unk2E0 = 10;
                        GRID->row1 += 11;
                        if (GRID->row1 >= 304)
                            GRID->row1 = 303;
                    }
                } else if (sp->unk2E0 < 10) {
                    sp->unk2E0++;
                } else if (ST.unk0 == 104) {
                    if (GRID->row0 < 303)
                        GRID->row0++;
                } else {
                    if (GRID->row1 < 303)
                        GRID->row1++;
                }
                PAD.unkC = 0;
            }
            if ((sp->unk2DC == 2 && ST.unk0 == 104) || sp->unk2DC != 2) {
                if (PAD.unkC & 0x400) {
                    func_80057d10();
                    PAD.unkC = 0;
                    if (ST.unk0 == 104 && GRID->row0 == 0)
                        sp->unk2E0 = 0;
                    GRID->row0 -= 11;
                    if (GRID->row0 < 0)
                        GRID->row0 = 0;
                }
                if (PAD.unkC & 0x100) {
                    func_80057d10();
                    PAD.unkC = 0;
                    if (ST.unk0 == 104 && GRID->row0 == 303)
                        sp->unk2E0 = 10;
                    GRID->row0 += 11;
                    if (GRID->row0 >= 304)
                        GRID->row0 = 303;
                }
            }
            if ((sp->unk2DC == 2 && ST.unk0 == 98) || sp->unk2DC != 2) {
                if (PAD.unkC & 0x800) {
                    func_80057d10();
                    PAD.unkC = 0;
                    if (ST.unk0 == 98 && GRID->row1 == 0)
                        sp->unk2E0 = 0;
                    GRID->row1 -= 11;
                    if (GRID->row1 < 0)
                        GRID->row1 = 0;
                }
                if (PAD.unkC & 0x200) {
                    func_80057d10();
                    PAD.unkC = 0;
                    if (ST.unk0 == 98 && GRID->row1 == 303)
                        sp->unk2E0 = 10;
                    GRID->row1 += 11;
                    if (GRID->row1 >= 304)
                        GRID->row1 = 303;
                }
            }
            if (ST.unk4) {
                if (ST.unk0 == 104)
                    cur = GRID->row0 + sp->unk2E0;
                else
                    cur = GRID->row1 + sp->unk2E0;
                anchor = ST.unk10;
                if (anchor < cur) {
                    ST.unk8 = anchor;
                    ST.unkC = cur;
                } else {
                    if (cur < anchor)
                        ST.unk8 = cur;
                    else
                        ST.unk8 = anchor;
                    ST.unkC = anchor;
                }
            }
            if (sp->unk2DC == 2 && !func_8006616c()) {
                if (ST.unk0 == 104)
                    id = GRID->ids0[GRID->row0 + sp->unk2E0];
                else
                    id = GRID->ids1[GRID->row1 + sp->unk2E0];
                if (ST.unk2 != id && id != -1) {
                    func_8006620c(6, vis_table_c.unk0118[id].unk5[0] + 200);
                    ST.unk2 = id;
                }
            }
        }
        func_800164b4();
        func_80068258();
        sp->unk2E4 = (sp->unk2E4 + 1) % 32;
    }
    for (i = 0, n = GRID->count0; n; i++) {
        if (GRID->ids0[i] != -1) {
            DAT_801c7590.counts[GRID->ids0[i]].flag = 0;
            n--;
        }
    }
    for (i = 0, n = GRID->count1; n; i++) {
        if (GRID->ids1[i] != -1) {
            DAT_801c7590.counts[GRID->ids1[i]].flag = 1;
            n--;
        }
    }
}

/* Closes the screen once the CD ring is idle. */
ADDRESS(0x80067580, 0x74)
void func_80067580(void)
{
    MenuScratch *sp = MENU_SCRATCH;

    while (func_8006616c())
        ;
    func_80058c4c();
    func_800152f8(sp->unk0.grid.ids0);
    func_800152f8(sp->unk0.grid.ids1);
    func_800152f8(sp->unk0.grid.messages);
    func_80058d30();
    func_80057a3c();
}

ADDRESS(0x800675f4, 0x30)
void func_800675f4(void)
{
    func_80066240();
    func_800666b0();
    func_80067580();
}

/*
 * The detail panel of the id under the cursor: its image (when the CD is
 * idle), its fourteen table C counts and its two figures.
 */
/* Not exact: like func_80031674 (src/st/ui.c), retail spills the panel
 * position, text x/y and the id to a 120-byte frame with $t1 as temporary
 * and forms each scratchpad address with its own lui; this build keeps them
 * in registers. */
ADDRESS(0x80067624, 0x5c8)
void func_80067624(void)
{
    int text_x, text_y;
    s16 x, y;
    s16 id;
    int none;
    int n, k;
    int i, j;
    int name_x, count_x;
    u8 *count;
    CVECTOR *label, *value;
    u8 whole, tenths;

    if (ST.unk0 == 104) {
        x = 334;
        y = 9;
        text_x = 342;
    } else {
        x = 10;
        y = 9;
        text_x = 18;
    }
    text_y = 135;
    MENU_SCRATCH->unk350.w = 192;
    MENU_SCRATCH->unk350.x = x;
    MENU_SCRATCH->unk350.h = 124;
    MENU_SCRATCH->unk350.y = y;
    none = 0;
    if (ST.unk0 == 104) {
        id = MENU_SCRATCH->unk0.grid.ids0[MENU_SCRATCH->unk0.grid.row0 + MENU_SCRATCH->unk2E0];
        none = id == -1;
    } else {
        id = MENU_SCRATCH->unk0.grid.ids1[MENU_SCRATCH->unk0.grid.row1 + MENU_SCRATCH->unk2E0];
        if (id == -1)
            none = 1;
    }
    if (!none) {
        if (!func_8006616c()) {
            MENU_SCRATCH->unk358.vx = 640;
            MENU_SCRATCH->unk358.vy = 0;
            func_80028364(&MENU_SCRATCH->unk350, &MENU_SCRATCH->unk358, NULL, NULL, 16);
            MENU_SCRATCH->unk350.w = 100;
            MENU_SCRATCH->unk358.vx = 832;
            MENU_SCRATCH->unk350.x += 192;
            func_80028364(&MENU_SCRATCH->unk350, &MENU_SCRATCH->unk358, NULL, NULL, 16);
        }
        n = 0;
        k = 2;
        count = vis_table_c.unk0118[id].unk10;
        DAT_800881ae = 1;
        name_x = text_x - 4;
        count_x = text_x;
        for (i = 0; i < 4; i++) {
            for (j = 0; j < 4; j++) {
                if (n == 14)
                    goto done;
                n++;
                if (*count) {
                    func_8002732c(MESSAGE(200 + k), name_x, text_y + j * 15, &MENU_SCRATCH->style[0]);
                    func_80027440(*count | 0x13000000, count_x + 40, text_y + j * 15,
                                  &MENU_SCRATCH->style[4]);
                }
                count++;
                k++;
            }
            if (n == 14)
                break;
            count_x += 72;
            name_x += 72;
        }
    done:
        DAT_800881ae = 0;
        text_y += 60;
        if (vis_table_c.unk0118[id].unk27) {
            if (vis_table_c.unk0118[id].unk28 < 4) {
                label = &MENU_SCRATCH->style[2];
                value = &MENU_SCRATCH->style[6];
            } else {
                label = &MENU_SCRATCH->style[0];
                value = &MENU_SCRATCH->style[4];
            }
            func_8002732c(MESSAGE(29), text_x, text_y, label);
            func_80027440(vis_table_c.unk0118[id].unk28 | 0x3000000, text_x + 70, text_y, value);
            func_80026ecc(14, text_x + 106, text_y, label);
            func_80027440(vis_table_c.unk0118[id].unk27 | 0x3000000, text_x + 122, text_y, value);
        }
        text_y += 15;
        if (vis_table_c.unk0118[id].unk24) {
            whole = vis_table_c.unk0118[id].unk24 * 100u / 454;
            tenths = vis_table_c.unk0118[id].unk24 * 1000 / 454 - whole * 10;
            func_8002732c(MESSAGE(28), text_x, text_y, &MENU_SCRATCH->style[0]);
            func_80027440(whole | 0x3000000, text_x + 70, text_y, &MENU_SCRATCH->style[4]);
            func_80026ecc(18, text_x + 109, text_y, &MENU_SCRATCH->style[0]);
            func_80027440(tenths | 0x1000000, text_x + 116, text_y, &MENU_SCRATCH->style[4]);
            func_80026ecc(53, text_x + 132, text_y, &MENU_SCRATCH->style[0]);
        }
    }
    func_80029548(x, y, 296, 222);
}

/*
 * Draws one grid (104 or 98): the cursor bar, the scroll arrows and bar, the
 * header image and eleven rows of names, the held range highlighted.
 */
/* Not exact: retail loads unk2E0 in both arms of the cursor bar x choice and
 * allocates the row loop (grid byte reloaded from its stack slot)
 * differently. */
ADDRESS(0x80067bec, 0x66c)
void func_80067bec(u8 grid)
{
    MenuScratch *sp = MENU_SCRATCH;
    RECT unused5;
    CVECTOR unused0 = {96, 96, 96};
    RECT bar;
    RECT unused1;
    RECT unused2;
    RECT unused3 = {314, 119, 16, 8};
    RECT unused4 = {625, 119, 16, 8};
    RECT up0 = {26, 28, 24, 8};
    RECT down0 = {26, 216, 24, 8};
    RECT knob0 = {16, 35, 10, 3};
    RECT up1 = {612, 28, 24, 8};
    RECT down1 = {612, 216, 24, 8};
    RECT knob1 = {611, 35, 10, 3};
    CVECTOR knob = {40, 40, 48};
    int i;
    int idx;
    s16 id;
    s16 *list;
    CVECTOR *color;
    int x, y;

    if (ST.unk0 == 104)
        sp->unk330.x = 26;
    else
        sp->unk330.x = 340;
    sp->unk330.y = sp->unk2E0 * 16 + 33;
    sp->unk330.w = 270;
    sp->unk330.h = 15;
    func_80027d18(&sp->unk330, &sp->unk32C, 513, 1);
    if (grid == 104) {
        if (sp->unk0.grid.row0 > 0)
            func_8002916c(&up0, &sp->unk32C, 3072);
        if (sp->unk0.grid.row0 < 303)
            func_8002916c(&down0, &sp->unk32C, 1024);
    } else {
        if (sp->unk0.grid.row1 > 0)
            func_8002916c(&up1, &sp->unk32C, 3072);
        if (sp->unk0.grid.row1 < 303)
            func_8002916c(&down1, &sp->unk32C, 1024);
    }
    if (grid == 104) {
        knob0.y = sp->unk0.grid.row0 * 173 / 303 + 35;
        func_80028220(&knob0, &knob, 1);
        func_800286a8(1, 0, 0);
        bar.x = knob0.x - 2;
        bar.y = knob0.y - 1;
        bar.w = knob0.w;
        bar.h = knob0.h;
    } else {
        knob1.y = sp->unk0.grid.row1 * 173 / 303 + 35;
        func_80028220(&knob1, &knob, 1);
        func_800286a8(1, 0, 0);
        bar.x = knob1.x - 2;
        bar.y = knob1.y - 1;
        bar.w = knob1.w;
        bar.h = knob1.h;
    }
    func_80028220(&bar, &knob, 1);
    func_800286a8(2, 0, 0);
    if (grid == 104) {
        sp->unk33C.x = 60;
        sp->unk33C.y = 12;
        sp->unk33C.w = 196;
        sp->unk33C.h = 16;
        sp->unk358.vx = 896;
        sp->unk358.vy = 448;
    } else {
        sp->unk33C.x = 384;
        sp->unk33C.y = 12;
        sp->unk33C.w = 196;
        sp->unk33C.h = 16;
        sp->unk358.vx = 896;
        sp->unk358.vy = 464;
    }
    sp->unk35C.vx = 544;
    sp->unk35C.vy = 502;
    func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, 3);
    {
        CVECTOR held = {0, 128, 128, 201};
        CVECTOR empty = {0, 96, 96, 201};
        RECT box;

        if (grid == 104)
            x = 28;
        else
            x = 342;
        y = 35;
        sp->style[0].cd |= 0x80;
        for (i = 0; i < 11; i++, y += 16) {
            if (grid == 104) {
                idx = sp->unk0.grid.row0 + i;
                list = sp->unk0.grid.ids0;
            } else {
                idx = sp->unk0.grid.row1 + i;
                list = sp->unk0.grid.ids1;
            }
            id = list[idx];
            color = &sp->style[0];
            if (grid == ST.unk0 && ST.unk4 && idx >= ST.unk8 && idx <= ST.unkC) {
                if (id != -1) {
                    color = &held;
                } else {
                    box.x = x;
                    box.y = y;
                    box.w = 266;
                    box.h = 12;
                    func_80028220(&box, &empty, 0);
                }
            }
            if (id != -1)
                func_8002732c(MESSAGE_IN(sp->unk0.grid.messages, id), x, y, color);
        }
        sp->style[0].cd &= 0x7f;
    }
    if (grid == 104)
        func_80029548(10, 9, 296, 222);
    else
        func_80029548(334, 9, 296, 222);
}

/* Draws the screen: the text colours, the cursor blink, one grid with its
 * detail panel or both grids, and the arrow toward the current grid. */
/* Not exact: as func_80066004: retail stores the 48 group before loading the
 * blink phase; this build hoists the load (7 words). */
ADDRESS(0x80068258, 0x1e8)
void func_80068258(void)
{
    RECT arrow = {319, 110, 24, 16};
    CVECTOR color = {16, 48, 16, 0};
    MenuScratch *sp;

    func_800174c4();
    sp = MENU_SCRATCH;
    sp->style[2].b = sp->style[6].b = sp->style[3].g = sp->style[7].g = sp->style[3].b =
        sp->style[7].b = 48;
    sp->style[0].b = sp->style[4].b = sp->style[0].g = sp->style[4].g = sp->style[0].r =
        sp->style[4].r = sp->style[3].r = sp->style[7].r = 96;
    sp->style[2].r = sp->style[6].r = 98;
    sp->style[2].g = sp->style[6].g = 66;
    sp->style[0].cd = sp->style[2].cd = sp->style[3].cd = 73;
    sp->style[4].cd = sp->style[6].cd = sp->style[7].cd = 49;
    if (MENU_SCRATCH->unk2E4 < 16)
        sp->unk360 = 8 - MENU_SCRATCH->unk2E4;
    else
        sp->unk360 = MENU_SCRATCH->unk2E4 - 24;
    sp->unk32C.b = 0;
    sp->unk32C.r = (u8)sp->unk360 * 4 + 96;
    sp->unk32C.g = (u8)sp->unk360 * 2 + 48;
    if (sp->unk2DC & 0x8000)
        func_800319a8();
    if (sp->unk2DC & 0x1) {
        func_80067bec(104);
        func_80067bec(98);
    } else {
        func_80067bec(ST.unk0 == 104 ? 104 : 98);
        func_80067624();
    }
    if (ST.unk0 == 104)
        func_8002916c(&arrow, &color, 0);
    else
        func_8002916c(&arrow, &color, 2048);
    func_800286a8(1, 0, 0);
    func_8002c9e8();
    func_80017680();
}

/*
 * Unreferenced debug tool: shows the other frame buffer and moves the
 * display area with the D-pad (raw pad bits, active low) until START,
 * polling the host debugger each frame; then restores it.
 */
/* Not exact: retail keeps %hi(DAT_801cd132) in $s1 across the adjust loop
 * and sign-extends the raw bits before inverting them, and enters the second
 * wait loop after one test; this build re-forms the address for each read. */
ADDRESS(0x80068440, 0x214)
void func_80068440(void)
{
    RECT saved;
    DISPENV *disp;
    DRAWENV *draw;
    s16 bits;

    world_state.draw[0].isbg = 0;
    world_state.draw[1].isbg = 0;
    world_state.frame = world_state.frame == 0;
    DrawSync(0);
    VSync(0);
    PutDispEnv(&world_state.disp[world_state.frame]);
    PutDrawEnv(&world_state.draw[world_state.frame]);
    disp = &world_state.disp[world_state.frame];
    draw = &world_state.draw[world_state.frame];
    func_80014cfc((u32 *)&saved, (u32 *)&disp->disp, 2);
    while (!(DAT_801cd132 & 0x8))
        ;
    for (;;) {
        bits = ~DAT_801cd132;
        if (bits & 0x2000)
            disp->disp.x += 4;
        if (bits & 0x8000)
            disp->disp.x -= 4;
        if (bits & 0x1000)
            disp->disp.y -= 4;
        if (bits & 0x4000)
            disp->disp.y += 4;
        if (bits & 0x8)
            break;
        disp->disp.x &= 0x3ff;
        disp->disp.y &= 0x1ff;
        VSync(0);
        PutDispEnv(disp);
        PutDrawEnv(draw);
        pollhost();
    }
    while (!(DAT_801cd132 & 0x8))
        ;
    func_80014cfc((u32 *)&disp->disp, (u32 *)&saved, 2);
    world_state.draw[0].isbg = 1;
    world_state.draw[1].isbg = 1;
}
