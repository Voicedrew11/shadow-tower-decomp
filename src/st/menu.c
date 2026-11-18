/*
 * ST.EXE: the pause menu and the screens it runs (WIP module,
 * 0x80058c04..0x80063638): the menu entry and page loop, the equipment
 * pictures and attribute totals, the figure recomputation, the equipment,
 * stored-item, item storage and item use pages, the attribute point
 * screen, the shops, the area list, the options and the memory card
 * screens. One translation unit (-G8): its constant pool (.rodata
 * 0x800128d0..0x80013738) has no 8-byte break between functions, its
 * switch tables (0x80013738..0x80013880) follow, and GCC merged the screen
 * RECT {0, 0, 320, 480} (.sdata 0x800880ac) between func_8005aa80 and
 * func_800633d4. The pool starts after st.sound_bank's switch table, so
 * the unit starts at 0x80058c04; src/st/menu_vram.c is a unit of its own
 * (its RECT literals repeat this unit's instead of merging with them).
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/kernel.h>
#include <psyq/libc.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBETC.H>
#include <st/memory.h>
#include <st/cd.h>
#include <st/data.h>
#include <st/gfx.h>
#include <st/mesh.h>
#include <st/scratch.h>
#include <st/vis.h>
#include <st/effect.h>
#include <st/menu.h>
#include <st/hud.h>
#include <st/area.h>
#include <st/funcs.h>
#include <st/player.h>
#include <st/sound.h>
#include <st/menu_main.h>
#include <st/stats.h>
#include <st/card.h>
#include <st/pad.h>
#include <st/save.h>
#include <st/item.h>

RODATA(0x800128d0, 0xe68)
JUMPTABLES(0x80013738, 0x148)
SDATA(0x800880a4, 0x3f)

/* The CD file buffer (DAT_80088c88) follows cd_state; these screens spell
 * it from cd_state's base, which GCC then shares with cd_state.unkA84. */
#define CD_BUFFER ((u_long *)(&cd_state + 1))

ADDRESS(0x80058c04, 0x20)
void func_80058c04(void)
{
    func_80026e04();
}

ADDRESS(0x80058c24, 0x28)
void func_80058c24(void)
{
    func_800595d4();
    func_80026e90();
}

/* Loads member 0 of archive 3: the menu messages, then the menu image. */
ADDRESS(0x80058c4c, 0x7c)
void func_80058c4c(void)
{
    u_long *image;

    image = DAT_80088c88;
    func_80016030(3, 0, image);
    func_80014cfc((u32 *)&menu_messages, (u32 *)(image + 1), 5376);
    image = (u_long *)((u8 *)image + (DAT_80088c88[0] + 4));
    func_8003ec5c(image + 1, &MENU_SCRATCH->unk2E6[0], &MENU_SCRATCH->unk2E6[1], 1);
}

ADDRESS(0x80058cc8, 0x68)
void func_80058cc8(void)
{
    RECT rect = {576, 384, 64, 16};
    DVECTOR size = {576, 480};

    ResetGraph(1);
    func_80021158();
    func_800214a4();
}

/* Reloads the menu and the viewer's equipment models after the menu. */
ADDRESS(0x80058d30, 0x94)
void func_80058d30(void)
{
    func_80021158();
    func_80058c4c();
    func_80044404();
    func_8003f12c(viewer.unk38[3], 0, 1);
    func_8003f12c(viewer.unk38[16], 1, 1);
    func_8003f12c(viewer.unk38[4], 2, 1);
    func_8003f12c(viewer.unk38[17], 3, 1);
    func_8003ee38(viewer.unk38[6], 0);
    func_8003ee38(viewer.unk38[12], 1);
    func_800214a4();
}

/* Opens a menu screen: the VRAM positions, the menu loads and the HUD. */
ADDRESS(0x80058dc4, 0x110)
void func_80058dc4(int arg0, int arg1)
{
    MenuScratch *sp = MENU_SCRATCH;
    u_long *image;

    sp->unk2E6[0].vx = 960;
    sp->unk2E6[0].vy = 256;
    sp->unk2E6[1].vx = 560;
    sp->unk2E6[1].vy = 495;
    sp->unk2E6[2].vx = 896;
    sp->unk2E6[2].vy = 256;
    sp->unk2E6[3].vx = 560;
    sp->unk2E6[3].vy = 497;
    sp->unk2E6[4].vx = 768;
    sp->unk2E6[4].vy = 256;
    func_80015ad4();
    image = DAT_80088c88;
    func_80016030(3, 0, image);
    func_80014cfc((u32 *)&menu_messages, (u32 *)(image + 1), 5376);
    image = (u_long *)((u8 *)image + (DAT_80088c88[0] + 4));
    func_8003ec5c(image + 1, &sp->unk2E6[0], &sp->unk2E6[1], 1);
    func_80058c04();
    sp->unk2E5 = 0;
    SCRATCHPAD->unk208 = 0;
    if (arg0)
        sp->unk2DC = 256;
    else
        sp->unk2DC = 1;
    DrawSync(0);
}

ADDRESS(0x80058ed4, 0x20)
void func_80058ed4(void)
{
    func_80058c24();
}

/* The pause menu: runs the selected page until one closes the menu. */
ADDRESS(0x80058ef4, 0x310)
void func_80058ef4(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    u_long *image;

    sp->unk2E6[0].vx = 960;
    sp->unk2E6[0].vy = 256;
    sp->unk2E6[1].vx = 560;
    sp->unk2E6[1].vy = 495;
    sp->unk2E6[2].vx = 896;
    sp->unk2E6[2].vy = 256;
    sp->unk2E6[3].vx = 560;
    sp->unk2E6[3].vy = 497;
    sp->unk2E6[4].vx = 768;
    sp->unk2E6[4].vy = 256;
    func_80015ad4();
    image = DAT_80088c88;
    func_80016030(3, 0, image);
    func_80014cfc((u32 *)&menu_messages, (u32 *)(image + 1), 5376);
    image = (u_long *)((u8 *)image + (DAT_80088c88[0] + 4));
    func_8003ec5c(image + 1, &sp->unk2E6[0], &sp->unk2E6[1], 1);
    func_80058c04();
    sp->unk2DE = 0;
    func_80057a3c();
    DrawSync(0);
    for (;;) {
        if (DAT_801cd180.unk4 & 0x2000) {
            func_80057cec();
            DAT_801cd180.unk4 = 0;
            break;
        }
        if (DAT_801cd180.unk4 & 0x1000) {
            func_80057cc8();
            DAT_801cd180.unk4 = 0;
            sp->unk2E5 = 0;
            func_80060018();
        }
        if (DAT_801cd180.unk4 & 0x4000) {
            func_80057cc8();
            DAT_801cd180.unk4 = 0;
            if ((s16)sp->unk2DE == 0) {
                sp->unk2E5 = 0;
                if (func_8005ec10() == 1)
                    break;
            } else if ((s16)sp->unk2DE == 1) {
                sp->unk2E5 = 0;
                func_80064ff0();
            } else if ((s16)sp->unk2DE == 2) {
                viewer.unk21D = 1;
                sp->unk2E5 = 1;
                if (DAT_801cd180.unk2 & 0x800)
                    func_8005d25c();
                else
                    func_800675f4();
            } else if ((s16)sp->unk2DE == 3) {
                viewer.unk21D = 1;
                sp->unk2E5 = 1;
                func_8005c7fc();
            } else if ((s16)sp->unk2DE == 4) {
                viewer.unk21D = 1;
                sp->unk2E5 = 1;
                func_8005ad00();
            } else if ((s16)sp->unk2DE == 5) {
                sp->unk2E5 = 0;
                func_80060334();
            } else if ((s16)sp->unk2DE == 6) {
                sp->unk2DC = 1;
                sp->unk2E5 = 0;
                sp->unk206 = 1;
                if (func_80062c28(1) == 1)
                    break;
            } else if ((s16)sp->unk2DE == 7) {
                sp->unk2E5 = 0;
                if (func_80063078() == 1)
                    break;
            }
        }
        if (DAT_801cd180.unkC & 0x40) {
            func_80057d10();
            DAT_801cd180.unkC = 0;
            sp->unk2DE++;
            if ((s16)sp->unk2DE >= 8)
                sp->unk2DE = 0;
        }
        if (DAT_801cd180.unkC & 0x10) {
            func_80057d10();
            DAT_801cd180.unkC = 0;
            sp->unk2DE--;
            if ((s16)sp->unk2DE < 0)
                sp->unk2DE = 7;
        }
        sp->unk2E5 = 0;
        func_800164b4();
        func_80029b74();
    }
    func_80057a3c();
    func_80058c24();
}

/*
 * Loads and uploads the picture of equipment slot `slot` (none: its empty
 * picture); a two-handed weapon also copies its second half.
 */
ADDRESS(0x80059204, 0x3d0)
void func_80059204(int slot)
{
    DVECTOR pos[21] = {
        {740, 152}, {796, 152}, {852, 152}, {932, 0},   {908, 152}, {964, 152}, {740, 124},
        {740, 180}, {796, 180}, {640, 124}, {640, 189}, {852, 180}, {788, 124}, {908, 180},
        {964, 180}, {852, 152}, {932, 73},  {796, 208}, {852, 208}, {908, 208}, {964, 208},
    };
    RECT rect;
    u16 *id = &viewer.unk38[slot];
    int part = -1;
    int none;
    int first;
    int file;
    u_long *image;
    u_long *data;

    cd_state.unkA84 = (u_long *)(&cd_state + 1);
    switch (slot) {
    case 0:
    case 13:
        first = 7;
        none = 202;
        break;
    case 1:
    case 14:
        first = 8;
        none = 238;
        break;
    case 2:
    case 15:
        first = 4;
        none = 124;
        break;
    case 3:
        first = 1;
        none = 0;
        part = 0;
        break;
    case 4:
        first = 1;
        none = 0;
        part = 1;
        break;
    case 16:
        first = 6;
        none = 166;
        part = 0;
        break;
    case 17:
        first = 6;
        none = 166;
        part = 1;
        break;
    case 7:
        first = 9;
        none = 259;
        break;
    case 8:
        first = 2;
        none = 72;
        break;
    case 9:
        first = 3;
        none = 93;
        if (*id != 0xffff && vis_table_c.unk0118[*id].unk26 == 10)
            part = 0;
        break;
    case 10:
        first = 5;
        none = 145;
        break;
    case 5:
    case 11:
    case 18:
    case 19:
    case 20:
        first = 10;
        none = 275;
        break;
    case 6:
    case 12:
        first = 977;
        none = 976;
        break;
    default:
        return;
    }
    if (slot == 6 || slot == 12)
        file = *id != 0xffff ? *id + first : none;
    else
        file = (*id != 0xffff ? *id + first : none) * 3 + 2;
    func_80016030(7, file, cd_state.unkA84);
    image = cd_state.unkA84;
    if (part != -1) {
        if (part == 1)
            image = (u_long *)((u8 *)image + (image[0] + 4));
        data = image + 1;
    } else {
        data = image;
    }
    func_8003ec5c(data, &pos[slot], NULL, 1);
    if (*id != 0xffff && vis_table_c.unk0118[*id].unk26 == 9) {
        if (slot == 3) {
            rect.x = pos[3].vx;
            rect.y = pos[3].vy;
            rect.w = 92;
            rect.h = 73;
            DrawSync(0);
            MoveImage(&rect, pos[16].vx, pos[16].vy);
        } else if (slot == 4) {
            rect.x = pos[4].vx;
            rect.y = pos[4].vy;
            rect.w = 56;
            rect.h = 28;
            DrawSync(0);
            MoveImage(&rect, pos[17].vx, pos[17].vy);
        }
    } else if (*id != 0xffff && vis_table_c.unk0118[*id].unk26 == 10) {
        image = (u_long *)((u8 *)image + (image[0] + 4));
        func_8003ec5c(image + 1, &pos[2], NULL, 1);
    }
}

/* Uploads the pictures of the five quick item slots, 24 lines apart. */
ADDRESS(0x800595d4, 0x188)
void func_800595d4(void)
{
    u32 ids[5];
    DVECTOR pos = {896, 256};
    RECT rect;
    int used;
    int i;
    int size;

    i = used = 0;
    ids[0] = viewer.unk38[5];
    ids[1] = viewer.unk38[19];
    ids[2] = viewer.unk38[11];
    ids[3] = viewer.unk38[20];
    ids[4] = viewer.unk38[18];
    cd_state.unkA84 = (u_long *)(&cd_state + 1);
    for (i = 0; i < 5; i++) {
        if (used + 2048 > 0x12000) {
            DrawSync(0);
            cd_state.unkA84 = (u_long *)(&cd_state + 1);
            used = 0;
        }
        if (ids[i] != 0xffff) {
            size = func_80016030(6, vis_table_c.unk0118[ids[i]].unk5[0] + 400, cd_state.unkA84)
                   << 11;
            func_8003ec5c(cd_state.unkA84, &pos, NULL, 1);
            used += size;
            cd_state.unkA84 = (u_long *)((u8 *)cd_state.unkA84 + size);
        } else {
            rect.x = pos.vx;
            rect.y = pos.vy;
            rect.w = rect.h = 24;
            ClearImage(&rect, 0, 0, 0);
        }
        pos.vy += 24;
    }
}

/*
 * Applies the two-handed rules after the selected slot changed (from the
 * slot copy at scratch +0x292 when `redraw` is 0): a two-handed weapon
 * empties the other hand's slots; with `redraw` 1 the changed pictures are
 * reloaded.
 *
 * Not exact (8 bytes longer): retail forms the slot 2/15 stores from a
 * viewer+0x3c base and folds the -1 of the second-hand index into the
 * address constant; this build stores from the viewer base and keeps the
 * -2 as a displacement, and the final switch's blocks are ordered
 * differently.
 */
ADDRESS(0x8005975c, 0x590)
void func_8005975c(int redraw)
{
    MenuScratch *sp = MENU_SCRATCH;

    if (redraw == 0)
        *(MenuSlots *)sp->unk292 = *MENU_SLOTS;
    if (viewer.unk38[9] != 0xffff && vis_table_c.unk0118[viewer.unk38[9]].unk26 == 10) {
        if (sp->unk2E0 == 2 || sp->unk2E0 == 16 || sp->unk2E0 == 9 || sp->unk2E0 == 10
            || sp->unk2E0 == 11) {
            viewer.unk38[2] = 0xffff;
            viewer.unk38[9] = 0xffff;
            viewer.unk38[10] = 0xffff;
            if (redraw == 1) {
                func_80059204(2);
                func_80059204(9);
                func_80059204(10);
            }
        }
    }
    if (viewer.unk38[3] != 0xffff && vis_table_c.unk0118[viewer.unk38[3]].unk26 == 9) {
        if (sp->unk2E0 == 3 || sp->unk2E0 == 17) {
            viewer.unk38[3] = 0xffff;
            viewer.unk38[16] = 0xffff;
            if (redraw == 1) {
                func_80059204(3);
                func_80059204(16);
            }
        }
    }
    if (viewer.unk38[4] != 0xffff && vis_table_c.unk0118[viewer.unk38[4]].unk26 == 9) {
        if (sp->unk2E0 == 4 || sp->unk2E0 == 18) {
            viewer.unk38[4] = 0xffff;
            viewer.unk38[17] = 0xffff;
            if (redraw == 1) {
                func_80059204(4);
                func_80059204(17);
            }
        }
    }
    if (sp->unk2C0 != -1) {
        if (vis_table_c.unk0118[sp->unk2C0].unk26 == 10) {
            viewer.unk38[2] = 0xffff;
            viewer.unk38[10] = 0xffff;
            if (redraw == 1) {
                func_80059204(2);
                func_80059204(10);
            }
        }
        if (vis_table_c.unk0118[sp->unk2C0].unk26 == 9) {
            if (sp->unk2E0 == 3) {
                viewer.unk38[16] = 0xffff;
                if (redraw == 1)
                    func_80059204(16);
            } else if (sp->unk2E0 == 4) {
                viewer.unk38[17] = 0xffff;
                if (redraw == 1)
                    func_80059204(17);
            }
        }
    }
    if (sp->unk2E0 == 2 || sp->unk2E0 == 16) {
        if (sp->unk2C0 == -1) {
            viewer.unk38[15] = viewer.unk38[2] = 0xffff;
        } else {
            viewer.unk38[2] = sp->unk2C0;
        }
        if (redraw == 1)
            func_80059204(2);
    } else if (sp->unk2E0 < 10) {
        if (sp->unk2C0 == -1)
            MENU_SLOTS->ids[sp->unk2E0] = 0xffff;
        else
            MENU_SLOTS->ids[sp->unk2E0] = sp->unk2C0;
        if (redraw == 1)
            func_80059204(sp->unk2E0);
    } else {
        if (sp->unk2C0 == -1)
            MENU_SLOTS->ids[sp->unk2E0 - 1] = 0xffff;
        else
            MENU_SLOTS->ids[sp->unk2E0 - 1] = sp->unk2C0;
        if (redraw == 1)
            func_80059204(sp->unk2E0 - 1);
    }
    switch (sp->unk2E0) {
    case 0:
        viewer.unk38[6] = 0xffff;
        break;
    case 14:
        viewer.unk38[12] = 0xffff;
        break;
    default:
        return;
    }
    if (redraw == 1) {
        DrawSync(0);
        func_80059204(sp->unk2E0 ? 12 : 6);
    }
}

/* Restores the equipment slots from the scratch copy. */
ADDRESS(0x80059cec, 0xd4)
void func_80059cec(void)
{
    *MENU_SLOTS = *(MenuSlots *)MENU_SCRATCH->unk292;
}

/*
 * Totals the attribute rows from the viewer's figures and equipment, and
 * fills one record per shown equipment slot with its figures and kind.
 */
ADDRESS(0x80059dc0, 0x5ac)
void func_80059dc0(void)
{
    MenuAttrScratch *at = MENU_ATTR;
    u16 *id;
    Vis44 *rec;
    int i;
    int j;

    for (i = 0; i < 14; i++) {
        at->unk25C[i] = 0;
        at->unk240[i] = viewer.unk9A[i];
        at->unk224[i] = viewer.unk7E[i];
        at->unk208[i] = 0;
    }
    id = viewer.unk38;
    for (i = 0; i < 11; i++) {
        if (*id != 0xffff) {
            for (j = 0; j < 14; j++)
                at->unk208[j] += vis_table_c.unk0118[*id].unk10[j];
        }
        if (i == 3)
            id += 4;
        else if (i == 7)
            id += 3;
        else if (i == 9)
            id += 2;
        else
            id += 1;
    }
    if (viewer.unk2C & 0x80) {
        at->unk208[1] >>= 1;
        at->unk240[1] >>= 1;
    }
    for (i = 0; i < 14; i++)
        at->unk25C[i] = at->unk208[i] - at->unk224[i];
    for (i = 0; i < 14; i++) {
        at->unk1EC[i] = at->unk240[i] + at->unk208[i];
        if (at->unk1EC[i] >= 1000)
            at->unk1EC[i] = 999;
        if (at->unk1EC[i] < 0)
            at->unk1B4[i] = 0;
        else
            at->unk1B4[i] = at->unk1EC[i];
        if (at->unk208[i] < 0)
            at->unk1D0[i] = 0;
        else
            at->unk1D0[i] = at->unk208[i];
    }
    id = viewer.unk38;
    at->unk168 = at->recs;
    at->unk2BC = 0;
    for (i = 0; i < 21; i++, id++) {
        if ((u32)(i - 4) < 3 || i == 11 || i == 12 || i == 15 || i >= 17)
            continue;
        if (*id == 0xffff) {
            fill_words((u32 *)at->unk168, 0, 8);
        } else {
            rec = &vis_table_c.unk0118[*id];
            at->unk168->unk0 = rec->unk24;
            for (j = 0; j < 14; j++)
                at->unk168->unk4[j] = rec->unk10[j];
            at->unk168->unk2 = 0;
            at->unk168->unk1 = 0;
            if (rec->kind2 >= 9) {
                at->unk168->unk1 = rec->kind2 - 9;
                at->unk168->unk2 = rec->amount2;
            }
            if (i != 4 && i != 17)
                at->unk2BC += rec->unk24;
        }
        at->unk168++;
    }
    if (viewer.unk38[6] != 0xffff) {
        at->unk16C = viewer.unk38[6] / 3;
        at->unk16E = *(at->unk16C + effects.defs[viewer.unk38[6]].unkA);
    } else {
        at->unk16C = 0xffff;
        at->unk16E = 0;
    }
    if (viewer.unk38[12] != 0xffff) {
        at->unk170 = viewer.unk38[12] / 3;
        at->unk172 = *(at->unk170 + effects.defs[viewer.unk38[12]].unkA);
    } else {
        at->unk170 = 0xffff;
        at->unk172 = 0;
    }
    for (i = 0; i < 6; i++) {
        at->unk286[i] = 0;
        at->unk27A[i] = 0;
    }
    at->unk27A[at->recs[0].unk1] += at->recs[0].unk2;
    at->unk27A[at->recs[8].unk1] += at->recs[8].unk2;
    at->unk27A[at->recs[1].unk1] += at->recs[1].unk2;
    at->unk27A[at->recs[9].unk1] += at->recs[9].unk2;
    at->unk27A[at->recs[4].unk1] += at->recs[4].unk2;
    at->unk286[at->recs[5].unk1] += at->recs[5].unk2;
    at->unk286[at->recs[6].unk1] += at->recs[6].unk2;
    at->unk286[at->recs[2].unk1] += at->recs[2].unk2;
    at->unk286[at->recs[7].unk1] += at->recs[7].unk2;
    at->unk286[at->recs[1].unk1] += at->recs[1].unk2;
    at->unk286[at->recs[9].unk1] += at->recs[9].unk2;
}

/* Copies the two recomputed figure rows into the viewer. */
ADDRESS(0x8005a36c, 0x3c)
void func_8005a36c(void)
{
    StatsScratch *sp = STATS_SCRATCH;
    int i;

    for (i = 0; i < 14; i++) {
        viewer.unk62[i] = sp->unk1EC[i];
        viewer.unk7E[i] = sp->unk208[i];
    }
}

/*
 * Recomputes the viewer's derived figures from the attribute rows
 * func_80059dc0 totals (after func_8005975c applies a slot change when
 * `apply` is set), then restores the slots (`apply`) or copies the rows
 * into the viewer.
 *
 * Not exact (4 bytes long): retail keeps the halved figures in the
 * division registers and reloads recs[3].unk1 every pass of the first
 * loop; this build copies b6 to another register and hoists that load.
 */
