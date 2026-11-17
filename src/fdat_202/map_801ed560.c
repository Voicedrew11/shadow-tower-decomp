/*
 * FDAT_202.BIN: the table B switch handler (WIP module; a unit of its own until its neighbours in
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

/* Not exact: the enable loop uses another induction-field base and
 * assigns its state constants to different argument registers. */
/*
 * Table B entry 33 (a switch) state 0xff turns the area's table B entries
 * off: kinds 0x29, 0x2b and 0x94 stop (0x94 also loses its scale), 0xaf is
 * unlinked; state 0 turns them back on. Without flag 18 the lighting of
 * area 20 follows.
 */
ADDRESS(0x801ed560, 0x1b0)
void func_801ed560(void)
{
    Vis88 *sw = &vis_table_b.entries[33];
    int i;

    if (fdat_801eda08 == 0 && sw->unk0C == 0xff) {
        Vis88 *b;
        for (i = 239, b = vis_table_b.entries; i != -1; i--, b++) {
            if (b->unk0a == 0x29 || b->unk0a == 0x2b || b->unk0a == 0x94) {
                b->unk05 = 0xff;
                if (b->unk0a == 0x94) {
                    b->unk3C.vx = 0;
                    b->unk3C.vy = 0;
                    b->unk3C.vz = 0;
                }
            } else if (b->unk0a == 0xaf) {
                b->unk05 = 0xff;
                b->unk0a = 0xffff;
            }
        }
        fdat_801eda08 = 0xff;
        if (!(DAT_801cba08.unk1440.word >> 18 & 1))
            lighting.area_mode[20] = 0x41;
    } else if (fdat_801eda08 == 0xff && sw->unk0C == 0) {
        Vis88 *b;
        for (i = 239, b = vis_table_b.entries; i != -1; i--, b++) {
            if (b->unk0a == 0x29)
                b->unk05 = 3;
            if (b->unk0a == 0x2b)
                b->unk05 = 5;
            if (b->unk0a == 0x94) {
                b->unk05 = 2;
                b->unk0C = 0;
                b->unk3C.vx = 0;
                b->unk3C.vy = 0;
                b->unk3C.vz = 0;
                b->unk03 = 5;
                *(u8 *)&b->unk50 = 0;
                b->unk04 |= 0x40;
            }
        }
        fdat_801eda08 = 0;
        if (!(DAT_801cba08.unk1440.word >> 18 & 1))
            lighting.area_mode[20] = 0x22;
    }
}
