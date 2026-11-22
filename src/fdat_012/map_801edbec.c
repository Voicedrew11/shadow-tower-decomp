/*
 * FDAT_012.BIN: the entry's fall (WIP module; a unit of its own until its neighbours in
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

/*
 * The falling entry's stages (Vis212.unk1C byte 0): 1 falls and pushes the
 * viewer away while near, 2 lands (animation 6), 3 walks off (animation
 * 7) until x 0x3c800. Its hit flash (unk34) runs down meanwhile.
 */
ADDRESS(0x801edbec, 0x320)
void func_801edbec(Vis212 *e)
{
    Vis192 *r = &vis_table_a.unk3500[e->unk02];
    VECTOR pos;
    int d;

    e->unk2A += e->unkB0;
    switch (e->unk1C.bytes[0]) {
    case 1:
        if (e->unk4C == 0 && (short)e->unk2A > 0x800) {
            e->unk08 = 6;
            e->unk2A = 0;
            e->unkB0 = 0x100;
            e->unk1C.bytes[0] = 2;
            break;
        }
        if ((short)e->unk2A < 0x400) {
            e->unk2A += e->unkB0;
        } else {
            if ((short)e->unk2A < 0x680)
                e->unk16 -= r->unk0E;
            e->unk2A += e->unkB0;
            if ((short)e->unk2A >= 0x1000)
                e->unk2A = 0xfff;
            fdat_801eb880(0xc00, 0x80);
        }
        pos.vx = e->unk74.vx + e->unk9C.vx;
        pos.vz = e->unk74.vz + e->unk9C.vz;
        d = e->unk40;
        if (d < e->unk2E + 0x420) {
            d -= 0x420;
            d = e->unk2E - d;
            viewer.position.vx += d * rcos((short)e->unk56 - 0x400) >> 12;
            viewer.position.vz += d * rsin((short)e->unk56 - 0x400) >> 12;
        }
        break;
    case 2:
        e->unk2A += e->unkB0;
        if ((short)e->unk2A >= 0x1000) {
            e->unk08 = 7;
            e->unkB0 = 0xa0;
            e->unk1C.bytes[0] = 3;
            e->unk0E = 1;
            e->unk2A = 0;
            e->unk50 &= ~2;
        }
        break;
    case 3: {
        int target_x = 0x3c800;
        int target_z = 0x47000;

        if (e->unk74.vx < target_x) {
            creature_despawn(e);
            break;
        }
        fdat_801eb814(ratan2(e->unk74.vz - target_z, e->unk74.vx - target_x) + 0xc00, 0x20);
        fdat_801eb880(e->unk8E - 0x400, 0x40);
        e->unk2A += e->unkB0;
        if ((short)e->unk2A >= 0x1000)
            e->unk2A = 0;
        break;
    }
    }
    if (e->unk34 >= 0) {
        e->unk34 -= abs(e->unkB0);
        if (e->unk34 >= 0)
            return;
    }
    e->unk34 = -1;
    e->unk1C.bytes[2] &= 0xdf;
}