ADDRESS(0x8005a3a8, 0x6d8)
void func_8005a3a8(int apply)
{
    MenuAttrScratch *at = MENU_ATTR;
    u16 b6;
    u16 b8;
    u16 *p;
    u16 min;
    u16 max;
    int i;

    if (apply)
        func_8005975c(0);
    func_80059dc0();
    viewer.unk0 = at->unk1B4[0] * 5 + at->unk1B4[1] + at->unk1B4[2] + at->unk1B4[3]
                  + at->unk1B4[4] + at->unk1B4[5] + at->unk1B4[6];
    viewer.unk4 = at->unk1B4[7] * 5 + at->unk1B4[8] + at->unk1B4[9] + at->unk1B4[10]
                  + at->unk1B4[11] + at->unk1B4[12] + at->unk1B4[13];
    b6 = viewer.unk0 / 80;
    viewer.unkB6 = b6;
    b8 = viewer.unk4 / 80;
    viewer.unkB8 = b8;
    if (viewer.unk2C & 0x100) {
        viewer.unkB6 = b6 / 2;
        viewer.unkB8 = b8 / 2;
    }
    if (viewer.unk0 == 0)
        viewer.unk0 = 1;
    if (viewer.unk4 == 0)
        viewer.unk4 = 1;
    if (viewer.unkB6 == 0)
        viewer.unkB6 = 1;
    if (viewer.unkB8 == 0)
        viewer.unkB8 = 1;
    if (viewer.unk38[3] != 0xffff) {
        viewer.unkBA[0] = viewer.unkB6 + (at->unk1D0[4] >> 1);
        viewer.unkBA[1] = viewer.unkB6 + (at->unk1D0[5] >> 1);
        viewer.unkBA[2] = viewer.unkB6 + (at->unk1D0[6] >> 1);
        p = viewer.unkC0;
        for (i = 0; i < 6; i++) {
            if (at->recs[3].unk1 == i) {
                *p = viewer.unkB8 + (at->recs[3].unk2 >> 1);
                if (at->recs[1].unk1 == i)
                    *p += at->recs[1].unk2 >> 2;
                if (at->recs[9].unk1 == i)
                    *p += at->recs[9].unk2 >> 2;
            } else {
                *p = 0;
            }
        }
    } else {
        viewer.unkBA[0] = viewer.unkBA[1] = viewer.unkBA[2] = viewer.unkC0[0] = viewer.unkC0[1]
            = viewer.unkC0[2] = viewer.unkC0[3] = viewer.unkC0[4] = viewer.unkC0[5] = 0;
    }
    if (at->unk16C != 0xffff)
        viewer.unkE = viewer.unkB8 + at->unk16E + at->unk1B4[at->unk16C + 11] / 6;
    else
        viewer.unkE = 0;
    if (at->unk170 != 0xffff)
        viewer.unk10 = viewer.unkB8 + at->unk172 + at->unk1B4[at->unk170 + 11] / 6;
    else
        viewer.unk10 = 0;
    viewer.unk12 = viewer.unkB6 + ((at->unk1B4[2] - at->recs[10].unk4[2]) >> 2);
    viewer.unk14 = viewer.unkB8 + ((at->unk1B4[9] - at->recs[10].unk4[9]) >> 2);
    viewer.unk16 = at->recs[10].unk4[2];
    viewer.unk18 = at->recs[10].unk4[9];
    viewer.unk1E = (at->unk1B4[3] >> 4) + 50;
    viewer.unk20 = (at->unk1B4[10] >> 4) + 50;
    viewer.unk1A = (at->unk1B4[1] >> 3) + 100;
    viewer.unk1C = (at->unk1B4[8] >> 3) + 100;
    viewer.unkCC[0] = at->unk27A[0] / 5;
    viewer.unkCC[1] = at->unk27A[1] / 5;
    viewer.unkCC[2] = at->unk27A[2] / 5;
    viewer.unkCC[3] = at->unk27A[3] / 5;
    viewer.unkCC[4] = at->unk27A[4] / 5;
    viewer.unkCC[5] = at->unk27A[5] / 5;
    viewer.unkD8[0] = ((at->unk1B4[11] >> 2) + at->unk286[0]) / 7;
    viewer.unkD8[1] = ((at->unk1B4[11] >> 2) + at->unk286[1]) / 7;
    viewer.unkD8[2] = ((at->unk1B4[12] >> 2) + at->unk286[2]) / 7;
    viewer.unkD8[3] = ((at->unk1B4[12] >> 2) + at->unk286[3]) / 7;
    viewer.unkD8[4] = ((at->unk1B4[13] >> 2) + at->unk286[4]) / 7;
    viewer.unkD8[5] = ((at->unk1B4[13] >> 2) + at->unk286[5]) / 7;
    for (i = 0; i < 6; i++) {
        if (at->recs[10].unk1 == i)
            viewer.unkE4 = at->unk1B4[i / 2 + 11] + (at->recs[10].unk2 >> 1);
    }
    min = 0xffff;
    max = 0;
    if (max < viewer.unkBA[0])
        max = viewer.unkBA[0];
    if (viewer.unkBA[0] < min)
        min = viewer.unkBA[0];
    if (max < viewer.unkBA[1])
        max = viewer.unkBA[1];
    if (viewer.unkBA[1] < min)
        min = viewer.unkBA[1];
    if (max < viewer.unkBA[2])
        max = viewer.unkBA[2];
    if (viewer.unkBA[2] < min)
        min = viewer.unkBA[2];
    viewer.unkA = min;
    viewer.unkC = max;
    viewer.unk22 = at->unk2BC;
    viewer.unk24 = viewer.unk0 / 12 + 120;
    if (apply)
        func_80059cec();
    else
        func_8005a36c();
}

/*
 * Fades the screen to black over sixteen frames: each frame subtracts
 * 16 from the display through a translucent quad without clearing.
 *
 * Not exact (size-exact): retail builds the POLY_F4 tag before its XY
 * stores, as in func_80021158; this source builds it after them.
 */
ADDRESS(0x8005aa80, 0x280)
void func_8005aa80(void)
{
    POLY_F4 poly;
    DR_MODE mode;
    RECT clear = {0, 0, 320, 480};
    u_long ot[1];
    int n;

    n = 16;
    while (n--) {
        world_state.draw[0].isbg = 0;
        world_state.draw[1].isbg = 0;
        world_state.draw[1].dfe = 1;
        world_state.draw[0].dfe = 1;
        world_state.draw[1].isbg = 0;
        world_state.draw[0].isbg = 0;
        PutDrawEnv(&world_state.draw[world_state.frame]);
        PutDrawEnv(&world_state.draw[1 - world_state.frame]);
        ClearOTagR(ot, 1);
        setcode(&poly, 0x28);
        setSemiTrans(&poly, 1);
        setXY4(&poly, 0, 0, world_state.draw[1 - world_state.frame].clip.w, 0,
               0, world_state.draw[1 - world_state.frame].clip.h,
               world_state.draw[1 - world_state.frame].clip.w,
               world_state.draw[1 - world_state.frame].clip.h);
        mesh_add_prim(ot, &poly, 5);
        poly.r0 = poly.g0 = poly.b0 = 16;
        SetDrawMode(&mode, 0, 0, GetTPage(0, 2, 640, 0), NULL);
        addPrim(ot, &mode);
        VSync(0);
        DrawOTag(ot);
    }
    world_state.draw[0].isbg = 1;
    world_state.draw[1].isbg = 1;
}

/*
 * The equipment page: sorts the table C ids held into one list per kind,
 * draws the slot pictures, then lets the player pick a slot (unk2DC 1)
 * and an item for it from its list (unk2C7: the list kind; 10 is the
 * magic grid), recomputing the figures as the choice changes.
 *
 * Not exact (0x38 bytes short): archive/buffer lifetimes and several case
 * joins/load orders differ. The saved magic-page row can be -1 and must
 * stay signed across frame callbacks; file arithmetic stays promoted until
 * the call boundary. The switch-table addends still differ from retail.
 */
ADDRESS(0x8005ad00, 0x1afc)
void func_8005ad00(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    DVECTOR picture = {640, 0};
    RECT unused[4];
    DVECTOR image = {896, 448};
    DVECTOR clut = {544, 502};
    DVECTOR unused2;
    CdlLOC loc;
    u_long *p;
    s16 *slot;
    s16 base;
    s16 offset;
    int file;
    int debounce;
    int prev;
    int row;
    u16 cur;
    u8 kind;
    int n;
    int i;

    base = 0;
    offset = 0;
    func_80015a94(&cd_state.unkA00.file.pos, cd_state.unkA00.sectors[1], &loc);
    CdControl(CdlSetloc, (u_char *)&loc, NULL);
    ResetGraph(1);
    func_8005aa80();
    sp->unk2CC = func_800152d0(21504);
    sp->unk2D0 = func_800152d0(144);
    p = CD_BUFFER;
    func_80016030(3, 1, p);
    func_80014cfc((u32 *)&menu_messages, (u32 *)(p + 1), 5376);
    p = (u_long *)((u8 *)p + (CD_BUFFER[0] + 4));
    func_8003ec5c(p + 1, &sp->unk2E6[0], &sp->unk2E6[1], 1);
    p = (u_long *)((u8 *)p + (*p + 4));
    func_80014cfc(sp->unk2D0, (u32 *)(p + 1), 36);
    p = CD_BUFFER;
    func_80016030(3, 4, p);
    func_80014cfc((u32 *)sp->unk2CC, (u32 *)(p + 1), 5376);
    p = (u_long *)((u8 *)p + (CD_BUFFER[0] + 4));
    func_8003ec5c(p + 1, &sp->unk2E6[2], &sp->unk2E6[3], 1);
    func_80016030(3, 5, CD_BUFFER);
    func_8003ec5c(CD_BUFFER, &sp->unk2E6[4], NULL, 1);
    DrawSync(0);
    func_80016030(3, 7, CD_BUFFER);
    func_8003ec5c(CD_BUFFER, &image, &clut, 1);
    DrawSync(0);
    sp->unk0.vram[0].vx = 640;
    sp->unk0.vram[0].vy = 256;
    sp->unk0.vram[1].vx = 560;
    sp->unk0.vram[1].vy = 498;
    sp->unk174[0] = func_800152d0(40);
    sp->unk174[1] = func_800152d0(60);
    sp->unk174[2] = func_800152d0(40);
    sp->unk174[3] = func_800152d0(40);
    sp->unk174[4] = func_800152d0(70);
    sp->unk174[5] = func_800152d0(30);
    sp->unk174[6] = func_800152d0(140);
    sp->unk174[7] = func_800152d0(70);
    sp->unk174[8] = func_800152d0(40);
    sp->unk174[9] = func_800152d0(92);
    sp->unk1B0 = func_800152d0(140);
    sp->unk19C[0] = sp->unk19C[1] = sp->unk19C[2] = sp->unk19C[3] = sp->unk19C[4]
        = sp->unk19C[5] = sp->unk19C[6] = sp->unk19C[7] = sp->unk19C[8] = sp->unk19C[9] = 0;
    for (i = 0; i < 314; i++) {
        if (DAT_801c7590.counts[i].count == 0 || DAT_801c7590.counts[i].flag)
            continue;
        kind = vis_table_c.unk0118[i].unk26;
        if (kind == 0xff)
            continue;
        if (kind == 9) {
            slot = &sp->unk174[6][sp->unk19C[6]++];
        } else if (kind == 10) {
            slot = &sp->unk174[1][sp->unk19C[1]++];
        } else if (kind == 20 || kind == 30) {
            if (i == 266 || i == 310 || i == 311 || i == 312 || i == 313)
                continue;
            slot = &sp->unk174[9][sp->unk19C[9]++];
        } else {
            slot = &sp->unk174[kind][sp->unk19C[kind]++];
        }
        *slot = i;
    }
    func_80031cfc();
    DrawSync(0);
    func_800172bc();
    sp->unk2C4 = -1;
    for (i = 0; i < 14; i++) {
        if (sp->unk2C4 < viewer.unk62[i])
            sp->unk2C4 = viewer.unk62[i];
    }
    prev = 0;
    sp->unk2DC = 1;
    sp->unk2C6 = 0;
    sp->unk2E0 = 0;
    debounce = 0;
    sp->unk2D8 = viewer.unk2;
    sp->unk2DA = viewer.unk6;
    func_8005a3a8(0);
    func_80057a3c();
    {
        RECT clear = {640, 0, 384, 256};
        int slots[20] = {3, 4, 8, 9, 2, 10, 16, 17, 0, 13, 1, 14, 7, 5, 11, 18, 19, 20, 6, 12};

        DAT_800881aa = 0;
        ClearImage(&clear, 0, 0, 0);
        DrawSync(0);
        DAT_80088192 = 0;
        func_8002fc4c();
        func_8002fc4c();
        func_80059204(slots[0]);
        func_8002fc4c();
        for (i = 1; i < 20; i++) {
            if ((viewer.unk38[3] == 0xffff || vis_table_c.unk0118[viewer.unk38[3]].unk26 != 9
                 || slots[i] != 16)
                && (viewer.unk38[4] == 0xffff || vis_table_c.unk0118[viewer.unk38[4]].unk26 != 9
                    || slots[i] != 17)
                && (viewer.unk38[9] == 0xffff || vis_table_c.unk0118[viewer.unk38[9]].unk26 != 10
                    || (slots[i] != 2 && slots[i] != 10 && slots[i] != 15))) {
                func_80059204(slots[i]);
                func_8002fc4c();
                func_80057d10();
            }
        }
    }
    func_8002fc4c();
    func_80057d10();
    func_80057a3c();
    for (;;) {
        if (debounce != 0) {
            DAT_801cd180.unkC = 0;
            debounce--;
        }
        if (sp->unk2DC == 1) {
            if (DAT_801cd180.unk4 & 0x800)
                DAT_800881aa = 1;
            else if (DAT_801cd180.unk4 & 0x400)
                DAT_800881aa = 0;
            if (DAT_801cd180.unk2 & 0x1000) {
                if (sp->unk2C6 == 0)
                    func_80057cc8();
                sp->unk2C6 = 1;
            } else {
                sp->unk2C6 = 0;
            }
            if (DAT_801cd180.unk4 & 0x2000) {
                func_80057cec();
                DAT_801cd180.unk4 = 0;
                break;
            }
            if (DAT_801cd180.unk4 & 0x4000) {
                DAT_801cd180.unk4 = 0;
                sp->unk2DC = 2;
                if (sp->unk2E0 == 16)
                    cur = viewer.unk38[2];
                else if (sp->unk2E0 < 10)
                    cur = viewer.unk38[sp->unk2E0];
                else
                    cur = viewer.unk38[sp->unk2E0 - 1];
                sp->unk2C2 = 0;
                switch (sp->unk2E0) {
                case 3:
                case 4:
                    n = sp->unk19C[6];
                    for (i = 0; i < n; i++) {
                        if ((sp->unk2E0 == 3 && sp->unk174[6][i] != viewer.unk38[4])
                            || (sp->unk2E0 == 4 && sp->unk174[6][i] != viewer.unk38[3]))
                            sp->unk1B0[sp->unk2C2++] = sp->unk174[6][i];
                    }
                    base = 0;
                    func_80057cc8();
                    sp->unk2C7 = 6;
                    offset = 1;
                    break;
                case 17:
                case 18:
                    n = sp->unk19C[7];
                    for (i = 0; i < n; i++) {
                        if ((sp->unk2E0 == 17 && sp->unk174[7][i] != viewer.unk38[17])
                            || (sp->unk2E0 == 18 && sp->unk174[7][i] != viewer.unk38[16]))
                            sp->unk1B0[sp->unk2C2++] = sp->unk174[7][i];
                    }
                    base = 166;
                    func_80057cc8();
                    sp->unk2C7 = 7;
                    offset = 6;
                    break;
                case 8:
                    n = sp->unk19C[0];
                    for (i = 0; i < n; i++)
                        sp->unk1B0[sp->unk2C2++] = sp->unk174[0][i];
                    base = 72;
                    func_80057cc8();
                    sp->unk2C7 = 0;
                    offset = 2;
                    break;
                case 9:
                case 10:
                    n = sp->unk19C[1];
                    for (i = 0; i < n; i++)
                        sp->unk1B0[sp->unk2C2++] = sp->unk174[1][i];
                    offset = 3;
                    func_80057cc8();
                    sp->unk2C7 = 1;
                    base = 93;
                    break;
                case 2:
                case 16:
                    n = sp->unk19C[2];
                    for (i = 0; i < n; i++)
                        sp->unk1B0[sp->unk2C2++] = sp->unk174[2][i];
                    offset = 4;
                    func_80057cc8();
                    sp->unk2C7 = 2;
                    base = 124;
                    break;
                case 11:
                    n = sp->unk19C[3];
                    for (i = 0; i < n; i++)
                        sp->unk1B0[sp->unk2C2++] = sp->unk174[3][i];
                    offset = 5;
                    func_80057cc8();
                    sp->unk2C7 = 3;
                    base = 145;
                    break;
                case 0:
                case 14:
                    n = sp->unk19C[4];
                    for (i = 0; i < n; i++) {
                        if ((sp->unk2E0 == 0 && sp->unk174[4][i] != viewer.unk38[13])
                            || (sp->unk2E0 == 14 && sp->unk174[4][i] != viewer.unk38[0]))
                            sp->unk1B0[sp->unk2C2++] = sp->unk174[4][i];
                    }
                    offset = 7;
                    func_80057cc8();
                    sp->unk2C7 = 4;
                    base = 202;
                    break;
                case 1:
                case 15:
                    n = sp->unk19C[8];
                    for (i = 0; i < n; i++) {
                        if ((sp->unk2E0 == 1 && sp->unk174[8][i] != viewer.unk38[14])
                            || (sp->unk2E0 == 15 && sp->unk174[8][i] != viewer.unk38[1]))
                            sp->unk1B0[sp->unk2C2++] = sp->unk174[8][i];
                    }
                    offset = 8;
                    func_80057cc8();
                    sp->unk2C7 = 8;
                    base = 238;
                    break;
                case 7:
                    n = sp->unk19C[5];
                    for (i = 0; i < n; i++)
                        sp->unk1B0[sp->unk2C2++] = sp->unk174[5][i];
                    offset = 9;
                    func_80057cc8();
                    sp->unk2C7 = 5;
                    base = 259;
                    break;
                case 5:
                case 12:
                case 19:
                case 20:
                case 21:
                    n = sp->unk19C[9];
                    for (i = 0; i < n; i++) {
                        if ((sp->unk174[9][i] != viewer.unk38[5] && sp->unk174[9][i] != viewer.unk38[19]
                             && sp->unk174[9][i] != viewer.unk38[11]
                             && sp->unk174[9][i] != viewer.unk38[20]
                             && sp->unk174[9][i] != viewer.unk38[18])
                            || (sp->unk2E0 == 5 && sp->unk174[9][i] == viewer.unk38[5])
                            || (sp->unk2E0 == 20 && sp->unk174[9][i] == viewer.unk38[19])
                            || (sp->unk2E0 == 12 && sp->unk174[9][i] == viewer.unk38[11])
                            || (sp->unk2E0 == 21 && sp->unk174[9][i] == viewer.unk38[20])
                            || (sp->unk2E0 == 19 && sp->unk174[9][i] == viewer.unk38[18]))
                            sp->unk1B0[sp->unk2C2++] = sp->unk174[9][i];
                    }
                    offset = 10;
                    func_80057cc8();
                    sp->unk2C7 = 9;
                    base = 275;
                    break;
                case 6:
                case 13:
                    if ((sp->unk2E0 != 6 || viewer.unk38[0] == 0xffff)
                        && (sp->unk2E0 != 13 || viewer.unk38[13] == 0xffff)) {
                        func_80057d34();
                        sp->unk2DC = 1;
                        goto next;
                    }
                    func_80057cc8();
                    sp->unk2C7 = 10;
                    sp->unk2BE = cur;
                    if (cur == 0xffff)
                        sp->unk2BE = 0;
                    offset = 977;
                    base = 976;
                    sp->unk2C0 = sp->unk2BE;
                    func_8005a3a8(1);
                    break;
                default:
                    offset = 0;
                    base = 0;
                    break;
                }
                if (sp->unk2C7 == 10) {
                    sp->unk358.vx = 640;
                    sp->unk358.vy = 0;
                    cur = viewer.unk38[13];
                    if (sp->unk2E0 == 6)
                        cur = viewer.unk38[0];
                    sp->unk2D4 = sp->unk2D0 + cur - 196;
                    func_80016030(7, 975, CD_BUFFER);
                    func_8003ec5c(CD_BUFFER, &sp->unk358, NULL, 1);
                    func_80016030(7, sp->unk2BE + 995, CD_BUFFER);
                    func_8003ec5c(CD_BUFFER, &sp->unk0.vram[0], &sp->unk0.vram[1], 1);
                } else {
                    if (cur == 0xffff) {
                        sp->unk2BE = -1;
                    } else {
                        for (i = 0; i < sp->unk2C2; i++) {
                            if (sp->unk1B0[i] == cur) {
                                sp->unk2BE = i;
                                break;
                            }
                        }
                    }
                    if (sp->unk2BE == -1) {
                        sp->unk2C0 = sp->unk2BE;
                        file = base;
                    } else {
                        sp->unk2C0 = sp->unk1B0[sp->unk2BE];
                        file = sp->unk2C0 + offset;
                    }
                    func_80016030(7, file * 3, CD_BUFFER);
                    func_8003ec5c(CD_BUFFER, &sp->unk0.vram[0], &sp->unk0.vram[1], 1);
                    DrawSync(0);
                    func_80016030(7, file * 3 + 1, CD_BUFFER);
                    func_8003ec5c(CD_BUFFER, &picture, NULL, 1);
                    func_8005a3a8(0);
                }
                DAT_801cd180.unk4 = 0;
                goto moved;
            }
            if (DAT_801cd180.unkC & 0x20) {
                debounce = 1;
                switch (sp->unk2E0) {
                case 0:
                    sp->unk2E0 = 6;
                    break;
                case 5:
                    sp->unk2E0 = 20;
                    break;
                case 6:
                    sp->unk2E0 = 7;
                    break;
                case 7:
                    sp->unk2E0 = 13;
                    break;
                case 12:
                    sp->unk2E0 = 21;
                    break;
                case 13:
                    sp->unk2E0 = 14;
                    break;
                case 20:
                    sp->unk2E0 = 12;
                    break;
                case 21:
                    sp->unk2E0 = 19;
                    break;
                default:
                    if (sp->unk2E0 < 14)
                        sp->unk2E0 += 7;
                    break;
                }
            } else if (DAT_801cd180.unkC & 0x80) {
                debounce = 1;
                switch (sp->unk2E0) {
                case 6:
                    sp->unk2E0 = 0;
                    break;
                case 7:
                    sp->unk2E0 = 6;
                    break;
                case 12:
                    sp->unk2E0 = 20;
                    break;
                case 13:
                    sp->unk2E0 = 7;
                    break;
                case 14:
                    sp->unk2E0 = 13;
                    break;
                case 19:
                    sp->unk2E0 = 21;
                    break;
                case 20:
                    sp->unk2E0 = 5;
                    break;
                case 21:
                    sp->unk2E0 = 12;
                    break;
                default:
                    if (sp->unk2E0 > 5)
                        sp->unk2E0 -= 7;
                    break;
                }
            } else if (DAT_801cd180.unkC & 0x40) {
                debounce = 1;
                switch (sp->unk2E0) {
                case 9:
                    sp->unk2E0 = 11;
                    break;
                case 6:
                    sp->unk2E0 = 1;
                    break;
                case 13:
                    sp->unk2E0 = 15;
                    break;
                default:
                    if (sp->unk2E0 != 5 && sp->unk2E0 != 20 && sp->unk2E0 != 12 && sp->unk2E0 != 21
                        && sp->unk2E0 != 19)
                        sp->unk2E0++;
                    break;
                }
            } else if (DAT_801cd180.unkC & 0x10) {
                debounce = 1;
                switch (sp->unk2E0) {
                case 10:
                    sp->unk2E0 = 8;
                    break;
                case 20:
                case 21:
                    sp->unk2E0 = 11;
                    break;
                default:
                    if (sp->unk2E0 != 0 && sp->unk2E0 != 6 && sp->unk2E0 != 7 && sp->unk2E0 != 13
                        && sp->unk2E0 != 14)
                        sp->unk2E0--;
                    break;
                }
            } else {
                goto idle;
            }
            DAT_801cd180.unkC = 0;
            sp->unk2E4 = 0;
    idle:
            DAT_801cd180.unk4 = 0;
    moved:
            if (sp->unk2E0 != prev) {
                func_80057d10();
                prev = sp->unk2E0;
            }
        } else {
            row = sp->unk2BE;
            if (sp->unk2C7 != 10) {
                if (DAT_801cd180.unk4 & 0x4000) {
                    if (sp->unk2C0 == -1 || vis_table_c.unk0118[sp->unk2C0].unk27 == 0
                        || vis_table_c.unk0118[sp->unk2C0].unk28 != 0
                        || vis_table_c.unk0118[sp->unk2C0].unk26 == 4) {
                        func_80057cc8();
                        func_8005975c(1);
                        func_8005a3a8(0);
                        if (viewer.unk0 < sp->unk2D8)
                            viewer.unk2 = viewer.unk0;
                        if (viewer.unk4 < sp->unk2DA)
                            viewer.unk6 = viewer.unk4;
                        sp->unk2DC = 1;
                        DAT_801cd180.unk4 = 0;
                    } else {
                        func_80057d34();
                        DAT_801cd180.unk4 = 0;
                    }
                } else if (DAT_801cd180.unkC & 0x20) {
                    func_80057d10();
                    if (sp->unk2BE++ < sp->unk2C2 - 1) {
                        sp->unk2C0 = sp->unk1B0[sp->unk2BE];
                    } else {
                        sp->unk2C0 = -1;
                        sp->unk2BE = -1;
                    }
                } else if (DAT_801cd180.unkC & 0x80) {
                    func_80057d10();
                    if (sp->unk2BE < 0)
                        sp->unk2BE = sp->unk2C2;
                    sp->unk2BE--;
                    sp->unk2C0 = sp->unk2BE;
                    if (sp->unk2BE != -1)
                        sp->unk2C0 = sp->unk1B0[sp->unk2BE];
                }
                if (DAT_801cd180.unkC & 0xa0) {
                    debounce = 3;
                    file = base;
                    if (sp->unk2BE != -1)
                        file = sp->unk2C0 + offset;
                    func_80016030(7, file * 3, CD_BUFFER);
                    func_8003ec5c(CD_BUFFER, &sp->unk0.vram[0], &sp->unk0.vram[1], 1);
                    func_80016030(7, file * 3 + 1, CD_BUFFER);
                    func_8003ec5c(CD_BUFFER, &picture, NULL, 1);
                    func_8005a3a8(1);
                    DAT_801cd180.unkC = 0;
                }
            } else {
                if (DAT_801cd180.unkC & 0x20) {
                    debounce = 1;
                    func_80057d10();
                    DAT_801cd180.unkC = 0;
                    if (sp->unk2BE < 15)
                        sp->unk2BE += 3;
                } else if (DAT_801cd180.unkC & 0x80) {
                    debounce = 1;
                    func_80057d10();
                    DAT_801cd180.unkC = 0;
                    if (sp->unk2BE > 1)
                        sp->unk2BE -= 3;
                } else if (DAT_801cd180.unkC & 0x10) {
                    debounce = 1;
                    func_80057d10();
                    DAT_801cd180.unkC = 0;
                    if (sp->unk2BE % 3 != 0)
                        sp->unk2BE--;
                } else if (DAT_801cd180.unkC & 0x40) {
                    debounce = 1;
                    func_80057d10();
                    DAT_801cd180.unkC = 0;
                    if (sp->unk2BE % 3 != 2)
                        sp->unk2BE++;
                } else if (DAT_801cd180.unk4 & 0x4000) {
                    DAT_801cd180.unk4 = 0;
                    if (*sp->unk2D4 & (1 << sp->unk2BE)) {
                        func_80057cc8();
                        sp->unk33C.w = 48;
                        sp->unk33C.h = 24;
                        sp->unk33C.x = (s16)(sp->unk2BE / 3) * 48 + 640;
                        sp->unk33C.y = (s16)(sp->unk2BE % 3) * 24;
                        if (sp->unk2E0 == 6) {
                            n = 740;
                            viewer.unk38[6] = sp->unk2BE;
                        } else {
                            n = 788;
                            viewer.unk38[12] = sp->unk2BE;
                        }
                        MoveImage(&sp->unk33C, n, 124);
                        sp->unk2DC = 1;
                        func_8005a3a8(0);
                    } else {
                        func_80057d34();
                    }
                }
                if (sp->unk2BE != row) {
                    func_80016030(7, sp->unk2BE + 995, CD_BUFFER);
                    func_8003ec5c(CD_BUFFER, &sp->unk0.vram[0], &sp->unk0.vram[1], 1);
                    sp->unk2C0 = sp->unk2BE;
                    func_8005a3a8(1);
                }
            }
            if (DAT_801cd180.unk4 & 0x2000) {
                func_80057cec();
                DAT_801cd180.unk4 = 0;
                func_8005a3a8(0);
                sp->unk2DC = 1;
            } else if (sp->unk2C7 != 10) {
                if (DAT_801cd180.unk2 & 0x1000) {
                    if (sp->unk2C6 == 0)
                        func_80057cc8();
                    sp->unk2C6 = 1;
                } else {
                    sp->unk2C6 = 0;
                }
            }
        }
    next:
        DAT_80088192 = 1;
        func_800164b4();
        func_8002fc4c();
        sp->unk2E4 = (sp->unk2E4 + 1) % 32;
    }
    func_800152f8(sp->unk174[0]);
    func_800152f8(sp->unk174[1]);
    func_800152f8(sp->unk174[2]);
    func_800152f8(sp->unk174[3]);
    func_800152f8(sp->unk174[4]);
    func_800152f8(sp->unk174[5]);
    func_800152f8(sp->unk174[6]);
    func_800152f8(sp->unk174[7]);
    func_800152f8(sp->unk174[8]);
    func_800152f8(sp->unk174[9]);
    func_800152f8(sp->unk2CC);
    func_800152f8(sp->unk2D0);
    func_800152f8(sp->unk1B0);
    func_80058d30();
    func_80057a3c();
}

