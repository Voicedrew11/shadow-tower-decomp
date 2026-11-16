/*
 * ST.EXE: the player's held items and magic: per-hand swing and cast state
 * in the viewer block, the stamina bars, the magic records and the step
 * that moves the viewer (WIP module; the rest of its translation unit
 * reaches back into the band I player units and on to 0x80042d74, see the
 * .rdata note).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/data.h>
#include <st/funcs.h>
#include <st/gfx.h>
#include <st/vis.h>
#include <st/player.h>

/* Per hand: starts the queued magic, or recovers the second bar. */
ADDRESS(0x80040024, 0x288)
void func_80040024(u8 hand)
{
    int active;
    int rate = 36;

    if (viewer.unk13C[hand] != -1) {
        if (hand) {
            if (viewer.unk38[13] != 0xffff && (vis_table_c.unk0118 + viewer.unk38[13])->unk28)
                func_80040f04(viewer.unk14C[hand], hand);
        } else {
            if (viewer.unk38[0] != 0xffff && (vis_table_c.unk0118 + viewer.unk38[0])->unk28)
                func_80040f04(viewer.unk14C[0], 0);
        }
        viewer.unk13C[hand] = -1;
        return;
    }
    active = 0;
    if (viewer.unk38[0] != 0xffff && (vis_table_c.unk0118 + viewer.unk38[0])->unk28)
        active = viewer.unk38[6] != 0xffff;
    if (viewer.unk38[13] != 0xffff && (vis_table_c.unk0118 + viewer.unk38[13])->unk28 &&
        viewer.unk38[12] != 0xffff)
        active = 1;
    if (!viewer.unk1CB || viewer.unk190[12][0] != 0) {
        if (!active)
            return;
        if (viewer.unk188 == 0) {
            viewer.unk182 += (viewer.unk62[8] / (viewer.unk62[8] + 1000) * rate + 36) << 1;
            if (viewer.unk182 > 5120)
                viewer.unk182 = 5120;
        } else {
            viewer.unk188--;
        }
    } else {
        if (viewer.unk182 - 128 > 0)
            viewer.unk182 -= 128;
        else
            viewer.unk182 = 0;
    }
}

