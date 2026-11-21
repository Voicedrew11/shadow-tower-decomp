/*
 * VS.EXE: moving and drawing the hit-spark particles (WIP module).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <INLINE_C.H>
#include <vs/data.h>

/*
 * Moves every live particle (gravity 20 per frame; one reaching the
 * ground stops there), then, if any lives, draws those on the ground as
 * 2x2 tiles fading out over their last 32 frames and those in the air as
 * single pixels.
 */
ADDRESS(0x8002d998, 0x340)
void func_8002d998(void)
{
    VsRecord20 *rec = DAT_801c4a58;
    int count = 0;
    int i = 2000;
    MATRIX m;
    long z;
    StPacketBuffer *packet;
    TILE *tile;
    TILE_1 *dot;

    while (i--) {
        if (rec->unk10 != 0) {
            rec->unk10--;
            rec->unk0.vx += rec->unk8.vx;
            rec->unk0.vy += rec->unk8.vy;
            if (rec->unk0.vy > 0) {
                rec->unk0.vy = 0;
                rec->unk8.vx = 0;
                rec->unk8.vy = 0;
                rec->unk8.vz = 0;
            }
            count++;
            rec->unk0.vz += rec->unk8.vz;
            rec->unk8.vy += 20;
        }
        rec++;
    }
    if (count == 0)
        return;
    m.m[0][0] = 4096;
    m.m[0][1] = 0;
    m.m[0][2] = 0;
    m.m[1][0] = 0;
    m.m[1][1] = 4096;
    m.m[1][2] = 0;
    m.m[2][0] = 0;
    m.m[2][1] = 0;
    m.m[2][2] = 4096;
    m.t[0] = -DAT_801624d0.unk2CA78.vx;
    m.t[1] = -DAT_801624d0.unk2CA78.vy;
    m.t[2] = -DAT_801624d0.unk2CA78.vz;
    CompMatrix(&DAT_801624d0.unk2C998, &m, &m);
    gte_SetRotMatrix(&m);
    gte_SetTransMatrix(&m);
    rec = DAT_801c4a58;
    i = 2000;
    while (i--) {
        if (rec->unk10 != 0) {
            if (rec->unk0.vy == 0) {
                gte_ldv0(&rec->unk0);
                gte_rtps();
                packet = DAT_801624d0.packet;
                tile = (TILE *)packet->cur;
                packet->cur = (u_long *)(tile + 1);
                setTile(tile);
                tile->w = 2;
                tile->h = 2;
                gte_stsxy(&tile->x0);
                gte_stsz(&z);
                if (rec->unk10 < 32)
                    tile->r0 = rec->unk10 * 100 / 32;
                else
                    tile->r0 = 100;
                tile->g0 = 0;
                tile->b0 = 0;
                addPrim(&DAT_801624d0.ot_cur[4091], tile);
            } else {
                gte_ldv0(&rec->unk0);
                gte_rtps();
                packet = DAT_801624d0.packet;
                dot = (TILE_1 *)packet->cur;
                packet->cur = (u_long *)(dot + 1);
                setTile1(dot);
                gte_stsxy(&dot->x0);
                gte_stsz(&z);
                setRGB0(dot, 100, 0, 0);
                addPrim(&DAT_801624d0.ot_cur[1], dot);
            }
        }
        rec++;
    }
}