/*
 * The stored-item page: lists the ids set in DAT_801cceb0 five to a row,
 * three rows shown with their pictures, and shows the detail page of the
 * confirmed one (func_80030ab8 draws the page).
 *
 * Not exact (4 bytes long): register allocation differs from the bit
 * scan on (retail keeps the bit index in $s1, which later holds the row
 * count).
 */
ADDRESS(0x8005c7fc, 0xa60)
void func_8005c7fc(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    DVECTOR image = {896, 448};
    DVECTOR clut = {544, 502};
    DVECTOR detail = {960, 446};
    DVECTOR detail_clut = {560, 498};
    u8 unused[64];
    u32 *bits;
    u32 bit;
    u_long *p;
    int col;
    int row;
    int first;
    int used;
    int size;
    int n;
    int i;
    int k;

    func_80058cc8();
    sp->unk0.store.messages = func_800152d0(21504);
    sp->unk0.store.items = func_800152d0(6644);
    sp->unk0.store.tex[0].vx = 640;
    sp->unk0.store.tex[0].vy = 144;
    sp->unk0.store.tex[1].vx = 640;
    sp->unk0.store.tex[1].vy = 192;
    sp->unk0.store.tex[2].vx = 736;
    sp->unk0.store.tex[2].vy = 144;
    sp->unk0.store.tex[3].vx = 736;
    sp->unk0.store.tex[3].vy = 192;
    sp->unk0.store.tex[4].vx = 832;
    sp->unk0.store.tex[4].vy = 144;
    sp->unk0.store.tex[5].vx = 928;
    sp->unk0.store.tex[5].vy = 0;
    sp->unk0.store.tex[6].vx = 928;
    sp->unk0.store.tex[6].vy = 48;
    sp->unk0.store.tex[7].vx = 928;
    sp->unk0.store.tex[7].vy = 96;
    sp->unk0.store.tex[8].vx = 928;
    sp->unk0.store.tex[8].vy = 144;
    sp->unk0.store.tex[9].vx = 928;
    sp->unk0.store.tex[9].vy = 192;
    sp->unk0.store.tex[10].vx = 640;
    sp->unk0.store.tex[10].vy = 256;
    sp->unk0.store.tex[11].vx = 640;
    sp->unk0.store.tex[11].vy = 304;
    sp->unk0.store.tex[12].vx = 640;
    sp->unk0.store.tex[12].vy = 352;
    sp->unk0.store.tex[13].vx = 640;
    sp->unk0.store.tex[13].vy = 400;
    sp->unk0.store.tex[14].vx = 640;
    sp->unk0.store.tex[14].vy = 448;
    sp->unk0.store.image.vx = 640;
    sp->unk0.store.image.vy = 0;
    sp->unk0.store.kinds = func_800152d0(151);
    sp->unk0.store.count = 0;
    bits = DAT_801cceb0[0];
    bit = 1;
    n = 0;
    for (i = 0; i < 151; i++) {
        if (n == 32) {
            n = 0;
            bit = 1;
            bits++;
        }
        if (*bits & bit)
            sp->unk0.store.kinds[sp->unk0.store.count++] = i;
        bit <<= 1;
        n++;
    }
    sp->unk0.store.rows = sp->unk0.store.count / 5;
    sp->unk0.store.last = sp->unk0.store.count % 5;
    sp->unk2E2 = sp->unk0.store.rows - 2;
    if (sp->unk2E2 < 0)
        sp->unk2E2 = 0;
    p = CD_BUFFER;
    func_80016030(3, 1, p);
    func_80014cfc((u32 *)&menu_messages, (u32 *)(p + 1), 5376);
    p = (u_long *)((u8 *)p + (CD_BUFFER[0] + 4));
    func_8003ec5c(p + 1, &sp->unk2E6[0], &sp->unk2E6[1], 1);
    p = CD_BUFFER;
    func_80016030(3, 2, p);
    func_80014cfc((u32 *)sp->unk0.store.messages, (u32 *)(p + 1), 5376);
    p = (u_long *)((u8 *)p + (CD_BUFFER[0] + 4));
    func_8003ec5c(p + 1, &sp->unk2E6[2], &sp->unk2E6[3], 1);
    func_80016030(3, 5, CD_BUFFER);
    func_8003ec5c(CD_BUFFER, &sp->unk2E6[4], NULL, 1);
    DrawSync(0);
    func_80016030(3, 7, CD_BUFFER);
    func_8003ec5c(CD_BUFFER, &image, &clut, 1);
    DrawSync(0);
    func_80016030(4, 400, CD_BUFFER);
    func_80014cfc((u32 *)sp->unk0.store.items, (u32 *)CD_BUFFER, 1661);
    cd_state.unkA84 = CD_BUFFER;
    n = 15;
    if (sp->unk0.store.count < 16)
        n = sp->unk0.store.count;
    used = 0;
    for (i = 0; i < n; i++) {
        k = sp->unk2E2 * 5 + i;
        if (k < sp->unk0.store.count) {
            if (used + 0x2800 > 0x12000) {
                DrawSync(0);
                cd_state.unkA84 = CD_BUFFER;
                used = 0;
            }
            size = func_80016030(4, sp->unk0.store.kinds[k], cd_state.unkA84);
            func_8003ec5c(cd_state.unkA84, &sp->unk0.store.tex[k % 15], NULL, 1);
            used += size << 11;
            cd_state.unkA84 = (u_long *)((u8 *)cd_state.unkA84 + (size << 11));
        }
    }
    func_80031cfc();
    DrawSync(0);
    func_800172bc();
    sp->unk2DC = 1;
    sp->unk2E0 = 0;
    func_80057a3c();
    for (;;) {
        first = sp->unk2E2;
        if (sp->unk2DC == 1) {
            col = (s16)(sp->unk2E0 % 5);
            row = (s16)(sp->unk2E0 / 5);
            if (DAT_801cd180.unk4 & 0x2000) {
                func_80057cec();
                DAT_801cd180.unk4 = 0;
                func_800152f8(sp->unk0.store.messages);
                func_800152f8(sp->unk0.store.kinds);
                func_800152f8(sp->unk0.store.items);
                func_80058c4c();
                func_80058d30();
                func_80057a3c();
                return;
            }
            if (DAT_801cd180.unkC & 0x20) {
                func_80057d10();
                DAT_801cd180.unkC = 0;
                if (col < 4)
                    sp->unk2E0++;
            }
            if (DAT_801cd180.unkC & 0x80) {
                func_80057d10();
                DAT_801cd180.unkC = 0;
                if (col != 0)
                    sp->unk2E0--;
            }
            if (DAT_801cd180.unkC & 0x10) {
                func_80057d10();
                if (DAT_801cd180.unk2 & 0x8000) {
                    sp->unk2E2 -= 3;
                    if (sp->unk2E2 < 0)
                        sp->unk2E2 = 0;
                } else if (row > 0) {
                    sp->unk2E0 -= 5;
                } else if (sp->unk2E2 > 0) {
                    sp->unk2E2--;
                }
                DAT_801cd180.unkC = 0;
            }
            if (DAT_801cd180.unkC & 0x40) {
                func_80057d10();
                if (DAT_801cd180.unk2 & 0x8000) {
                    if (sp->unk0.store.rows > 2) {
                        sp->unk2E2 += 3;
                        if (sp->unk2E2 > sp->unk0.store.rows - 2)
                            sp->unk2E2 = sp->unk0.store.rows - 2;
                    }
                } else if (sp->unk2E2 + row < sp->unk0.store.rows) {
                    if (row < 2)
                        sp->unk2E0 += 5;
                    else if (sp->unk2E2 <= sp->unk0.store.rows - 3)
                        sp->unk2E2++;
                }
                DAT_801cd180.unkC = 0;
            }
            if (DAT_801cd180.unkC & 0x800) {
                func_80057d10();
                DAT_801cd180.unkC = 0;
                sp->unk2E2 -= 3;
                if (sp->unk2E2 < 0)
                    sp->unk2E2 = 0;
            }
            if (DAT_801cd180.unkC & 0x200) {
                func_80057d10();
                DAT_801cd180.unkC = 0;
                if (sp->unk0.store.rows > 2) {
                    sp->unk2E2 += 3;
                    if (sp->unk2E2 > sp->unk0.store.rows - 2)
                        sp->unk2E2 = sp->unk0.store.rows - 2;
                }
            }
            if (DAT_801cd180.unk4 & 0x5000) {
                func_80057cc8();
                DAT_801cd180.unk4 = 0;
                k = sp->unk2E2 * 5 + sp->unk2E0;
                if (k < sp->unk0.store.count) {
                    if (k != -1) {
                        p = CD_BUFFER;
                        func_80016030(4, sp->unk0.store.kinds[k] + 200, p);
                        func_8003ec5c(p + 1, &sp->unk0.store.image, NULL, 1);
                        p = (u_long *)((u8 *)p + (CD_BUFFER[0] + 4));
                        func_8003ec5c(p + 1, &detail, &detail_clut, 1);
                    }
                    sp->unk2DC = 2;
                }
            }
            if (sp->unk2E2 != first) {
                if (first < sp->unk2E2) {
                    n = sp->unk2E2 - first;
                    k = first + 3;
                } else {
                    n = first - sp->unk2E2;
                    k = sp->unk2E2;
                }
                k *= 5;
                cd_state.unkA84 = CD_BUFFER;
                used = 0;
                for (i = 0; i < n * 5; i++, k++) {
                    if (k < sp->unk0.store.count) {
                        if (used + 0x2800 > 0x12000) {
                            DrawSync(0);
                            cd_state.unkA84 = CD_BUFFER;
                            used = 0;
                        }
                        size = func_80016030(4, sp->unk0.store.kinds[k], cd_state.unkA84);
                        func_8003ec5c(cd_state.unkA84, &sp->unk0.store.tex[k % 15], NULL, 1);
                        used += size << 11;
                        cd_state.unkA84 = (u_long *)((u8 *)cd_state.unkA84 + (size << 11));
                    }
                }
            }
        } else if (DAT_801cd180.unk4 & 0x2000) {
            func_80057cec();
            DAT_801cd180.unk4 = 0;
            sp->unk2DC = 1;
        }
        func_800164b4();
        func_80030ab8();
        sp->unk2E4 = (sp->unk2E4 + 1) % 32;
    }
}

/*
 * The item storage page: the items carried (ids0) and stored (ids1, the
 * count flag bit) as two four-column grids of five rows; confirming a cell
 * twice moves its item to the first free entry of the other list. Leaving
 * writes the flag bits back.
 *
 * Not exact (0x14 bytes long): register allocation and the order of
 * the cell loads differ from the first CD load on.
 */
