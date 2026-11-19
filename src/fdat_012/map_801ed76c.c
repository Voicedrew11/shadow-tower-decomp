/*
 * FDAT_012.BIN: the per-entry handler (WIP module; a unit of its own until its neighbours in
 * the run are exact).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <psyq/libc.h>
#include <st/vis.h>
#include <st/data.h>
#include <st/anim.h>
#include <st/hud.h>
#include <st/menu.h>
#include <st/light.h>
#include <st/xa.h>
#include <st/fdat.h>

void func_801eda38(Vis212 *e);
void func_801edbec(Vis212 *e);
void func_801edf0c(Vis212 *e);

/*
 * The current entry's handler: kind 4 drops once record's stream counter
 * reaches 4 (or when hit) and then runs func_801edbec; kind 10 plays the
 * scene word fdat_801edfac's stages; kinds 1 and 9 bob by their frame;
 * then the animation steps as fdat_801ed6e4 does.
 */
ADDRESS(0x801ed76c, 0x274)
void func_801ed76c(void)
{
    Vis212 *e = vis_table_a.unk6138;
    Vis192 *r = vis_table_a.unk6134;
    int a;
    int offset;

    if (e->unk02 == 4) {
        if (DAT_801cba08.unk1448[r->unk09] == 4 && e->unk1C.bytes[0] == 0)
            func_801eda38(e);
        if (e->unk0F == 0x80) {
            func_801edbec(e);
            return;
        }
        if (e->unk64 != 0)
            func_801eda38(e);
    } else if (e->unk02 == 10) {
        if ((unsigned)fdat_801edfac < 2) {
            fdat_801eb7b4(e, 8);
            e->unkB0 = 0;
            e->unk1C.bytes[3] = 1;
            func_801edf0c(e);
            return;
        }
        if (fdat_801edfac == 2) {
            e->unk2A += e->unkB0;
            fdat_801eba4c(e);
            if (0xfff - e->unkB0 < (short)e->unk2A) {
                fdat_801edfac = 3;
                e->unk1C.bytes[3] = 0;
            }
            return;
        }
    } else if (e->unk02 == 1 || e->unk02 == 9) {
        if (e->unk0F == 0x20) {
            /* The phase is sign-extended in the variable itself (retail
             * loads it with LH and halves that register); through a (short)
             * cast cse merges the halving into the extension's shift. */
            a = e->unk2A << 16;
            a >>= 16;
            if (a < 0x400) {
                a >>= 1;
                e->unk74.vy = e->unk84 - a;
            } else if (a < 0x800) {
                offset = (a - 0x400) * 5 >> 2;
                offset -= 0x200;
                e->unk74.vy = e->unk84 + offset;
            } else {
                offset = (a - 0x800) * 3 >> 3;
                offset -= 0x300;
                e->unk74.vy = e->unk84 - offset;
            }
        } else {
            e->unk74.vy = e->unk84;
        }
    }
    e->unk2A += e->unkB0;
    if (fdat_801ece7c(e) == 1)
        return;
    if (e->unkB0 >= 0 ? (short)e->unk2A >= 4096 : (short)e->unk2A <= 0) {
        e->unk1A = 0xff;
        fdat_801ed4a0();
    }
    if (e->unkA8 != NULL) {
        fdat_801ebec4(e);
        e->unk1A = 0xf0;
    }
}
