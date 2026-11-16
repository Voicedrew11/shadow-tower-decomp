/*
 * ST.EXE player figure recomputation, band P side (WIP module; the
 * translation unit continues from band O's func_80059dc0 and its callees).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <st/data.h>
#include <st/stats.h>

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