ADDRESS(0x8005d25c, 0x16c8)
void func_8005d25c(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    RECT clear;
    DVECTOR picture = {640, 0};
    char name0[] = "\\st\\itemcg\\icon1\\0000.tim";
    char name1[] = "\\st\\itemcg\\icon5\\0000.tim";
    DVECTOR image = {896, 448};
    DVECTOR clut = {544, 502};
    RECT move;
    s16 *list;
    s16 r0;
    s16 r1;
    s16 shown;
    s16 n;
    s16 id;
    int used;
    int size;
    int col;
    int row;
    int i;
    int j;
    int k;

    func_80058cc8();
    sp->unk0.grid.messages = func_800152d0(21504);
    func_80016030(3, 1, CD_BUFFER);
    func_80014cfc((u32 *)&menu_messages, (u32 *)(CD_BUFFER + 1), 5376);
    func_8003ec5c((u_long *)((u8 *)CD_BUFFER + (CD_BUFFER[0] + 4)) + 1, &sp->unk2E6[0],
                  &sp->unk2E6[1], 1);
    func_80016030(3, 4, CD_BUFFER);
    func_80014cfc((u32 *)sp->unk0.grid.messages, (u32 *)(CD_BUFFER + 1), 5376);
    func_8003ec5c((u_long *)((u8 *)CD_BUFFER + (CD_BUFFER[0] + 4)) + 1, &sp->unk2E6[2],
                  &sp->unk2E6[3], 1);
    func_80016030(3, 5, CD_BUFFER);
    func_8003ec5c(CD_BUFFER, &sp->unk2E6[4], NULL, 1);
    DrawSync(0);
    func_80016030(3, 7, CD_BUFFER);
    func_8003ec5c(CD_BUFFER, &image, &clut, 1);
    DrawSync(0);
    sp->unk0.grid.count0 = 0;
    sp->unk0.grid.count1 = 0;
    sp->unk0.grid.ids0 = func_800152d0(628);
    sp->unk0.grid.ids1 = func_800152d0(628);
    if (sp->unk0.grid.ids0 == NULL || sp->unk0.grid.ids1 == NULL)
        exit(1);
    fill_words((u32 *)sp->unk0.grid.ids0, -1, 157);
    fill_words((u32 *)sp->unk0.grid.ids1, -1, 157);
    sp->unk0.grid.count0 = 0;
    sp->unk0.grid.count1 = 0;
    for (i = 0; i < 314; i++) {
        if (vis_table_c.unk0118[i].unk5[0] != 0xff && i < 314 && i != 266
            && DAT_801c7590.counts[i].count != 0 && i != viewer.unk38[0] && i != viewer.unk38[1]
            && i != viewer.unk38[2] && i != viewer.unk38[3] && i != viewer.unk38[4]
            && i != viewer.unk38[5] && i != viewer.unk38[7] && i != viewer.unk38[8]
            && i != viewer.unk38[9] && i != viewer.unk38[10] && i != viewer.unk38[11]
            && i != viewer.unk38[13] && i != viewer.unk38[14] && i != viewer.unk38[15]
            && i != viewer.unk38[16] && i != viewer.unk38[17] && i != viewer.unk38[18]
            && i != viewer.unk38[19] && i != viewer.unk38[20]) {
            if (DAT_801c7590.counts[i].flag)
                sp->unk0.grid.ids1[sp->unk0.grid.count1++] = i;
            else
                sp->unk0.grid.ids0[sp->unk0.grid.count0++] = i;
        }
    }
    sp->unk0.grid.tex = func_800152d0(160);
    if (sp->unk0.grid.tex == NULL)
        exit(1);
    for (i = 0; i < 20; i++) {
        sp->unk0.grid.tex[i].vx = (s16)(i / 4) * 56 + 640;
        sp->unk0.grid.tex[i].vy = ((s16)i - (s16)(i / 4) * 4) * 28 + 124;
    }
    for (i = 20, n = 0; i < 29; i++, n += 28) {
        sp->unk0.grid.tex[i].vx = 960;
        sp->unk0.grid.tex[i].vy = n;
    }
    for (i = 29; i < 40; i++) {
        k = i - 29;
        sp->unk0.grid.tex[i].vx = (s16)(k / 9) * 56 + 640;
        sp->unk0.grid.tex[i].vy = ((s16)k - (s16)(k / 9) * 9) * 28 + 256;
    }
    used = 0;
    clear.w = 56;
    clear.h = 28;
    cd_state.unkA84 = CD_BUFFER;
    for (i = 0, n = 40; n--; i++) {
        if (used + 0x1000 > 0x12000) {
            DrawSync(0);
            cd_state.unkA84 = CD_BUFFER;
            used = 0;
        }
        if (i < 20) {
            if (i < sp->unk0.grid.count0) {
                id = sp->unk0.grid.ids0[i];
                goto load;
            }
        } else if (i - 20 < sp->unk0.grid.count1) {
            id = sp->unk0.grid.ids1[i - 20];
        load:
            size = func_80016030(6, vis_table_c.unk0118[id].unk5[0], cd_state.unkA84);
            func_8003ec5c(cd_state.unkA84, &sp->unk0.grid.tex[i], NULL, 1);
            used += size << 11;
            cd_state.unkA84 = (u_long *)((u8 *)cd_state.unkA84 + (size << 11));
            continue;
        }
        clear.x = sp->unk0.grid.tex[i].vx;
        clear.y = sp->unk0.grid.tex[i].vy;
        ClearImage(&clear, 0, 0, 0);
    }
    shown = -1;
    sp->unk2E0 = 0;
    sp->unk0.grid.row0 = 0;
    sp->unk0.grid.row1 = 0;
    sp->unk2DC = 1;
    func_80031cfc();
    DrawSync(0);
    func_800172bc();
    func_80057a3c();
    for (;;) {
        r0 = sp->unk0.grid.row0;
        r1 = sp->unk0.grid.row1;
        if (DAT_801cd180.unk4 & 0x2000) {
            func_80057cec();
            DAT_801cd180.unk4 = 0;
            if (sp->unk2DC & 0x8000) {
                sp->unk2DC &= 0x7fff;
            } else if (sp->unk2DC == 2) {
                sp->unk2DC = 1;
            } else {
                for (i = 0, n = sp->unk0.grid.count0; n != 0; i++) {
                    if (sp->unk0.grid.ids0[i] != -1) {
                        n--;
                        DAT_801c7590.counts[sp->unk0.grid.ids0[i]].flag = 0;
                    }
                }
                for (i = 0, n = sp->unk0.grid.count1; n != 0; i++) {
                    if (sp->unk0.grid.ids1[i] != -1) {
                        n--;
                        DAT_801c7590.counts[sp->unk0.grid.ids1[i]].flag = 1;
                    }
                }
                func_80058c4c();
                func_800152f8(sp->unk0.grid.ids0);
                func_800152f8(sp->unk0.grid.ids1);
                func_800152f8(sp->unk0.grid.tex);
                func_800152f8(sp->unk0.grid.messages);
                func_80058d30();
                func_80057a3c();
                return;
            }
        }
        if (DAT_801cd180.unk4 & 0x4000) {
            func_80057cc8();
            DAT_801cd180.unk4 = 0;
            col = (s16)(sp->unk2E0 % 8);
            row = sp->unk2E0 / 8;
            if (!(sp->unk2DC & 0x8000)) {
                if (col < 4) {
                    k = (sp->unk0.grid.row0 + row) * 4;
                    list = sp->unk0.grid.ids0;
                } else {
                    k = (sp->unk0.grid.row1 + row) * 4 - 4;
                    list = sp->unk0.grid.ids1;
                }
                if (list[k + col] != -1)
                    sp->unk2DC |= 0x8000;
            } else {
                if (col < 4) {
                    for (j = 0; j < sp->unk0.grid.count1; j++) {
                        if (sp->unk0.grid.ids1[j] == -1)
                            break;
                    }
                    k = (s16)(row * 4 + col + sp->unk0.grid.row0 * 4);
                    sp->unk0.grid.ids1[(s16)j] = sp->unk0.grid.ids0[k];
                    sp->unk0.grid.ids0[k] = -1;
                    k %= 20;
                    move.x = sp->unk0.grid.tex[k].vx;
                    move.y = sp->unk0.grid.tex[k].vy;
                    move.w = 56;
                    move.h = 28;
                    if (sp->unk0.grid.row1 * 4 <= (s16)j && (s16)j < sp->unk0.grid.row1 * 4 + 20)
                        MoveImage(&move, sp->unk0.grid.tex[(s16)j % 20 + 20].vx,
                                  sp->unk0.grid.tex[(s16)j % 20 + 20].vy);
                    ClearImage(&move, 0, 0, 0);
                    sp->unk0.grid.count1++;
                    sp->unk0.grid.count0--;
                } else {
                    for (j = 0; j < sp->unk0.grid.count0; j++) {
                        if (sp->unk0.grid.ids0[j] == -1)
                            break;
                    }
                    k = (s16)(row * 4 + col - 4 + sp->unk0.grid.row1 * 4);
                    sp->unk0.grid.ids0[(s16)j] = sp->unk0.grid.ids1[k];
                    sp->unk0.grid.ids1[k] = -1;
                    k %= 20;
                    move.x = sp->unk0.grid.tex[k + 20].vx;
                    move.y = sp->unk0.grid.tex[k + 20].vy;
                    move.w = 56;
                    move.h = 28;
                    if (sp->unk0.grid.row0 * 4 <= (s16)j && (s16)j < sp->unk0.grid.row0 * 4 + 20)
                        MoveImage(&move, sp->unk0.grid.tex[(s16)j % 20].vx,
                                  sp->unk0.grid.tex[(s16)j % 20].vy);
                    ClearImage(&move, 0, 0, 0);
                    sp->unk0.grid.count0++;
                    sp->unk0.grid.count1--;
                }
                sp->unk2DC &= 0x7fff;
            }
        }
        if (!(sp->unk2DC & 0x8000)) {
            if (DAT_801cd180.unk4 & 0x1000) {
                func_80057cc8();
                DAT_801cd180.unk4 = 0;
                if (sp->unk2DC == 1)
                    sp->unk2DC = 2;
                else if (sp->unk2DC == 2)
                    sp->unk2DC = 1;
            }
            if (DAT_801cd180.unkC & 0x20) {
                func_80057d10();
                DAT_801cd180.unkC = 0;
                if (sp->unk2E0 % 8 != 7)
                    sp->unk2E0++;
            }
            if (DAT_801cd180.unkC & 0x80) {
                func_80057d10();
                DAT_801cd180.unkC = 0;
                if (sp->unk2E0 & 7)
                    sp->unk2E0--;
            }
            if (DAT_801cd180.unkC & 0x10) {
                func_80057d10();
                if (DAT_801cd180.unk2 & 0x8000) {
                    if ((s16)(sp->unk2E0 % 8) < 4) {
                        sp->unk0.grid.row0 -= 5;
                        if (sp->unk0.grid.row0 < 0)
                            sp->unk0.grid.row0 = 0;
                    } else {
                        sp->unk0.grid.row1 -= 5;
                        if (sp->unk0.grid.row1 < 0)
                            sp->unk0.grid.row1 = 0;
                    }
                } else if (sp->unk2E0 >= 8) {
                    sp->unk2E0 -= 8;
                } else if ((s16)(sp->unk2E0 % 8) < 4) {
                    if (sp->unk0.grid.row0 > 0)
                        sp->unk0.grid.row0--;
                } else if (sp->unk0.grid.row1 > 0) {
                    sp->unk0.grid.row1--;
                }
                DAT_801cd180.unkC = 0;
            }
            if (DAT_801cd180.unkC & 0x40) {
                func_80057d10();
                if (DAT_801cd180.unk2 & 0x8000) {
                    if ((s16)(sp->unk2E0 % 8) < 4) {
                        sp->unk0.grid.row0 += 5;
                        if (sp->unk0.grid.row0 > 73)
                            sp->unk0.grid.row0 = 73;
                    } else {
                        sp->unk0.grid.row1 += 5;
                        if (sp->unk0.grid.row1 > 73)
                            sp->unk0.grid.row1 = 73;
                    }
                } else if (sp->unk2E0 < 32) {
                    sp->unk2E0 += 8;
                } else if ((s16)(sp->unk2E0 % 8) < 4) {
                    if (sp->unk0.grid.row0 < 73)
                        sp->unk0.grid.row0++;
                } else if (sp->unk0.grid.row1 < 73) {
                    sp->unk0.grid.row1++;
                }
                DAT_801cd180.unkC = 0;
            }
            if (DAT_801cd180.unkC & 0x400) {
                func_80057d10();
                DAT_801cd180.unkC = 0;
                sp->unk0.grid.row0 -= 5;
                if (sp->unk0.grid.row0 < 0)
                    sp->unk0.grid.row0 = 0;
            }
            if (DAT_801cd180.unkC & 0x100) {
                func_80057d10();
                DAT_801cd180.unkC = 0;
                sp->unk0.grid.row0 += 5;
                if (sp->unk0.grid.row0 > 73)
                    sp->unk0.grid.row0 = 73;
            }
            if (DAT_801cd180.unkC & 0x800) {
                func_80057d10();
                DAT_801cd180.unkC = 0;
                sp->unk0.grid.row1 -= 5;
                if (sp->unk0.grid.row1 < 0)
                    sp->unk0.grid.row1 = 0;
            }
            if (DAT_801cd180.unkC & 0x200) {
                func_80057d10();
                DAT_801cd180.unkC = 0;
                sp->unk0.grid.row1 += 5;
                if (sp->unk0.grid.row1 > 73)
                    sp->unk0.grid.row1 = 73;
            }
            if (sp->unk2DC == 2) {
                col = (s16)(sp->unk2E0 % 8);
                row = sp->unk2E0 / 8;
                if (col < 4)
                    id = sp->unk0.grid.ids0[sp->unk0.grid.row0 * 4 + row * 4 + col];
                else
                    id = sp->unk0.grid.ids1[sp->unk0.grid.row1 * 4 + row * 4 + col - 4];
                if (shown != id && id != -1) {
                    shown = id;
                    func_80016030(6, vis_table_c.unk0118[id].unk5[0] + 200, CD_BUFFER);
                    func_8003ec5c(CD_BUFFER, &picture, NULL, 1);
                }
            }
            if (sp->unk0.grid.row0 != r0) {
                if (r0 < sp->unk0.grid.row0) {
                    n = (sp->unk0.grid.row0 - r0) * 4;
                    k = sp->unk0.grid.row0 * 4 - (n - 20);
                } else {
                    n = (r0 - sp->unk0.grid.row0) * 4;
                    k = sp->unk0.grid.row0 * 4;
                }
                used = 0;
                clear.w = 56;
                clear.h = 28;
                cd_state.unkA84 = CD_BUFFER;
                while (n--) {
                    if (used + 0x1000 > 0x12000) {
                        DrawSync(0);
                        cd_state.unkA84 = CD_BUFFER;
                        used = 0;
                    }
                    if (sp->unk0.grid.ids0[k] == -1) {
                        clear.x = sp->unk0.grid.tex[k % 20].vx;
                        clear.y = sp->unk0.grid.tex[k % 20].vy;
                        ClearImage(&clear, 0, 0, 0);
                    } else {
                        size = func_80016030(6, vis_table_c.unk0118[sp->unk0.grid.ids0[k]].unk5[0],
                                             cd_state.unkA84);
                        func_8003ec5c(cd_state.unkA84, &sp->unk0.grid.tex[k % 20], NULL, 1);
                        used += size << 11;
                        cd_state.unkA84 = (u_long *)((u8 *)cd_state.unkA84 + (size << 11));
                    }
                    k++;
                }
            }
            if (sp->unk0.grid.row1 != r1) {
                if (r1 < sp->unk0.grid.row1) {
                    n = (sp->unk0.grid.row1 - r1) * 4;
                    k = sp->unk0.grid.row1 * 4 - (n - 20);
                } else {
                    n = (r1 - sp->unk0.grid.row1) * 4;
                    k = sp->unk0.grid.row1 * 4;
                }
                used = 0;
                clear.w = 56;
                clear.h = 28;
                cd_state.unkA84 = CD_BUFFER;
                while (n--) {
                    if (used + 0x1000 > 0x12000) {
                        DrawSync(0);
                        cd_state.unkA84 = CD_BUFFER;
                        used = 0;
                    }
                    if (sp->unk0.grid.ids1[k] == -1) {
                        clear.x = sp->unk0.grid.tex[k % 20 + 20].vx;
                        clear.y = sp->unk0.grid.tex[k % 20 + 20].vy;
                        ClearImage(&clear, 0, 0, 0);
                    } else {
                        size = func_80016030(6, vis_table_c.unk0118[sp->unk0.grid.ids1[k]].unk5[0],
                                             cd_state.unkA84);
                        func_8003ec5c(cd_state.unkA84, &sp->unk0.grid.tex[k % 20 + 20], NULL, 1);
                        used += size << 11;
                        cd_state.unkA84 = (u_long *)((u8 *)cd_state.unkA84 + (size << 11));
                    }
                    k++;
                }
            }
        }
        func_800164b4();
        func_80031aec();
        sp->unk2E4 = (sp->unk2E4 + 1) % 32;
    }
}

/*
 * Lets the player spend points on the fourteen attributes (two columns of
 * seven, none above 999), then adds them to viewer.unk9A and the total in
 * viewer.unk8. Returns 0 when cancelled, else 1.
 */
ADDRESS(0x8005e924, 0x2ec)
int func_8005e924(int points)
{
    MenuScratch *sp = MENU_SCRATCH;
    int result;
    int i;

    sp->unk0.stats.value = points;
    sp->unk0.stats.selected = 0;
    for (i = 0; i < 14; i++) {
        sp->unk0.stats.base[0][i] = viewer.unk9A[i];
        sp->unk0.stats.bonus[0][i] = 0;
    }
    func_80057a3c();
    do {
        if (DAT_801cd180.unk4 & 0x2000) {
            DAT_801cd180.unk4 = 0;
            func_80057cec();
            result = 0;
            goto done;
        }
        if (DAT_801cd180.unkC & 0x10) {
            DAT_801cd180.unkC = 0;
            func_80057d10();
            if (sp->unk0.stats.selected % 7 == 0)
                sp->unk0.stats.selected += 6;
            else
                sp->unk0.stats.selected--;
        }
        if (DAT_801cd180.unkC & 0x40) {
            DAT_801cd180.unkC = 0;
            func_80057d10();
            if (sp->unk0.stats.selected % 7 == 6)
                sp->unk0.stats.selected -= 6;
            else
                sp->unk0.stats.selected++;
        }
        if (DAT_801cd180.unkC & 0xa0) {
            DAT_801cd180.unkC = 0;
            func_80057d10();
            if (sp->unk0.stats.selected < 7)
                sp->unk0.stats.selected += 7;
            else
                sp->unk0.stats.selected -= 7;
        }
        if (DAT_801cd180.unkC & 0x4000) {
            DAT_801cd180.unkC = 0;
            if (sp->unk0.stats.value > 0) {
                if (sp->unk0.stats.base[0][sp->unk0.stats.selected]
                        + sp->unk0.stats.bonus[0][sp->unk0.stats.selected]
                    < 999) {
                    if (sp->unk0.stats.value >= 2)
                        func_80057cc8();
                    sp->unk0.stats.bonus[0][sp->unk0.stats.selected]++;
                    sp->unk0.stats.value--;
                } else {
                    func_80057d34();
                }
            }
        }
        func_800164b4();
        func_8002a5a4();
    } while (sp->unk0.stats.value > 0);
    func_8002a5a4();
    for (i = 0; i < 14; i++) {
        viewer.unk9A[i] += sp->unk0.stats.bonus[0][i];
        viewer.unk8 += sp->unk0.stats.bonus[0][i];
    }
    result = 1;
done:
    func_80057a3c();
    return result;
}

/*
 * The item use page: lists the usable items held (table C ids 268..313
 * of kind 20 or 30), five rows with their icons, and uses the confirmed
 * one (func_80054a54). Returns the use result (DAT_801cba08.unk16D8), 0 when
 * cancelled; items from 310 and item 288 recompute the viewer's figures.
 */
ADDRESS(0x8005ec10, 0x77c)
int func_8005ec10(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    int result = 0;
    u8 icons[48] = {
        0x00, 0xff, 0x01, 0x02, 0x03, 0xff, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
        0x0a, 0x0b, 0x0c, 0xff, 0xff, 0xff, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0xff,
        0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d,
        0x1e, 0x1f, 0x20, 0x21, 0xff, 0xff, 0xff, 0xff, 0x22, 0x22, 0x22, 0x22,
    };
    RECT rect;
    RECT row;
    u8 *image;
    u_long *p;
    int reload;
    int first;
    int from;
    int to;
    int n;
    int i;
    int k;
    s16 id;

    sp->unk0.page_list.icons[0].vx = 896;
    sp->unk0.page_list.icons[0].vy = 440;
    sp->unk0.page_list.icons[1].vx = 920;
    sp->unk0.page_list.icons[1].vy = 440;
    sp->unk0.page_list.icons[2].vx = 896;
    sp->unk0.page_list.icons[2].vy = 464;
    sp->unk0.page_list.icons[3].vx = 920;
    sp->unk0.page_list.icons[3].vy = 464;
    sp->unk0.page_list.icons[4].vx = 896;
    sp->unk0.page_list.icons[4].vy = 488;
    sp->unk0.block = func_800152d0(21504);
    image = (u8 *)CD_BUFFER;
    func_80016030(3, 3, (u_long *)image);
    func_80014cfc(sp->unk0.block, (u32 *)(image + 4), 5376);
    image += *(u32 *)image + 4;
    func_8003ec5c(image + 4, &sp->unk2E6[2], &sp->unk2E6[3], 1);
    cd_state.unkA84 = CD_BUFFER;
    func_80016030(6, 800, cd_state.unkA84);
    sp->unk0.page_list.entries = func_800152d0(92);
    sp->unk0.page_list.count = 0;
    for (i = 268; i < 314; i++) {
        if ((vis_table_c.unk0118[i].unk26 == 20 || vis_table_c.unk0118[i].unk26 == 30)
            && DAT_801c7590.counts[i].count != 0 && DAT_801c7590.counts[i].flag == 0)
            sp->unk0.page_list.entries[sp->unk0.page_list.count++] = i;
    }
    n = 5;
    if (sp->unk0.page_list.count < 5)
        n = sp->unk0.page_list.count;
    for (i = 0; i < n; i++) {
        k = icons[sp->unk0.page_list.entries[i] - 266];
        p = CD_BUFFER;
        while (k--)
            p = (u_long *)((u8 *)p + (*p + 4));
        rect.x = sp->unk0.page_list.icons[i].vx;
        rect.y = sp->unk0.page_list.icons[i].vy;
        rect.w = rect.h = 24;
        LoadImage(&rect, p + 6);
    }
    sp->unk2E2 = 0;
    sp->unk2E0 = 0;
    func_80057a3c();
    reload = 0;
    for (;;) {
        first = sp->unk2E2;
        if (DAT_801cd180.unk4 & 0x4000) {
            DAT_801cd180.unk4 = 0;
            if (sp->unk2E2 + sp->unk2E0 < sp->unk0.page_list.count)
                func_80054a54(sp->unk0.page_list.entries[sp->unk2E2 + sp->unk2E0]);
            else
                DAT_801cba08.unk16D8 = 2;
            if (DAT_801cba08.unk16D8 == 1)
                func_80057cc8();
            else if (DAT_801cba08.unk16D8 == 2)
                func_80057d34();
            else
                func_80057cec();
            if (DAT_801cba08.unk16D8 != 2) {
                result = DAT_801cba08.unk16D8;
                if (DAT_801cba08.unk16D8 == 1) {
                    id = sp->unk0.page_list.entries[sp->unk2E2 + sp->unk2E0];
                    if (id >= 310 || id == 288)
                        reload = 1;
                }
                break;
            }
        }
        if (DAT_801cd180.unk4 & 0x2000) {
            func_80057cec();
            DAT_801cd180.unk4 = 0;
            break;
        }
        if (DAT_801cd180.unkC & 0x10) {
            func_80057d10();
            DAT_801cd180.unkC = 0;
            if (sp->unk2E0 == 0) {
                if (sp->unk2E2 > 0)
                    sp->unk2E2--;
            } else {
                sp->unk2E0--;
            }
            sp->unk2E4 = 0;
        }
        if (DAT_801cd180.unkC & 0x40) {
            func_80057d10();
            DAT_801cd180.unkC = 0;
            if (sp->unk2E0 == 4) {
                if (sp->unk2E2 < sp->unk0.page_list.count - 5)
                    sp->unk2E2++;
            } else {
                sp->unk2E0++;
            }
            sp->unk2E4 = 0;
        }
        if (DAT_801cd180.unkC & 0x20) {
            func_80057d10();
            DAT_801cd180.unkC = 0;
            sp->unk2E2 += 5;
            if (sp->unk2E2 > sp->unk0.page_list.count - 5) {
                if (sp->unk0.page_list.count < 5)
                    sp->unk2E2 = 0;
                else
                    sp->unk2E2 = sp->unk0.page_list.count - 5;
            }
            sp->unk2E4 = 0;
        }
        if (DAT_801cd180.unkC & 0x80) {
            func_80057d10();
            DAT_801cd180.unkC = 0;
            sp->unk2E2 -= 5;
            if (sp->unk2E2 < 0)
                sp->unk2E2 = 0;
            sp->unk2E4 = 0;
        }
        if (sp->unk2E2 != first) {
            if (first < sp->unk2E2) {
                from = first + 5;
                to = sp->unk2E2 + 5;
            } else {
                from = sp->unk2E2;
                to = first;
            }
            for (i = from; i < to; i++) {
                k = icons[sp->unk0.page_list.entries[i] - 266];
                p = CD_BUFFER;
                while (k--)
                    p = (u_long *)((u8 *)p + (*p + 4));
                row.x = sp->unk0.page_list.icons[i % 5].vx;
                row.y = sp->unk0.page_list.icons[i % 5].vy;
                row.w = row.h = 24;
                LoadImage(&row, p + 6);
            }
        }
        func_800164b4();
        func_8002a1d0();
        sp->unk2E4 = (sp->unk2E4 + 1) % 32;
    }
    func_80057a3c();
    func_800152f8(sp->unk0.block);
    func_800152f8(sp->unk0.page_list.entries);
    if (reload)
        func_8005a3a8(0);
    {
        u8 *restore = (u8 *)CD_BUFFER;
        func_80016030(3, 9, (u_long *)restore);
        func_8003ec5c(restore + 4, NULL, NULL, 0);
        restore += *(u32 *)restore + 4;
        func_8003ec5c(restore + 4, NULL, NULL, 0);
    }
    return result;
}

