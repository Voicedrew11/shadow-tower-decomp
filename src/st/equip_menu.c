/*
 * ST.EXE: the equipment screen (WIP module): the slot list, the candidate
 * list of one slot, the ring sub-list and their panels. The screen's open
 * and close (0x800637c0, 0x80063954) sit in band P's range; this unit
 * starts at the first band Q function.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/memory.h>
#include <st/cd.h>
#include <st/data.h>
#include <st/gfx.h>
#include <st/effect.h>
#include <st/text.h>
#include <st/menu.h>
#include <st/vis.h>
#include <st/equip_menu.h>
#include <st/card.h>
#include <st/sound.h>
#include <st/funcs.h>

RODATA(0x80013880, 0x20)
JUMPTABLES(0x800138d8, 0x9c)
SDATA(0x8008810c, 0x20)

/* The list row and column saved while the ring sub-list is open. */
DATA(0x800881e8, 0x4, ".sbss")
static s16 DAT_800881e8[2];

#define EQ DAT_801e0ce8
#define PAD DAT_801cd180
#define SLOT(k) (&viewer.unk38[k])


/* Runs func_8005a3a8(0) with the list state kept, then clamps
 * the two viewer figures it may have raised. */
ADDRESS(0x80063bdc, 0xd4)
void func_80063bdc(void)
{
    MenuMessages *messages = MENU_SCRATCH->unk0.list.messages;
    s16 *ids = MENU_SCRATCH->unk0.list.ids;
    s16 count = MENU_SCRATCH->unk0.list.count;
    u16 unkA = MENU_SCRATCH->unk0.list.unkA;

    MENU_SCRATCH->unk2D8 = viewer.unk2;
    MENU_SCRATCH->unk2DA = viewer.unk6;
    func_8005a3a8(0);
    if (viewer.unk0 < MENU_SCRATCH->unk2D8)
        viewer.unk2 = viewer.unk0;
    if (viewer.unk4 < MENU_SCRATCH->unk2DA)
        viewer.unk6 = viewer.unk4;
    MENU_SCRATCH->unk0.list.messages = messages;
    MENU_SCRATCH->unk0.list.ids = ids;
    MENU_SCRATCH->unk0.list.count = count;
    MENU_SCRATCH->unk0.list.unkA = unkA;
}

/* The candidate list of one slot: pages through it, equips the shown id
 * (dropping the slots it excludes) or clears the slot, and loads the
 * shown id's image. */
