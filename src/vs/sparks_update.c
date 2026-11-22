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
void sparks_update(void)
{
    VsSpark *rec = sparks;
    int count = 0;
    int i = 2000;
    MATRIX m;
    long z;
    StPacketBuffer *packet;
    TILE *tile;
    TILE_1 *dot;

    while (i--) {
        if (rec->life != 0) {
            rec->life--;
            rec->position.vx += rec->velocity.vx;
            rec->position.vy += rec->velocity.vy;
            if (rec->position.vy > 0) {
                rec->position.vy = 0;
                rec->velocity.vx = 0;
                rec->velocity.vy = 0;
                rec->velocity.vz = 0;
            }
            count++;
            rec->position.vz += rec->velocity.vz;
            rec->velocity.vy += 20;
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
    m.t[0] = -vs_gfx_state.camera_pos.vx;
    m.t[1] = -vs_gfx_state.camera_pos.vy;
    m.t[2] = -vs_gfx_state.camera_pos.vz;
    CompMatrix(&vs_gfx_state.view_matrix, &m, &m);
    gte_SetRotMatrix(&m);
    gte_SetTransMatrix(&m);
    rec = sparks;
    i = 2000;
    while (i--) {
        if (rec->life != 0) {
            if (rec->position.vy == 0) {
                gte_ldv0(&rec->position);
                gte_rtps();
                packet = vs_gfx_state.packet;
                tile = (TILE *)packet->cur;
                packet->cur = (u_long *)(tile + 1);
                setTile(tile);
                tile->w = 2;
                tile->h = 2;
                gte_stsxy(&tile->x0);
                gte_stsz(&z);
                if (rec->life < 32)
                    tile->r0 = rec->life * 100 / 32;
                else
                    tile->r0 = 100;
                tile->g0 = 0;
                tile->b0 = 0;
                addPrim(&vs_gfx_state.ot_cur[4091], tile);
            } else {
                gte_ldv0(&rec->position);
                gte_rtps();
                packet = vs_gfx_state.packet;
                dot = (TILE_1 *)packet->cur;
                packet->cur = (u_long *)(dot + 1);
                setTile1(dot);
                gte_stsxy(&dot->x0);
                gte_stsz(&z);
                setRGB0(dot, 100, 0, 0);
                addPrim(&vs_gfx_state.ot_cur[1], dot);
            }
        }
        rec++;
    }
}