/*
 * Copies the texture page at 896,256 up to 896,0, then loads the menu
 * messages (CD archive 3 member 0) and their font image, and the message
 * copy and image of member 3.
 */
ADDRESS(0x8005f38c, 0x170)
void func_8005f38c(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    RECT page = {896, 256, 64, 256};
    u8 *p;

    sp->unk2E6[0].vx = 960;
    sp->unk2E6[0].vy = 256;
    sp->unk2E6[1].vx = 560;
    sp->unk2E6[1].vy = 495;
    sp->unk2E6[2].vx = 896;
    sp->unk2E6[2].vy = 256;
    sp->unk2E6[3].vx = 560;
    sp->unk2E6[3].vy = 497;
    sp->unk0.block = func_800152d0(21504);
    MoveImage(&page, 896, 0);
    func_80015ad4();
    p = (u8 *)DAT_80088c88;
    func_80016030(3, 0, (u_long *)p);
    func_80014cfc((u32 *)&menu_messages, (u32 *)(p + 4), 5376);
    p += *(u32 *)p + 4;
    func_8003ec5c(p + 4, &sp->unk2E6[0], &sp->unk2E6[1], 1);
    p = (u8 *)DAT_80088c88;
    func_80016030(3, 3, (u_long *)p);
    func_80014cfc(sp->unk0.block, (u32 *)(p + 4), 5376);
    p += *(u32 *)p + 4;
    func_8003ec5c(p + 4, &sp->unk2E6[2], &sp->unk2E6[3], 1);
    func_80058c04();
    func_80057a3c();
}

/* Copies the saved page back to 896,256. */
ADDRESS(0x8005f4fc, 0x50)
void func_8005f4fc(void)
{
    RECT saved = {896, 0, 64, 256};

    MoveImage(&saved, 896, 256);
}

/*
 * The shop screens over the game (func_8005f38c opens them): unk2DC bit 0
 * lists the shop's stock (DAT_801c7590.unk13A ids with stock left and
 * func_80052d40 set), bits 1/2 the items held but not equipped to trade
 * for items 284/285, otherwise the damaged items held to repair for
 * viewer.unk2. Bit 15 is the confirmation step.
 *
 * Not exact: retail's mode & 6 tests read a word, and the count at 266
 * is updated through a halfword container while preserving adjacent bits.
 * The corresponding shared members currently produce narrower accesses.
 */
ADDRESS(0x8005f54c, 0xacc)
void func_8005f54c(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    DVECTOR pos = {448, 256};
    Vis44 *rec;
    int shown;
    int done;
    int cost;
    int kind;
    int i;

    func_8005f38c();
    sp->unk2E2 = 0;
    sp->unk2E0 = 0;
    if (sp->unk2DC & 1) {
        sp->unk0.list.ids = func_800152d0(28);
        sp->unk0.list.count = 0;
        for (i = 0; i < 14; i++) {
            if (DAT_801c7590.unk156[i] != 0 && func_80052d40(DAT_801c7590.unk13A[i]) != 0)
                sp->unk0.list.ids[sp->unk0.list.count++] = i;
        }
    } else if (sp->unk2DC & 6) {
        sp->unk0.list.ids = func_800152d0(530);
        sp->unk0.list.count = 0;
        for (i = 0; i < 266; i++) {
            if (DAT_801c7590.counts[i].count != 0 && i != viewer.unk38[0] && i != viewer.unk38[1]
                && i != viewer.unk38[2] && i != viewer.unk38[3] && i != viewer.unk38[4]
                && i != viewer.unk38[5] && i != viewer.unk38[7] && i != viewer.unk38[8]
                && i != viewer.unk38[9] && i != viewer.unk38[10] && i != viewer.unk38[11]
                && i != viewer.unk38[13] && i != viewer.unk38[14] && i != viewer.unk38[15]
                && i != viewer.unk38[16] && i != viewer.unk38[17] && i != viewer.unk38[18]
                && i != viewer.unk38[19] && i != viewer.unk38[20])
                sp->unk0.list.ids[sp->unk0.list.count++] = i;
        }
    } else {
        sp->unk0.list.ids = func_800152d0(530);
        sp->unk0.list.count = 0;
        for (i = 0; i < 266; i++) {
            if (DAT_801c7590.counts[i].count != 0
                && vis_table_c.unk0118[i].unk27 != vis_table_c.unk0118[i].unk28)
                sp->unk0.list.ids[sp->unk0.list.count++] = i;
        }
    }
    shown = -1;
    for (;;) {
        if (sp->unk2DC & 0x8000) {
            if (DAT_801cd180.unk4 & 0x2000) {
                func_80057cec();
                DAT_801cd180.unk4 = 0;
                sp->unk2DC &= ~0x8000;
            }
            if (DAT_801cd180.unk4 & 0x4000) {
                DAT_801cd180.unk4 = 0;
                if (sp->unk0.list.unkA == 1) {
                    done = 1;
                    if (sp->unk2DC & 1) {
                        rec = &vis_table_c.unk0118[DAT_801c7590.unk13A[sp->unk0.list.ids[sp->unk2E2 + sp->unk2E0]]];
                        if (rec->unk2 > DAT_801c7590.counts[266].count)
                            goto refuse;
                        DAT_801c7590.counts[266].count -= rec->unk2;
                        DAT_801c7590.counts[DAT_801c7590.unk13A[sp->unk0.list.ids[sp->unk2E2 + sp->unk2E0]]].count++;
                        DAT_801c7590.unk156[sp->unk0.list.ids[sp->unk2E2 + sp->unk2E0]]--;
                        i = sp->unk2E2 + sp->unk2E0;
                        if (DAT_801c7590.unk156[sp->unk0.list.ids[i]] != 0)
                            goto bought;
                        goto compact_buy_test;
                    compact_buy:
                        sp->unk0.list.ids[i] = sp->unk0.list.ids[i + 1];
                        i++;
                    compact_buy_test:
                        if (i < sp->unk0.list.count - 1)
                            goto compact_buy;
                    } else if (sp->unk2DC & 6) {
                        int exchange = 285;

                        if (sp->unk2DC & 2)
                            exchange = 284;
                        DAT_801c7590.counts[sp->unk0.list.ids[sp->unk2E2 + sp->unk2E0]].count--;
                        DAT_801c7590.counts[exchange].count++;
                        i = sp->unk2E2 + sp->unk2E0;
                        goto compact_trade_test;
                    compact_trade:
                        sp->unk0.list.ids[i] = sp->unk0.list.ids[i + 1];
                        i++;
                    compact_trade_test:
                        if (i < sp->unk0.list.count - 1)
                            goto compact_trade;
                    } else {
                        rec = &vis_table_c.unk0118[sp->unk0.list.ids[sp->unk2E2 + sp->unk2E0]];
                        cost = rec->unk3 * 2 / 3 * (rec->unk27 - rec->unk28);
                        if (rec->unk28 == 0)
                            cost *= 2;
                        if (cost >= viewer.unk2)
                            goto refuse;
                        viewer.unk2 -= cost;
                        rec->unk28 = rec->unk27;
                        i = sp->unk2E2 + sp->unk2E0;
                        goto compact_repair_test;
                    compact_repair:
                        sp->unk0.list.ids[i] = sp->unk0.list.ids[i + 1];
                        i++;
                    compact_repair_test:
                        if (i < sp->unk0.list.count - 1)
                            goto compact_repair;
                    }
                    sp->unk0.list.count--;
                bought:
                    func_80057cc8();
                    sp->unk2DC &= ~0x8000;
                    goto moved;
                refuse:
                    done = 0;
                    func_80057d34();
                moved:
                    if (done && sp->unk0.list.count <= sp->unk2E2 + 4) {
                        if (sp->unk2E2 > 0)
                            sp->unk2E2--;
                        else if (sp->unk2E0 > 0)
                            sp->unk2E0--;
                    }
                } else {
                    func_80057cec();
                    DAT_801cd180.unk4 = 0;
                    sp->unk2DC &= ~0x8000;
                }
            }
            if (DAT_801cd180.unkC & 0x80) {
                func_80057d10();
                DAT_801cd180.unkC = 0;
                sp->unk2E4 = 0;
                sp->unk0.list.unkA = 1;
            }
            if (DAT_801cd180.unkC & 0x20) {
                func_80057d10();
                DAT_801cd180.unkC = 0;
                sp->unk2E4 = 0;
                sp->unk0.list.unkA = 0;
            }
        } else {
            if (DAT_801cd180.unk4 & 0x2000) {
                func_80057cec();
                DAT_801cd180.unk4 = 0;
                break;
            }
            if (DAT_801cd180.unkC & 0x10) {
                func_80057d10();
                DAT_801cd180.unkC = 0;
                if (sp->unk2E0 == 0) {
                    if (sp->unk2E2 > 0)
                        sp->unk2E2--;
                } else {
                    sp->unk2E0--;
                }
            }
            if (DAT_801cd180.unkC & 0x40) {
                func_80057d10();
                DAT_801cd180.unkC = 0;
                if (sp->unk2E0 == 4) {
                    if (sp->unk2E2 < sp->unk0.list.count - 5)
                        sp->unk2E2++;
                } else {
                    sp->unk2E0++;
                }
            }
            if (DAT_801cd180.unk4 & 0x4000) {
                DAT_801cd180.unk4 = 0;
                if (sp->unk2E2 + sp->unk2E0 < sp->unk0.list.count) {
                    if (sp->unk2DC & 1)
                        kind = DAT_801c7590.unk13A[sp->unk0.list.ids[sp->unk2E2 + sp->unk2E0]];
                    else
                        kind = sp->unk0.list.ids[sp->unk2E2 + sp->unk2E0];
                    kind = vis_table_c.unk0118[kind].unk5[0];
                    func_80057cc8();
                    sp->unk0.list.unkA = 1;
                    sp->unk2DC |= 0x8000;
                    if (kind != shown) {
                        func_80016030(6, kind + 600, DAT_80088c88);
                        func_8003ec5c(DAT_80088c88, &pos, NULL, 1);
                        shown = kind;
                    }
                } else {
                    func_80057d34();
                }
            }
        }
        func_800164b4();
        func_8002e310();
        sp->unk2E4 = (sp->unk2E4 + 1) % 32;
    }
    func_8005f4fc();
    func_80057a3c();
    func_800152f8(sp->unk0.list.ids);
    func_800152f8(sp->unk0.block);
    func_80058c24();
}

/*
 * The area list page: marks the areas not yet visited (89) and the groups
 * with none visited (99), shows ten rows from the first row and scrolls
 * them while the list is shown (DAT_800881a8).
 */
ADDRESS(0x80060018, 0x31c)
void func_80060018(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    s32 areas[35] = {
        90, 4,  5,  1,  6,  91, 11, 27, 12, 28, 15, 33, 92, 3,  20, 24, 18, 93,
        10, 21, 32, 37, 94, 9,  8,  13, 38, 95, 2,  17, 96, 25, 26, 14, 23,
    };
    s32 *head;
    int seen;
    int moved;
    int i;

    head = NULL;
    func_80057a3c();
    seen = 0;
    sp->unk0.map.unk00 = 0;
    sp->unk0.map.current = -1;
    for (i = 0; i < 35; i++) {
        if (areas[i] >= 90) {
            if (head != NULL && !seen)
                *head = 99;
            head = &areas[i];
            seen = 0;
            continue;
        }
        if (DAT_801cba08.unk1478[areas[i]] & 1)
            seen = 1;
        else
            areas[i] = 89;
        if (sp->unk0.map.current == -1 && areas[i] == area_state.area)
            sp->unk0.map.current = *head;
    }
    if (!seen)
        *head = 99;
    for (i = 0; i < 10; i++)
        sp->unk0.map.ids[i] = areas[i];
    for (;;) {
        moved = 0;
        if (DAT_801cd180.unk4 & 0x2000) {
            DAT_800881a8 = 0;
            DAT_801cd180.unk4 = 0;
            func_80057cec();
            break;
        }
        if (DAT_800881a8 == 1) {
            if (DAT_801cd180.unkC & 0x10) {
                DAT_801cd180.unkC = 0;
                if (sp->unk0.map.unk00 > 0) {
                    func_80057d10();
                    moved = 1;
                    sp->unk0.map.unk00--;
                }
            }
            if (DAT_801cd180.unkC & 0x40) {
                DAT_801cd180.unkC = 0;
                if (sp->unk0.map.unk00 < 25) {
                    func_80057d10();
                    moved = 1;
                    sp->unk0.map.unk00++;
                }
            }
            DAT_8008818c = sp->unk0.map.unk00;
        } else {
            DAT_8008818c = 0;
        }
        if (moved) {
            for (i = 0; i < 10; i++)
                sp->unk0.map.ids[i] = areas[sp->unk0.map.unk00 + i];
        }
        if (DAT_801cd180.unkC & 0x400) {
            DAT_801cd180.unkC = 0;
            DAT_800881a8 = 0;
        }
        if (DAT_801cd180.unkC & 0x800) {
            DAT_801cd180.unkC = 0;
            DAT_800881a8 = 1;
        }
        func_800164b4();
        func_8002a954();
    }
    func_80057a3c();
}

/*
 * Page 1 (unk2DC 1) edits the option bytes DAT_801cd120.unk1 (a 0..63
 * slider) and unk2..unk5 (on/off), row 5 opens page 2. Page 2 lists the
 * twelve button masks (DAT_801cd180.unk22) as button numbers
 * (MenuState.levels); a row waits for the next button (unk2E2), row 12
 * restores the defaults, row 13 returns, and buttons used twice mark
 * levels.flags.
 * Not exact (size-exact): retail keeps the scratchpad in s0 and the
 * decode cursor in s1; this build reverses them and uses different
 * counter registers. The cursor setup before the sound call, slider-first
 * branches and final halfword pointer walk now agree with retail.
 */
ADDRESS(0x80060334, 0x878)
void func_80060334(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    int i;
    int used;
    u8 *option;
    u16 *key;

    sp->unk2E0 = 0;
    sp->unk2DC = 1;
    func_80057a3c();
    while (1) {
        if (sp->unk2DC == 1) {
            if (DAT_801cd180.unk4 & 0x2000) {
                func_80057cec();
                DAT_801cd180.unk4 = 0;
                break;
            }
            if (DAT_801cd180.unkC & 0x10) {
                func_80057d10();
                DAT_801cd180.unkC = 0;
                if (sp->unk2E0 == 0)
                    sp->unk2E0 = 5;
                else
                    sp->unk2E0--;
            }
            if (DAT_801cd180.unkC & 0x40) {
                func_80057d10();
                DAT_801cd180.unkC = 0;
                if (sp->unk2E0 == 5)
                    sp->unk2E0 = 0;
                else
                    sp->unk2E0++;
            }
            if (DAT_801cd180.unk4 & 0x4000) {
                DAT_801cd180.unk4 = 0;
                if (sp->unk2E0 == 5) {
                    const u16 *decode = DAT_801cd180.unk22;
                    func_80057cc8();
                    for (i = 0; i < 12; i++, decode++) {
                        if (*decode & 0x1000)
                            sp->unk0.levels.level[i] = 0;
                        else if (*decode & 0x2000)
                            sp->unk0.levels.level[i] = 1;
                        else if (*decode & 0x8000)
                            sp->unk0.levels.level[i] = 2;
                        else if (*decode & 0x4000)
                            sp->unk0.levels.level[i] = 3;
                        else if (*decode & 0x80)
                            sp->unk0.levels.level[i] = 4;
                        else if (*decode & 0x10)
                            sp->unk0.levels.level[i] = 5;
                        else if (*decode & 0x20)
                            sp->unk0.levels.level[i] = 6;
                        else if (*decode & 0x40)
                            sp->unk0.levels.level[i] = 7;
                        else if (*decode & 0x400)
                            sp->unk0.levels.level[i] = 8;
                        else if (*decode & 0x100)
                            sp->unk0.levels.level[i] = 9;
                        else if (*decode & 0x800)
                            sp->unk0.levels.level[i] = 10;
                        else if (*decode & 0x200)
                            sp->unk0.levels.level[i] = 11;
                    }
                    sp->unk2E0 = 0;
                    sp->unk2DC = 2;
                    sp->unk2E2 = 0;
                    sp->unk0.levels.flags = 0;
                }
            }
            if (DAT_801cd180.unkC & 0x20) {
                option = &DAT_801cd120.unk1;
                func_80057d10();
                DAT_801cd180.unkC = 0;
                if (sp->unk2E0 <= 0) {
                    if (option[sp->unk2E0] < 63)
                        option[sp->unk2E0]++;
                } else if (sp->unk2E0 < 5) {
                    option[sp->unk2E0] = 0;
                }
            }
            if (DAT_801cd180.unkC & 0x80) {
                option = &DAT_801cd120.unk1;
                func_80057d10();
                DAT_801cd180.unkC = 0;
                if (sp->unk2E0 <= 0) {
                    if (option[sp->unk2E0] != 0)
                        option[sp->unk2E0]--;
                } else if (sp->unk2E0 < 5) {
                    option[sp->unk2E0] = 1;
                }
            }
        } else if (sp->unk2E2 == 0) {
            if (DAT_801cd180.unk4 & 0x2000) {
                if (sp->unk0.levels.flags == 0) {
                    func_8005738c();
                    func_80057cc8();
                    DAT_801cd180.unk4 = 0;
                    sp->unk2DC = 1;
                    sp->unk2E0 = 5;
                } else {
                    func_80057d34();
                    DAT_801cd180.unk4 = 0;
                }
            }
            if (DAT_801cd180.unk4 & 0x4000) {
                DAT_801cd180.unk4 = 0;
                if (sp->unk2E0 < 12) {
                    func_80057cc8();
                    sp->unk2E4 = 0;
                    sp->unk2E2 = 1;
                } else if (sp->unk2E0 == 12) {
                    func_80057cc8();
                    func_800573bc();
                    func_8005738c();
                    sp->unk0.levels.level[0] = 5;
                    sp->unk0.levels.level[1] = 7;
                    sp->unk0.levels.level[2] = 4;
                    sp->unk0.levels.level[3] = 6;
                    sp->unk0.levels.level[4] = 8;
                    sp->unk0.levels.level[5] = 10;
                    sp->unk0.levels.level[6] = 9;
                    sp->unk0.levels.level[7] = 11;
                    sp->unk0.levels.level[9] = 2;
                    sp->unk0.levels.flags = 0;
                    sp->unk0.levels.level[8] = 0;
                    sp->unk0.levels.level[11] = 1;
                    sp->unk0.levels.level[10] = 3;
                } else if (sp->unk0.levels.flags == 0) {
                    func_8005738c();
                    func_80057cc8();
                    DAT_801cd180.unk4 = 0;
                    sp->unk2DC = 1;
                    sp->unk2E0 = 5;
                } else {
                    func_80057d34();
                    DAT_801cd180.unk4 = 0;
                }
            }
            if (DAT_801cd180.unk4 & 8) {
                func_80057d10();
                DAT_801cd180.unk4 = 0;
                sp->unk2E0 = 13;
            }
            if (DAT_801cd180.unkC & 0x10) {
                func_80057d10();
                DAT_801cd180.unkC = 0;
                if (sp->unk2E0 == 0)
                    sp->unk2E0 = 12;
                else if (sp->unk2E0 == 6)
                    sp->unk2E0 = 13;
                else if (sp->unk2E0 == 12)
                    sp->unk2E0 = 5;
                else if (sp->unk2E0 == 13)
                    sp->unk2E0 = 11;
                else
                    sp->unk2E0--;
            }
            if (DAT_801cd180.unkC & 0x40) {
                func_80057d10();
                DAT_801cd180.unkC = 0;
                if (sp->unk2E0 == 12)
                    sp->unk2E0 = 0;
                else if (sp->unk2E0 == 5)
                    sp->unk2E0 = 12;
                else if (sp->unk2E0 == 11)
                    sp->unk2E0 = 13;
                else if (sp->unk2E0 == 13)
                    sp->unk2E0 = 6;
                else
                    sp->unk2E0++;
            }
            if (DAT_801cd180.unkC & 0x20) {
                func_80057d10();
                DAT_801cd180.unkC = 0;
                if (sp->unk2E0 < 6)
                    sp->unk2E0 += 6;
                else if (sp->unk2E0 == 12)
                    sp->unk2E0 = 13;
                else if (sp->unk2E0 == 13)
                    sp->unk2E0 = 12;
                else
                    sp->unk2E0 -= 6;
            }
            if (DAT_801cd180.unkC & 0x80) {
                func_80057d10();
                DAT_801cd180.unkC = 0;
                if (sp->unk2E0 < 7)
                    sp->unk2E0 += 6;
                else if (sp->unk2E0 == 12)
                    sp->unk2E0 = 13;
                else if (sp->unk2E0 == 13)
                    sp->unk2E0 = 12;
                else
                    sp->unk2E0 -= 6;
            }
        } else if (DAT_801cd180.unk4) {
            key = &DAT_801cd180.unk22[sp->unk2E0];
            if (DAT_801cd180.unk4 & 0x1000) {
                sp->unk0.levels.level[sp->unk2E0] = 0;
                *key = 0x1000;
            } else if (DAT_801cd180.unk4 & 0x2000) {
                sp->unk0.levels.level[sp->unk2E0] = 1;
                *key = 0x2000;
            } else if (DAT_801cd180.unk4 & 0x8000) {
                sp->unk0.levels.level[sp->unk2E0] = 2;
                *key = 0x8000;
            } else if (DAT_801cd180.unk4 & 0x4000) {
                sp->unk0.levels.level[sp->unk2E0] = 3;
                *key = 0x4000;
            } else if (DAT_801cd180.unk4 & 0x80) {
                sp->unk0.levels.level[sp->unk2E0] = 4;
                *key = 0x80;
            } else if (DAT_801cd180.unk4 & 0x10) {
                sp->unk0.levels.level[sp->unk2E0] = 5;
                *key = 0x10;
            } else if (DAT_801cd180.unk4 & 0x20) {
                sp->unk0.levels.level[sp->unk2E0] = 6;
                *key = 0x20;
            } else if (DAT_801cd180.unk4 & 0x40) {
                sp->unk0.levels.level[sp->unk2E0] = 7;
                *key = 0x40;
            } else if (DAT_801cd180.unk4 & 0x400) {
                sp->unk0.levels.level[sp->unk2E0] = 8;
                *key = 0x400;
            } else if (DAT_801cd180.unk4 & 0x100) {
                sp->unk0.levels.level[sp->unk2E0] = 9;
                *key = 0x100;
            } else if (DAT_801cd180.unk4 & 0x800) {
                sp->unk0.levels.level[sp->unk2E0] = 10;
                *key = 0x800;
            } else if (DAT_801cd180.unk4 & 0x200) {
                sp->unk0.levels.level[sp->unk2E0] = 11;
                *key = 0x200;
            }
            sp->unk0.levels.flags = 0;
            {
                const u16 *scan = DAT_801cd180.unk22;
                used = 15;
                for (i = 0; i < 12; i++, scan++) {
                    if (*scan & used)
                        sp->unk0.levels.flags |= *scan;
                    else
                        used |= *scan;
                }
            }
            DAT_801cd180.unk4 = 0;
            DAT_801cd180.unkC = 0;
            sp->unk2E2 = 0;
        }
        func_800164b4();
        if (sp->unk2DC == 2)
            func_8002b84c();
        else
            func_8002b2e8();
        sp->unk2E4 = (sp->unk2E4 + 1) % 32;
    }
    func_80057a3c();
}

