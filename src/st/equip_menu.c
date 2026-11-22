/*
 * ST.EXE: the equipment screen and an id grid screen (WIP module, one
 * translation unit): the slot list, the candidate list of one slot, the
 * ring sub-list and their panels, then two four-column grids of table C ids
 * the player moves between, its frame and its CD loads (sharing the id grid
 * layout and panels of src/st/ui.c, MenuGrid). The screen's open and close
 * (0x800637c0, 0x80063954) sit in band P's range. Retail keeps the id grid's
 * constants (0x800138a0) between the equipment screen's (0x80013880) and the
 * equipment screen's switch tables (0x800138d8), which only one -G8 object
 * produces.
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
#include <psyq/kernel.h>
#include <LIBETC.H>
#include <LIBSN.H>
#include <psyq/libc.h>
#include <st/idgrid.h>
#include <st/pad.h>
#include <st/menu_main.h>

RODATA(0x80013880, 0xf4)
SDATA(0x8008810c, 0x7c)

/* The list row and column saved while the ring sub-list is open. */
DATA(0x800881e8, 0x4, ".sbss")
static s16 DAT_800881e8[2];

#define EQ DAT_801e0ce8
#define PAD DAT_801cd180
#define SLOT(k) (&viewer.unk38[k])

/* Runs player_compute_derived_stats(0) with the list state kept, then clamps
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
    player_compute_derived_stats(0);
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
        mem_free(sp->unk0.list.ids);
    } else if (PAD.unk4 & 0x4000) {
        if (EQ.unk0 != -1) {
            if (EQ.unk10 != SLOT(0) && EQ.unk10 != SLOT(13) && vis_table_c.unk0118[EQ.unk0].unk27 &&
                !vis_table_c.unk0118[EQ.unk0].unk28) {
                func_80057d34();
            } else {
                func_80057cc8();
                PAD.unk4 = 0;
                sp->unk2DC &= ~0x8;
                mem_free(sp->unk0.list.ids);
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
            mem_free(sp->unk0.list.ids);
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
            cd_read_member(6, 801, DAT_80088c88);
        else
            cd_read_member(6, vis_table_c.unk0118[EQ.unk0].unk5[0] + 600, DAT_80088c88);
        func_8003ec5c(DAT_80088c88, &pos, NULL, 1);
        EQ.unk4 = EQ.unk0;
    }
}

/*
 * The slot list: moves the cursor; on confirm lists the held ids that fit
 * the slot (or, for the two ring slots, opens the ring sub-list) and loads
 * the equipped id's image. Returns 1 on cancel.
 *
 * The list loop reaches the item records through a pointer to the table
 * set just before it, and every case of its switch tests the record's
 * kind and adds the id itself; jump2 merges the copies. Both matter to the
 * allocation only: the pointer keeps the table's address out of loop.c's
 * hoisting budget, and the written-out adds are what rank the scratchpad
 * ahead of `kind` and `cur` for the saved registers.
 */