ADDRESS(0x80063cb0, 0x638)
void func_80063cb0(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    DVECTOR pos = {448, 256};

    if (PAD.unkC & 0x20) {
        func_80057d10();
        if (EQ.unk8 == sp->unk0.list.count - 1) {
            EQ.unk8 = -1;
            EQ.unk0 = -1;
        } else {
            EQ.unk8++;
            EQ.unk0 = sp->unk0.list.ids[EQ.unk8];
        }
        EQ.unkC = 1;
    } else if (PAD.unkC & 0x80) {
        func_80057d10();
        if (EQ.unk8 == -1)
            EQ.unk8 = sp->unk0.list.count - 1;
        else
            EQ.unk8--;
        if (EQ.unk8 == -1)
            EQ.unk0 = EQ.unk8;
        else
            EQ.unk0 = sp->unk0.list.ids[EQ.unk8];
        EQ.unkC = 1;
    } else if (PAD.unk4 & 0x2000) {
        func_80057cec();
        PAD.unk4 = 0;
        sp->unk2DC &= ~0x8;
        func_800152f8(sp->unk0.list.ids);
    } else if (PAD.unk4 & 0x4000) {
        if (EQ.unk0 != -1) {
            if (EQ.unk10 != SLOT(0) && EQ.unk10 != SLOT(13) && vis_table_c.unk0118[EQ.unk0].unk27 &&
                !vis_table_c.unk0118[EQ.unk0].unk28) {
                func_80057d34();
            } else {
                func_80057cc8();
                PAD.unk4 = 0;
                sp->unk2DC &= ~0x8;
                func_800152f8(sp->unk0.list.ids);
                if (EQ.unk10 == SLOT(0) && EQ.unk0 != viewer.unk38[0])
                    viewer.unk38[6] = 0xffff;
                if (EQ.unk10 == SLOT(13) && EQ.unk0 != viewer.unk38[13])
                    viewer.unk38[12] = 0xffff;
                *EQ.unk10 = EQ.unk0;
                if (EQ.unk10 == SLOT(2))
                    viewer.unk38[15] = EQ.unk0;
                if (EQ.unk10 == SLOT(9) && viewer.unk38[9] != 0xffff &&
                    vis_table_c.unk0118[viewer.unk38[9]].unk26 == 10) {
                    viewer.unk38[2] = 0xffff;
                    viewer.unk38[10] = 0xffff;
                }
                if ((EQ.unk10 == SLOT(10) || EQ.unk10 == SLOT(2)) && viewer.unk38[9] != 0xffff &&
                    *EQ.unk10 != 0xffff && vis_table_c.unk0118[viewer.unk38[9]].unk26 == 10)
                    viewer.unk38[9] = 0xffff;
                if (EQ.unk10 == SLOT(3) && viewer.unk38[3] != 0xffff &&
                    vis_table_c.unk0118[viewer.unk38[3]].unk26 == 9)
                    viewer.unk38[16] = 0xffff;
                if (EQ.unk10 == SLOT(4) && viewer.unk38[4] != 0xffff &&
                    vis_table_c.unk0118[viewer.unk38[4]].unk26 == 9)
                    viewer.unk38[17] = 0xffff;
                if (EQ.unk10 == SLOT(16) && viewer.unk38[3] != 0xffff && viewer.unk38[16] != 0xffff &&
                    vis_table_c.unk0118[viewer.unk38[3]].unk26 == 9)
                    viewer.unk38[3] = 0xffff;
                if (EQ.unk10 == SLOT(17) && viewer.unk38[4] != 0xffff && viewer.unk38[17] != 0xffff &&
                    vis_table_c.unk0118[viewer.unk38[4]].unk26 == 9)
                    viewer.unk38[4] = 0xffff;
            }
        } else {
            func_80057cc8();
            PAD.unk4 = 0;
            sp->unk2DC &= ~0x8;
            func_800152f8(sp->unk0.list.ids);
            *EQ.unk10 = 0xffff;
            if (EQ.unk10 == SLOT(0))
                viewer.unk38[6] = 0xffff;
            if (EQ.unk10 == SLOT(13))
                viewer.unk38[12] = 0xffff;
        }
        func_80063bdc();
        PAD.unk4 = 0;
    }
    if (EQ.unk0 != EQ.unk4) {
        if (EQ.unk0 == -1)
            func_80016030(6, 801, DAT_80088c88);
        else
            func_80016030(6, vis_table_c.unk0118[EQ.unk0].unk5[0] + 600, DAT_80088c88);
        func_8003ec5c(DAT_80088c88, &pos, NULL, 1);
        EQ.unk4 = EQ.unk0;
    }
}

/*
 * The slot list: moves the cursor; on confirm lists the held ids that fit
 * the slot (or, for the two ring slots, opens the ring sub-list) and loads
 * the equipped id's image. Returns 1 on cancel.
 */
/* Not exact: input/scratch/current-id registers, clear-loop induction and
 * the return schedule differ. Both this function and func_80064ba8 currently
 * have their retail extents; the internal switch-label residue remains. */