/* Clears any pending SwCARD event. */
ADDRESS(0x80060bac, 0x54)
void func_80060bac(void)
{
    MenuCardScratch *sp = MENU_CARD_SCRATCH;

    TestEvent(sp->sw_events[0]);
    TestEvent(sp->sw_events[1]);
    TestEvent(sp->sw_events[2]);
    TestEvent(sp->sw_events[3]);
}

/* Clears any pending HwCARD event. */
ADDRESS(0x80060c00, 0x54)
void func_80060c00(void)
{
    MenuCardScratch *sp = MENU_CARD_SCRATCH;

    TestEvent(sp->hw_events[0]);
    TestEvent(sp->hw_events[1]);
    TestEvent(sp->hw_events[2]);
    TestEvent(sp->hw_events[3]);
}

ADDRESS(0x80060c54, 0x190)
void func_80060c54(void)
{
    MenuCardScratch *sp;

    EnterCriticalSection();
    MENU_CARD_SCRATCH->sw_events[0] = OpenEvent(SwCARD, EvSpIOE, EvMdNOINTR, NULL);
    MENU_CARD_SCRATCH->sw_events[1] = OpenEvent(SwCARD, EvSpERROR, EvMdNOINTR, NULL);
    MENU_CARD_SCRATCH->sw_events[2] = OpenEvent(SwCARD, EvSpTIMOUT, EvMdNOINTR, NULL);
    MENU_CARD_SCRATCH->sw_events[3] = OpenEvent(SwCARD, EvSpNEW, EvMdNOINTR, NULL);
    MENU_CARD_SCRATCH->hw_events[0] = OpenEvent(HwCARD, EvSpIOE, EvMdNOINTR, NULL);
    MENU_CARD_SCRATCH->hw_events[1] = OpenEvent(HwCARD, EvSpERROR, EvMdNOINTR, NULL);
    MENU_CARD_SCRATCH->hw_events[2] = OpenEvent(HwCARD, EvSpTIMOUT, EvMdNOINTR, NULL);
    MENU_CARD_SCRATCH->hw_events[3] = OpenEvent(HwCARD, EvSpNEW, EvMdNOINTR, NULL);
    sp = MENU_CARD_SCRATCH;
    EnableEvent(sp->sw_events[0]);
    EnableEvent(sp->sw_events[1]);
    EnableEvent(sp->sw_events[2]);
    EnableEvent(sp->sw_events[3]);
    EnableEvent(sp->hw_events[0]);
    EnableEvent(sp->hw_events[1]);
    EnableEvent(sp->hw_events[2]);
    EnableEvent(sp->hw_events[3]);
    ExitCriticalSection();
}

ADDRESS(0x80060de4, 0xf0)
void func_80060de4(void)
{
    MenuCardScratch *sp = MENU_CARD_SCRATCH;

    EnterCriticalSection();
    DisableEvent(sp->sw_events[0]);
    DisableEvent(sp->sw_events[1]);
    DisableEvent(sp->sw_events[2]);
    DisableEvent(sp->sw_events[3]);
    DisableEvent(sp->hw_events[0]);
    DisableEvent(sp->hw_events[1]);
    DisableEvent(sp->hw_events[2]);
    DisableEvent(sp->hw_events[3]);
    CloseEvent(sp->sw_events[0]);
    CloseEvent(sp->sw_events[1]);
    CloseEvent(sp->sw_events[2]);
    CloseEvent(sp->sw_events[3]);
    CloseEvent(sp->hw_events[0]);
    CloseEvent(sp->hw_events[1]);
    CloseEvent(sp->hw_events[2]);
    CloseEvent(sp->hw_events[3]);
    ExitCriticalSection();
}

/* Waits for a SwCARD event: 0 done, 1 error, 2 timeout, 3 new card. */
ADDRESS(0x80060ed4, 0xb4)
int func_80060ed4(void)
{
    MenuCardScratch *sp = MENU_CARD_SCRATCH;
    long ioe, error, timeout, new;

    while (1) {
        ioe = TestEvent(sp->sw_events[0]);
        error = TestEvent(sp->sw_events[1]);
        timeout = TestEvent(sp->sw_events[2]);
        new = TestEvent(sp->sw_events[3]);
        if (ioe == 1)
            return 0;
        if (error == 1)
            return 1;
        if (timeout == 1)
            return 2;
        if (new == 1)
            return 3;
    }
}

/* The same wait on the HwCARD events. */
ADDRESS(0x80060f88, 0xb4)
int func_80060f88(void)
{
    MenuCardScratch *sp = MENU_CARD_SCRATCH;
    long ioe, error, timeout, new;

    while (1) {
        ioe = TestEvent(sp->hw_events[0]);
        error = TestEvent(sp->hw_events[1]);
        timeout = TestEvent(sp->hw_events[2]);
        new = TestEvent(sp->hw_events[3]);
        if (ioe == 1)
            return 0;
        if (error == 1)
            return 1;
        if (timeout == 1)
            return 2;
        if (new == 1)
            return 3;
    }
}

/*
 * Reads port's directory into its MenuCard: when unk2DC's high byte is set
 * it first measures the space all files use, then lists this game's files
 * and records the lowest unused file number in unk204.
 */
ADDRESS(0x8006103c, 0x3ac)
void func_8006103c(int port)
{
    MenuScratch *sp = MENU_SCRATCH;
    MenuCardScratch *cs = (MenuCardScratch *)sp;
    struct DIRENTRY dir[15];
    char path[32];
    int used[8];
    int total, i, n;

    sp->unk1F4 = &cs->cards[port];
    if (sp->unk2DC & 0xff00) {
        strcpy(path, "bu");
        strcat(path, itoa(port));
        strcat(path, "0:*");
        sp->unk1F4->count = 0;
        if (firstfile(path, &dir[sp->unk1F4->count])) {
            do {
                sp->unk1F4->count++;
            } while (nextfile(&dir[sp->unk1F4->count]));
        }
        total = 0;
        for (i = 0; i < sp->unk1F4->count; i++)
            total += dir[i].size;
        if (total > 0x1a000)
            sp->unk1F4->unkE4 = 0;
        else
            sp->unk1F4->unkE4 = 1;
    }
    strcpy(path, "bu");
    strcat(path, itoa(port));
    strcat(path, "0:");
    strcat(path, "BASLUS-00863FS_ST");
    strcat(path, "*");
    sp->unk1F4->count = 0;
    if (firstfile(path, &dir[sp->unk1F4->count])) {
        do {
            sp->unk1F4->count++;
        } while (nextfile(&dir[sp->unk1F4->count]));
    }
    if (sp->unk1F4->count == 0) {
        sp->unk204 = 1;
        if (sp->unk2DC & 0xff00) {
            if (sp->unk1F4->unkE4 == 0)
                sp->unk1F4->status = 5;
            else
                sp->unk1F4->status = 0;
        } else {
            sp->unk1F4->status = 4;
        }
        return;
    }
    for (i = 7; i >= 0; i--)
        used[i] = 0;
    for (i = 0; i < sp->unk1F4->count; i++) {
        strcpy(sp->unk1F4->files[i].name, dir[i].name);
        n = (dir[i].name[17] - '0') * 10 + (dir[i].name[18] - '0');
        used[n] = -1;
        sp->unk1F4->files[i].unk14 = n;
    }
    for (i = 1; i < 8; i++) {
        if (used[i] != -1)
            break;
    }
    sp->unk204 = i;
    sp->unk1F4->status = 0;
}

/*
 * Probes port with _card_info (and, for mode 1, clears and loads it with
 * _card_clear/_card_load), retrying up to ten times, and leaves the result
 * in the port's MenuCard status.
 */
ADDRESS(0x800613e8, 0x190)
void func_800613e8(int port, int mode)
{
    MenuCardScratch *sp = MENU_CARD_SCRATCH;
    int retry;
    int result;

    func_80060c54();
    retry = 9;
    do {
        func_80060bac();
        _card_info(port << 4);
        result = func_80060ed4();
        if (result == 0) {
            sp->cards[port].status = 0;
        } else if (result == 1) {
            if (retry == 0) {
                sp->cards[port].status = 2;
                break;
            }
            continue;
        } else if (result == 2) {
            if (retry == 0) {
                sp->cards[port].status = 2;
                break;
            }
            continue;
        } else if (result == 3) {
            sp->cards[port].status = 1;
        }
        if (mode == 1) {
            func_80060c00();
            _card_clear(port << 4);
            if (func_80060f88() != 0) {
                sp->cards[port].status = 2;
                continue;
            }
            func_80060bac();
            _card_load(port << 4);
            result = func_80060ed4();
            if (result == 1 || result == 2) {
                if (retry == 0) {
                    sp->cards[port].status = 2;
                    break;
                }
                continue;
            }
            if (result == 0) {
                sp->cards[port].status = 0;
            } else if (result == 3) {
                sp->cards[port].status = 3;
            }
            break;
        }
    } while (retry--);
    func_80060de4();
}

/* Probes port; 0 with a message id in unk0 when it has no usable card. */
ADDRESS(0x80061578, 0x60)
int func_80061578(int port)
{
    MenuScratch *sp = MENU_SCRATCH;

    func_800613e8(port, 0);
    if (sp->unk1F4->status == 1) {
        sp->unk0.half[0] = 351;
    } else if (sp->unk1F4->status == 2) {
        sp->unk0.half[0] = 350;
    } else {
        return 1;
    }
    return 0;
}

/*
 * A yes/no dialog over the menu loop: returns the choice in unk202, or -1
 * when the card in the selected port goes away (probed every third frame).
 */
ADDRESS(0x800615d8, 0x120)
int func_800615d8(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    int n = 0;

    sp->unk2E4 = 0;
    sp->unk202 = 0;
    func_80057a3c();
    while (1) {
        if (n == 0) {
            if (func_80061578(sp->unk1F8) == 0)
                return -1;
            n = 3;
        }
        if (DAT_801cd180.unk4 & 0x4000) {
            func_80057cc8();
            DAT_801cd180.unk4 = 0;
            break;
        }
        if (DAT_801cd180.unk4 & 0x2000) {
            func_80057cec();
            DAT_801cd180.unk4 = 0;
            sp->unk202 = 0;
            break;
        }
        if (DAT_801cd180.unk4 & 0x80) {
            func_80057d10();
            DAT_801cd180.unk4 = 0;
            sp->unk202 = 1;
        }
        if (DAT_801cd180.unk4 & 0x20) {
            func_80057d10();
            DAT_801cd180.unk4 = 0;
            sp->unk202 = 0;
        }
        n--;
        func_8002ca84();
    }
    func_80057a3c();
    return sp->unk202;
}

/* The port choice: 1 with unk1F4 set to the chosen port, 0 when cancelled. */
ADDRESS(0x800616f8, 0x154)
int func_800616f8(void)
{
    MenuScratch *sp = MENU_SCRATCH;

    sp->unk1F8 = 0;
    sp->unk2E4 = 0;
    func_80057a3c();
    while (1) {
        if (DAT_801cd180.unk4 & 0x4000) {
            func_80057cc8();
            DAT_801cd180.unk4 = 0;
            sp->unk1F4 = &((MenuCardScratch *)sp)->cards[sp->unk1F8];
            break;
        }
        if (DAT_801cd180.unk4 & 0x2000) {
            func_80057cec();
            DAT_801cd180.unk4 = 0;
            sp->unk1F8 = -1;
            break;
        }
        if (DAT_801cd180.unkC & 0x10) {
            func_80057d10();
            DAT_801cd180.unkC = 0;
            sp->unk1F8 = 0;
            sp->unk2E4 = 0;
        }
        if (DAT_801cd180.unkC & 0x40) {
            func_80057d10();
            DAT_801cd180.unkC = 0;
            sp->unk1F8 = 1;
            sp->unk2E4 = 0;
        }
        func_8002ca84();
        sp->unk2E4 = (sp->unk2E4 + 1) % 32;
    }
    func_80057a3c();
    return sp->unk1F8 != -1;
}

/*
 * Reads each listed file's header bytes into its MenuRecord.
 */
ADDRESS(0x8006184c, 0x1bc)
int func_8006184c(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    int retry;
    char path[32];
    CardHeader header;
    int i, fd;

    retry = 3;
    i = sp->unk1F4->count;
    while (i--) {
        if (func_80061578(sp->unk1F8) == 0)
            return -1;
        strcpy(path, "bu");
        strcat(path, itoa(sp->unk1F8));
        strcat(path, "0:");
        strcat(path, sp->unk1F4->files[i].name);
        fd = open(path, O_RDONLY);
        if (fd == -1) {
            sp->unk0.half[0] = 354;
            return 0;
        }
    read:
        lseek(fd, 0, SEEK_SET);
        if (read(fd, &header, sizeof(header)) == -1) {
            if (retry == 0) {
                close(fd);
                return 0;
            }
            retry--;
            lseek(fd, 0, SEEK_SET);
            goto read;
        }
        (i + sp->unk1F4->files)->kind = header.title.kind;
        (i + sp->unk1F4->files)->unk1A = header.title.unk3A;
        (i + sp->unk1F4->files)->unk1C = header.title.unk3C;
        (i + sp->unk1F4->files)->unk1E = header.title.unk3E;
        close(fd);
    }
    return 1;
}

/*
 * The file list: moves the cursor over the port's files (plus a "new file"
 * row when there is room) and asks for confirmation. Returns 1 with the
 * chosen row in unk1FE, 0 when cancelled, -1 when the card goes away.
 */
ADDRESS(0x80061a08, 0x32c)
int func_80061a08(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    int n = 0;

    if ((sp->unk2DC & 0xff00) && sp->unk1F4->unkE4) {
        sp->unk1FA = sp->unk1F4->count + 1;
        sp->unk1FC = 1;
    } else {
        sp->unk1FA = sp->unk1F4->count;
        sp->unk1FC = 0;
    }
    sp->unk0.half[1] = 0;
    sp->unk1FE = 0;
    sp->unk200 = 0;
    sp->unk2E4 = 0;
    func_80057a3c();
    while (1) {
        if (n == 0) {
            if (func_80061578(sp->unk1F8) == 0)
                return -1;
            n = 20;
        }
        if (DAT_801cd180.unk4 & 0x4000) {
            func_80057cc8();
            DAT_801cd180.unk4 = 0;
            sp->unk0.half[1] = 1;
            sp->unk2E4 = 32;
            sp->unk202 = 1;
            while (1) {
                if (DAT_801cd180.unk4 & 0x4000) {
                    func_80057cc8();
                    DAT_801cd180.unk4 = 0;
                    break;
                }
                if (DAT_801cd180.unk4 & 0x2000) {
                    func_80057cec();
                    DAT_801cd180.unk4 = 0;
                    sp->unk202 = 0;
                    break;
                }
                if (DAT_801cd180.unk4 & 0x80) {
                    func_80057d10();
                    DAT_801cd180.unk4 = 0;
                    sp->unk202 = 1;
                }
                if (DAT_801cd180.unk4 & 0x20) {
                    func_80057d10();
                    DAT_801cd180.unk4 = 0;
                    sp->unk202 = 0;
                }
                func_8002ca84();
            }
            if (sp->unk202)
                break;
            sp->unk0.half[1] = 0;
            continue;
        }
        if (DAT_801cd180.unk4 & 0x2000) {
            func_80057cec();
            DAT_801cd180.unk4 = 0;
            sp->unk1FE = -1;
            break;
        }
        if (DAT_801cd180.unkC & 0x10) {
            func_80057d10();
            DAT_801cd180.unkC = 0;
            if (sp->unk1FE == 0) {
                if (sp->unk200 > 0)
                    sp->unk200--;
            } else {
                sp->unk1FE--;
            }
            sp->unk2E4 = 0;
        }
        if (DAT_801cd180.unkC & 0x40) {
            func_80057d10();
            DAT_801cd180.unkC = 0;
            if (sp->unk1FE == 2) {
                if (sp->unk200 < sp->unk1FA - 3)
                    sp->unk200++;
            } else if (sp->unk1FE + sp->unk200 < sp->unk1FA - 1) {
                sp->unk1FE++;
            }
            sp->unk2E4 = 0;
        }
        func_8002ca84();
        sp->unk2E4 = (sp->unk2E4 + 1) % 32;
        n--;
    }
    func_80057a3c();
    if (sp->unk1FE == -1)
        return 0;
    sp->unk1FE += sp->unk200;
    return 1;
}

/*
 * Not exact (size-exact): the working offset starts after the positive
 * size guard, both checksum loop initializations and the successful-frame
 * updates agree. The input data copy is earlier, and the offset/probe
 * countdown use s2/s1 instead of retail's s1/s2. Both versions keep -1 in s8.
 *
 * Writes size bytes from data to the open file fd at offset, 127 bytes per
 * 128-byte frame with an inverted byte sum in the last byte, reading each
 * frame back to verify it (three tries). Returns the sum of the frame sums,
 * or -1.
 */
ADDRESS(0x80061d34, 0x1b4)
int func_80061d34(int fd, int offset, u8 *data, int size)
{
    MenuScratch *sp = MENU_SCRATCH;
    u8 frame[128];
    u8 check[128];
    int retry = 0;
    int n = 0;
    u8 total = 0;
    u8 sum;
    int i;

    if (size > 0) {
        int frame_offset = offset;

        do {
            frame[127] = i = 0;
            for (; i < 127; i++) {
                frame[i] = *data;
                frame[127] += *data++;
            }
            frame[127] = ~frame[127];
        again:
            if (n == 0) {
                if (func_80061578(sp->unk1F8) == 0)
                    return -1;
                n = 3;
            }
            if (retry >= 3) {
                sp->unk0.half[0] = 355;
                return -1;
            }
            retry++;
            lseek(fd, frame_offset, SEEK_SET);
            if (write(fd, frame, 128) == -1)
                goto again;
            lseek(fd, frame_offset, SEEK_SET);
            if (read(fd, check, 128) == -1)
                goto again;
            sum = i = 0;
            for (; i < 127; i++)
                sum += check[i];
            sum = ~sum;
            if (sum != frame[127])
                goto again;
            size -= 127;
            frame_offset += 128;
            retry = 0;
            total += sum;
            n--;
        } while (size > 0);
    }
    return total;
}

/*
 * Reads size bytes back the same way, verifying each frame's sum.
 * Not exact (size-exact): the working offset now starts after the positive-
 * size guard. The data, offset, frame base and probe countdown still use
 * different saved registers, and the checksum counters are zeroed in a
 * different order. A wide checksum or explicit frame pointer diverges more.
 */
ADDRESS(0x80061ee8, 0x174)
int func_80061ee8(int fd, int offset, u8 *data, int size)
{
    MenuScratch *sp = MENU_SCRATCH;
    u8 frame[128];
    int retry = 0;
    int n = 0;
    u8 total = 0;
    u8 sum;
    int i;

    if (size > 0) {
        int frame_offset = offset;

        do {
        again:
            if (n == 0) {
                if (func_80061578(sp->unk1F8) == 0)
                    return -1;
                n = 3;
            }
            if (retry >= 3)
                return -1;
            retry++;
            lseek(fd, frame_offset, SEEK_SET);
            if (read(fd, frame, 128) == -1) {
                sp->unk0.half[0] = 354;
                goto again;
            }
            sum = 0;
            for (i = 0; i < 127; i++)
                sum += frame[i];
            sum = ~sum;
            if (sum != frame[127]) {
                sp->unk0.half[0] = 349;
                goto again;
            }
            for (i = 0; i < 127; i++)
                *data++ = frame[i];
            size -= 127;
            frame_offset += 128;
            retry = 0;
            n--;
            total += frame[127];
        } while (size > 0);
    }
    return total;
}

/*
 * Loads the selected save file (row unk1FE) into data: opens it, skips the
 * header and reads 7560 bytes, checking the sum against the record's.
 */
ADDRESS(0x8006205c, 0x1cc)
int func_8006205c(u8 *data)
{
    MenuScratch *sp = MENU_SCRATCH;
    char path[32];
    CardHeader header;
    int retry = 3;
    int fd, sum;

open:
    strcpy(path, "bu");
    strcat(path, itoa(sp->unk1F8));
    strcat(path, "0:");
    strcat(path, sp->unk1F4->files[sp->unk1FE].name);
    fd = open(path, O_WRONLY);
    if (fd == -1) {
        if (retry) {
            retry--;
            goto open;
        }
        sp->unk0.half[0] = 354;
        return 0;
    }
read:
    lseek(fd, 0, SEEK_SET);
    if (read(fd, &header, sizeof(header)) == -1) {
        if (retry) {
            retry--;
            lseek(fd, 0, SEEK_SET);
            goto read;
        }
        sp->unk0.half[0] = 354;
        close(fd);
        return 0;
    }
    sum = func_80061ee8(fd, sizeof(header), data, 7560);
    if (sum == -1 || sum != sp->unk1F4->files[sp->unk1FE].unk1E) {
        if (sum != sp->unk1F4->files[sp->unk1FE].unk1E)
            sp->unk0.half[0] = 349;
        if (retry == 0) {
            func_80057a3c();
            close(fd);
            return 0;
        }
        retry--;
        lseek(fd, 0, SEEK_SET);
        goto read;
    }
    close(fd);
    return 1;
}