ADDRESS(0x800642e8, 0x8c0)
int func_800642e8(int *sel)
{
    MenuScratch *sp = MENU_SCRATCH;
    DVECTOR pos = {448, 256};
    int kind;
    int cur;
    int i;
    VisTableC *table;

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
                u32 *bits;

                if (EQ.unk10 == SLOT(6))
                    bits = &DAT_801e0d08.bits[viewer.unk38[0] - 196];
                else
                    bits = &DAT_801e0d08.bits[viewer.unk38[13] - 196];
                DAT_801e0d08.count = 0;
                for (i = 0; i < 12; i++)
                    DAT_801e0d08.entries[i] = 255;
                for (i = 0; i < 12; i++) {
                    if (*bits & (1 << entries[i]))
                        DAT_801e0d08.entries[DAT_801e0d08.count++] = entries[i];
                }
                if (DAT_801e0d08.count) {
                    func_80057cc8();
                    sp->unk2DC = (sp->unk2DC | 0x2) & ~0x1;
                    DAT_800881e8[0] = sp->unk2E2;
                    DAT_800881e8[1] = sp->unk2E0;
                    sp->unk2E2 = 0;
                    sp->unk2E0 = 0;
                } else {
                    func_80057d34();
                }
            } else {
                func_80057d34();
            }
            return 0;
        }
        func_80057cc8();
        sp->unk0.list.ids = mem_alloc(532);
        sp->unk0.list.count = 0;
        table = &vis_table_c;
        for (i = 0; i < 266; i++) {
            if (!DAT_801c7590.counts[i].count || DAT_801c7590.counts[i].flag ||
                table->unk0118[i].unk26 == 255)
                continue;
            switch (kind) {
            case 8:
                if (i == (EQ.unk10 == SLOT(1) ? viewer.unk38[14] : viewer.unk38[1]))
                    continue;
                if (table->unk0118[i].unk26 == kind)
                    sp->unk0.list.ids[sp->unk0.list.count++] = i;
                break;
            case 4:
                if (i == (EQ.unk10 == SLOT(0) ? viewer.unk38[13] : viewer.unk38[0]))
                    continue;
                if (table->unk0118[i].unk26 == kind)
                    sp->unk0.list.ids[sp->unk0.list.count++] = i;
                break;
            case 6:
                if (EQ.unk10 == SLOT(3) && i == viewer.unk38[4])
                    continue;
                if (EQ.unk10 == SLOT(4) && i == viewer.unk38[3])
                    continue;
                if (table->unk0118[i].unk26 == 6 || table->unk0118[i].unk26 == 9)
                    sp->unk0.list.ids[sp->unk0.list.count++] = i;
                break;
            case 1:
                if (table->unk0118[i].unk26 == 1 || table->unk0118[i].unk26 == 10)
                    sp->unk0.list.ids[sp->unk0.list.count++] = i;
                break;
            case 7:
                if (EQ.unk10 == SLOT(16) && i == viewer.unk38[17])
                    continue;
                if (EQ.unk10 == SLOT(17) && i == viewer.unk38[16])
                    continue;
                if (table->unk0118[i].unk26 == kind)
                    sp->unk0.list.ids[sp->unk0.list.count++] = i;
                break;
            default:
                if (table->unk0118[i].unk26 == kind)
                    sp->unk0.list.ids[sp->unk0.list.count++] = i;
                break;
            }
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
            cd_read_member(6, 801, DAT_80088c88);
            EQ.unk0 = -1;
            EQ.unk4 = -1;
        } else {
            cd_read_member(6, i + 600, DAT_80088c88);
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
        frame_rate_update();
        func_80066004();
        sp->unk2E4 = (sp->unk2E4 + 1) % 32;
    }
    func_80057a3c();
}

ADDRESS(0x80064ff0, 0x30)
void func_80064ff0(void)
{
    menu_equip_open();
    func_80064ee4();
    menu_equip_close();
}

/* Unreferenced: copies the packet area to the current frame's clip and
 * dims the screen. */
