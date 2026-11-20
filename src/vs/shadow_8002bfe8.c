/*
 * VS.EXE: a fighter's ground shadow (WIP module).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <INLINE_C.H>
#include <vs/data.h>
#include <vs/buffer.h>

/*
 * Draws fighter n's shadow: a semi-transparent textured square of its hit
 * radius scaled by scale/4096, on the ground under it, through the camera
 * matrix.
 */
ADDRESS(0x8002bfe8, 0x2dc)
void func_8002bfe8(int n, int scale)
{
    VsFighter *base = DAT_800633b0.fighter;
    VsFighter *f = base + n;
    int r = f->unk4DF18.unk1B4 * scale / 4096;
    MATRIX m;
    SVECTOR v[4];
    StPacketBuffer *packet;
    POLY_FT4 *p;

    v[0].vx = -r;
    v[0].vy = 0;
    v[0].vz = -r;
    v[1].vx = r;
    v[1].vy = 0;
    v[1].vz = -r;
    v[2].vx = -r;
    v[2].vy = 0;
    v[2].vz = r;
    v[3].vx = r;
    v[3].vy = 0;
    v[3].vz = r;
    m.t[0] = f->unk4DEF0.vx - vs_gfx_state.unk2CA78.vx;
    m.t[1] = -vs_gfx_state.unk2CA78.vy;
    m.t[2] = f->unk4DEF0.vz - vs_gfx_state.unk2CA78.vz;
    m.m[0][0] = 4096;
    m.m[0][1] = 0;
    m.m[0][2] = 0;
    m.m[1][0] = 0;
    m.m[1][1] = 4096;
    m.m[1][2] = 0;
    m.m[2][0] = 0;
    m.m[2][1] = 0;
    m.m[2][2] = 4096;
    CompMatrix(&vs_gfx_state.unk2C998, &m, &m);
    gte_SetRotMatrix(&m);
    gte_SetTransMatrix(&m);
    gte_ldv3c(&v[0]);
    gte_rtpt();
    gte_stsxy3(&v[0], &v[1], &v[2]);
    gte_ldv0(&v[3]);
    gte_rtps();
    gte_stsxy(&v[3]);
    packet = vs_gfx_state.packet;
    p = (POLY_FT4 *)packet->cur;
    packet->cur = (u_long *)(p + 1);
    setPolyFT4(p);
    setSemiTrans(p, 1);
    setXY4(p, v[0].vx, v[0].vy, v[1].vx, v[1].vy, v[2].vx, v[2].vy, v[3].vx, v[3].vy);
    setUV4(p, 164, 0, 227, 0, 164, 63, 227, 63);
    p->clut = 30816;
    p->tpage = 93;
    setRGB0(p, 255, 255, 255);
    addPrim(&vs_gfx_state.ot_cur[4090], p);
}
