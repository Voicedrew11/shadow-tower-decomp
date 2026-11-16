/*
 * ST.EXE visible id list builder (WIP module): func_80046270 fills the
 * source id list; the union of the sources' 512-bit masks becomes the
 * visible id list that func_8001c800/func_8001c88c search.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <st/vis.h>

extern void func_80046270(void *arg, void *a1, void *a2);
extern u_long *func_80014d5c(u_long *dst, u_long value, int count);

ADDRESS(0x8001c100, 0x164)
void func_8001c100(void *arg)
{
    u_long mask[16];
    u_long unused[4];
    u_long *bits;
    int i;
    int j;
    int k;
    u16 id;

    DAT_801b5948.n_sources = 0;
    func_80046270(arg, 0, 0);
    func_80014d5c(mask, 0, 16);
    i = DAT_801b5948.n_sources;
    while (i--) {
        bits = DAT_8017d6d0[DAT_801b5948.sources[i]].bits;
        for (j = 0; j < 16; j++) {
            mask[j] |= *bits++;
        }
    }
    DAT_801b5948.n_visible = 0;
    for (id = j = 0; j < 16; j++) {
        for (k = 0; k < 32; k++) {
            if (mask[j] & 1) {
                if (DAT_801b5920.unk00 != 1 || DAT_801b5920.unk1d == 0) {
                    DAT_801b5948.visible[DAT_801b5948.n_visible] = id;
                    DAT_801b5948.n_visible++;
                }
            }
            mask[j] >>= 1;
            id++;
        }
    }
}
