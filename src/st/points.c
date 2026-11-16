/*
 * ST.EXE: the attribute point screen (WIP module; one translation unit
 * with the screens around it, see src/st/menu_vram.c).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <st/data.h>
#include <st/menu.h>
#include <st/cd.h>
#include <st/funcs.h>
#include <st/card.h>

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
