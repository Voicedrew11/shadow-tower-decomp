/*
 * ST.EXE: model texture shifting, primitive-pointer slots, scratchpad GTE
 * helpers, lighting blends and colour clamps (WIP module).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <INLINE_C.H>
#include <st/gfx.h>
#include <st/mesh.h>
#include <st/scratch.h>
#include <st/light.h>

/* Texture fields shared by the textured primitive kinds. */
typedef struct {
    u8 u0, v0;
    u16 cba;
    u8 u1, v1;
    u16 tsb;
    u8 u2, v2;
    u16 unkA;
    u8 u3, v3;
} TexPrim;

/*
 * Moves every textured primitive of the first `count` parts (all parts
 * when zero) by a texture-page offset and a CLUT offset.
 */
ADDRESS(0x800177a0, 0x4bc)
void func_800177a0(Model *model, DVECTOR *uv, DVECTOR *clut, u8 count)
{
    ModelPart *part;
    u8 *p;
    TexPrim *t;
    int n, i;

    n = count;
    if (n == 0)
        n = model->parts_count;
    for (part = model->parts; n--; part++) {
        p = (u8 *)model + (part->offset + 12);
        for (i = part->counts[0]; i--; p += 20) {
            t = (TexPrim *)p;
            t->tsb += (uv->vx >> 6) + ((uv->vy >> 8) << 4);
            t->cba += (clut->vy << 6) + (clut->vx >> 4);
            t->u0 += uv->vx & 0x3f;
            t->v0 += uv->vy & 0xff;
            t->u1 += uv->vx & 0x3f;
            t->v1 += uv->vy & 0xff;
            t->u2 += uv->vx & 0x3f;
            t->v2 += uv->vy & 0xff;
        }
        for (i = part->counts[2]; i--; p += 28) {
            t = (TexPrim *)p;
            t->tsb += (uv->vx >> 6) + ((uv->vy >> 8) << 4);
            t->cba += (clut->vy << 6) + (clut->vx >> 4);
            t->u0 += uv->vx & 0x3f;
            t->v0 += uv->vy & 0xff;
            t->u1 += uv->vx & 0x3f;
            t->v1 += uv->vy & 0xff;
            t->u2 += uv->vx & 0x3f;
            t->v2 += uv->vy & 0xff;
            t->u3 += uv->vx & 0x3f;
            t->v3 += uv->vy & 0xff;
        }
        for (i = part->counts[1]; i--; p += 24) {
            t = (TexPrim *)p;
            t->tsb += (uv->vx >> 6) + ((uv->vy >> 8) << 4);
            t->cba += (clut->vy << 6) + (clut->vx >> 4);
            t->u0 += uv->vx & 0x3f;
            t->v0 += uv->vy & 0xff;
            t->u1 += uv->vx & 0x3f;
            t->v1 += uv->vy & 0xff;
            t->u2 += uv->vx & 0x3f;
            t->v2 += uv->vy & 0xff;
        }
        for (i = part->counts[3]; i--; p += 32) {
            t = (TexPrim *)p;
            t->tsb += (uv->vx >> 6) + ((uv->vy >> 8) << 4);
            t->cba += (clut->vy << 6) + (clut->vx >> 4);
            t->u0 += uv->vx & 0x3f;
            t->v0 += uv->vy & 0xff;
            t->u1 += uv->vx & 0x3f;
            t->v1 += uv->vy & 0xff;
            t->u2 += uv->vx & 0x3f;
            t->v2 += uv->vy & 0xff;
            t->u3 += uv->vx & 0x3f;
            t->v3 += uv->vy & 0xff;
        }
        /* The untextured kinds only count down (retail keeps the loops). */
        for (i = part->counts[4]; i--;)
            ;
        for (i = part->counts[6]; i--;)
            ;
        for (i = part->counts[5]; i--;)
            ;
        for (i = part->counts[7]; i--;)
            ;
    }
}

ADDRESS(0x80017c5c, 0x30)
void func_80017c5c(u16 index, void *p)
{
    world_state.unk8308[index] = p;
    world_state.unk8318 = p;
}

ADDRESS(0x80017c8c, 0x24)
void func_80017c8c(u16 index, void *p)
{
    world_state.unk8308[index] = p;
}

ADDRESS(0x80017cb0, 0x14)
u_char *func_80017cb0(u16 offset)
{
    return SCRATCHPAD->vertices + offset;
}

/* Loads three screen points from the scratchpad buffer and stores NCLIP. */
ADDRESS(0x80017cc4, 0x64)
void func_80017cc4(u16 i0, u16 i1, u16 i2)
{
    ScratchPad *sp = SCRATCHPAD;
    u_char *base = sp->vertices;

    sp->unk2C = (u_long *)(base + i0);
    sp->unk30 = (u_long *)(base + i1);
    sp->unk34 = (u_long *)(base + i2);
    gte_ldsxy3(*sp->unk2C, *sp->unk30, *sp->unk34);
    gte_nclip();
    gte_stopz(&SCRATCHPAD->opz);
}

/*
 * Blends the current lighting with one of three targets by t / 4096.
 *
 * Not exact (4 bytes long): retail keeps t in t0 and the scratchpad base in
 * a3 and interleaves each blend's multiplies; this build schedules them one
 * after another.
 */
