/*
 * ST.EXE: the status screens' slot restore and attribute totals (WIP module,
 * 0x80059cec..0x8005a36c). It continues src/st/menu_pics.c; its caller
 * func_8005a3a8 opens the next band and may share its file.
 */
#include <kf/lib/address.h>
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

/* Restores the equipment slots from the scratch copy. */
ADDRESS(0x80059cec, 0xd4)
void func_80059cec(void)
{
    *MENU_SLOTS = *(MenuSlots *)MENU_SCRATCH->unk292;
}

/*
 * Totals the attribute rows from the viewer's figures and equipment, and
 * fills one record per shown equipment slot with its figures and kind.
 *
 * Not exact: retail reads the effect row index (unk16C/unk170) before the
 * slot id and folds the +0xa field offset into the effects address
 * constant; this build keeps it as a displacement.
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
        at->unk16E = effects.defs[viewer.unk38[6]].unkA[at->unk16C];
    } else {
        at->unk16C = 0xffff;
        at->unk16E = 0;
    }
    if (viewer.unk38[12] != 0xffff) {
        at->unk170 = viewer.unk38[12] / 3;
        at->unk172 = effects.defs[viewer.unk38[12]].unkA[at->unk170];
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