ADDRESS(0x80065020, 0xbc)
void func_80065020(void)
{
    RECT r = {0, 0, 320, 240};
    CVECTOR c = {32, 32, 32};

    LoadImage(&world_state.draw[world_state.frame].clip, world_state.packets[1].end);
    ui_fill_rect(&r, &c, 1);
    ui_add_draw_mode(2, 0, 0);
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
    ui_draw_image(&MENU_SCRATCH->unk33C, &MENU_SCRATCH->unk358, &MENU_SCRATCH->unk35C, NULL, 3);
    ui_draw_window(16, 16, 288, 208);
    if (MENU_SCRATCH->unk2DC & 0x1) {
        RECT r = {0, 0, 320, 240};
        CVECTOR c = {32, 32, 32, 0};

        ui_fill_rect(&r, &c, 1);
        ui_add_draw_mode(2, 0, 0);
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
    ui_draw_rect_outline(&MENU_SCRATCH->unk330, &MENU_SCRATCH->unk32C, 257, 1);
    ui_add_draw_mode(1, 0, 0);
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

        ui_draw_arrow(&right, &sp->unk32C, 0);
        ui_draw_arrow(&left, &sp->unk32C, 2048);
        ui_add_draw_mode(1, 0, 0);
    }
    id = EQ.unk0;
    if (id != -1) {
        sp->style[0].cd |= 0x80;
        ui_draw_text(MESSAGE_IN(sp->unk0.list.messages, id), 24, 176, &sp->style[0]);
        sp->style[0].cd &= 0x7f;
        if (vis_table_c.unk0118[id].unk27) {
            if (vis_table_c.unk0118[id].unk28 < 4) {
                label = &sp->style[2];
                value = &sp->style[6];
            } else {
                label = &sp->style[0];
                value = &sp->style[4];
            }
            ui_draw_text(MESSAGE(109), 48, 190, label);
            ui_draw_number(vis_table_c.unk0118[id].unk28 | 0x3000000, 104, 190, value);
            ui_draw_glyph(74, 136, 190, label);
            ui_draw_number(vis_table_c.unk0118[id].unk27 | 0x3000000, 148, 190, value);
        }
    }
    ui_draw_text(MESSAGE(103), 165, 46, &sp->style[0]);
    y = 46;
    ui_draw_text(MESSAGE(103), 165, y, &sp->style[0]);
    for (i = 0; i < 7; i++) {
        if (id == -1) {
            ui_draw_number_fixed(0x13000000, 207, y, &sp->style[4]);
            ui_draw_number_fixed(0x13000000, 273, y, &sp->style[4]);
        } else {
            ui_draw_number_fixed(vis_table_c.unk0118[id].unk10[i] | 0x13000000, 207, y, &sp->style[4]);
            ui_draw_number_fixed(vis_table_c.unk0118[id].unk10[i + 7] | 0x13000000, 273, y, &sp->style[4]);
        }
        y += 14;
    }
    if (id != -1) {
        if (vis_table_c.unk0118[id].unk24) {
            DAT_8008819e = vis_table_c.unk0118[id].unk24 * 100 / 454;
            DAT_80088188 = vis_table_c.unk0118[id].unk24 * 1000 / 454;
            ui_draw_text(MESSAGE(104), 221, 156, &sp->style[0]);
            ui_draw_number_fixed(DAT_8008819e | 0x3000000, 249, 156, &sp->style[4]);
            ui_draw_glyph(78, 269, 156, &sp->style[0]);
            ui_draw_number_fixed((u16)(DAT_80088188 - DAT_8008819e * 10) | 0x1000000, 275, 156,
                          &sp->style[4]);
            ui_draw_glyph(108, 281, 156, &sp->style[0]);
        }
        sp->unk350.x = 20;
        sp->unk350.y = 40;
        sp->unk350.w = 128;
        sp->unk350.h = 128;
        sp->unk358.vx = 448;
        sp->unk358.vy = 256;
        ui_draw_image(&sp->unk350, &sp->unk358, NULL, NULL, 16);
    } else {
        sp->unk350.x = 28;
        sp->unk350.y = 40;
        sp->unk350.w = 128;
        sp->unk350.h = 128;
        sp->unk358.vx = 448;
        sp->unk358.vy = 256;
        ui_draw_image(&sp->unk350, &sp->unk358, NULL, NULL, 16);
    }
}

/* The slot list: the two load figures, then five slots with what each
 * holds and its figures. */
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
    ui_draw_text(MESSAGE(915), 113, 26, label);
    ui_draw_number(DAT_8008819e | 0x3000000, 181, 26, value);
    ui_draw_glyph(78, 212, 26, label);
    ui_draw_number((u16)(DAT_80088188 - DAT_8008819e * 10) | 0x1000000, 217, 26, value);
    ui_draw_glyph(74, 230, 26, label);
    DAT_8008819e = viewer.unk24 * 100 / 454;
    DAT_80088188 = viewer.unk24 * 1000 / 454;
    ui_draw_number(DAT_8008819e | 0x3000000, 243, 26, value);
    ui_draw_glyph(78, 274, 26, label);
    ui_draw_number((u16)(DAT_80088188 - DAT_8008819e * 10) | 0x1000000, 279, 26, value);
    for (i = 0; i < 5; i++) {
        y = 48 + i * 36;
        y2 = 64 + i * 36;
        ui_draw_text(MESSAGE(sp->unk2E2 + i + 900), 28, y, &sp->style[0]);
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

                ui_draw_text(MESSAGE(916 + entries[cur]), 92, y, &sp->style[0]);
            } else {
                ui_draw_text(MESSAGE(935), 92, y, &sp->style[0]);
            }
        } else if (cur != 0xffff) {
            sp->style[0].cd |= 0x80;
            sp->style[3].cd |= 0x80;
            ui_draw_text(MESSAGE_IN(sp->unk0.list.messages, cur), 92, y, &sp->style[0]);
            sp->style[0].cd &= 0x7f;
            sp->style[3].cd &= 0x7f;
        } else {
            ui_draw_text(MESSAGE(934), 92, y, &sp->style[0]);
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
            ui_draw_text(MESSAGE(109), 92, y2, label2);
            ui_draw_number(vis_table_c.unk0118[cur].unk28 | 0x3000000, 172, y2, value2);
            ui_draw_glyph(74, 204, y2, label2);
            ui_draw_number(vis_table_c.unk0118[cur].unk27 | 0x3000000, 220, y2, value2);
        }
    }
}