ADDRESS(0x80017d28, 0x718)
void func_80017d28(u32 t, int flags)
{
    ScratchPad *sp = SCRATCHPAD;
    StLight *cur = world_state.light;
    StLight *to;
    short m[3][3];
    CVECTOR back1, back2, back3;
    u32 s;

    sp->unk394 = cur;
    if (flags & 0x20) {
        to = &lighting.light[47];
        s = 4096 - t;
        sp->unk64 = cur->unk68;
        sp->unk394 = to;
        sp->unk68 = cur->unk6A;
        m[0][0] = (to->color.m[0][0] * s + cur->color.m[0][0] * t) >> 12;
        m[0][1] = (to->color.m[0][1] * s + cur->color.m[0][1] * t) >> 12;
        m[0][2] = (to->color.m[0][2] * s + cur->color.m[0][2] * t) >> 12;
        m[1][0] = (to->color.m[1][0] * s + cur->color.m[1][0] * t) >> 12;
        m[1][1] = (to->color.m[1][1] * s + cur->color.m[1][1] * t) >> 12;
        m[1][2] = (to->color.m[1][2] * s + cur->color.m[1][2] * t) >> 12;
        m[2][0] = (to->color.m[2][0] * s + cur->color.m[2][0] * t) >> 12;
        m[2][1] = (to->color.m[2][1] * s + cur->color.m[2][1] * t) >> 12;
        m[2][2] = (to->color.m[2][2] * s + cur->color.m[2][2] * t) >> 12;
        back1.r = (to->back.r * s + cur->back.r * t) >> 12;
        back1.g = (to->back.g * s + cur->back.g * t) >> 12;
        back1.b = (to->back.b * s + cur->back.b * t) >> 12;
        gte_SetColorMatrix(m);
        gte_SetBackColor(back1.r, back1.g, back1.b);
    } else if (flags & 0x10) {
        to = &lighting.light[46];
        sp->unk64 = cur->unk68;
        s = 4096 - t;
        sp->unk394 = to;
        sp->unk68 = cur->unk6A;
        m[0][0] = ((to->color.m[0][0] + cur->color.m[0][0]) * s) >> 13;
        m[0][1] = ((to->color.m[0][1] + cur->color.m[0][1]) * s) >> 13;
        m[0][2] = ((to->color.m[0][2] + cur->color.m[0][2]) * s) >> 13;
        m[1][0] = ((to->color.m[1][0] + cur->color.m[1][0]) * s) >> 13;
        m[1][1] = ((to->color.m[1][1] + cur->color.m[1][1]) * s) >> 13;
        m[1][2] = ((to->color.m[1][2] + cur->color.m[1][2]) * s) >> 13;
        m[2][0] = ((to->color.m[2][0] + cur->color.m[2][0]) * s) >> 13;
        m[2][1] = ((to->color.m[2][1] + cur->color.m[2][1]) * s) >> 13;
        m[2][2] = ((to->color.m[2][2] + cur->color.m[2][2]) * s) >> 13;
        back2.r = ((to->back.r + cur->back.r) * s) >> 13;
        back2.g = ((to->back.g + cur->back.g) * s) >> 13;
        back2.b = ((to->back.b + cur->back.b) * s) >> 13;
        gte_SetColorMatrix(m);
        gte_SetBackColor(back2.r, back2.g, back2.b);
    } else if (flags & 0x8) {
        to = &lighting.light[45];
        sp->unk64 = cur->unk68;
        sp->unk394 = to;
        sp->unk68 = cur->unk6A;
        m[0][0] = ((to->color.m[0][0] + cur->color.m[0][0]) * t) >> 13;
        m[0][1] = ((to->color.m[0][1] + cur->color.m[0][1]) * t) >> 13;
        m[0][2] = ((to->color.m[0][2] + cur->color.m[0][2]) * t) >> 13;
        m[1][0] = ((to->color.m[1][0] + cur->color.m[1][0]) * t) >> 13;
        m[1][1] = ((to->color.m[1][1] + cur->color.m[1][1]) * t) >> 13;
        m[1][2] = ((to->color.m[1][2] + cur->color.m[1][2]) * t) >> 13;
        m[2][0] = ((to->color.m[2][0] + cur->color.m[2][0]) * t) >> 13;
        m[2][1] = ((to->color.m[2][1] + cur->color.m[2][1]) * t) >> 13;
        m[2][2] = ((to->color.m[2][2] + cur->color.m[2][2]) * t) >> 13;
        back3.r = ((to->back.r + cur->back.r) * t) >> 13;
        back3.g = ((to->back.g + cur->back.g) * t) >> 13;
        back3.b = ((to->back.b + cur->back.b) * t) >> 13;
        gte_SetColorMatrix(m);
        gte_SetBackColor(back3.r, back3.g, back3.b);
    } else {
        gte_SetBackColor(cur->back.r, cur->back.g, cur->back.b);
        gte_SetColorMatrix(cur->color.m);
        sp->unk64 = cur->unk68;
        sp->unk68 = cur->unk6A;
    }
}

ADDRESS(0x80018440, 0x40)
void mesh_ft3_tint_a(CVECTOR *c)
{
    if (c->b > 85)
        c->b = 85;
    c->r = c->b * 3;
    c->g = c->b * 2;
}

ADDRESS(0x80018480, 0x44)
void mesh_ft3_tint_b(CVECTOR *c)
{
    if (c->r > 85)
        c->r = 85;
    c->b = c->r * 3;
    c->g = c->r * 3;
}