/*
 * Not exact (99.7%): the header's icon byte is stored before the unk1FE
 * load that retail schedules first (li v0,18 / sb for lhu v0,510; li v1,18).
 *
 * Saves to the selected file, or to a new file numbered unk204: builds the
 * header (the title with the file number, the area name and the play
 * time, an icon by file number), writes the 7560 bytes of data after it,
 * then the header again with the data's sum, and records the position the
 * game resumes from.
 */
ADDRESS(0x80062228, 0x998)
int func_80062228(u8 *data)
{
    MenuScratch *sp = MENU_SCRATCH;
    int retry = 3;
    char num[8];
    char path[32];
    CardHeader header;
    u16 cluts[7][16] = {
    {
        0x7fff, 0x03bf, 0x033f, 0x05dd, 0x027d, 0x0275, 0x060e, 0x060c,
        0x05e9, 0x09a6, 0x1560, 0x0ce0, 0x0000, 0x0000, 0x0000, 0x0000,
    },
    {
        0x7fff, 0x019b, 0x6739, 0x1018, 0x0535, 0x00f2, 0x1991, 0x0c10,
        0x112d, 0x00ac, 0x008a, 0x09a6, 0x1560, 0x0ce0, 0x0000, 0x249c,
    },
    {
        0x7fff, 0x6739, 0x5aff, 0x00f2, 0x019b, 0x0ce0, 0x1560, 0x2200,
        0x5a23, 0x28c0, 0x5180, 0x6f9f, 0x1018, 0x0000, 0x0d20, 0x2a40,
    },
    {
        0x7fff, 0x2d6b, 0x4210, 0x4631, 0x4e73, 0x5ad6, 0x6739, 0x739c,
        0x6e80, 0x76c9, 0x5180, 0x0000, 0x0421, 0x0000, 0x0000, 0x0000,
    },
    {
        0x7fff, 0x39ce, 0x5294, 0x0421, 0x6739, 0x0ce7, 0x1fbd, 0x02d6,
        0x0e52, 0x0dac, 0x0e69, 0x0d85, 0x0d24, 0x08a2, 0x0ce0, 0x0000,
    },
    {
        0x7fff, 0x00f2, 0x10eb, 0x019b, 0x2e15, 0x25d3, 0x3678, 0x25d2,
        0x2e36, 0x1d90, 0x1db2, 0x112d, 0x08a7, 0x0000, 0x0000, 0x0000,
    },
    {
        0x4500, 0x2108, 0x318c, 0x4e73, 0x739c, 0x263d, 0x0d98, 0x019b,
        0x04ca, 0x473f, 0x010d, 0x5962, 0x2ca0, 0x2418, 0x49bc, 0x0000,
    },
    };
    u8 icons[7][0x100] = {
    {
        0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0x6c, 0x65, 0x69, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
        0x5c, 0x57, 0x59, 0xc9, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0x95, 0x96, 0x66, 0xcc, 0xcc, 0xcc, 0xcc,
        0xcc, 0xc7, 0x6a, 0x9a, 0xc9, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0x56, 0xc6, 0xcc, 0xcc, 0xcc,
        0xcc, 0xcc, 0xcc, 0x9a, 0x99, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0x56, 0x65, 0xcc, 0xcc, 0xcc,
        0xcc, 0xcc, 0xac, 0x99, 0xa9, 0xc9, 0xcc, 0xcc, 0xcc, 0xcc, 0x56, 0x55, 0x65, 0x98, 0xcc, 0xcc,
        0xcc, 0xac, 0xaa, 0x9a, 0x56, 0x59, 0xc9, 0xcc, 0xcc, 0x56, 0x55, 0x55, 0x59, 0x59, 0x69, 0xcc,
        0xac, 0x39, 0x33, 0x33, 0x59, 0x69, 0x59, 0xc9, 0x56, 0x55, 0x55, 0x65, 0x6a, 0xca, 0x6a, 0xca,
        0x89, 0xcc, 0xcc, 0x89, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
        0xcc, 0x56, 0x96, 0xc6, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0x7c, 0x95, 0x95, 0xcc, 0xcc, 0xcc, 0xcc,
        0xcc, 0x56, 0x69, 0x59, 0xc6, 0xcc, 0xcc, 0xcc, 0xcc, 0x7c, 0xac, 0x56, 0xc6, 0xcc, 0xcc, 0xcc,
        0xcc, 0xcc, 0xcc, 0x9a, 0xc9, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0x56, 0xc6, 0xcc, 0xcc, 0xcc,
        0xcc, 0xcc, 0xcc, 0x9a, 0xc9, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0x56, 0xc6, 0xcc, 0xcc, 0xcc,
        0xcc, 0xcc, 0xac, 0x99, 0xa9, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0x56, 0x55, 0x65, 0x98, 0xcc, 0xcc,
        0xcc, 0xac, 0x44, 0x94, 0x56, 0x59, 0xc9, 0xcc, 0xcc, 0x46, 0x2a, 0x42, 0x59, 0x59, 0x69, 0xcc,
        0xac, 0x39, 0x1b, 0x31, 0x59, 0x69, 0x59, 0xc9, 0x56, 0x55, 0x55, 0x65, 0x6a, 0xca, 0x6a, 0xca,
        0x89, 0xcc, 0xcc, 0x89, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    },
    {
        0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee,
        0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee,
        0xee, 0xbe, 0xee, 0xed, 0xed, 0xbe, 0xee, 0xee, 0xee, 0xee, 0xdc, 0x14, 0xd4, 0xec, 0xeb, 0xee,
        0xee, 0xee, 0x5b, 0x11, 0x51, 0xeb, 0xec, 0xee, 0xee, 0xee, 0x4e, 0x11, 0x41, 0xee, 0xee, 0xee,
        0xee, 0xee, 0x14, 0xc0, 0x10, 0xe4, 0xee, 0xee, 0xee, 0xee, 0x14, 0x11, 0x11, 0xe4, 0xee, 0xee,
        0xee, 0xee, 0x45, 0x44, 0x44, 0xe5, 0xee, 0xee, 0xee, 0xae, 0x99, 0x99, 0x99, 0xa9, 0xee, 0xee,
        0xee, 0xee, 0xee, 0x68, 0xe8, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0x8e, 0xee, 0xee, 0xee, 0xee,
        0xee, 0xee, 0xee, 0x9e, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee,
        0xee, 0xee, 0xee, 0xef, 0xef, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xf3, 0xe3, 0xee, 0xee, 0xee,
        0xee, 0xee, 0xce, 0x37, 0xc7, 0xee, 0xee, 0xee, 0xee, 0xce, 0xbe, 0xbe, 0xbe, 0xce, 0xee, 0xee,
        0xee, 0xeb, 0xcd, 0xbd, 0xcd, 0xed, 0xeb, 0xee, 0xee, 0xee, 0xde, 0x14, 0xd4, 0xee, 0xee, 0xee,
        0xee, 0xee, 0x5e, 0x11, 0x51, 0xee, 0xee, 0xee, 0xee, 0xee, 0x4e, 0x02, 0x42, 0xee, 0xee, 0xee,
        0xee, 0xee, 0x24, 0xc0, 0x20, 0xe4, 0xee, 0xee, 0xee, 0xee, 0x14, 0x02, 0x12, 0xe4, 0xee, 0xee,
        0xee, 0xee, 0x45, 0x44, 0x44, 0xe5, 0xee, 0xee, 0xee, 0xae, 0x99, 0x99, 0x99, 0xa9, 0xee, 0xee,
        0xee, 0xee, 0xee, 0x68, 0xe8, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0x8e, 0xee, 0xee, 0xee, 0xee,
        0xee, 0xee, 0xee, 0x9e, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee,
    },
    {
        0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
        0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
        0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xd7, 0xdd, 0xdd, 0xdd,
        0xdd, 0xfe, 0x67, 0x6d, 0x7d, 0xdf, 0xdd, 0xdd, 0x5d, 0x76, 0x56, 0xd6, 0x67, 0xfe, 0xdd, 0xdd,
        0x80, 0x5b, 0x65, 0x75, 0x56, 0x6d, 0xdd, 0xdd, 0x98, 0xea, 0xf7, 0xe6, 0xd5, 0x5d, 0xdd, 0xdd,
        0xab, 0x62, 0x7f, 0x5e, 0xdd, 0x6d, 0xdd, 0xdd, 0x65, 0xe5, 0xe6, 0xe5, 0xdd, 0xe5, 0xdd, 0xdd,
        0x7d, 0x56, 0x55, 0xd6, 0xdd, 0xd6, 0xdd, 0xdd, 0xdd, 0x7f, 0xe6, 0xdd, 0x5d, 0xd7, 0xdd, 0xdd,
        0xdd, 0xdd, 0xdd, 0x5d, 0x6e, 0x64, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
        0xdd, 0x11, 0x11, 0xdd, 0x6d, 0xdd, 0xdd, 0xdd, 0x0d, 0xb8, 0x00, 0xd2, 0x4d, 0xd7, 0xdd, 0xdd,
        0x80, 0xa9, 0xc0, 0xc0, 0x4d, 0xdf, 0xdd, 0xdd, 0xb0, 0x2a, 0x00, 0x1c, 0x3d, 0xd6, 0xdd, 0xdd,
        0x00, 0x00, 0x20, 0x20, 0x6d, 0xd5, 0xdd, 0xdd, 0x01, 0xc0, 0x00, 0x2c, 0x56, 0xdd, 0xdd, 0xdd,
        0x1d, 0x2c, 0x1c, 0x52, 0xdd, 0x76, 0xdd, 0xdd, 0xdd, 0x11, 0xc2, 0xe5, 0x56, 0xdd, 0xdd, 0xdd,
        0xdd, 0xdd, 0xdd, 0x76, 0xdf, 0xdd, 0xdd, 0xdd, 0x6d, 0x44, 0x53, 0x5d, 0x76, 0xd6, 0xdd, 0xdd,
        0xdd, 0x6f, 0xd5, 0xdd, 0x65, 0x6f, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0x57, 0xdd, 0xdd,
        0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xd6, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0x5d, 0xde, 0xdd, 0xdd,
        0xdd, 0xdd, 0xdd, 0x5d, 0x6e, 0x54, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
    },
    {
        0xbb, 0xbb, 0xbb, 0xbb, 0x5b, 0x76, 0xb0, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xb5, 0xbb, 0x0b, 0xbb,
        0xbb, 0xb7, 0xbb, 0x5b, 0xb7, 0xbb, 0xbb, 0xb0, 0x6b, 0xb0, 0xbb, 0x3b, 0x60, 0x0b, 0xbb, 0xb7,
        0x7b, 0x60, 0x77, 0x67, 0x70, 0xbb, 0xb7, 0xb7, 0x05, 0x00, 0x00, 0x00, 0x00, 0xb5, 0xb6, 0xb6,
        0xc6, 0x00, 0x03, 0x00, 0x0c, 0x36, 0xbb, 0xb6, 0x01, 0x50, 0x37, 0x00, 0x00, 0xb1, 0xbb, 0xb5,
        0x05, 0x00, 0x00, 0x00, 0x00, 0xb5, 0x5b, 0xbb, 0x6b, 0x00, 0x00, 0x00, 0x70, 0xbb, 0x3b, 0xbb,
        0xbb, 0x76, 0x77, 0x77, 0x67, 0xb3, 0xb3, 0xbb, 0xbb, 0xbb, 0x53, 0x76, 0x00, 0x27, 0xbb, 0xbb,
        0xbb, 0xbb, 0x64, 0x07, 0x70, 0xb5, 0xbb, 0xbb, 0xbb, 0xbb, 0x5b, 0x76, 0x56, 0xbb, 0xbb, 0xbb,
        0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb,
        0xbb, 0xbb, 0xbb, 0xbb, 0x5b, 0x76, 0xb0, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xb6, 0xbb, 0x0b, 0xbb,
        0xbb, 0xbb, 0xbb, 0x6b, 0xbb, 0xbb, 0xbb, 0xb0, 0xbb, 0xbb, 0xbb, 0x5b, 0xbb, 0x7b, 0xbb, 0xb7,
        0xbb, 0xbb, 0xbb, 0x4b, 0xb2, 0xbb, 0xb6, 0xb7, 0x67, 0xb0, 0xbb, 0xbb, 0x60, 0xb7, 0xb5, 0xb6,
        0x7b, 0x60, 0x77, 0x67, 0x70, 0x42, 0xbb, 0xb6, 0x05, 0x00, 0x00, 0x00, 0x00, 0xbb, 0xbb, 0xb5,
        0xc6, 0x0c, 0x03, 0xc0, 0x0c, 0xb6, 0x4b, 0xbb, 0x01, 0x50, 0x37, 0x00, 0x00, 0xb1, 0x3b, 0xbb,
        0x05, 0x00, 0x90, 0x00, 0x00, 0xb5, 0xb2, 0xbb, 0x6b, 0x00, 0x80, 0x00, 0x70, 0x26, 0xbb, 0xbb,
        0xbb, 0x76, 0xa7, 0x77, 0x67, 0xb5, 0xbb, 0xbb, 0xbb, 0xbb, 0x4b, 0x65, 0x56, 0xbb, 0xbb, 0xbb,
        0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb,
    },
    {
        0xff, 0xff, 0xdf, 0xab, 0xdb, 0xff, 0xff, 0xff, 0xff, 0xff, 0xbd, 0xaa, 0xba, 0xfd, 0xff, 0xff,
        0x20, 0xff, 0xac, 0xac, 0xac, 0xfc, 0x2f, 0xf0, 0x02, 0xd4, 0x3c, 0xb3, 0x3c, 0xdc, 0x04, 0xf2,
        0x1f, 0xcf, 0x63, 0x87, 0x9d, 0xc7, 0x1f, 0xff, 0xff, 0xbf, 0x33, 0x33, 0x73, 0xb5, 0xff, 0xff,
        0xff, 0xbf, 0x33, 0x33, 0x33, 0xb3, 0xff, 0xff, 0xff, 0xcf, 0x79, 0x33, 0x67, 0xb5, 0xff, 0xff,
        0xff, 0xcf, 0x57, 0x3c, 0xe8, 0xc8, 0xff, 0xff, 0xff, 0xcf, 0xc3, 0xcc, 0x33, 0xcc, 0xff, 0xff,
        0xff, 0xdf, 0xac, 0xab, 0xaa, 0xdc, 0xff, 0xff, 0xff, 0xff, 0xbc, 0xaa, 0xba, 0xfc, 0xff, 0xff,
        0xff, 0xff, 0xdf, 0xcc, 0xdc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xdf, 0xab, 0xdb, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xcd, 0xaa, 0xba, 0xfd, 0xff, 0xff, 0xff, 0xff, 0xac, 0xab, 0xac, 0xfc, 0xff, 0xff,
        0x1f, 0xd4, 0x3c, 0xc3, 0x3c, 0xdb, 0x14, 0xff, 0x04, 0xcf, 0x63, 0x7e, 0x9d, 0xc7, 0x0f, 0xf4,
        0x10, 0xbf, 0x93, 0x97, 0x73, 0xc5, 0x1f, 0xf0, 0xff, 0xbf, 0x33, 0x33, 0x33, 0xb3, 0xff, 0xff,
        0xff, 0xbf, 0x79, 0x33, 0x33, 0xb3, 0xff, 0xff, 0xff, 0xcf, 0x57, 0x3c, 0x78, 0xc8, 0xff, 0xff,
        0xff, 0xcf, 0xc3, 0xcb, 0x33, 0xcc, 0xff, 0xff, 0xff, 0xdf, 0xab, 0xac, 0xaa, 0xdb, 0xff, 0xff,
        0xff, 0xff, 0xbc, 0xaa, 0xca, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xdf, 0xcc, 0xdc, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    },
    {
        0xdd, 0xdd, 0x2b, 0x2d, 0xdb, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0x5b, 0x6c, 0xb5, 0xdd, 0xdd, 0xdd,
        0xdd, 0xdd, 0xc2, 0x68, 0xb6, 0xdc, 0xdd, 0xdd, 0xdd, 0xdd, 0x87, 0x66, 0x86, 0xb5, 0xdd, 0xdd,
        0xdd, 0xbd, 0x68, 0x6c, 0x66, 0x58, 0xdc, 0xdd, 0xdd, 0x82, 0x66, 0x66, 0x66, 0x86, 0xdb, 0xdd,
        0xdd, 0x7c, 0x66, 0x66, 0x67, 0x66, 0xc5, 0xdd, 0xdd, 0x2d, 0x9b, 0x77, 0x66, 0x66, 0x28, 0xdd,
        0xdd, 0xdd, 0x5c, 0x68, 0x66, 0x66, 0x28, 0xdd, 0xdd, 0xdd, 0x82, 0xb6, 0x66, 0x66, 0xb8, 0xdd,
        0xd0, 0xd0, 0x82, 0x6c, 0x66, 0x66, 0xb8, 0xdd, 0xd0, 0xd0, 0x8b, 0x66, 0x66, 0x66, 0x58, 0xdb,
        0xdd, 0xdd, 0x52, 0x85, 0x88, 0x58, 0xb5, 0xdd, 0xdd, 0xdd, 0xcd, 0x2c, 0x22, 0xc2, 0xdc, 0xdd,
        0x1d, 0xd3, 0x2c, 0xcd, 0xcd, 0xcd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
        0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xcd, 0xb2, 0x2b, 0xdc, 0xdd, 0xdd,
        0xdd, 0x22, 0x4c, 0x44, 0x24, 0xcb, 0xdc, 0xdd, 0xbd, 0xc8, 0x22, 0x22, 0x42, 0x42, 0xc5, 0xdd,
        0xad, 0x66, 0x48, 0x44, 0x68, 0x66, 0x24, 0xdd, 0xdd, 0x6a, 0x66, 0x66, 0x66, 0x66, 0x24, 0xdd,
        0xdd, 0xca, 0x6c, 0xc6, 0x6c, 0x66, 0xb4, 0xdd, 0xdd, 0x6a, 0x66, 0x66, 0x66, 0x68, 0xb8, 0xdd,
        0xdd, 0xad, 0x66, 0x66, 0x86, 0x65, 0xb8, 0xdd, 0xdd, 0x8a, 0xb7, 0x9b, 0x57, 0x66, 0x28, 0xdd,
        0xdd, 0x6b, 0x28, 0xa2, 0x68, 0x66, 0xc4, 0xdd, 0xdd, 0x5a, 0x6b, 0xb6, 0xa5, 0x66, 0x54, 0xdb,
        0xdd, 0x7b, 0x84, 0x48, 0x48, 0x48, 0xb7, 0xdd, 0xdd, 0xcd, 0x22, 0xbb, 0xbb, 0x22, 0xdc, 0xdd,
        0xdd, 0xdc, 0xdc, 0xdc, 0xcd, 0xcd, 0xcd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
    },
    {
        0xff, 0xff, 0x9f, 0xff, 0xff, 0xf9, 0xff, 0xff, 0xff, 0xff, 0xff, 0x85, 0x51, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0x9f, 0xf5, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x61, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xb2, 0x95, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1f, 0x0b, 0xc6, 0xf0, 0xff, 0xff,
        0xff, 0xff, 0x2f, 0xcf, 0xf0, 0xfb, 0xff, 0xff, 0xff, 0xff, 0x1f, 0x8c, 0x0c, 0xff, 0xff, 0xff,
        0xff, 0xff, 0x7f, 0x7f, 0x75, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf1, 0xf5, 0xf9, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xf3, 0xf0, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf2, 0x0c, 0xfb, 0xff, 0xff, 0xff,
        0xff, 0x3f, 0xf1, 0xcf, 0xfb, 0xff, 0xff, 0xff, 0xff, 0x1f, 0x42, 0xff, 0xfb, 0x43, 0xff, 0xff,
        0xff, 0xf4, 0x31, 0xc4, 0x40, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0x9f, 0xff, 0xff, 0xf9, 0xef, 0xff, 0xff, 0xff, 0xff, 0x85, 0x51, 0xff, 0xef, 0xfd,
        0xff, 0xff, 0xff, 0x9f, 0xf5, 0xef, 0xff, 0xff, 0xff, 0xff, 0xff, 0x6f, 0xff, 0xf7, 0x3f, 0xf4,
        0xff, 0xff, 0x71, 0xb3, 0x15, 0x2b, 0x11, 0xf2, 0xff, 0x21, 0xcf, 0x0b, 0x06, 0xfc, 0x2f, 0xf1,
        0x1f, 0xff, 0xff, 0xcc, 0xc0, 0xff, 0x3f, 0xf1, 0xff, 0xff, 0xff, 0x8f, 0xfc, 0xff, 0x3f, 0xf1,
        0xff, 0xff, 0xff, 0x7f, 0xf5, 0xff, 0x4f, 0xff, 0xff, 0xff, 0xff, 0xf5, 0xf9, 0xff, 0x4f, 0xff,
        0xff, 0xff, 0xff, 0xf0, 0xf0, 0xff, 0x42, 0xff, 0xff, 0xff, 0xff, 0x0c, 0xfb, 0xff, 0xf3, 0xff,
        0xff, 0xff, 0xff, 0xcf, 0xfb, 0xff, 0xf4, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfb, 0x4f, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xcf, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    },
    };
    char title[64] = "\x82\x72\x82\x73\x81\x40\x82\x4f\x82\x4f\x81\x40\x81\x40\x81\x40\x81\x40\x81\x40\x81\x40\x81\x40\x81\x40\x81\x40\x81\x40\x81\x40\x81\x40\x81\x40\x81\x40\x81\x40\x81\x40\x82\x72\x82\x6f\x81\x40\x81\x40\x81\x40\x82\x4f\0\0\0\0\0\0\0\0\0\0";
    char zero[2] = "\x82\x4f";
    u16 areas[42][14] = {
    {
        0x7282, 0x6782, 0x6082, 0x6382, 0x6e82, 0x7682, 0x4081, 0x7382,
        0x6e82, 0x7682, 0x6482, 0x7182, 0x4081, 0x4081,
    },
    {
        0x6782, 0x7482, 0x6c82, 0x6082, 0x6d82, 0x4081, 0x7682, 0x6e82,
        0x7182, 0x6b82, 0x6382, 0x4081, 0x4081, 0x4081,
    },
    {
        0x6c82, 0x6e82, 0x6d82, 0x7282, 0x7382, 0x6482, 0x7182, 0x4081,
        0x7682, 0x6e82, 0x7182, 0x6b82, 0x6382, 0x4081,
    },
    {
        0x6582, 0x6882, 0x7182, 0x6482, 0x4081, 0x7682, 0x6e82, 0x7182,
        0x6b82, 0x6382, 0x4081, 0x4081, 0x4081, 0x4081,
    },
    {
        0x6782, 0x7482, 0x6c82, 0x6082, 0x6d82, 0x4081, 0x7682, 0x6e82,
        0x7182, 0x6b82, 0x6382, 0x4081, 0x4081, 0x4081,
    },
    {
        0x6782, 0x7482, 0x6c82, 0x6082, 0x6d82, 0x4081, 0x7682, 0x6e82,
        0x7182, 0x6b82, 0x6382, 0x4081, 0x4081, 0x4081,
    },
    {
        0x6782, 0x7482, 0x6c82, 0x6082, 0x6d82, 0x4081, 0x7682, 0x6e82,
        0x7182, 0x6b82, 0x6382, 0x4081, 0x4081, 0x4081,
    },
    {
        0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081,
        0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081,
    },
    {
        0x6882, 0x6b82, 0x6b82, 0x7482, 0x7282, 0x6882, 0x6e82, 0x6d82,
        0x4081, 0x7682, 0x6e82, 0x7182, 0x6b82, 0x6382,
    },
    {
        0x6882, 0x6b82, 0x6b82, 0x7482, 0x7282, 0x6882, 0x6e82, 0x6d82,
        0x4081, 0x7682, 0x6e82, 0x7182, 0x6b82, 0x6382,
    },
    {
        0x7682, 0x6082, 0x7382, 0x6482, 0x7182, 0x7682, 0x6e82, 0x7182,
        0x6b82, 0x6382, 0x4081, 0x4081, 0x4081, 0x4081,
    },
    {
        0x6482, 0x6082, 0x7182, 0x7382, 0x6782, 0x4081, 0x7682, 0x6e82,
        0x7182, 0x6b82, 0x6382, 0x4081, 0x4081, 0x4081,
    },
    {
        0x6482, 0x6082, 0x7182, 0x7382, 0x6782, 0x4081, 0x7682, 0x6e82,
        0x7182, 0x6b82, 0x6382, 0x4081, 0x4081, 0x4081,
    },
    {
        0x6882, 0x6b82, 0x6b82, 0x7482, 0x7282, 0x6882, 0x6e82, 0x6d82,
        0x4081, 0x7682, 0x6e82, 0x7182, 0x6b82, 0x6382,
    },
    {
        0x6382, 0x6482, 0x6082, 0x7382, 0x6782, 0x4081, 0x7682, 0x6e82,
        0x7182, 0x6b82, 0x6382, 0x4081, 0x4081, 0x4081,
    },
    {
        0x6482, 0x6082, 0x7182, 0x7382, 0x6782, 0x4081, 0x7682, 0x6e82,
        0x7182, 0x6b82, 0x6382, 0x4081, 0x4081, 0x4081,
    },
    {
        0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081,
        0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081,
    },
    {
        0x6c82, 0x6e82, 0x6d82, 0x7282, 0x7382, 0x6482, 0x7182, 0x4081,
        0x7682, 0x6e82, 0x7182, 0x6b82, 0x6382, 0x4081,
    },
    {
        0x6582, 0x6882, 0x7182, 0x6482, 0x4081, 0x7682, 0x6e82, 0x7182,
        0x6b82, 0x6382, 0x4081, 0x4081, 0x4081, 0x4081,
    },
    {
        0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081,
        0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081,
    },
    {
        0x6582, 0x6882, 0x7182, 0x6482, 0x4081, 0x7682, 0x6e82, 0x7182,
        0x6b82, 0x6382, 0x4081, 0x4081, 0x4081, 0x4081,
    },
    {
        0x7682, 0x6082, 0x7382, 0x6482, 0x7182, 0x4081, 0x7682, 0x6e82,
        0x7182, 0x6b82, 0x6382, 0x4081, 0x4081, 0x4081,
    },
    {
        0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081,
        0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081,
    },
    {
        0x6382, 0x6482, 0x6082, 0x7382, 0x6782, 0x4081, 0x7682, 0x6e82,
        0x7182, 0x6b82, 0x6382, 0x4081, 0x4081, 0x4081,
    },
    {
        0x6582, 0x6882, 0x7182, 0x6482, 0x4081, 0x7682, 0x6e82, 0x7182,
        0x6b82, 0x6382, 0x4081, 0x4081, 0x4081, 0x4081,
    },
    {
        0x6382, 0x6482, 0x6082, 0x7382, 0x6782, 0x4081, 0x7682, 0x6e82,
        0x7182, 0x6b82, 0x6382, 0x4081, 0x4081, 0x4081,
    },
    {
        0x6382, 0x6482, 0x6082, 0x7382, 0x6782, 0x4081, 0x7682, 0x6e82,
        0x7182, 0x6b82, 0x6382, 0x4081, 0x4081, 0x4081,
    },
    {
        0x6482, 0x6082, 0x7182, 0x7382, 0x6782, 0x4081, 0x7682, 0x6e82,
        0x7182, 0x6b82, 0x6382, 0x4081, 0x4081, 0x4081,
    },
    {
        0x6482, 0x6082, 0x7182, 0x7382, 0x6782, 0x4081, 0x7682, 0x6e82,
        0x7182, 0x6b82, 0x6382, 0x4081, 0x4081, 0x4081,
    },
    {
        0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081,
        0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081,
    },
    {
        0x7582, 0x6e82, 0x6882, 0x6382, 0x4081, 0x4081, 0x4081, 0x4081,
        0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081,
    },
    {
        0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081,
        0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081,
    },
    {
        0x7682, 0x6082, 0x7382, 0x6482, 0x7182, 0x4081, 0x7682, 0x6e82,
        0x7182, 0x6b82, 0x6382, 0x4081, 0x4081, 0x4081,
    },
    {
        0x6482, 0x6082, 0x7182, 0x7382, 0x6782, 0x4081, 0x7682, 0x6e82,
        0x7182, 0x6b82, 0x6382, 0x4081, 0x4081, 0x4081,
    },
    {
        0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081,
        0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081,
    },
    {
        0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081,
        0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081,
    },
    {
        0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081,
        0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081,
    },
    {
        0x7682, 0x6082, 0x7382, 0x6482, 0x7182, 0x7682, 0x6e82, 0x7182,
        0x6b82, 0x6382, 0x4081, 0x4081, 0x4081, 0x4081,
    },
    {
        0x6882, 0x6b82, 0x6b82, 0x7482, 0x7282, 0x6882, 0x6e82, 0x6d82,
        0x4081, 0x7682, 0x6e82, 0x7182, 0x6b82, 0x6382,
    },
    {
        0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081,
        0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081,
    },
    {
        0x7282, 0x6782, 0x6082, 0x6382, 0x6e82, 0x7682, 0x4081, 0x7382,
        0x6e82, 0x7682, 0x6482, 0x7182, 0x4081, 0x4081,
    },
    {
        0x7282, 0x6782, 0x6082, 0x6382, 0x6e82, 0x7682, 0x4081, 0x7382,
        0x6e82, 0x7682, 0x6482, 0x7182, 0x4081, 0x4081,
    },
    };
    int new_file, n, fd, sum;

    new_file = sp->unk1FE == sp->unk1F4->count;
    strcpy(header.magic, "SC");
    header.icon = 0x12;
    header.blocks = 2;
    memset(header.unk44, 0, sizeof(header.unk44));
    if (new_file)
        n = sp->unk204;
    else
        n = sp->unk1F4->files[sp->unk1FE].unk14;
    memcpy(header.clut, cluts[n % 7], sizeof(header.clut));
    memcpy(header.icons, icons[n % 7], sizeof(header.icons));
    title[6] = zero[0];
    title[7] = zero[1] + n / 10;
    title[8] = zero[0];
    title[9] = zero[1] + n % 10;
    func_80014d2c((u16 *)&title[12], areas[area_state.area], 14);
    if (viewer.unk8 >= 1000) {
        title[46] = zero[0];
        title[47] = zero[1] + viewer.unk8 / 1000;
    }
    if (viewer.unk8 >= 100) {
        title[48] = zero[0];
        title[49] = zero[1] + viewer.unk8 / 100 % 10;
    }
    if (viewer.unk8 >= 10) {
        title[50] = zero[0];
        title[51] = zero[1] + viewer.unk8 / 10 % 10;
    }
    title[52] = zero[0];
    title[53] = zero[1] + viewer.unk8 % 10;
    ((CardTitle *)title)->kind = area_state.area;
    ((CardTitle *)title)->unk3A = viewer.unk8;
    ((CardTitle *)title)->unk3C = viewer.unk28;
    memcpy(&header.title, title, sizeof(title));
open:
    strcpy(path, "bu");
    strcat(path, itoa(sp->unk1F8));
    strcat(path, "0:");
    if (new_file) {
        strcat(path, "BASLUS-00863FS_ST");
        num[0] = sp->unk204 / 10 + '0';
        num[1] = sp->unk204 % 10 + '0';
        num[2] = '\0';
        strcat(path, num);
        fd = open(path, O_CREAT | (2 << 16));
        if (fd == -1) {
            if (retry == 0) {
                sp->unk0.half[0] = 355;
                return 0;
            }
            retry--;
            goto open;
        }
        close(fd);
    } else {
        strcat(path, sp->unk1F4->files[sp->unk1FE].name);
    }
    fd = open(path, O_RDWR);
    if (fd == -1) {
        if (retry == 0) {
            sp->unk0.half[0] = 355;
            return 0;
        }
        retry--;
        goto open;
    }
write:
    lseek(fd, 0, SEEK_SET);
    if (write(fd, &header, sizeof(header)) == -1) {
        if (retry == 0) {
            sp->unk0.half[0] = 355;
            close(fd);
            return 0;
        }
        retry--;
        lseek(fd, 0, SEEK_SET);
        goto write;
    }
    sum = func_80061d34(fd, sizeof(header), data, 7560);
    if (sum == -1) {
        if (retry == 0) {
            func_80057a3c();
            close(fd);
            return 0;
        }
        retry--;
        lseek(fd, 0, SEEK_SET);
        goto write;
    }
    header.title.unk3E = sum;
    lseek(fd, 0, SEEK_SET);
    if (write(fd, &header, sizeof(header)) == -1) {
        if (retry == 0) {
            sp->unk0.half[0] = 355;
            close(fd);
            return 0;
        }
        retry--;
        lseek(fd, 0, SEEK_SET);
        goto write;
    }
    viewer.unk22C = area_state.unk4;
    viewer.unk22D = viewer.position.vx >> 12;
    viewer.unk22E = viewer.position.vy >> 12;
    viewer.unk22F = viewer.position.vz >> 12;
    viewer.unk230 = viewer.position.vx - (viewer.unk22D << 12);
    viewer.unk232 = viewer.position.vy - (viewer.unk22E << 12);
    viewer.unk234 = viewer.position.vz - (viewer.unk22F << 12);
    viewer.unk236 = viewer.unk278.vy + 2048;
    close(fd);
    func_80057a3c();
    return 1;
}