ADDRESS(0x800642e8, 0x8c0)
int func_800642e8(int *sel)
{
    MenuScratch *sp = MENU_SCRATCH;
    DVECTOR pos = {448, 256};
    int kind;
    int cur;
    int i;

    if (PAD.unk4 & 0x2000) {
        func_80057cec();
        PAD.unk4 = 0;
        return 1;
    }
    if (PAD.unkC & 0x10) {
        func_80057d10();
        PAD.unkC = 0;
        if (sp->unk2E0 == 0) {
            if (sp->unk2E2 > 0)
                sp->unk2E2--;
        } else {
            sp->unk2E0--;
        }
    } else if (PAD.unkC & 0x40) {
        func_80057d10();
        PAD.unkC = 0;
        if (sp->unk2E0 == 4) {
            if (sp->unk2E2 < 10)
                sp->unk2E2++;
        } else {
            sp->unk2E0++;
        }
    } else if (PAD.unkC & 0x800) {
        func_80057d10();
        PAD.unkC = 0;
        if (sp->unk2E2 == 0)
            sp->unk2E0 = 0;
        else if (sp->unk2E2 >= 5)
            sp->unk2E2 -= 5;
        else
            sp->unk2E2 = 0;
    } else if (PAD.unkC & 0x200) {
        func_80057d10();
        PAD.unkC = 0;
        if (sp->unk2E2 == 10)
            sp->unk2E0 = 4;
        else if (sp->unk2E2 < 6)
            sp->unk2E2 += 5;
        else
            sp->unk2E2 = 10;
    } else if (PAD.unk4 & 0x4000) {
        PAD.unk4 = 0;
        switch (sp->unk2E2 + sp->unk2E0) {
        case 0:
            kind = 0;
            cur = viewer.unk38[8];
            EQ.unk10 = SLOT(8);
            break;
        case 1:
            kind = 1;
            cur = viewer.unk38[9];
            EQ.unk10 = SLOT(9);
            break;
        case 2:
            kind = 2;
            cur = viewer.unk38[2];
            EQ.unk10 = SLOT(2);
            break;
        case 3:
            kind = 3;
            cur = viewer.unk38[10];
            EQ.unk10 = SLOT(10);
            break;
        case 4:
            kind = 6;
            cur = viewer.unk38[3];
            EQ.unk10 = SLOT(3);
            break;
        case 5:
            kind = 6;
            cur = viewer.unk38[4];
            EQ.unk10 = SLOT(4);
            break;
        case 6:
            kind = 7;
            cur = viewer.unk38[16];
            EQ.unk10 = SLOT(16);
            break;
        case 7:
            kind = 7;
            cur = viewer.unk38[17];
            EQ.unk10 = SLOT(17);
            break;
        case 8:
            kind = 8;
            cur = viewer.unk38[1];
            EQ.unk10 = SLOT(1);
            break;
        case 9:
            kind = 8;
            cur = viewer.unk38[14];
            EQ.unk10 = SLOT(14);
            break;
        case 10:
            kind = 5;
            cur = viewer.unk38[7];
            EQ.unk10 = SLOT(7);
            break;
        case 11:
            kind = 4;
            cur = viewer.unk38[0];
            EQ.unk10 = SLOT(0);
            break;
        case 12:
            kind = -1;
            cur = viewer.unk38[6];
            EQ.unk10 = SLOT(6);
            break;
        case 13:
            kind = 4;
            cur = viewer.unk38[13];
            EQ.unk10 = SLOT(13);
            break;
        case 14:
            kind = -1;
            cur = viewer.unk38[12];
            EQ.unk10 = SLOT(12);
            break;
        default:
            cur = 0xffff;
            kind = -1;
            EQ.unk10 = NULL;
            break;
        }
        if (kind == -1) {
            if ((EQ.unk10 == SLOT(6) && viewer.unk38[0] != 0xffff) ||
                (EQ.unk10 == SLOT(12) && viewer.unk38[13] != 0xffff)) {
                u8 entries[] = {0, 1, 3, 4, 6, 7, 9, 10, 12, 13, 15, 16};
                EquipSubList *list;
                u32 *bits;

                if (EQ.unk10 == SLOT(6))
                    bits = &DAT_801e0d08.bits[viewer.unk38[0] - 196];
                else
                    bits = &DAT_801e0d08.bits[viewer.unk38[13] - 196];
                list = &DAT_801e0d08;
                list->count = 0;
                for (i = 11; i >= 0; i--)
                    list->entries[i] = 255;
                list = &DAT_801e0d08;
                for (i = 0; i < 12; i++) {
                    if (*bits & (1 << entries[i]))
                        list->entries[list->count++] = entries[i];
                }
                if (DAT_801e0d08.count) {
                    func_80057cc8();
                    DAT_800881e8[0] = sp->unk2E2;
                    DAT_800881e8[1] = sp->unk2E0;
                    sp->unk2E2 = 0;
                    sp->unk2E0 = 0;
                    sp->unk2DC = (sp->unk2DC | 0x2) & ~0x1;
                    return 0;
                }
            }
            func_80057d34();
            return 0;
        }
        func_80057cc8();
        sp->unk0.list.ids = func_800152d0(532);
        sp->unk0.list.count = 0;
        for (i = 0; i < 266; i++) {
            if (!DAT_801c7590.counts[i].count || DAT_801c7590.counts[i].flag ||
                vis_table_c.unk0118[i].unk26 == 255)
                continue;
            switch (kind) {
            case 8:
                if (i == (EQ.unk10 == SLOT(1) ? viewer.unk38[14] : viewer.unk38[1]))
                    continue;
                break;
            case 4:
                if (i == (EQ.unk10 == SLOT(0) ? viewer.unk38[13] : viewer.unk38[0]))
                    continue;
                break;
            case 6:
                if (EQ.unk10 == SLOT(3) && i == viewer.unk38[4])
                    continue;
                if (EQ.unk10 == SLOT(4) && i == viewer.unk38[3])
                    continue;
                if (vis_table_c.unk0118[i].unk26 == 6 || vis_table_c.unk0118[i].unk26 == 9)
                    goto add;
                continue;
            case 1:
                if (vis_table_c.unk0118[i].unk26 == 1 || vis_table_c.unk0118[i].unk26 == 10)
                    goto add;
                continue;
            case 7:
                if (EQ.unk10 == SLOT(16) && i == viewer.unk38[17])
                    continue;
                if (EQ.unk10 == SLOT(17) && i == viewer.unk38[16])
                    continue;
                break;
            }
            if (vis_table_c.unk0118[i].unk26 != kind)
                continue;
        add:
            sp->unk0.list.ids[sp->unk0.list.count++] = i;
        }
        if (cur == 0xffff) {
            EQ.unk8 = -1;
        } else {
            for (i = 0; i < sp->unk0.list.count; i++) {
                if (sp->unk0.list.ids[i] == cur)
                    break;
            }
            if (i == sp->unk0.list.count)
                i = -1;
            EQ.unk8 = i;
        }
        if (cur != 0xffff)
            i = vis_table_c.unk0118[cur].unk5[0];
        else
            i = -1;
        sp->unk2DC |= 0x8;
        if (cur == 0xffff) {
            func_80016030(6, 801, DAT_80088c88);
            EQ.unk0 = -1;
            EQ.unk4 = -1;
        } else {
            func_80016030(6, i + 600, DAT_80088c88);
            EQ.unk0 = cur;
            EQ.unk4 = cur;
        }
        func_8003ec5c(DAT_80088c88, &pos, NULL, 1);
        *sel = cur;
    }
    return 0;
}

