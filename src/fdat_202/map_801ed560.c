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

/* Not exact (one attempt, retries off; 47.0%): first divergence retail `lui v1,0x0`, this source `lui v1,0x0`. */
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
    Vis88 *b;
    int i;
    u16 kind;

    if (fdat_801eda08 == 0) {
        if (sw->unk0C == 0xff) {
            b = vis_table_b.entries;
            for (i = 239; i != -1; i--) {
                kind = b->unk0a;
                if (kind == 0x29 || kind == 0x2b || kind == 0x94) {
                    b->unk05 = 0xff;
                    if (kind == 0x94) {
                        b->unk3C.vx = 0;
                        b->unk3C.vy = 0;
                        b->unk3C.vz = 0;
                    }
                } else if (kind == 0xaf) {
                    b->unk05 = 0xff;
                    b->unk0a = 0xffff;
                }
                b++;
            }
            fdat_801eda08 = 0xff;
            if (!(DAT_801cba08.unk1440.word >> 18 & 1))
                lighting.area_mode[20] = 0x41;
        }
    } else if (fdat_801eda08 == 0xff && sw->unk0C == 0) {
        b = vis_table_b.entries;
        for (i = 239; i != -1; i--) {
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
            b++;
        }
        fdat_801eda08 = 0;
        if (!(DAT_801cba08.unk1440.word >> 18 & 1))
            lighting.area_mode[20] = 0x22;
    }
}