/* The ring sub-list: five entries with their names and effect figures. */
/* The text base is added as an integer after the offset, and the name table
 * pointer is assigned inside the loop: retail adds in that operand order
 * and keeps the table address in the loop. */
ADDRESS(0x80065de0, 0x160)
void func_80065de0(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    int i = 0;
    EquipSubList *list = &DAT_801e0d08;
    CVECTOR *label = &sp->style[0];
    u16 *offsets = menu_messages.offsets;
    u32 text = (u32)menu_messages.text;
    int y, y2;
    u8 *names = NULL;

    y2 = 64;
    y = 48;
    for (; i < 5; i++) {
        if (*(list->entries + (sp->unk2E2 + i)) != 255) {
            names = DAT_8008092c;
            ui_draw_text(
                (u16 *)(offsets[916 + names[*(list->entries + (sp->unk2E2 + i))]] + text), 28, y,
                label);
            ui_draw_text((u16 *)(offsets[936] + text), 140, y2, label);
            ui_draw_number(effects.defs[*(list->entries + (sp->unk2E2 + i))].unk2 | 0x3000000,
                          204, y2, &sp->style[4]);
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
    ui_fill_rect(&r, &c, 1);
    ui_add_draw_mode(2, 0, 0);
}

/* Draws the screen: the text colours, the cursor blink and the active
 * list's panels. */
ADDRESS(0x80066004, 0x168)
void func_80066004(void)
{
    MenuScratch *sp;

    gfx_begin_frame();
    sp = MENU_SCRATCH;
    sp->style[3].r = sp->style[7].r = 96;
    sp->style[3].g = sp->style[7].g = sp->style[3].b = sp->style[7].b = 48;
    sp->style[0].b = sp->style[4].b = sp->style[0].g = sp->style[4].g = sp->style[0].r =
        sp->style[4].r = 96;
    sp->style[2].r = sp->style[6].r = 98;
    sp->style[2].g = sp->style[6].g = 66;
    sp->style[2].b = sp->style[6].b = 48;
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
    gfx_flip();
    func_80065f40();
    gfx_draw_ot();
}

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
ADDRESS(0x800661c8, 0x44)
void func_800661c8(void)
{
    DVECTOR pos = {640, 0};

    func_8003ec5c(DAT_80088c88, &pos, NULL, 1);
    DrawSync(0);
}

ADDRESS(0x8006620c, 0x34)
void func_8006620c(u16 archive, u16 member)
{
    DVECTOR unused;

    cd_queue_read_member(archive, member, (u8 *)DAT_80088c88, func_800661c8);
}

/*
 * Opens the screen: loads its message tables and images, then lists every
 * held table C id (except 266 and the selected viewer.unk38 slots) into
 * the grid of its flag bit. Slots 6 and 12 are not filtered. Message files
 * use a length-prefixed cursor; raw TIM files use the load buffer directly.
 */
ADDRESS(0x80066240, 0x470)
void func_80066240(void)
{
    DVECTOR tex = {896, 448};
    DVECTOR clut = {544, 502};
    RECT unused;
    char name[] = "\\st\\itemcg\\icon1\\0000.tim";
    u_long *tim;
    MenuScratch *sp;
    int i;

    DAT_801e0d38.unk0 = 104;
    DAT_801e0d38.unk4 = 0;
    DAT_801e0d38.unk10 = DAT_801e0d38.unkC = DAT_801e0d38.unk8 = -1;
    menu_show_loading_screen();
    MENU_SCRATCH->unk0.grid.messages = mem_alloc(21504);
    tim = DAT_80088c88;
    cd_read_member(3, 1, tim);
    copy_words((u32 *)&menu_messages, (u32 *)(tim + 1), 5376);
    tim = (u_long *)((u8 *)tim + (*tim + sizeof(*tim)));
    func_8003ec5c(tim + 1, &MENU_SCRATCH->unk2E6[0], &MENU_SCRATCH->unk2E6[1], 1);
    tim = DAT_80088c88;
    cd_read_member(3, 4, tim);
    sp = MENU_SCRATCH;
    copy_words((u32 *)sp->unk0.grid.messages, (u32 *)(tim + 1), 5376);
    tim = (u_long *)((u8 *)tim + (*tim + sizeof(*tim)));
    func_8003ec5c(tim + 1, &sp->unk2E6[2], &sp->unk2E6[3], 1);
    cd_read_member(3, 5, DAT_80088c88);
    func_8003ec5c(DAT_80088c88, &sp->unk2E6[4], NULL, 1);
    DrawSync(0);
    cd_read_member(3, 7, DAT_80088c88);
    func_8003ec5c(DAT_80088c88, &tex, &clut, 1);
    DrawSync(0);
    sp->unk0.grid.count0 = 0;
    sp->unk0.grid.count1 = 0;
    sp->unk0.grid.ids0 = mem_alloc(628);
    sp->unk0.grid.ids1 = mem_alloc(628);
    if (sp->unk0.grid.ids0 == NULL || sp->unk0.grid.ids1 == NULL)
        exit(1);
    fill_words((u32 *)sp->unk0.grid.ids0, -1, 157);
    fill_words((u32 *)sp->unk0.grid.ids1, -1, 157);
    sp->unk0.grid.count0 = 0;
    sp->unk0.grid.count1 = 0;
    for (i = 0; i < 314; i++) {
        if (vis_table_c.unk0118[i].unk5[0] != 255 && i < 314 && i != 266 &&
            DAT_801c7590.counts[i].count && i != viewer.unk38[0] && i != viewer.unk38[1] &&
            i != viewer.unk38[2] && i != viewer.unk38[3] && i != viewer.unk38[4] &&
            i != viewer.unk38[5] && i != viewer.unk38[7] && i != viewer.unk38[8] &&
            i != viewer.unk38[9] && i != viewer.unk38[10] && i != viewer.unk38[11] &&
            i != viewer.unk38[13] && i != viewer.unk38[14] &&
            i != viewer.unk38[15] && i != viewer.unk38[16] && i != viewer.unk38[17] &&
            i != viewer.unk38[18] && i != viewer.unk38[19] && i != viewer.unk38[20]) {
            if (!DAT_801c7590.counts[i].flag)
                sp->unk0.grid.ids0[sp->unk0.grid.count0++] = i;
            else
                sp->unk0.grid.ids1[sp->unk0.grid.count1++] = i;
        }
    }
    sp->unk2DC = 1;
    sp->unk2E0 = 0;
    sp->unk0.grid.row0 = 0;
    sp->unk0.grid.row1 = 0;
    DAT_801e0d38.unk2 = -1;
    clear_vram();
    DrawSync(0);
    gfx_setup_640x240();
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
ADDRESS(0x800666b0, 0xed0)
void func_800666b0(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    DVECTOR unused0;
    DVECTOR pos = {640, 0};
    char name1[] = "\\st\\itemcg\\icon1\\0000.tim";
    char name5[] = "\\st\\itemcg\\icon5\\0000.tim";
    DVECTOR unused1;
    s16 id;
    s16 n;
    s16 k;
    s16 idx;
    int i, j;
    int range;
    int hole;
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
                    for (range = ST.unk8; range <= ST.unkC; range++) {
                        idx = range;
                        if (GRID->ids0[idx] != -1) {
                            k = GRID->count1;
                            for (hole = 0; hole < GRID->count1; hole++) {
                                if (GRID->ids1[hole] == -1) {
                                    k = hole;
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
                    for (range = ST.unk8; range <= ST.unkC; range++) {
                        idx = range;
                        if (GRID->ids1[idx] != -1) {
                            k = GRID->count0;
                            for (hole = 0; hole < GRID->count0; hole++) {
                                if (GRID->ids0[hole] == -1) {
                                    k = hole;
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
                    if (ST.unk0 == 104) {
                        cur = GRID->row0;
                        found = GRID->ids0[cur + sp->unk2E0] == -1;
                    } else {
                        cur = GRID->row1;
                        if (GRID->ids1[cur + sp->unk2E0] == -1)
                            found = 1;
                    }
                } else {
                    found = 1;
                    if (ST.unk0 == 104) {
                        for (i = ST.unk8; i <= ST.unkC; i++) {
                            if (GRID->ids0[i] != -1) {
                                found = 0;
                                break;
                            }
                        }
                    } else {
                        for (i = ST.unk8; i <= ST.unkC; i++) {
                            if (GRID->ids1[i] != -1) {
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
        frame_rate_update();
        func_80068258();
        sp->unk2E4 = (sp->unk2E4 + 1) % 32;
    }
    for (i = 0, n = GRID->count0; n;) {
        if (GRID->ids0[i] != -1) {
            DAT_801c7590.counts[GRID->ids0[i]].flag = 0;
            n--;
        }
        i++;
    }
    for (i = 0, n = GRID->count1; n;) {
        if (GRID->ids1[i] != -1) {
            DAT_801c7590.counts[GRID->ids1[i]].flag = 1;
            n--;
        }
        i++;
    }
}

/* Closes the screen once the CD ring is idle. */
ADDRESS(0x80067580, 0x74)
void func_80067580(void)
{
    MenuScratch *sp = MENU_SCRATCH;

    while (func_8006616c())
        ;
    menu_load_main_messages();
    mem_free(sp->unk0.grid.ids0);
    mem_free(sp->unk0.grid.ids1);
    mem_free(sp->unk0.grid.messages);
    menu_reload_assets();
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
ADDRESS(0x80067624, 0x5c8)
void func_80067624(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    int text_x, text_y;
    s16 id;
    int i;
    s16 x, y;
    s16 selected;
    short none;
    int n, k;
    int j;
    u8 *count;
    CVECTOR *label, *value;
    u8 whole, tenths;

    if (ST.unk0 == 104) {
        x = 334;
        y = 9;
        text_x = 342;
        text_y = 135;
        sp->unk350.w = 192;
        sp->unk350.x = x;
        sp->unk350.h = 124;
        sp->unk350.y = y;
    } else {
        x = 10;
        y = 9;
        text_x = 18;
        text_y = 135;
        sp->unk350.w = 192;
        sp->unk350.x = x;
        sp->unk350.h = 124;
        sp->unk350.y = y;
    }
    none = 0;
    if (ST.unk0 == 104) {
        selected = sp->unk0.grid.row0 + sp->unk2E0;
        none = sp->unk0.grid.ids0[selected] == -1;
        id = sp->unk0.grid.ids0[selected];
    } else {
        selected = sp->unk0.grid.row1 + sp->unk2E0;
        id = sp->unk0.grid.ids1[selected];
        if (sp->unk0.grid.ids1[selected] == -1)
            none = 1;
    }
    if (!none) {
        if (!func_8006616c()) {
            sp->unk358.vx = 640;
            sp->unk358.vy = 0;
            ui_draw_image(&sp->unk350, &sp->unk358, NULL, NULL, 16);
            sp->unk350.w = 100;
            sp->unk358.vx = 832;
            sp->unk350.x += 192;
            ui_draw_image(&sp->unk350, &sp->unk358, NULL, NULL, 16);
        }
        n = 0;
        k = 2;
        count = vis_table_c.unk0118[id].unk10;
        DAT_800881ae = 1;
        for (i = 0; i < 4; i++) {
            for (j = 0; j < 4; j++) {
                if (n == 14)
                    goto done;
                n++;
                if (*count) {
                    ui_draw_text(MESSAGE(200 + k), text_x - 4 + i * 72, text_y + j * 15, &sp->style[0]);
                    ui_draw_number(*count | 0x13000000, text_x + i * 72 + 40, text_y + j * 15,
                                  &sp->style[4]);
                }
                count++;
                k++;
            }
            if (n == 14)
                break;
        }
    done:
        DAT_800881ae = 0;
        text_y += 60;
        if (vis_table_c.unk0118[id].unk27) {
            if (vis_table_c.unk0118[id].unk28 < 4) {
                label = &sp->style[2];
                value = &sp->style[6];
            } else {
                label = &sp->style[0];
                value = &sp->style[4];
            }
            ui_draw_text(MESSAGE(29), text_x, text_y, label);
            ui_draw_number(vis_table_c.unk0118[id].unk28 | 0x3000000, text_x + 70, text_y, value);
            ui_draw_glyph(14, text_x + 106, text_y, label);
            ui_draw_number(vis_table_c.unk0118[id].unk27 | 0x3000000, text_x + 122, text_y, value);
        }
        text_y += 15;
        if (vis_table_c.unk0118[id].unk24) {
            whole = vis_table_c.unk0118[id].unk24 * 100 / 454;
            tenths = vis_table_c.unk0118[id].unk24 * 1000 / 454 - whole * 10;
            ui_draw_text(MESSAGE(28), text_x, text_y, &sp->style[0]);
            ui_draw_number(whole | 0x3000000, text_x + 70, text_y, &sp->style[4]);
            ui_draw_glyph(18, text_x + 109, text_y, &sp->style[0]);
            ui_draw_number(tenths | 0x1000000, text_x + 116, text_y, &sp->style[4]);
            ui_draw_glyph(53, text_x + 132, text_y, &sp->style[0]);
        }
    }
    ui_draw_panel(x, y, 296, 222);
}

/*
 * Draws one grid (104 or 98): the cursor bar, the scroll arrows and bar, the
 * header image and eleven rows of names, the held range highlighted.
 */
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
    CVECTOR *color;
    int x, y;
    int row;

    if (ST.unk0 == 104) {
        sp->unk330.x = 26;
        sp->unk330.y = sp->unk2E0 * 16 + 33;
    } else {
        sp->unk330.x = 340;
        sp->unk330.y = sp->unk2E0 * 16 + 33;
    }
    sp->unk330.w = 270;
    sp->unk330.h = 15;
    ui_draw_rect_outline(&sp->unk330, &sp->unk32C, 513, 1);
    if (grid == 104) {
        if (sp->unk0.grid.row0 > 0)
            ui_draw_arrow(&up0, &sp->unk32C, 3072);
        if (sp->unk0.grid.row0 < 303)
            ui_draw_arrow(&down0, &sp->unk32C, 1024);
    } else {
        if (sp->unk0.grid.row1 > 0)
            ui_draw_arrow(&up1, &sp->unk32C, 3072);
        if (sp->unk0.grid.row1 < 303)
            ui_draw_arrow(&down1, &sp->unk32C, 1024);
    }
    if (grid == 104) {
        knob0.y = sp->unk0.grid.row0 * 173 / 303 + 35;
        ui_fill_rect(&knob0, &knob, 1);
        ui_add_draw_mode(1, 0, 0);
        bar.x = knob0.x - 2;
        bar.y = knob0.y - 1;
        bar.w = knob0.w;
        bar.h = knob0.h;
    } else {
        knob1.y = sp->unk0.grid.row1 * 173 / 303 + 35;
        ui_fill_rect(&knob1, &knob, 1);
        ui_add_draw_mode(1, 0, 0);
        bar.x = knob1.x - 2;
        bar.y = knob1.y - 1;
        bar.w = knob1.w;
        bar.h = knob1.h;
    }
    ui_fill_rect(&bar, &knob, 1);
    ui_add_draw_mode(2, 0, 0);
    if (grid == 104) {
        sp->unk33C.x = 60;
        sp->unk33C.y = 12;
        sp->unk33C.w = 196;
        sp->unk33C.h = 16;
        sp->unk358.vx = 896;
        sp->unk358.vy = 448;
        sp->unk35C.vx = 544;
        sp->unk35C.vy = 502;
    } else {
        sp->unk33C.x = 384;
        sp->unk33C.y = 12;
        sp->unk33C.w = 196;
        sp->unk33C.h = 16;
        sp->unk358.vx = 896;
        sp->unk358.vy = 464;
        sp->unk35C.vx = 544;
        sp->unk35C.vy = 502;
    }
    ui_draw_image(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, 3);
    {
        CVECTOR held = {0, 128, 128, 201};
        CVECTOR empty = {0, 96, 96, 201};
        RECT box;

        if (grid == 104) {
            x = 28;
            y = 35;
        } else {
            x = 342;
            y = 35;
        }
        sp->style[0].cd |= 0x80;
        for (i = 0; i < 11; i++) {
            if (grid == 104) {
                row = sp->unk0.grid.row0 + i;
                id = sp->unk0.grid.ids0[row];
                idx = row;
            } else {
                row = sp->unk0.grid.row1 + i;
                id = sp->unk0.grid.ids1[row];
                idx = row;
            }
            color = &sp->style[0];
            if (grid == ST.unk0 && ST.unk4 && idx >= ST.unk8 && idx <= ST.unkC) {
                color = &held;
                if (id == -1) {
                    box.x = x;
                    box.y = y + i * 16;
                    box.w = 266;
                    box.h = 12;
                    ui_fill_rect(&box, &empty, 0);
                }
            } else {
                color = &sp->style[0];
            }
            if (id != -1)
                ui_draw_text(MESSAGE_IN(sp->unk0.grid.messages, id), x, y + i * 16, color);
        }
        sp->style[0].cd &= 0x7f;
    }
    if (grid == 104)
        ui_draw_panel(10, 9, 296, 222);
    else
        ui_draw_panel(334, 9, 296, 222);
}

/* Draws the screen: the text colours, the cursor blink, one grid with its
 * detail panel or both grids, and the arrow toward the current grid. */
ADDRESS(0x80068258, 0x1e8)
void func_80068258(void)
{
    RECT arrow = {319, 110, 24, 16};
    CVECTOR color = {16, 48, 16, 0};
    MenuScratch *sp;

    gfx_begin_frame();
    sp = MENU_SCRATCH;
    sp->style[3].r = sp->style[7].r = 96;
    sp->style[3].g = sp->style[7].g = sp->style[3].b = sp->style[7].b = 48;
    sp->style[0].b = sp->style[4].b = sp->style[0].g = sp->style[4].g = sp->style[0].r =
        sp->style[4].r = 96;
    sp->style[2].r = sp->style[6].r = 98;
    sp->style[2].g = sp->style[6].g = 66;
    sp->style[2].b = sp->style[6].b = 48;
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
        menu_draw_button_prompt();
    if (sp->unk2DC & 0x1) {
        func_80067bec(104);
        func_80067bec(98);
    } else {
        func_80067bec(ST.unk0 == 104 ? 104 : 98);
        func_80067624();
    }
    if (ST.unk0 == 104)
        ui_draw_arrow(&arrow, &color, 0);
    else
        ui_draw_arrow(&arrow, &color, 2048);
    ui_add_draw_mode(1, 0, 0);
    menu_draw_background();
    gfx_end_frame();
}

/* The pad's button word (active low) read as one volatile halfword at its
 * own address; retail forms that address directly in each load. */
#define RAW_BUTTONS (*(volatile s16 *)&DAT_801cd130.unk2)

/*
 * Unreferenced debug tool: shows the other frame buffer and moves the
 * display area with the D-pad (raw pad bits, active low) until START,
 * polling the host debugger each frame; then restores it.
 */
ADDRESS(0x80068440, 0x214)
void func_80068440(void)
{
    RECT saved;
    DISPENV *disp;
    DRAWENV *draw;
    int bits;

    world_state.draw[0].isbg = 0;
    world_state.draw[1].isbg = 0;
    world_state.frame = world_state.frame == 0;
    DrawSync(0);
    VSync(0);
    PutDispEnv(&world_state.disp[world_state.frame]);
    PutDrawEnv(&world_state.draw[world_state.frame]);
    disp = &world_state.disp[world_state.frame];
    draw = &world_state.draw[world_state.frame];
    copy_words((u32 *)&saved, (u32 *)&disp->disp, 2);
    while (!(RAW_BUTTONS & 0x8))
        ;
    for (;;) {
        bits = ~RAW_BUTTONS;
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
    while (!(RAW_BUTTONS & 0x8))
        ;
    copy_words((u32 *)&disp->disp, (u32 *)&saved, 2);
    world_state.draw[0].isbg = 1;
    world_state.draw[1].isbg = 1;
}