/* Formats the memory card in port. */
ADDRESS(0x80062bc0, 0x68)
long func_80062bc0(int port)
{
    char path[32];

    strcpy(path, "bu");
    strcat(path, itoa(port));
    strcat(path, "0:");
    return format(path);
}

/*
 * Not exact (size-exact): this build keeps the button word's address in s0
 * across the result waits; retail forms it again for each load and store.
 * A structured empty while hoists the nonvolatile read; scoping the save
 * buffer does not change the listing. The counter interrupt's 800153b4 ->
 * 80057744 path updates this halfword while the retail waits reload it.
 * A narrow volatile while read reloads it correctly but peels the loop
 * entries, adding 48 bytes and moving the following exact functions.
 * Keeping the goto waits with qualified field pointers instead forms
 * full addresses (+12 bytes), also moving those functions.
 *
 * The memory card save/load screen (unk2DC's high byte set: save). Steps
 * through port choice, probe, format offer, directory, file headers, file
 * choice and the transfer, shifting unk2DC's step bit as it goes; returns
 * 0 when the port choice is cancelled, else 1 after the result message.
 */
ADDRESS(0x80062c28, 0x450)
int func_80062c28(int arg)
{
    MenuScratch *sp = MENU_SCRATCH;
    int result;
    u8 *buf;

    sp->unk0.half[0] = 0;
    sp->unk0.half[1] = 0;
    sp->unk206 = arg;
    func_80057a3c();
    func_8002ca84();
    func_8002ca84();
    sp->unk2DC <<= 1;
select:
    if (func_800616f8() == 0)
        return 0;
    sp->unk2DC <<= 1;
    func_8002ca84();
    func_8002ca84();
    func_800613e8(sp->unk1F8, 1);
    if ((sp->unk2DC & 0xff00) && sp->unk1F4->status == 3) {
        sp->unk2DC <<= 1;
        sp->unk0.half[1] = 0;
        result = func_800615d8();
        if (result == 0) {
            sp->unk2DC >>= 2;
            goto select;
        }
        if (result == -1) {
            sp->unk2DC <<= 4;
            goto done;
        }
        sp->unk0.half[1] = 1;
        func_8002ca84();
        func_8002ca84();
        if (func_80062bc0(sp->unk1F8) == 0) {
            sp->unk0.half[0] = 356;
            sp->unk2DC <<= 4;
            goto done;
        }
        sp->unk2DC <<= 1;
    } else {
        if (sp->unk1F4->status == 2) {
            sp->unk0.half[0] = 350;
            sp->unk2DC <<= 5;
            goto done;
        }
        sp->unk2DC <<= 2;
    }
    func_8006103c(sp->unk1F8);
    if (sp->unk1F4->status != 0) {
        if (sp->unk1F4->status == 4)
            sp->unk0.half[0] = 352;
        else if (sp->unk1F4->status == 5)
            sp->unk0.half[0] = 353;
        sp->unk2DC <<= 3;
        goto done;
    }
    result = func_8006184c();
    if (result == 0) {
        sp->unk0.half[0] = 352;
        sp->unk2DC <<= 3;
        goto done;
    }
    if (result == -1) {
        sp->unk2DC <<= 3;
        goto done;
    }
    result = func_80061a08();
    if (result == 0) {
        sp->unk2DC >>= 3;
        goto select;
    }
    if (result == -1) {
        sp->unk2DC <<= 3;
        goto done;
    }
    sp->unk0.half[0] = 0;
    sp->unk2DC <<= 1;
    func_8002ca84();
    func_8002ca84();
    func_800613e8(sp->unk1F8, 0);
    if (sp->unk1F4->status == 1) {
        sp->unk0.half[0] = 351;
        sp->unk2DC <<= 2;
        goto done;
    }
    if (sp->unk1F4->status == 2) {
        sp->unk0.half[0] = 350;
        sp->unk2DC <<= 2;
        goto done;
    }
    buf = func_800152d0(7686);
    if (buf == NULL) {
        sp->unk2DC <<= 2;
        goto done;
    }
    if (sp->unk2DC & 0xff00) {
        func_800562d4((StSaveData *)buf, sp->unk208);
        result = func_80062228(buf);
    } else {
        result = func_8006205c(buf);
        if (result)
            func_80056aa0((StSaveData *)buf);
    }
    func_800152f8(buf);
    if (result == 0) {
        if (sp->unk0.half[0] == 0)
            if (sp->unk2DC & 0xff00)
                sp->unk0.half[0] = 355;
            else
                sp->unk0.half[0] = 354;
        sp->unk2DC <<= 2;
        goto done;
    }
    sp->unk2DC <<= 1;
done:
    func_8002ca84();
    func_8002ca84();
    if (sp->unk2DC & 0x80) {
    wait0:
        if (DAT_801cd180.unk4 == 0)
            goto wait0;
        func_80057cec();
        DAT_801cd180.unk4 = 0;
        sp->unk2DC = 2;
        goto select;
    }
    if (sp->unk2DC & 0x8000) {
    wait1:
        if (DAT_801cd180.unk4 == 0)
            goto wait1;
        func_80057cec();
        DAT_801cd180.unk4 = 0;
        sp->unk2DC = 0x200;
        goto select;
    }
    if (sp->unk2DC & 0xff00) {
    wait2:
        if (DAT_801cd180.unk4 == 0)
            goto wait2;
        func_80057cec();
        DAT_801cd180.unk4 = 0;
    }
    viewer.unk2A = 1;
    func_80057a3c();
    return 1;
}

/*
 * The quit dialog: returns 1 after setting DAT_800881a4 to 2 (leave for
 * TITLE.EXE) when the first choice is confirmed, else 0.
 */
ADDRESS(0x80063078, 0x134)
int func_80063078(void)
{
    MenuScratch *sp = MENU_SCRATCH;

    sp->unk2E0 = 1;
    func_80057a3c();
    while (1) {
        if (DAT_801cd180.unk4 & 0x2000) {
            func_80057cec();
            DAT_801cd180.unk4 = 0;
            break;
        }
        if (DAT_801cd180.unkC & 0x80) {
            func_80057d10();
            DAT_801cd180.unkC = 0;
            sp->unk2E0 = 0;
        }
        if (DAT_801cd180.unkC & 0x20) {
            func_80057d10();
            DAT_801cd180.unkC = 0;
            sp->unk2E0 = 1;
        }
        if (DAT_801cd180.unk4 & 0x4000) {
            func_80057cc8();
            DAT_801cd180.unk4 = 0;
            if (sp->unk2E0 != 0)
                break;
            DAT_800881a4 = 2;
            return 1;
        }
        func_800164b4();
        func_8002ce84();
        sp->unk2E4 = (sp->unk2E4 + 1) % 32;
    }
    func_80057a3c();
    return 0;
}

/*
 * The pause screen: runs func_80031d50 until button 8, keeping the play
 * clock (viewer.unk33/unk34) still.
 * The controller owner becomes live after the draw setup. The explicit
 * top-tested wait keeps the button read before each pair of frame calls.
 */
ADDRESS(0x800631ac, 0xb4)
void func_800631ac(void)
{
    s32 seconds = viewer.unk34;
    u8 frames = viewer.unk33;
    Data801cd180 *pad;

    func_80015ad4();
    func_80058c04();
    func_80057a3c();
    DrawSync(0);
    pad = &DAT_801cd180;
pause_wait:
    if (pad->unk4 & 8)
        goto pause_done;
    func_800164b4();
    func_80031d50();
    goto pause_wait;
pause_done:
    func_80057a3c();
    func_80058c24();
    viewer.unk34 = seconds;
    viewer.unk33 = frames;
}

/*
 * A two-choice page over the menu font (CD archive 3 member 0): returns
 * the choice.
 * Not exact (93.8%): retail keeps the file buffer's base in s0 and forms
 * the image address as (size + 4) + base + 4; this build reuses base + 4.
 */
ADDRESS(0x80063260, 0x174)
int func_80063260(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    u8 *p;

    sp->unk2E6[0].vx = 960;
    sp->unk2E6[0].vy = 256;
    sp->unk2E6[1].vx = 560;
    sp->unk2E6[1].vy = 495;
    func_80015ad4();
    p = (u8 *)DAT_80088c88;
    func_80016030(3, 0, (u_long *)p);
    func_80014cfc((u32 *)&menu_messages, (u32 *)(p + 4), 5376);
    p += *(u32 *)p + 4;
    func_8003ec5c(p + 4, &sp->unk2E6[0], &sp->unk2E6[1], 1);
    sp->unk0.word = 0;
    sp->unk2E4 = 0;
    func_80057a3c();
    while (1) {
        if (DAT_801cd180.unk4 & 0x4000) {
            func_80057cc8();
            DAT_801cd180.unk4 = 0;
            break;
        }
        if (DAT_801cd180.unkC & 0x40) {
            func_80057d10();
            DAT_801cd180.unkC = 0;
            sp->unk0.word = 1;
        }
        if (DAT_801cd180.unkC & 0x10) {
            func_80057d10();
            DAT_801cd180.unkC = 0;
            sp->unk0.word = 0;
        }
        sp->unk2E5 = 0;
        func_800164b4();
        func_8002cef8();
        sp->unk2E4 = (sp->unk2E4 + 1) % 32;
    }
    func_80057a3c();
    return sp->unk0.word;
}

/*
 * The memory card choice screen: loads the menu messages and image, then
 * runs the two-row choice until a row is confirmed; row 0 runs the save
 * screen (func_80062c28) and returns once it reports 1.
 */
ADDRESS(0x800633d4, 0x264)
void func_800633d4(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    RECT clear = {0, 0, 320, 480};
    u_long *image;
    int result;

    sp->unk2E6[0].vx = 960;
    sp->unk2E6[0].vy = 256;
    sp->unk2E6[1].vx = 560;
    sp->unk2E6[1].vy = 495;
    func_80015ad4();
    image = DAT_80088c88;
    func_80016030(3, 0, image);
    func_80014cfc((u32 *)&menu_messages, (u32 *)(image + 1), 5376);
    image = (u_long *)((u8 *)image + (DAT_80088c88[0] + 4));
    func_8003ec5c(image + 1, &sp->unk2E6[0], &sp->unk2E6[1], 1);
    ClearImage(&clear, 0, 0, 0);
    sp->unk2DE = 0;
    sp->unk2E4 = 0;
    func_80057a3c();
    sp->unk33C.x = 0;
    sp->unk33C.y = 0;
    sp->unk33C.w = 320;
    sp->unk33C.h = 240;
    for (;;) {
        if (DAT_801cd180.unk4 & 0x4000) {
            DAT_801cd180.unk4 = 0;
            func_80057cc8();
            if (sp->unk2DE != 0)
                break;
            ClearImage(&clear, 0, 0, 0);
            func_80015ad4();
            func_80055cf4(area_state.unk4);
            func_80042cfc();
            func_80057a3c();
            func_80057a68(0);
            func_80058dc4(1, 0);
            sp->unk208 = 1;
            result = func_80062c28(0);
            func_80057a3c();
            func_80058ed4();
            if (result == 1)
                break;
        }
        if (DAT_801cd180.unkC & 0x80) {
            func_80057d10();
            DAT_801cd180.unkC = 0;
            sp->unk2DE = 0;
        }
        if (DAT_801cd180.unkC & 0x20) {
            func_80057d10();
            DAT_801cd180.unkC = 0;
            sp->unk2DE = 1;
        }
        sp->unk2E5 = 0;
        func_800164b4();
        func_8002d0b0();
        sp->unk2E4 = (sp->unk2E4 + 1) % 32;
    }
    func_80057a3c();
    ClearImage(&clear, 0, 0, 0);
}