/* The ring sub-list: moves the cursor in its entries and equips one. */
ADDRESS(0x80064ba8, 0x33c)
void func_80064ba8(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    u8 entries[] = {0, 1, 3, 4, 6, 7, 9, 10, 12, 13, 15, 16};
    EquipSubList *list;
    u32 *bits;

    if (PAD.unk4 & 0x2000) {
        func_80057cec();
        PAD.unk4 = 0;
        sp->unk2DC = (sp->unk2DC & ~0x2) | 0x1;
        sp->unk2E2 = DAT_800881e8[0];
        sp->unk2E0 = DAT_800881e8[1];
    } else if (PAD.unkC & 0x10) {
        func_80057d10();
        PAD.unkC = 0;
        if (sp->unk2E0 == 0) {
            if (sp->unk2E2 > 0)
                sp->unk2E2--;
        } else {
            sp->unk2E0--;
        }
    } else if (PAD.unkC & 0x40) {
        func_80057d10();
        PAD.unkC = 0;
        if (sp->unk2E0 == 4) {
            if (sp->unk2E2 < DAT_801e0d08.count - 5)
                sp->unk2E2++;
        } else if (sp->unk2E2 + sp->unk2E0 < DAT_801e0d08.count - 1) {
            sp->unk2E0++;
        }
    } else if (PAD.unkC & 0x800) {
        func_80057d10();
        PAD.unkC = 0;
        if (sp->unk2E2 == 0)
            sp->unk2E0 = 0;
        else if (sp->unk2E2 >= 5)
            sp->unk2E2 -= 5;
        else
            sp->unk2E2 = 0;
    } else if (PAD.unkC & 0x200) {
        func_80057d10();
        PAD.unkC = 0;
        if (sp->unk2E2 + 5 >= DAT_801e0d08.count)
            sp->unk2E0 = DAT_801e0d08.count - sp->unk2E2 - 1;
        else if (sp->unk2E2 + 5 < DAT_801e0d08.count - 5)
            sp->unk2E2 += 5;
        else
            sp->unk2E2 = DAT_801e0d08.count - 5;
    } else if (PAD.unk4 & 0x4000) {
        PAD.unk4 = 0;
        if (EQ.unk10 == SLOT(6))
            bits = &DAT_801e0d08.bits[viewer.unk38[0] - 196];
        else
            bits = &DAT_801e0d08.bits[viewer.unk38[13] - 196];
        list = &DAT_801e0d08;
        if (!(*bits & (1 << list->entries[sp->unk2E2 + sp->unk2E0]))) {
            func_80057d34();
        } else {
            func_80057cc8();
            *EQ.unk10 = list->entries[sp->unk2E2 + sp->unk2E0];
            sp->unk2DC = (sp->unk2DC & ~0x2) | 0x1;
            sp->unk2E2 = DAT_800881e8[0];
            sp->unk2E0 = DAT_800881e8[1];
            func_80063bdc();
        }
    }
}

