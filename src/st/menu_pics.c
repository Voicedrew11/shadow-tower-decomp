/*
 * ST.EXE: the status screens' equipment pictures (WIP module,
 * 0x80059204..0x80059cec): the picture of one equipment slot, the pictures
 * of the quick items and the two-handed weapon rules over the slots. It
 * ends at the non-exact func_8005975c; the slot restore and the attribute
 * totals continue in src/st/menu_attr.c.
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
#include <st/vis.h>
#include <st/effect.h>
#include <st/menu.h>
#include <st/funcs.h>
#include <st/player.h>
#include <st/menu_main.h>

RODATA(0x800128d0, 0x54)
JUMPTABLES(0x80013738, 0x54)

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