/* The screen loop: the active list until the slot list cancels. */
ADDRESS(0x80064ee4, 0x10c)
void func_80064ee4(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    int sel;

    sp->unk2E2 = 0;
    sp->unk2E0 = 0;
    sel = -1;
    func_80057a3c();
    EQ.unkC = 0;
    for (;;) {
        if (EQ.unkC) {
            PAD.unkC = 0;
            EQ.unkC--;
        }
        if (sp->unk2DC & 0x8) {
            func_80063cb0();
        } else if (sp->unk2DC & 0x1) {
            if (func_800642e8(&sel))
                break;
        } else if (sp->unk2DC & 0x2) {
            func_80064ba8();
        }
        func_800164b4();
        func_80066004();
        sp->unk2E4 = (sp->unk2E4 + 1) % 32;
    }
    func_80057a3c();
}

ADDRESS(0x80064ff0, 0x30)
void func_80064ff0(void)
{
    func_800637c0();
    func_80064ee4();
    func_80063954();
}

/* Unreferenced: copies the packet area to the current frame's clip and
 * dims the screen. */
ADDRESS(0x80065020, 0xbc)
void func_80065020(void)
{
    RECT r = {0, 0, 320, 240};
    CVECTOR c = {32, 32, 32};

    LoadImage(&world_state.draw[world_state.frame].clip, world_state.packets[1].end);
    func_80028220(&r, &c, 1);
    func_800286a8(2, 0, 0);
}

/* The title image and frame; the screen is dimmed while the slot list
 * is not active. */
ADDRESS(0x800650dc, 0x114)
void func_800650dc(void)
{
    MENU_SCRATCH->unk33C.x = 32;
    MENU_SCRATCH->unk33C.y = 24;
    MENU_SCRATCH->unk33C.h = 16;
    MENU_SCRATCH->unk35C.vx = 544;
    MENU_SCRATCH->unk35C.vy = 511;
    MENU_SCRATCH->unk33C.w = 50;
    MENU_SCRATCH->unk358.vx = 576;
    MENU_SCRATCH->unk358.vy = 464;
    func_80028364(&MENU_SCRATCH->unk33C, &MENU_SCRATCH->unk358, &MENU_SCRATCH->unk35C, NULL, 3);
    func_80029618(16, 16, 288, 208);
    if (MENU_SCRATCH->unk2DC & 0x1) {
        RECT r = {0, 0, 320, 240};
        CVECTOR c = {32, 32, 32, 0};

        func_80028220(&r, &c, 1);
        func_800286a8(2, 0, 0);
    }
}

/* The slot list's cursor bar. */
ADDRESS(0x800651f0, 0x80)
void func_800651f0(void)
{
    MENU_SCRATCH->unk330.w = 272;
    MENU_SCRATCH->unk330.x = 24;
    MENU_SCRATCH->unk330.h = 34;
    MENU_SCRATCH->unk330.y = MENU_SCRATCH->unk2E0 * 36 + 44;
    func_80027d18(&MENU_SCRATCH->unk330, &MENU_SCRATCH->unk32C, 257, 1);
    func_800286a8(1, 0, 0);
}

/* The candidate list's panel: the arrows, the shown id's name, figures and
 * counts, and its image. */
ADDRESS(0x80065270, 0x500)
void func_80065270(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    s16 id;
    CVECTOR *label, *value;
    int y;
    int i;

    if (sp->unk0.list.count > 0) {
        RECT left = {24, 100, 16, 8};
        RECT right = {160, 100, 16, 8};

        func_8002916c(&right, &sp->unk32C, 0);
        func_8002916c(&left, &sp->unk32C, 2048);
        func_800286a8(1, 0, 0);
    }
    id = EQ.unk0;
    if (id != -1) {
        sp->style[0].cd |= 0x80;
        func_8002732c(MESSAGE_IN(sp->unk0.list.messages, id), 24, 176, &sp->style[0]);
        sp->style[0].cd &= 0x7f;
        if (vis_table_c.unk0118[id].unk27) {
            if (vis_table_c.unk0118[id].unk28 < 4) {
                label = &sp->style[2];
                value = &sp->style[6];
            } else {
                label = &sp->style[0];
                value = &sp->style[4];
            }
            func_8002732c(MESSAGE(109), 48, 190, label);
            func_80027440(vis_table_c.unk0118[id].unk28 | 0x3000000, 104, 190, value);
            func_80026ecc(74, 136, 190, label);
            func_80027440(vis_table_c.unk0118[id].unk27 | 0x3000000, 148, 190, value);
        }
    }
    func_8002732c(MESSAGE(103), 165, 46, &sp->style[0]);
    y = 46;
    func_8002732c(MESSAGE(103), 165, y, &sp->style[0]);
    for (i = 0; i < 7; i++) {
        if (id == -1) {
            func_800276bc(0x13000000, 207, y, &sp->style[4]);
            func_800276bc(0x13000000, 273, y, &sp->style[4]);
        } else {
            func_800276bc(vis_table_c.unk0118[id].unk10[i] | 0x13000000, 207, y, &sp->style[4]);
            func_800276bc(vis_table_c.unk0118[id].unk10[i + 7] | 0x13000000, 273, y, &sp->style[4]);
        }
        y += 14;
    }
    if (id != -1) {
        if (vis_table_c.unk0118[id].unk24) {
            DAT_8008819e = vis_table_c.unk0118[id].unk24 * 100 / 454;
            DAT_80088188 = vis_table_c.unk0118[id].unk24 * 1000 / 454;
            func_8002732c(MESSAGE(104), 221, 156, &sp->style[0]);
            func_800276bc(DAT_8008819e | 0x3000000, 249, 156, &sp->style[4]);
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
        func_80028364(&sp->unk350, &sp->unk358, NULL, NULL, 16);
    } else {
        sp->unk350.x = 28;
        sp->unk350.y = 40;
        sp->unk350.w = 128;
        sp->unk350.h = 128;
        sp->unk358.vx = 448;
        sp->unk358.vy = 256;
        func_80028364(&sp->unk350, &sp->unk358, NULL, NULL, 16);
    }
}

/* The slot list: the two load figures, then five slots with what each
 * holds and its figures. */
/* Not exact: same calls and switch; register allocation differs from the
 * first row loop on (retail: messages base in $s3, this build $s5). */
ADDRESS(0x80065770, 0x670)
void func_80065770(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    CVECTOR *label, *value;
    CVECTOR *label2, *value2;
    u16 cur;
    int row;
    int i, y, y2;

    if (viewer.unk22 > viewer.unk24) {
        label = &sp->style[2];
        value = &sp->style[6];
    } else {
        label = &sp->style[0];
        value = &sp->style[4];
    }
    DAT_8008819e = viewer.unk22 * 100 / 454;
    DAT_80088188 = viewer.unk22 * 1000 / 454;
    func_8002732c(MESSAGE(915), 113, 26, label);
    func_80027440(DAT_8008819e | 0x3000000, 181, 26, value);
    func_80026ecc(78, 212, 26, label);
    func_80027440((u16)(DAT_80088188 - DAT_8008819e * 10) | 0x1000000, 217, 26, value);
    func_80026ecc(74, 230, 26, label);
    DAT_8008819e = viewer.unk24 * 100 / 454;
    DAT_80088188 = viewer.unk24 * 1000 / 454;
    func_80027440(DAT_8008819e | 0x3000000, 243, 26, value);
    func_80026ecc(78, 274, 26, label);
    func_80027440((u16)(DAT_80088188 - DAT_8008819e * 10) | 0x1000000, 279, 26, value);
    for (i = 0, y = 48, y2 = 64; i < 5; i++, y += 36, y2 += 36) {
        func_8002732c(MESSAGE(900 + sp->unk2E2 + i), 28, y, &sp->style[0]);
        switch (sp->unk2E2 + i) {
        case 0:
            cur = viewer.unk38[8];
            break;
        case 1:
            cur = viewer.unk38[9];
            break;
        case 2:
            cur = viewer.unk38[2];
            break;
        case 3:
            cur = viewer.unk38[10];
            break;
        case 4:
            cur = viewer.unk38[3];
            break;
        case 5:
            cur = viewer.unk38[4];
            break;
        case 6:
            cur = viewer.unk38[16];
            break;
        case 7:
            cur = viewer.unk38[17];
            break;
        case 8:
            cur = viewer.unk38[1];
            break;
        case 9:
            cur = viewer.unk38[14];
            break;
        case 10:
            cur = viewer.unk38[7];
            break;
        case 11:
            cur = viewer.unk38[0];
            break;
        case 12:
            cur = viewer.unk38[6];
            break;
        case 13:
            cur = viewer.unk38[13];
            break;
        case 14:
            cur = viewer.unk38[12];
            break;
        default:
            cur = 0xffff;
            break;
        }
        row = sp->unk2E2 + i;
        if (row == 12 || row == 14) {
            if (cur != 0xffff) {
                u8 entries[] = {0, 1, 255, 2, 3, 255, 4, 5, 255, 6, 7, 255, 8, 9, 255, 10, 11, 255};

                func_8002732c(MESSAGE(916 + entries[cur]), 92, y, &sp->style[0]);
            } else {
                func_8002732c(MESSAGE(935), 92, y, &sp->style[0]);
            }
        } else if (cur != 0xffff) {
            sp->style[3].cd |= 0x80;
            sp->style[0].cd |= 0x80;
            func_8002732c(MESSAGE_IN(sp->unk0.list.messages, cur), 92, y, &sp->style[0]);
            sp->style[0].cd &= 0x7f;
            sp->style[3].cd &= 0x7f;
        } else {
            func_8002732c(MESSAGE(934), 92, y, &sp->style[0]);
        }
        row = sp->unk2E2 + i;
        if (row != 12 && row != 14 && cur != 0xffff && vis_table_c.unk0118[cur].unk27) {
            if (vis_table_c.unk0118[cur].unk28 < 4) {
                label2 = &sp->style[2];
                value2 = &sp->style[6];
            } else {
                label2 = &sp->style[0];
                value2 = &sp->style[4];
            }
            func_8002732c(MESSAGE(109), 92, y2, label2);
            func_80027440(vis_table_c.unk0118[cur].unk28 | 0x3000000, 172, y2, value2);
            func_80026ecc(74, 204, y2, label2);
            func_80027440(vis_table_c.unk0118[cur].unk27 | 0x3000000, 220, y2, value2);
        }
    }
}

/* The ring sub-list: five entries with their names and effect figures. */
/* Not exact: retail keeps the DAT_8008092c address in the loop (base in v0,
 * `addu v1,v1,v0`, entry address in a0) and adds the message offset first
 * (`addu a0,a0,s7`); here loop.c hoists the address (it then lands in the
 * reload register t0) and the sum is `strings + offset`. Ruled out: no
 * locals for list/label/offsets/strings (operand order then matches, the
 * saved registers rotate), a MenuMessages pointer, narrow y/i types. */
ADDRESS(0x80065de0, 0x160)
void func_80065de0(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    int i = 0;
    EquipSubList *list = &DAT_801e0d08;
    CVECTOR *label = &sp->style[0];
    u16 *offsets = menu_messages.offsets;
    u8 *strings = menu_messages.text;
    int y, y2;
    u8 entry;
    u8 *entry_slot;

    y2 = 64;
    y = 48;
    for (; i < 5; i++) {
        entry_slot = &list->entries[sp->unk2E2 + i];
        entry = *entry_slot;
        if (entry != 255) {
            func_8002732c((u16 *)(strings + offsets[916 + DAT_8008092c[entry]]), 28, y, label);
            func_8002732c((u16 *)(strings + offsets[936]), 140, y2, label);
            entry_slot = &list->entries[sp->unk2E2 + i];
            func_80027440(effects.defs[*entry_slot].unk2 | 0x3000000, 204, y2,
                          &sp->style[4]);
            y2 += 36;
            y += 36;
        }
    }
}

/* Unreferenced: as func_80065020 from 64 KiB into the packet area. */
ADDRESS(0x80065f40, 0xc4)
void func_80065f40(void)
{
    RECT r = {0, 0, 320, 240};
    CVECTOR c = {32, 32, 32};

    LoadImage(&world_state.draw[world_state.frame].clip,
              (u_long *)((u8 *)world_state.packets[1].end + 0x10000));
    func_80028220(&r, &c, 1);
    func_800286a8(2, 0, 0);
}

/* Draws the screen: the text colours, the cursor blink and the active
 * list's panels. */
/* Not exact: retail stores the 48 group before it loads the blink phase
 * (unk2E4) into $v1; this build hoists the load (7 words, $v0/$v1 swapped). */
ADDRESS(0x80066004, 0x168)
void func_80066004(void)
{
    MenuScratch *sp;

    func_800174c4();
    sp = MENU_SCRATCH;
    sp->style[2].b = sp->style[6].b = sp->style[3].g = sp->style[7].g = sp->style[3].b =
        sp->style[7].b = 48;
    sp->style[0].b = sp->style[4].b = sp->style[0].g = sp->style[4].g = sp->style[0].r =
        sp->style[4].r = sp->style[3].r = sp->style[7].r = 96;
    sp->style[2].r = sp->style[6].r = 98;
    sp->style[2].g = sp->style[6].g = 66;
    sp->style[0].cd = sp->style[2].cd = sp->style[3].cd = 57;
    sp->style[4].cd = sp->style[6].cd = sp->style[7].cd = 41;
    if (MENU_SCRATCH->unk2E4 < 16)
        sp->unk360 = 8 - MENU_SCRATCH->unk2E4;
    else
        sp->unk360 = MENU_SCRATCH->unk2E4 - 24;
    sp->unk32C.b = 0;
    sp->unk32C.r = (u8)sp->unk360 * 4 + 96;
    sp->unk32C.g = (u8)sp->unk360 * 2 + 48;
    if (!(sp->unk2DC & 0x8))
        func_800651f0();
    if (sp->unk2DC & 0x8)
        func_80065270();
    else if (sp->unk2DC & 0x1)
        func_80065770();
    else
        func_80065de0();
    func_800650dc();
    func_800175a4();
    func_80065f40();
    func_80017634();
}
