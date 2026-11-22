/*
 * ST.EXE: the world renderer and its neighbours as one translation unit
 * (WIP module, -G8): animated VRAM textures, the frame buffers and
 * primitives, the mesh drawers, the visibility lists, the world and model
 * drawers, the quads, fades, banks and object slots, the HUD and the menu
 * text, the lighting and the animation, 0x80016878..0x80034604. Retail
 * writes the constants of quad, model_draw, bank and ui (0x80011000) before
 * the switch tables of texanim, objects2, ui and anim (0x800113b0), each
 * table 8-aligned relative to 0x80011000: the order cc1 gives one -G8
 * object's read-only data. Where the object starts before texanim is not
 * fixed by its data.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <psyq/libc.h>
#include <st/data.h>
#include <st/gfx.h>
#include <LIBETC.H>
#include <INLINE_C.H>
#include <st/memory.h>
#include <st/cd.h>
#include <st/scratch.h>
#include <st/funcs.h>
#include <st/anim.h>
#include <st/mesh.h>
#include <st/light.h>
#include <st/vis.h>
#include <RAND.H>
#include <kf/lib/null.h>
#include <LIBCD.H>
#include <st/tim.h>
#include <GTEMAC.H>
#include <LIBSND.H>
#include <st/bank.h>
#include <LIBSPU.H>
#include <st/menu.h>
#include <st/objects.h>
#include <st/effect.h>
#include <st/hud.h>
#include <st/text.h>

RODATA(0x80011000, 0x4cc)
SDATA(0x80087ef4, 0x24)
SDATA(0x80087f1c, 0x18)
SDATA(0x80087f38, 0x30)
SDATA(0x80087f70, 0x14)
SDATA(0x80087f88, 0x8c)

/*
 * The module's first .sdata (retail 0x80087ed8..0x80087ef3, before quad's
 * literals): two RECTs and eight zero bytes nothing references, and the
 * NCDS input colour player_draw_equipment_model draws with (128, 128, 128), which it
 * loads $gp-relative.
 */
DATA(0x80087ed8, 0x8, ".sdata")
static const RECT DAT_80087ed8 = {320, 0, 320, 240};
DATA(0x80087ee0, 0x8, ".sdata")
static const RECT DAT_80087ee0 = {960, 256, 64, 240};
DATA(0x80087ee8, 0x4, ".sdata")
u_long equipment_model_rgb = 0x00808080;
DATA(0x80087eec, 0x8, ".sdata")
static u32 DAT_80087eec[2] = {0, 0};

#define TEXANIM_FREE 255
/* Slot area that is active everywhere. */
#define TEXANIM_ANY_AREA 255

ADDRESS(0x80016878, 0x2c)
void texanim_clear_all(void)
{
    StTexAnim *slot = world_state.texanim;
    int i = ST_TEXANIM_SLOTS;

    while (i--) {
        slot->type = TEXANIM_FREE;
        slot++;
    }
}

ADDRESS(0x800168a4, 0x3c)
StTexAnim *texanim_find_free(void)
{
    StTexAnim *slot = world_state.texanim;
    int i;

    for (i = ST_TEXANIM_SLOTS; i != 0; i--, slot++) {
        if (slot->type == TEXANIM_FREE)
            goto found;
    }
    return 0;
found:
    return slot;
}

/*
 * Returns no value although it is not void: retail keeps v0 live to the
 * exit, which decides the delay slot of the switch's range branch.
 */
ADDRESS(0x800168e0, 0x134)
int texanim_start(RECT *rect, int period, int step, int type, short area)
{
    StTexAnim *slot = texanim_find_free();
    RECT unused;

    if (slot != 0) {
        slot->type = type;
        slot->phase = 0;
        slot->area = area;
        slot->period = period;
        slot->step = step;
        slot->countdown = 0;
        slot->dst = slot->src = *rect;
        switch (type) {
        case 1:
        case 2:
        case 6:
        case 33:
        case 34:
            slot->src.x += slot->src.w >> 2;
            slot->dst.w = slot->dst.w >> 2;
            slot->src.w = slot->src.w >> 2;
            break;
        case 49:
            StoreImage(&slot->src, (u_long *)texanim_clut_buffer);
            break;
        }
    }
}

ADDRESS(0x80016a14, 0x5a4)
void texanim_update(void)
{
    StTexAnim *slot;
    RECT r;
    int i;
    int x, y, offset;
    u16 first;

    for (slot = world_state.texanim, i = ST_TEXANIM_SLOTS; i--;) {
        if (slot->area == area_state.area || slot->area == TEXANIM_ANY_AREA) {
            if (slot->countdown == 0) {
                r = slot->src;
                slot->countdown = slot->period;
                switch (slot->type) {
                case 1 ... 5:
                    /* Scroll vertically. */
                    slot->phase += slot->step;
                    if (slot->phase >= slot->src.h)
                        slot->phase -= slot->src.h;
                    x = slot->dst.x;
                    y = slot->dst.y + slot->phase;
                    r.h = slot->src.h - slot->phase;
                    MoveImage(&r, x, y);
                    if (slot->phase) {
                        y = slot->dst.y;
                        r.y = slot->src.y + r.h;
                        r.h = slot->phase;
                        MoveImage(&r, x, y);
                    }
                    break;
                case 6 ... 9: {
                    short row_phase, row_count;

                    /* Scroll each row horizontally, one more pixel per row. */
                    slot->phase += slot->step;
                    if (slot->phase >= slot->src.w)
                        slot->phase -= slot->src.w;
                    row_count = slot->src.h;
                    r.h = 1;
                    row_phase = slot->phase;
                    y = slot->dst.y;
                    for (; row_count--;) {
                        if (row_phase >= slot->src.w)
                            row_phase -= slot->src.w;
                        x = slot->dst.x + row_phase;
                        r.x = slot->src.x;
                        r.w = slot->src.w - row_phase;
                        MoveImage(&r, x, y);
                        if (row_phase) {
                            x = slot->dst.x;
                            r.x = slot->src.x + r.w;
                            r.w = row_phase;
                            MoveImage(&r, x, y);
                        }
                        y++;
                        row_phase++;
                        r.y++;
                    }
                    break;
                }
                case 33 ... 37: {
                    short column_phase, column_count;

                    /* Wave each column vertically. */
                    slot->phase += slot->step;
                    if (slot->phase >= slot->src.h)
                        slot->phase -= slot->src.h;
                    column_count = slot->src.w;
                    r.w = 1;
                    column_phase = slot->phase;
                    x = slot->dst.x;
                    for (; column_count--;) {
                        offset = slot->src.h / 2 +
                                 slot->src.h / 4 * rsin(4096 / slot->src.w * column_phase) / 4096;
                        if (offset >= slot->src.h)
                            offset -= slot->src.h;
                        y = slot->dst.y + offset;
                        r.y = slot->src.y;
                        r.h = slot->src.h - offset;
                        MoveImage(&r, x, y);
                        if (offset) {
                            y = slot->dst.y;
                            r.y = slot->src.y + r.h;
                            r.h = offset;
                            MoveImage(&r, x, y);
                        }
                        x++;
                        column_phase++;
                        r.x++;
                    }
                    break;
                }
                case 17 ... 20:
                    /* Step through frames stacked below the source. */
                    if (++slot->phase >= slot->step) {
                        slot->phase = 0;
                        LoadImage(&r, (u_long *)texanim_clut_buffer);
                    } else {
                        r.y += slot->phase * slot->src.h;
                        MoveImage(&r, slot->dst.x, slot->dst.y);
                    }
                    break;
                case 49:
                    /* Entry zero is fixed; rotate the fourteen following colours. */
                    {
                        u16 *colors = &texanim_clut_buffer[1];

                        first = colors[0];
                        colors[0] = colors[1];
                        colors[1] = colors[2];
                        colors[2] = colors[3];
                        colors[3] = colors[4];
                        colors[4] = colors[5];
                        colors[5] = colors[6];
                        colors[6] = colors[7];
                        colors[7] = colors[8];
                        colors[8] = colors[9];
                        colors[9] = colors[10];
                        colors[10] = colors[11];
                        colors[11] = colors[12];
                        colors[12] = colors[13];
                        colors[13] = first;
                        LoadImage(&r, (u_long *)(colors - 1));
                    }
                    break;
                case TEXANIM_FREE:
                    break;
                }
            } else {
                slot->countdown--;
            }
        }
        slot++;
    }
}

#define OT_LENGTH 4096
#define OT2_LENGTH 64
#define PACKET_WORDS (0x20000 / 4)

ADDRESS(0x80016fb8, 0x210)
void gfx_init(void)
{
    int h = 240;
    int i;
    u8 *p;

    ResetGraph(3);
    SetGraphDebug(0);
    InitGeom();
    gte_SetGeomOffset(160, 120);
    gte_SetGeomScreen(300);
    SetDefDrawEnv(&world_state.draw[0], 0, 0, 320, h);
    SetDefDispEnv(&world_state.disp[0], 0, h, 320, h);
    SetDefDrawEnv(&world_state.draw[1], 0, h, 320, h);
    SetDefDispEnv(&world_state.disp[1], 0, 0, 320, h);
    world_state.draw[0].dtd = 1;
    world_state.draw[1].dtd = 1;
    world_state.draw[0].isbg = 1;
    world_state.draw[1].isbg = 1;
    setRGB0(&world_state.draw[0], 0, 0, 0);
    setRGB0(&world_state.draw[1], 0, 0, 0);
    PutDispEnv(&world_state.disp[0]);
    PutDrawEnv(&world_state.draw[0]);
    gte_SetBackColor(80, 80, 80);
    world_state.frame = 255;
    world_state.packet_base = packet_buffers;
    world_state.packets[0].start = world_state.packet_base;
    world_state.packets[0].end = world_state.packets[0].start + PACKET_WORDS;
    world_state.packets[1].start = world_state.packets[0].end;
    world_state.packets[1].end = world_state.packets[1].start + PACKET_WORDS;
    world_state.unk1ECCF = 0;
    world_state.unk1ECCD = 0;
    world_state.unk1ECCE = 0;
    p = world_state.unk1ECC5;
    i = 8;
    while (i--)
        *p++ = 255;
    texanim_clear_all();
    world_state.unk1ECD2 = 255;
    world_state.unk1ECC3 = 13;
    world_state.no_plane_test = 0;
    world_state.unk1EDF0 = 0;
    world_state.draw_collision = 0;
    placement_objects.frame = 0;
    anim_slots_init();
    anim_sets_init();
    world_state.render_mode = 17;
    world_state.selected_light = 0;
    world_state.light_blend = 0;
    world_state.talk_overlay_state = 0;
}

ADDRESS(0x800171c8, 0xf4)
void gfx_setup_640x480(void)
{
    int h = 480;

    gte_SetGeomOffset(320, 240);
    gte_SetGeomScreen(300);
    SetDefDrawEnv(&world_state.draw[0], 0, 0, 640, h);
    SetDefDispEnv(&world_state.disp[0], 0, h, 640, h);
    SetDefDrawEnv(&world_state.draw[1], 0, 0, 640, h);
    SetDefDispEnv(&world_state.disp[1], 0, h, 640, h);
    world_state.disp[0].isinter = 1;
    world_state.draw[0].dfe = 0;
    world_state.draw[0].dtd = 1;
    world_state.draw[1].dtd = 1;
    world_state.draw[0].isbg = 1;
    world_state.draw[1].isbg = 1;
    setRGB0(&world_state.draw[0], 0, 0, 0);
    setRGB0(&world_state.draw[1], 0, 0, 0);
}

ADDRESS(0x800172bc, 0xec)
void gfx_setup_640x240(void)
{
    int h = 240;

    gte_SetGeomOffset(320, 120);
    gte_SetGeomScreen(300);
    SetDefDrawEnv(&world_state.draw[0], 0, 0, 640, h);
    SetDefDispEnv(&world_state.disp[0], 0, h, 640, h);
    SetDefDrawEnv(&world_state.draw[1], 0, h, 640, h);
    SetDefDispEnv(&world_state.disp[1], 0, 0, 640, h);
    world_state.draw[0].dtd = 1;
    world_state.draw[1].dtd = 1;
    world_state.draw[0].isbg = 1;
    world_state.draw[1].isbg = 1;
    setRGB0(&world_state.draw[0], 0, 0, 0);
    setRGB0(&world_state.draw[1], 0, 0, 0);
}

ADDRESS(0x800173a8, 0x11c)
void gfx_setup_320x240(void)
{
    int h = 240;

    gte_SetGeomOffset(160, 120);
    gte_SetGeomScreen(300);
    SetDefDrawEnv(&world_state.draw[0], 0, 0, 320, h);
    SetDefDispEnv(&world_state.disp[0], 0, h, 320, h);
    SetDefDrawEnv(&world_state.draw[1], 0, h, 320, h);
    SetDefDispEnv(&world_state.disp[1], 0, 0, 320, h);
    world_state.draw[0].dtd = 1;
    world_state.draw[1].dtd = 1;
    setRGB0(&world_state.draw[0], 0, 0, 0);
    setRGB0(&world_state.draw[1], 0, 0, 0);
    PutDrawEnv(&world_state.draw[0]);
    PutDrawEnv(&world_state.draw[1]);
    PutDispEnv(&world_state.disp[0]);
    PutDispEnv(&world_state.disp[1]);
}

ADDRESS(0x800174c4, 0xe0)
void gfx_begin_frame(void)
{
    world_state.frame = !world_state.frame;
    world_state.packet = &world_state.packets[world_state.frame];
    world_state.packet->cur = world_state.packet->start;
    world_state.ot_cur = world_state.ot[world_state.frame];
    ClearOTagR(world_state.ot_cur, OT_LENGTH);
    world_state.ot2_cur = world_state.ot2[world_state.frame];
    ClearOTagR(world_state.ot2_cur, OT2_LENGTH);
    world_state.vertex_face_counts[0] = world_state.vertex_face_counts[1] =
        world_state.prim_count = world_state.prim_alloc_count = 0;
    world_state.div_prim_count = 0;
    world_state.unk8330 = 0;
    world_state.unk8338[0] = 0;
    world_state.unk8338[1] = 0;
    world_state.unk8338[2] = 0;
    world_state.unk8338[3] = 0;
    world_state.unk8338[4] = 0;
}

ADDRESS(0x800175a4, 0x90)
void gfx_flip(void)
{
    DrawSync(0);
    VSync(3);
    PutDispEnv(&world_state.disp[world_state.frame]);
    PutDrawEnv(&world_state.draw[world_state.frame]);
}

ADDRESS(0x80017634, 0x4c)
void gfx_draw_ot(void)
{
    AddPrims(world_state.ot_cur, &world_state.ot2_cur[OT2_LENGTH - 1],
             world_state.ot2_cur);
    DrawOTag(&world_state.ot_cur[OT_LENGTH - 1]);
}

ADDRESS(0x80017680, 0x28)
void gfx_end_frame(void)
{
    gfx_flip();
    gfx_draw_ot();
}

ADDRESS(0x800176a8, 0xf8)
void gfx_set_camera(VECTOR *pos, SVECTOR *rot)
{
    SVECTOR angle;

    if (pos)
        world_state.camera_pos = *pos;
    if (rot)
        world_state.camera_rot = *rot;
    angle.vx = world_state.camera_rot.vx;
    angle.vz = world_state.camera_rot.vz;
    angle.vy = world_state.camera_rot.vy;
    RotMatrix(&angle, &world_state.view_matrix);
    matrix_rot_x(angle.vx, &world_state.pitch_matrix);
    matrix_quarter_turns_y(&world_state.view_matrix, world_state.view_quarter_turns);
}

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
void model_offset_textures(Model *model, DVECTOR *uv, DVECTOR *clut, u8 count)
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
    world_state.area_geometry[index] = p;
    world_state.unk8318 = p;
}

ADDRESS(0x80017c8c, 0x24)
void func_80017c8c(u16 index, void *p)
{
    world_state.area_geometry[index] = p;
}

ADDRESS(0x80017cb0, 0x14)
u_char *func_80017cb0(u16 offset)
{
    return SCRATCHPAD->vertices + offset;
}

/* Loads three screen points from the scratchpad buffer and stores NCLIP. */
ADDRESS(0x80017cc4, 0x64)
void mesh_nclip_triangle(u16 i0, u16 i1, u16 i2)
{
    ScratchPad *sp = SCRATCHPAD;
    u_char *base = sp->vertices;

    sp->unk2C = (u_long *)(base + i0);
    sp->unk30 = (u_long *)(base + i1);
    sp->vertex_out = (u_long *)(base + i2);
    gte_ldsxy3(*sp->unk2C, *sp->unk30, *sp->vertex_out);
    gte_nclip();
    gte_stopz(&SCRATCHPAD->opz);
}

/* Blends the current lighting with one of three targets by t / 4096. */
ADDRESS(0x80017d28, 0x718)
void lighting_load_gte(u32 t, int flags)
{
    ScratchPad *sp = SCRATCHPAD;
    StLight *cur = world_state.light;
    short m[3][3];
    CVECTOR back1, back2, back3;
    u32 to_00, current_00;
    u32 to_01, current_01;
    u32 to_02, current_02;
    u32 to_10, current_10;
    u32 to_11, current_11;
    u32 to_12, current_12;
    u32 to_20, current_20;
    u32 to_21, current_21;
    u32 to_22, current_22;
    u32 to_r, current_r;
    u32 to_g, current_g;
    u32 to_b, current_b;

    sp->model_light = cur;
    if (flags & 0x20) {
        StLight *to = &lighting.light[47];
        u32 s = 4096 - t;
        to_00 = to->color.m[0][0] * s;
        sp->depth_cue_near = cur->depth_cue_near;
        sp->depth_cue_far = cur->depth_cue_far;
        sp->model_light = to;
        current_00 = cur->color.m[0][0] * t;
        to_01 = to->color.m[0][1] * s;
        m[0][0] = (to_00 + current_00) >> 12;
        current_01 = cur->color.m[0][1] * t;
        to_02 = to->color.m[0][2] * s;
        m[0][1] = (to_01 + current_01) >> 12;
        current_02 = cur->color.m[0][2] * t;
        to_10 = to->color.m[1][0] * s;
        m[0][2] = (to_02 + current_02) >> 12;
        current_10 = cur->color.m[1][0] * t;
        to_11 = to->color.m[1][1] * s;
        m[1][0] = (to_10 + current_10) >> 12;
        current_11 = cur->color.m[1][1] * t;
        to_12 = to->color.m[1][2] * s;
        m[1][1] = (to_11 + current_11) >> 12;
        current_12 = cur->color.m[1][2] * t;
        to_20 = to->color.m[2][0] * s;
        m[1][2] = (to_12 + current_12) >> 12;
        current_20 = cur->color.m[2][0] * t;
        to_21 = to->color.m[2][1] * s;
        m[2][0] = (to_20 + current_20) >> 12;
        current_21 = cur->color.m[2][1] * t;
        to_22 = to->color.m[2][2] * s;
        m[2][1] = (to_21 + current_21) >> 12;
        current_22 = cur->color.m[2][2] * t;
        to_r = to->back.r * s;
        m[2][2] = (to_22 + current_22) >> 12;
        current_r = cur->back.r * t;
        back1.r = (to_r + current_r) >> 12;
        to_g = to->back.g * s;
        current_g = cur->back.g * t;
        to_b = to->back.b * s;
        back1.g = (to_g + current_g) >> 12;
        current_b = cur->back.b * t;
        back1.b = (to_b + current_b) >> 12;
        gte_SetColorMatrix(m);
        gte_SetBackColor(back1.r, back1.g, back1.b);
    } else if (flags & 0x10) {
        StLight *to = &lighting.light[46];
        sp->depth_cue_near = cur->depth_cue_near;
        sp->depth_cue_far = cur->depth_cue_far;
        sp->model_light = to;
        m[0][0] = ((to->color.m[0][0] + cur->color.m[0][0]) * (4096 - t)) >> 13;
        m[0][1] = ((to->color.m[0][1] + cur->color.m[0][1]) * (4096 - t)) >> 13;
        m[0][2] = ((to->color.m[0][2] + cur->color.m[0][2]) * (4096 - t)) >> 13;
        m[1][0] = ((to->color.m[1][0] + cur->color.m[1][0]) * (4096 - t)) >> 13;
        m[1][1] = ((to->color.m[1][1] + cur->color.m[1][1]) * (4096 - t)) >> 13;
        m[1][2] = ((to->color.m[1][2] + cur->color.m[1][2]) * (4096 - t)) >> 13;
        m[2][0] = ((to->color.m[2][0] + cur->color.m[2][0]) * (4096 - t)) >> 13;
        m[2][1] = ((to->color.m[2][1] + cur->color.m[2][1]) * (4096 - t)) >> 13;
        m[2][2] = ((to->color.m[2][2] + cur->color.m[2][2]) * (4096 - t)) >> 13;
        back2.r = ((to->back.r + cur->back.r) * (4096 - t)) >> 13;
        back2.g = ((to->back.g + cur->back.g) * (4096 - t)) >> 13;
        back2.b = ((to->back.b + cur->back.b) * (4096 - t)) >> 13;
        gte_SetColorMatrix(m);
        gte_SetBackColor(back2.r, back2.g, back2.b);
    } else if (flags & 0x8) {
        StLight *to = &lighting.light[45];
        sp->depth_cue_near = cur->depth_cue_near;
        sp->depth_cue_far = cur->depth_cue_far;
        sp->model_light = to;
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
        sp->depth_cue_near = cur->depth_cue_near;
        sp->depth_cue_far = cur->depth_cue_far;
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

ADDRESS(0x800184c4, 0x2e0)
void mesh_draw_ft3(int count, MeshFaceFT3 *face, u_long *ot, int flags)
{
    ScratchPad *sp = SCRATCHPAD;
    u_long *start;
    void (*tint)(CVECTOR *);
    MeshVertex *v0;
    MeshVertex *v1;
    MeshVertex *v2;
    POLY_FT3 *p;
    int t;
    MeshFaceFT3 *next;
    short z;

    tint = 0;
    start = sp->prim;
    sp->face_count += count;
    if (MESH_MODE & MESH_TINT_A) {
        tint = mesh_ft3_tint_a;
    } else if (MESH_MODE & MESH_TINT_B) {
        tint = mesh_ft3_tint_b;
    }
    while ((t = count, count = count - 1, t)) {
        v0 = (MeshVertex *)(sp->vertices + face->i0);
        v1 = (MeshVertex *)(sp->vertices + face->i1);
        v2 = (MeshVertex *)(sp->vertices + face->i2);
        gte_ldsxy3(*(u_long *)&v0->sxy, *(u_long *)&v1->sxy, *(u_long *)&v2->sxy);
        next = face + 1;
        gte_nclip();
        sp->rgb.cd = face->code | ((flags & 2) | 0x24);
        gte_stopz(&sp->opz);
        if (sp->opz > 0) {
            z = ((v0->z + v1->z + v2->z) >> 5) / 3 + face->depth_bias;
            if (z < sp->far_z && z > sp->near_z) {
                p = (POLY_FT3 *)sp->prim;
                sp->prim = (u_long *)(p + 1);
                gte_stsxy3_ft3(p);
                gte_ldv0(sp->normals + face->normal);
                *(u_long *)&p->u0 = face->uv0_clut;
                gte_ldrgb(&sp->rgb);
                *(u_long *)&p->u1 = face->uv1_tpage | (flags & 0x600000);
                gte_lddp((v0->p + v1->p + v2->p) / 3);
                *(u_short *)&p->u2 = face->uv2;
                gte_ncds_b();
                mesh_add_prim(ot + z, p, 7);
                gte_strgb(&p->r0);
                if (MESH_MODE & (MESH_TINT_A | MESH_TINT_B)) {
                    tint((CVECTOR *)&p->r0);
                }
            }
        }
        face = next;
    }
    sp->faces = (u8 *)face;
    sp->prim_count.count += (sp->prim - start) / 8;
}

ADDRESS(0x800187a4, 0x40)
void mesh_ft4_tint_a(CVECTOR *rgb)
{
    if (rgb->b > 85) {
        rgb->b = 85;
    }
    rgb->r = rgb->b * 3;
    rgb->g = rgb->b * 2;
}

ADDRESS(0x800187e4, 0x44)
void mesh_ft4_tint_b(CVECTOR *rgb)
{
    if (rgb->r > 85) {
        rgb->r = 85;
    }
    rgb->b = rgb->r * 3;
    rgb->g = rgb->r * 3;
}

ADDRESS(0x80018828, 0x2f4)
void mesh_draw_ft4(int count, MeshFaceFT4 *face, u_long *ot, int flags)
{
    ScratchPad *sp = SCRATCHPAD;
    u_long *start;
    void (*tint)(CVECTOR *);
    MeshVertex *v0;
    MeshVertex *v1;
    MeshVertex *v2;
    MeshVertex *v3;
    POLY_FT4 *p;
    int t;
    MeshFaceFT4 *next;
    short z;

    tint = 0;
    start = sp->prim;
    sp->face_count += count;
    if (MESH_MODE & MESH_TINT_A) {
        tint = mesh_ft4_tint_a;
    } else if (MESH_MODE & MESH_TINT_B) {
        tint = mesh_ft4_tint_b;
    }
    while ((t = count, count = count - 1, t)) {
        v0 = (MeshVertex *)(sp->vertices + face->i0);
        v1 = (MeshVertex *)(sp->vertices + face->i1);
        v2 = (MeshVertex *)(sp->vertices + face->i2);
        gte_ldsxy3(*(u_long *)&v0->sxy, *(u_long *)&v1->sxy, *(u_long *)&v2->sxy);
        next = face + 1;
        gte_nclip();
        sp->rgb.cd = face->code | ((flags & 2) | 0x2c);
        gte_stopz(&sp->opz);
        if (sp->opz > 0) {
            v3 = (MeshVertex *)(sp->vertices + face->i3);
            z = ((v0->z + v1->z + v2->z + v3->z) >> 7) + face->depth_bias;
            if (z < sp->far_z && z > sp->near_z) {
                p = (POLY_FT4 *)sp->prim;
                sp->prim = (u_long *)(p + 1);
                gte_stsxy3_ft4(p);
                gte_ldv0(sp->normals + face->normal);
                *(u_long *)&p->x3 = *(u_long *)&v3->sxy;
                gte_ldrgb(&sp->rgb);
                *(u_long *)&p->u0 = face->uv0_clut;
                gte_lddp((v0->p + v1->p + v2->p + v3->p) >> 2);
                *(u_long *)&p->u1 = face->uv1_tpage | (flags & 0x600000);
                gte_ncds_b();
                *(u_short *)&p->u2 = face->uv2;
                *(u_short *)&p->u3 = face->uv3;
                mesh_add_prim(ot + z, p, 9);
                gte_strgb(&p->r0);
                if (MESH_MODE & (MESH_TINT_A | MESH_TINT_B)) {
                    tint((CVECTOR *)&p->r0);
                }
            }
        }
        face = next;
    }
    sp->faces = (u8 *)face;
    sp->prim_count.count += (sp->prim - start) / 10;
}

ADDRESS(0x80018b1c, 0x40)
void mesh_gt3_tint_a(CVECTOR *rgb)
{
    if (rgb->b > 85) {
        rgb->b = 85;
    }
    rgb->r = rgb->b * 3;
    rgb->g = rgb->b * 2;
}

ADDRESS(0x80018b5c, 0x44)
void mesh_gt3_tint_b(CVECTOR *rgb)
{
    if (rgb->r > 85) {
        rgb->r = 85;
    }
    rgb->b = rgb->r * 3;
    rgb->g = rgb->r * 3;
}

ADDRESS(0x80018ba0, 0x324)
void mesh_draw_gt3(int count, MeshFaceGT3 *face, u_long *ot, int flags)
{
    ScratchPad *sp = SCRATCHPAD;
    u_long *start;
    void (*tint)(CVECTOR *);
    MeshVertex *v0;
    MeshVertex *v1;
    MeshVertex *v2;
    POLY_GT3 *p;
    int t;
    MeshFaceGT3 *next;
    short z;

    tint = 0;
    start = sp->prim;
    sp->face_count += count;
    if (MESH_MODE & MESH_TINT_A) {
        tint = mesh_gt3_tint_a;
    } else if (MESH_MODE & MESH_TINT_B) {
        tint = mesh_gt3_tint_b;
    }
    while ((t = count, count = count - 1, t)) {
        v0 = (MeshVertex *)(sp->vertices + face->i0);
        v1 = (MeshVertex *)(sp->vertices + face->i1);
        v2 = (MeshVertex *)(sp->vertices + face->i2);
        gte_ldsxy3(*(u_long *)&v0->sxy, *(u_long *)&v1->sxy, *(u_long *)&v2->sxy);
        next = face + 1;
        gte_nclip();
        sp->rgb.cd = face->code | ((flags & 2) | 0x34);
        gte_stopz(&sp->opz);
        if (sp->opz > 0) {
            z = ((v0->z + v1->z + v2->z) >> 5) / 3 + face->depth_bias;
            if (z < sp->far_z && z > sp->near_z) {
                p = (POLY_GT3 *)sp->prim;
                sp->prim = (u_long *)(p + 1);
                gte_stsxy3_gt3(p);
                gte_ldv3(sp->normals + face->n0, sp->normals + face->n1, sp->normals + face->n2);
                *(u_long *)&p->u0 = face->uv0_clut;
                gte_ldrgb(&sp->rgb);
                *(u_long *)&p->u1 = face->uv1_tpage | (flags & 0x600000);
                gte_lddp((v0->p * 2 + v1->p + v2->p) >> 2);
                *(u_short *)&p->u2 = face->uv2;
                gte_ncdt_b();
                mesh_add_prim(ot + z, p, 9);
                gte_strgb3(&p->r0, &p->r1, &p->r2);
                if (MESH_MODE & (MESH_TINT_A | MESH_TINT_B)) {
                    tint((CVECTOR *)&p->r0);
                    tint((CVECTOR *)&p->r1);
                    tint((CVECTOR *)&p->r2);
                }
            }
        }
        face = next;
    }
    sp->faces = (u8 *)face;
    sp->prim_count.count += (sp->prim - start) / 10;
}

ADDRESS(0x80018ec4, 0x40)
void mesh_gt4_tint_a(CVECTOR *rgb)
{
    if (rgb->b > 85) {
        rgb->b = 85;
    }
    rgb->r = rgb->b * 3;
    rgb->g = rgb->b * 2;
}

ADDRESS(0x80018f04, 0x44)
void mesh_gt4_tint_b(CVECTOR *rgb)
{
    if (rgb->r > 85) {
        rgb->r = 85;
    }
    rgb->b = rgb->r * 3;
    rgb->g = rgb->r * 3;
}

ADDRESS(0x80018f48, 0x38c)
void mesh_draw_gt4(int count, MeshFaceGT4 *face, u_long *ot, int flags)
{
    ScratchPad *sp = SCRATCHPAD;
    u_long *start;
    void (*tint)(CVECTOR *);
    MeshVertex *v0;
    MeshVertex *v1;
    MeshVertex *v2;
    MeshVertex *v3;
    POLY_GT4 *p;
    int t;
    MeshFaceGT4 *next;
    short z;

    tint = 0;
    start = sp->prim;
    sp->face_count += count;
    if (MESH_MODE & MESH_TINT_A) {
        tint = mesh_gt4_tint_a;
    } else if (MESH_MODE & MESH_TINT_B) {
        tint = mesh_gt4_tint_b;
    }
    while ((t = count, count = count - 1, t)) {
        v0 = (MeshVertex *)(sp->vertices + face->i0);
        v1 = (MeshVertex *)(sp->vertices + face->i1);
        v2 = (MeshVertex *)(sp->vertices + face->i2);
        gte_ldsxy3(*(u_long *)&v0->sxy, *(u_long *)&v1->sxy, *(u_long *)&v2->sxy);
        next = face + 1;
        gte_nclip();
        sp->rgb.cd = face->code | ((flags & 2) | 0x3c);
        gte_stopz(&sp->opz);
        if (sp->opz > 0) {
            v3 = (MeshVertex *)(sp->vertices + face->i3);
            z = ((v0->z + v1->z + v2->z + v3->z) >> 7) + face->depth_bias;
            if (z < sp->far_z && z > sp->near_z) {
                p = (POLY_GT4 *)sp->prim;
                sp->prim = (u_long *)(p + 1);
                gte_stsxy3_gt4(p);
                gte_ldv3(sp->normals + face->n0, sp->normals + face->n1, sp->normals + face->n2);
                *(u_long *)&p->x3 = *(u_long *)&v3->sxy;
                gte_ldrgb(&sp->rgb);
                *(u_long *)&p->u0 = face->uv0_clut;
                gte_lddp((v0->p + v1->p + v2->p + v3->p) >> 2);
                *(u_long *)&p->u1 = face->uv1_tpage | (flags & 0x600000);
                gte_ncdt_b();
                *(u_short *)&p->u2 = face->uv2;
                gte_strgb3(&p->r0, &p->r1, &p->r2);
                gte_ldv0(sp->normals + face->n3);
                gte_ldrgb(&sp->rgb);
                gte_lddp(v0->p);
                *(u_short *)&p->u3 = face->uv3;
                gte_ncds_b();
                mesh_add_prim(ot + z, p, 12);
                gte_strgb(&p->r3);
                if (MESH_MODE & (MESH_TINT_A | MESH_TINT_B)) {
                    tint((CVECTOR *)&p->r0);
                    tint((CVECTOR *)&p->r1);
                    tint((CVECTOR *)&p->r2);
                    tint((CVECTOR *)&p->r3);
                }
            }
        }
        face = next;
    }
    sp->faces = (u8 *)face;
    /* 12 words per primitive, although a POLY_GT4 is 13. */
    sp->prim_count.count += (sp->prim - start) / 12;
}

ADDRESS(0x800192d4, 0x1f0)
void mesh_draw_part(u_short part, u_int flags)
{
    ScratchPad *sp = SCRATCHPAD;
    int mode;

    mode = (flags >> 1) & 2;
    mode |= (flags & 3) << 21;
    sp->header = (MeshHeader *)(sp->model + 12) + part;
    sp->normals = sp->model + 12 + sp->header->normals;
    if (MESH_MODE & MESH_TINT_A) {
        if (MESH_MODE & MESH_TINT_SLOW) {
            sp->rgb.r = sp->rgb.g = sp->rgb.b = -120 - (MESH_TINT_LEVEL >> 13);
        } else {
            sp->rgb.r = sp->rgb.g = sp->rgb.b = -96 - (MESH_TINT_LEVEL >> 11);
        }
    } else {
        sp->rgb.r = sp->rgb.g = sp->rgb.b = 128;
    }
    if (MESH_MODE & MESH_TINT_A) {
        gte_SetFarColor(6, 4, 2);
    } else if (MESH_MODE & MESH_TINT_B) {
        gte_SetFarColor(2, 6, 6);
    } else {
        gte_SetFarColor(5, 5, 5);
    }
    sp->faces = sp->model + 12 + sp->header->faces;
    mesh_draw_ft3(sp->header->n_ft3, (MeshFaceFT3 *)sp->faces, sp->ot, mode);
    mesh_draw_ft4(sp->header->n_ft4, (MeshFaceFT4 *)sp->faces, sp->ot, mode);
    mesh_draw_gt3(sp->header->n_gt3, (MeshFaceGT3 *)sp->faces, sp->ot, mode);
    mesh_draw_gt4(sp->header->n_gt4, (MeshFaceGT4 *)sp->faces, sp->ot, mode);
}

ADDRESS(0x800194c4, 0x1fc)
void mesh_draw_ft3_nccs(int count, MeshFaceFT3 *face, u_long *ot, int flags)
{
    ScratchPad *sp = SCRATCHPAD;
    u_long *start;
    MeshVertex *v0;
    MeshVertex *v1;
    MeshVertex *v2;
    POLY_FT3 *p;
    MeshFaceFT3 *next;
    int t;
    short z;

    start = sp->prim;
    sp->face_count += count;
    while ((t = count, count = count - 1, t)) {
        v0 = (MeshVertex *)(sp->vertices + face->i0);
        v1 = (MeshVertex *)(sp->vertices + face->i1);
        v2 = (MeshVertex *)(sp->vertices + face->i2);
        gte_ldsxy3(*(u_long *)&v0->sxy, *(u_long *)&v1->sxy, *(u_long *)&v2->sxy);
        next = face + 1;
        gte_nclip();
        sp->rgb.cd = face->code | ((flags & 2) | 0x24);
        gte_stopz(&sp->opz);
        if (sp->opz > 0) {
            z = ((v0->z + v1->z + v2->z) >> 5) / 3 + face->depth_bias;
            if (z < sp->far_z && z > sp->near_z) {
                p = (POLY_FT3 *)sp->prim;
                sp->prim = (u_long *)(p + 1);
                gte_stsxy3_ft3(p);
                gte_ldv0(sp->normals + face->normal);
                gte_ldrgb(&sp->rgb);
                *(u_long *)&p->u0 = face->uv0_clut;
                gte_nccs_b();
                *(u_long *)&p->u1 = face->uv1_tpage | (flags & 0x600000);
                *(u_short *)&p->u2 = face->uv2;
                mesh_add_prim(ot + z, p, 7);
                gte_strgb(&p->r0);
            }
        }
        face = next;
    }
    sp->faces = (u8 *)face;
    sp->prim_count.count += (sp->prim - start) / 8;
}

ADDRESS(0x800196c0, 0x234)
void mesh_draw_ft4_nccs(int count, MeshFaceFT4 *face, u_long *ot, int flags)
{
    ScratchPad *sp = SCRATCHPAD;
    u_long *start;
    MeshVertex *v0;
    MeshVertex *v1;
    MeshVertex *v2;
    MeshVertex *v3;
    POLY_FT4 *p;
    MeshFaceFT4 *next;
    int t;
    short z;

    start = sp->prim;
    sp->face_count += count;
    while ((t = count, count = count - 1, t)) {
        v0 = (MeshVertex *)(sp->vertices + face->i0);
        v1 = (MeshVertex *)(sp->vertices + face->i1);
        v2 = (MeshVertex *)(sp->vertices + face->i2);
        gte_ldsxy3(*(u_long *)&v0->sxy, *(u_long *)&v1->sxy, *(u_long *)&v2->sxy);
        next = face + 1;
        gte_nclip();
        sp->rgb.cd = face->code | ((flags & 2) | 0x2c);
        gte_stopz(&sp->opz);
        if (sp->opz > 0) {
            v3 = (MeshVertex *)(sp->vertices + face->i3);
            z = ((v0->z + v1->z + v2->z + v3->z) >> 7) + face->depth_bias;
            if (z < sp->far_z && z > sp->near_z) {
                p = (POLY_FT4 *)sp->prim;
                sp->prim = (u_long *)(p + 1);
                gte_stsxy3_ft4(p);
                gte_ldv0(sp->normals + face->normal);
                gte_ldrgb(&sp->rgb);
                *(u_long *)&p->x3 = *(u_long *)&v3->sxy;
                gte_nccs_b();
                *(u_long *)&p->u0 = face->uv0_clut;
                *(u_long *)&p->u1 = face->uv1_tpage | (flags & 0x600000);
                *(u_short *)&p->u2 = face->uv2;
                *(u_short *)&p->u3 = face->uv3;
                mesh_add_prim(ot + z, p, 9);
                gte_strgb(&p->r0);
            }
        }
        face = next;
    }
    sp->faces = (u8 *)face;
    sp->prim_count.count += (sp->prim - start) / 10;
}

ADDRESS(0x800198f4, 0x240)
void mesh_draw_gt3_nccs(int count, MeshFaceGT3 *face, u_long *ot, int flags)
{
    ScratchPad *sp = SCRATCHPAD;
    u_long *start;
    MeshVertex *v0;
    MeshVertex *v1;
    MeshVertex *v2;
    POLY_GT3 *p;
    MeshFaceGT3 *next;
    int t;
    short z;

    start = sp->prim;
    sp->face_count += count;
    while ((t = count, count = count - 1, t)) {
        v0 = (MeshVertex *)(sp->vertices + face->i0);
        v1 = (MeshVertex *)(sp->vertices + face->i1);
        v2 = (MeshVertex *)(sp->vertices + face->i2);
        gte_ldsxy3(*(u_long *)&v0->sxy, *(u_long *)&v1->sxy, *(u_long *)&v2->sxy);
        next = face + 1;
        gte_nclip();
        sp->rgb.cd = face->code | ((flags & 2) | 0x34);
        gte_stopz(&sp->opz);
        if (sp->opz > 0) {
            z = ((v0->z + v1->z + v2->z) >> 5) / 3 + face->depth_bias;
            if (z < sp->far_z && z > sp->near_z) {
                gte_stsxy3_gt3(sp->prim);
                gte_ldv3(sp->normals + face->n0, sp->normals + face->n1, sp->normals + face->n2);
                p = (POLY_GT3 *)sp->prim;
                sp->prim = (u_long *)(p + 1);
                *(u_long *)&p->u0 = face->uv0_clut;
                gte_ldrgb(&sp->rgb);
                *(u_long *)&p->u1 = face->uv1_tpage | (flags & 0x600000);
                gte_ncct_b();
                *(u_short *)&p->u2 = face->uv2;
                mesh_add_prim(ot + z, p, 9);
                gte_strgb3(&p->r0, &p->r1, &p->r2);
            }
        }
        face = next;
    }
    sp->faces = (u8 *)face;
    sp->prim_count.count += (sp->prim - start) / 10;
}

ADDRESS(0x80019b34, 0x294)
void mesh_draw_gt4_nccs(int count, MeshFaceGT4 *face, u_long *ot, int flags)
{
    ScratchPad *sp = SCRATCHPAD;
    u_long *start;
    MeshVertex *v0;
    MeshVertex *v1;
    MeshVertex *v2;
    MeshVertex *v3;
    POLY_GT4 *p;
    MeshFaceGT4 *next;
    int t;
    short z;

    start = sp->prim;
    sp->face_count += count;
    while ((t = count, count = count - 1, t)) {
        v0 = (MeshVertex *)(sp->vertices + face->i0);
        v1 = (MeshVertex *)(sp->vertices + face->i1);
        v2 = (MeshVertex *)(sp->vertices + face->i2);
        gte_ldsxy3(*(u_long *)&v0->sxy, *(u_long *)&v1->sxy, *(u_long *)&v2->sxy);
        next = face + 1;
        gte_nclip();
        sp->rgb.cd = face->code | ((flags & 2) | 0x3c);
        gte_stopz(&sp->opz);
        if (sp->opz > 0) {
            v3 = (MeshVertex *)(sp->vertices + face->i3);
            z = ((v0->z + v1->z + v2->z + v3->z) >> 7) + face->depth_bias;
            if (z < sp->far_z && z > sp->near_z) {
                p = (POLY_GT4 *)sp->prim;
                sp->prim = (u_long *)(p + 1);
                gte_stsxy3_gt4(p);
                gte_ldv3(sp->normals + face->n0, sp->normals + face->n1, sp->normals + face->n2);
                *(u_long *)&p->x3 = *(u_long *)&v3->sxy;
                gte_ldrgb(&sp->rgb);
                *(u_long *)&p->u0 = face->uv0_clut;
                gte_ncct_b();
                *(u_long *)&p->u1 = face->uv1_tpage | (flags & 0x600000);
                *(u_short *)&p->u2 = face->uv2;
                gte_strgb3(&p->r0, &p->r1, &p->r2);
                gte_ldv0(sp->normals + face->n3);
                *(u_short *)&p->u3 = face->uv3;
                gte_ldrgb(&sp->rgb);
                mesh_set_tag(ot + z, p, 12);
                gte_nccs_b();
                mesh_set_ot(ot + z, p);
                gte_strgb(&p->r3);
            }
        }
        face = next;
    }
    sp->faces = (u8 *)face;
    /* 12 words per primitive, although a POLY_GT4 is 13. */
    sp->prim_count.count += (sp->prim - start) / 12;
}

ADDRESS(0x80019dc8, 0x1ec)
void mesh_draw_part_nccs(u_short part, u_int flags)
{
    ScratchPad *sp = SCRATCHPAD;
    int mode;

    mode = (flags >> 1) & 2;
    mode |= (flags & 3) << 21;
    sp->header = (MeshHeader *)(sp->model + 12) + part;
    sp->normals = sp->model + 12 + sp->header->normals;
    sp->faces = sp->model + 12 + sp->header->faces;
    if (MESH_MODE & MESH_TINT_A) {
        if (MESH_MODE & MESH_TINT_SLOW) {
            sp->rgb.r = sp->rgb.g = sp->rgb.b = -120 - (MESH_TINT_LEVEL >> 13);
        } else {
            sp->rgb.r = sp->rgb.g = sp->rgb.b = -96 - (MESH_TINT_LEVEL >> 11);
        }
    } else {
        sp->rgb.r = sp->rgb.g = sp->rgb.b = 128;
    }
    if (MESH_MODE & MESH_TINT_A) {
        gte_SetFarColor(6, 4, 2);
    } else if (MESH_MODE & MESH_TINT_B) {
        gte_SetFarColor(2, 6, 6);
    } else {
        gte_SetFarColor(5, 5, 5);
    }
    mesh_draw_ft3_nccs(sp->header->n_ft3, (MeshFaceFT3 *)sp->faces, sp->ot, mode);
    mesh_draw_ft4_nccs(sp->header->n_ft4, (MeshFaceFT4 *)sp->faces, sp->ot, mode);
    mesh_draw_gt3_nccs(sp->header->n_gt3, (MeshFaceGT3 *)sp->faces, sp->ot, mode);
    mesh_draw_gt4_nccs(sp->header->n_gt4, (MeshFaceGT4 *)sp->faces, sp->ot, mode);
}

ADDRESS(0x80019fb4, 0x40)
void mesh_div_ft3_tint_a(CVECTOR *rgb)
{
    if (rgb->b > 85) {
        rgb->b = 85;
    }
    rgb->r = rgb->b * 3;
    rgb->g = rgb->b * 2;
}

ADDRESS(0x80019ff4, 0x44)
void mesh_div_ft3_tint_b(CVECTOR *rgb)
{
    if (rgb->r > 85) {
        rgb->r = 85;
    }
    rgb->b = rgb->r * 3;
    rgb->g = rgb->r * 3;
}

ADDRESS(0x8001a038, 0x630)
void mesh_div_draw_ft3(int count, MeshFaceFT3 *face, u_long *ot, int flags)
{
    ScratchPad *sp = SCRATCHPAD;
    MeshDiv div;
    u_long *start;
    MeshVertex *v0;
    MeshVertex *v1;
    MeshVertex *v2;
    POLY_FT3 *p;
    int t;
    MeshFaceFT3 *next;
    void (*tint)(CVECTOR *);
    short z;

    tint = 0;
    start = sp->prim;
    sp->face_count += count;
    if (MESH_MODE & MESH_TINT_A) {
        tint = mesh_div_ft3_tint_a;
    } else if (MESH_MODE & MESH_TINT_B) {
        tint = mesh_div_ft3_tint_b;
    }
    while ((t = count, count = count - 1, t)) {
        v0 = (MeshVertex *)(sp->vertices + face->i0);
        v1 = (MeshVertex *)(sp->vertices + face->i1);
        v2 = (MeshVertex *)(sp->vertices + face->i2);
        gte_ldsxy3(*(u_long *)&v0->sxy, *(u_long *)&v1->sxy, *(u_long *)&v2->sxy);
        next = face + 1;
        gte_nclip();
        sp->rgb.cd = face->code | ((flags & 2) | 0x24);
        gte_stopz(&sp->opz);
        if (sp->opz > 0) {
            z = ((v0->z + v1->z + v2->z) >> 5) / 3 + face->depth_bias;
            if (z < sp->far_z && z > sp->near_z) {
                if (sp->opz > 0x4000) {
                    div.d3.ndiv = 1;
                    if (sp->opz > 0x8000) {
                        div.d3.ndiv = 2;
                        if (sp->opz > 0x10000) {
                            div.d3.ndiv = 3;
                        }
                    }
                    gte_ldv0(sp->normals + face->normal);
                    div.d3.pih = 320;
                    div.d3.piv = 240;
                    gte_ldrgb(&sp->rgb);
                    if (!(MESH_MODE & MESH_NO_DEPTH_CUE)) {
                        gte_lddp((v0->p + v1->p + v2->p) / 3);
                        gte_ncds();
                        div.d3.clut = MESH_WORD_HI(face->uv0_clut) + sp->clut_base;
                        div.d3.tpage = MESH_WORD_HI(face->uv1_tpage) | ((flags >> 16) & 0x60);
                        gte_strgb(&div.d3.rgbc);
                        if (MESH_MODE & (MESH_TINT_A | MESH_TINT_B)) {
                            tint(&div.d3.rgbc);
                        }
                    } else {
                        gte_nccs();
                        div.d3.clut = MESH_WORD_HI(face->uv0_clut) + sp->clut_base;
                        div.d3.tpage = MESH_WORD_HI(face->uv1_tpage) | ((flags >> 16) & 0x60);
                        gte_strgb(&div.d3.rgbc);
                    }
                    div.d3.ot = ot + z;
                    div.d3.cr[0].r0 = &div.d3.r0;
                    div.d3.cr[0].r1 = &div.d3.r1;
                    div.d3.cr[0].r2 = &div.d3.r2;
                    div.d3.r0.sxy = v0->sxy;
                    div.d3.r1.sxy = v1->sxy;
                    div.d3.r2.sxy = v2->sxy;
                    div.d3.r0.sz = v0->z;
                    div.d3.r1.sz = v1->z;
                    div.d3.r2.sz = v2->z;
                    div.d3.r0.v = *(SVECTOR *)(sp->local_vertices + face->i0);
                    div.d3.r1.v = *(SVECTOR *)(sp->local_vertices + face->i1);
                    div.d3.r2.v = *(SVECTOR *)(sp->local_vertices + face->i2);
                    ((u_long *)&div.d3.r0)[2] = face->uv0_clut;
                    ((u_long *)&div.d3.r1)[2] = face->uv1_tpage | (flags & 0x600000);
                    ((u_long *)&div.d3.r2)[2] = *(u_long *)&face->uv2;
                    sp->prim = RCpolyFT3((POLY_FT3 *)sp->prim, &div.d3);
                    if (sp->prim_end < sp->prim) {
                        return;
                    }
                } else {
                    p = (POLY_FT3 *)sp->prim;
                    sp->prim = (u_long *)(p + 1);
                    gte_stsxy3_ft3(p);
                    gte_ldv0(sp->normals + face->normal);
                    *(u_long *)&p->u0 = face->uv0_clut;
                    gte_ldrgb(&sp->rgb);
                    if (!(MESH_MODE & MESH_NO_DEPTH_CUE)) {
                        gte_lddp((v0->p + v1->p + v2->p) / 3);
                        gte_ncds();
                        *(u_long *)&p->u1 = face->uv1_tpage | (flags & 0x600000);
                        *(u_short *)&p->u2 = face->uv2;
                        gte_strgb(&p->r0);
                        if (MESH_MODE & (MESH_TINT_A | MESH_TINT_B)) {
                            tint((CVECTOR *)&p->r0);
                        }
                    } else {
                        gte_nccs();
                        *(u_long *)&p->u1 = face->uv1_tpage | (flags & 0x600000);
                        *(u_short *)&p->u2 = face->uv2;
                        gte_strgb(&p->r0);
                    }
                    mesh_add_prim(ot + z, p, 7);
                }
            }
        }
        face = next;
    }
    sp->faces = (u8 *)face;
    sp->prim_count.count += (sp->prim - start) / 8;
}

ADDRESS(0x8001a668, 0x40)
void mesh_div_ft4_tint_a(CVECTOR *rgb)
{
    if (rgb->b > 85) {
        rgb->b = 85;
    }
    rgb->r = rgb->b * 3;
    rgb->g = rgb->b * 2;
}

ADDRESS(0x8001a6a8, 0x44)
void mesh_div_ft4_tint_b(CVECTOR *rgb)
{
    if (rgb->r > 85) {
        rgb->r = 85;
    }
    rgb->b = rgb->r * 3;
    rgb->g = rgb->r * 3;
}

ADDRESS(0x8001a6ec, 0x70c)
void mesh_div_draw_ft4(int count, MeshFaceFT4 *face, u_long *ot, int flags)
{
    ScratchPad *sp = SCRATCHPAD;
    MeshDiv div;
    u_long *start;
    MeshVertex *v0;
    MeshVertex *v1;
    MeshVertex *v2;
    MeshVertex *v3;
    POLY_FT4 *p;
    int t;
    MeshFaceFT4 *next;
    void (*tint)(CVECTOR *);
    short z;

    tint = 0;
    start = sp->prim;
    sp->face_count += count;
    if (MESH_MODE & MESH_TINT_A) {
        tint = mesh_div_ft4_tint_a;
    } else if (MESH_MODE & MESH_TINT_B) {
        tint = mesh_div_ft4_tint_b;
    }
    while ((t = count, count = count - 1, t)) {
        v0 = (MeshVertex *)(sp->vertices + face->i0);
        v1 = (MeshVertex *)(sp->vertices + face->i1);
        v2 = (MeshVertex *)(sp->vertices + face->i2);
        gte_ldsxy3(*(u_long *)&v0->sxy, *(u_long *)&v1->sxy, *(u_long *)&v2->sxy);
        gte_nclip();
        p = (POLY_FT4 *)sp->prim;
        gte_stopz(&sp->opz);
        gte_stsxy3_ft4(p);
        v3 = (MeshVertex *)(sp->vertices + face->i3);
        gte_ldsxy0(*(u_long *)&v3->sxy);
        gte_nclip();
        next = face + 1;
        gte_stopz(&sp->opz2);
        if (sp->opz > 0 || sp->opz2 < 0) {
            gte_stsxy0(&p->x3);
            sp->rgb.cd = face->code | ((flags & 2) | 0x2c);
            z = ((v0->z + v1->z + v2->z + v3->z) >> 7) + face->depth_bias;
            if (z < sp->far_z && z > sp->near_z) {
                if (sp->opz > 0x4000 || sp->opz2 < -0x4000) {
                    div.d4.ndiv = 1;
                    if (sp->opz > 0x8000 || sp->opz2 < -0x8000) {
                        div.d4.ndiv = 2;
                        if (sp->opz > 0x10000 || sp->opz2 < -0x10000) {
                            div.d4.ndiv = 3;
                        }
                    }
                    gte_ldv0(sp->normals + face->normal);
                    div.d4.pih = 320;
                    div.d4.piv = 240;
                    gte_ldrgb(&sp->rgb);
                    if (!(MESH_MODE & MESH_NO_DEPTH_CUE)) {
                        gte_lddp((v0->p + v1->p + v2->p + v3->p) >> 2);
                        gte_ncds();
                        div.d4.clut = MESH_WORD_HI(face->uv0_clut) + sp->clut_base;
                        div.d4.tpage = MESH_WORD_HI(face->uv1_tpage) | ((flags >> 16) & 0x60);
                        gte_strgb(&div.d4.rgbc);
                        if (MESH_MODE & (MESH_TINT_A | MESH_TINT_B)) {
                            tint(&div.d4.rgbc);
                        }
                    } else {
                        gte_nccs();
                        div.d4.clut = MESH_WORD_HI(face->uv0_clut) + sp->clut_base;
                        div.d4.tpage = MESH_WORD_HI(face->uv1_tpage) | ((flags >> 16) & 0x60);
                        gte_strgb(&div.d4.rgbc);
                    }
                    div.d4.ot = ot + z;
                    div.d4.cr[0].r0 = &div.d4.r0;
                    div.d4.cr[0].r1 = &div.d4.r1;
                    div.d4.cr[0].r2 = &div.d4.r2;
                    div.d4.cr[0].r3 = &div.d4.r3;
                    div.d4.r0.sxy = v0->sxy;
                    div.d4.r1.sxy = v1->sxy;
                    div.d4.r2.sxy = v2->sxy;
                    div.d4.r3.sxy = v3->sxy;
                    div.d4.r0.sz = v0->z;
                    div.d4.r1.sz = v1->z;
                    div.d4.r2.sz = v2->z;
                    div.d4.r3.sz = v3->z;
                    div.d4.r0.v = *(SVECTOR *)(sp->local_vertices + face->i0);
                    div.d4.r1.v = *(SVECTOR *)(sp->local_vertices + face->i1);
                    div.d4.r2.v = *(SVECTOR *)(sp->local_vertices + face->i2);
                    div.d4.r3.v = *(SVECTOR *)(sp->local_vertices + face->i3);
                    *(u_long *)div.d4.r0.uv = face->uv0_clut;
                    *(u_long *)div.d4.r1.uv = face->uv1_tpage | (flags & 0x600000);
                    *(u_long *)div.d4.r2.uv = *(u_long *)&face->uv2;
                    *(u_long *)div.d4.r3.uv = *(u_long *)&face->uv3;
                    sp->prim = RCpolyFT4((POLY_FT4 *)sp->prim, &div.d4);
                    if (sp->prim_end < sp->prim) {
                        return;
                    }
                } else {
                    sp->prim = (u_long *)((POLY_FT4 *)sp->prim + 1);
                    gte_ldv0(sp->normals + face->normal);
                    gte_ldrgb(&sp->rgb);
                    *(u_long *)&p->u0 = face->uv0_clut;
                    *(u_long *)&p->u1 = face->uv1_tpage | (flags & 0x600000);
                    if (!(MESH_MODE & MESH_NO_DEPTH_CUE)) {
                        gte_lddp((v0->p + v1->p + v2->p + v3->p) >> 2);
                        gte_ncds();
                        *(u_short *)&p->u2 = face->uv2;
                        *(u_short *)&p->u3 = face->uv3;
                        gte_strgb(&p->r0);
                        if (MESH_MODE & (MESH_TINT_A | MESH_TINT_B)) {
                            tint((CVECTOR *)&p->r0);
                        }
                    } else {
                        gte_nccs();
                        *(u_short *)&p->u2 = face->uv2;
                        *(u_short *)&p->u3 = face->uv3;
                        gte_strgb(&p->r0);
                    }
                    mesh_add_prim(ot + z, p, 9);
                }
            }
        }
        face = next;
    }
    sp->faces = (u8 *)face;
    sp->prim_count.count += (sp->prim - start) / 10;
}

ADDRESS(0x8001adf8, 0x40)
void mesh_div_gt3_tint_a(CVECTOR *rgb)
{
    if (rgb->b > 85) {
        rgb->b = 85;
    }
    rgb->r = rgb->b * 3;
    rgb->g = rgb->b * 2;
}

ADDRESS(0x8001ae38, 0x44)
void mesh_div_gt3_tint_b(CVECTOR *rgb)
{
    if (rgb->r > 85) {
        rgb->r = 85;
    }
    rgb->b = rgb->r * 3;
    rgb->g = rgb->r * 3;
}

ADDRESS(0x8001ae7c, 0x6dc)
void mesh_div_draw_gt3(int count, MeshFaceGT3 *face, u_long *ot, int flags)
{
    POLY_GT3 *p;
    int t;
    u_long *start;
    short z;
    MeshVertex *v0;
    MeshVertex *v2;
    MeshVertex *v1;
    void (*tint)(CVECTOR *);
    MeshDiv div;
    MeshFaceGT3 *next;
    ScratchPad *sp = SCRATCHPAD;

    tint = 0;
    start = sp->prim;
    sp->face_count += count;
    if (MESH_MODE & MESH_TINT_A) {
        tint = mesh_div_gt3_tint_a;
    } else if (MESH_MODE & MESH_TINT_B) {
        tint = mesh_div_gt3_tint_b;
    }
    while ((t = count, count = count - 1, t)) {
        v0 = (MeshVertex *)(sp->vertices + face->i0);
        v1 = (MeshVertex *)(sp->vertices + face->i1);
        v2 = (MeshVertex *)(sp->vertices + face->i2);
        gte_ldsxy3(*(u_long *)&v0->sxy, *(u_long *)&v1->sxy, *(u_long *)&v2->sxy);
        next = face + 1;
        gte_nclip();
        sp->rgb.cd = face->code | ((flags & 2) | 0x34);
        gte_stopz(&sp->opz);
        if (sp->opz > 0) {
            z = ((v0->z + v1->z + v2->z) >> 5) / 3 + face->depth_bias;
            if (z < sp->far_z && z > sp->near_z) {
                if (sp->opz > 0x4000) {
                    div.d3.ndiv = 1;
                    if (sp->opz > 0x8000) {
                        div.d3.ndiv = 2;
                        if (sp->opz > 0x10000) {
                            div.d3.ndiv = 3;
                        }
                    }
                    gte_ldv3(sp->normals + face->n0, sp->normals + face->n1, sp->normals + face->n2);
                    div.d3.pih = 320;
                    div.d3.piv = 240;
                    gte_ldrgb(&sp->rgb);
                    if (!(MESH_MODE & MESH_NO_DEPTH_CUE)) {
                        gte_lddp((v0->p + v1->p + v2->p) / 3);
                        gte_ncdt();
                        div.d3.clut = MESH_WORD_HI(face->uv0_clut) + sp->clut_base;
                        div.d3.tpage = MESH_WORD_HI(face->uv1_tpage) | ((flags >> 16) & 0x60);
                        gte_strgb3(&div.d3.r0.c, &div.d3.r1.c, &div.d3.r2.c);
                        if (MESH_MODE & (MESH_TINT_A | MESH_TINT_B)) {
                            tint(&div.d3.r0.c);
                            tint(&div.d3.r1.c);
                            tint(&div.d3.r2.c);
                        }
                    } else {
                        gte_ncct();
                        div.d3.clut = MESH_WORD_HI(face->uv0_clut) + sp->clut_base;
                        div.d3.tpage = MESH_WORD_HI(face->uv1_tpage) | ((flags >> 16) & 0x60);
                        gte_strgb3(&div.d3.r0.c, &div.d3.r1.c, &div.d3.r2.c);
                    }
                    div.d3.rgbc = sp->rgb;
                    div.d3.ot = ot + z;
                    div.d3.cr[0].r0 = &div.d3.r0;
                    div.d3.cr[0].r1 = &div.d3.r1;
                    div.d3.cr[0].r2 = &div.d3.r2;
                    div.d3.r0.sxy = v0->sxy;
                    div.d3.r1.sxy = v1->sxy;
                    div.d3.r2.sxy = v2->sxy;
                    div.d3.r0.sz = v0->z;
                    div.d3.r1.sz = v1->z;
                    div.d3.r2.sz = v2->z;
                    div.d3.r0.v = *(SVECTOR *)(sp->local_vertices + face->i0);
                    div.d3.r1.v = *(SVECTOR *)(sp->local_vertices + face->i1);
                    div.d3.r2.v = *(SVECTOR *)(sp->local_vertices + face->i2);
                    ((u_long *)&div.d3.r0)[2] = face->uv0_clut;
                    ((u_long *)&div.d3.r1)[2] = face->uv1_tpage | (flags & 0x600000);
                    ((u_long *)&div.d3.r2)[2] = *(u_long *)&face->uv2;
                    sp->prim = RCpolyGT3((POLY_GT3 *)sp->prim, &div.d3);
                    if (sp->prim_end < sp->prim) {
                        return;
                    }
                } else {
                    p = (POLY_GT3 *)sp->prim;
                    sp->prim = (u_long *)(p + 1);
                    gte_stsxy3_gt3(p);
                    gte_ldv3(sp->normals + face->n0, sp->normals + face->n1, sp->normals + face->n2);
                    *(u_long *)&p->u0 = face->uv0_clut;
                    gte_ldrgb(&sp->rgb);
                    if (!(MESH_MODE & MESH_NO_DEPTH_CUE)) {
                        gte_lddp((v0->p + v1->p + v2->p) / 3);
                        gte_ncdt();
                        *(u_long *)&p->u1 = face->uv1_tpage | (flags & 0x600000);
                        *(u_short *)&p->u2 = face->uv2;
                        gte_strgb3(&p->r0, &p->r1, &p->r2);
                        if (MESH_MODE & (MESH_TINT_A | MESH_TINT_B)) {
                            tint((CVECTOR *)&p->r0);
                            tint((CVECTOR *)&p->r1);
                            tint((CVECTOR *)&p->r2);
                        }
                    } else {
                        gte_ncct();
                        *(u_long *)&p->u1 = face->uv1_tpage | (flags & 0x600000);
                        *(u_short *)&p->u2 = face->uv2;
                        gte_strgb3(&p->r0, &p->r1, &p->r2);
                    }
                    mesh_add_prim(ot + z, p, 9);
                }
            }
        }
        face = next;
    }
    sp->faces = (u8 *)face;
    sp->prim_count.count += (sp->prim - start) / 10;
}

ADDRESS(0x8001b558, 0x40)
void mesh_div_gt4_tint_a(CVECTOR *rgb)
{
    if (rgb->b > 85) {
        rgb->b = 85;
    }
    rgb->r = rgb->b * 3;
    rgb->g = rgb->b * 2;
}

ADDRESS(0x8001b598, 0x44)
void mesh_div_gt4_tint_b(CVECTOR *rgb)
{
    if (rgb->r > 85) {
        rgb->r = 85;
    }
    rgb->b = rgb->r * 3;
    rgb->g = rgb->r * 3;
}

/* The tint callbacks take only the colour; this renderer also passes each
 * vertex's depth, through an unprototyped pointer. */
ADDRESS(0x8001b5dc, 0x934)
void mesh_div_draw_gt4(int count, MeshFaceGT4 *face, u_long *ot, int flags)
{
    ScratchPad *sp = SCRATCHPAD;
    MeshDiv div;
    u_long *start;
    void (*tint)();
    MeshVertex *v0;
    MeshVertex *v1;
    MeshVertex *v2;
    MeshVertex *v3;
    POLY_GT4 *p;
    int t;
    MeshFaceGT4 *next;
    short z;

    tint = 0;
    start = sp->prim;
    sp->face_count += count;
    if (MESH_MODE & MESH_TINT_A) {
        tint = mesh_div_gt4_tint_a;
    } else if (MESH_MODE & MESH_TINT_B) {
        tint = mesh_div_gt4_tint_b;
    }
    while ((t = count, count = count - 1, t)) {
        v0 = (MeshVertex *)(sp->vertices + face->i0);
        v1 = (MeshVertex *)(sp->vertices + face->i1);
        v2 = (MeshVertex *)(sp->vertices + face->i2);
        gte_ldsxy3(*(u_long *)&v0->sxy, *(u_long *)&v1->sxy, *(u_long *)&v2->sxy);
        gte_nclip();
        p = (POLY_GT4 *)sp->prim;
        gte_stopz(&sp->opz);
        gte_stsxy3_gt4(p);
        v3 = (MeshVertex *)(sp->vertices + face->i3);
        gte_ldsxy0(*(u_long *)&v3->sxy);
        gte_nclip();
        next = face + 1;
        gte_stopz(&sp->opz2);
        if (sp->opz > 0 || sp->opz2 < 0) {
            gte_stsxy0(&p->x3);
            sp->rgb.cd = face->code | ((flags & 2) | 0x3c);
            z = ((v0->z + v1->z + v2->z + v3->z) >> 7) + face->depth_bias;
            if (z < sp->far_z && z > sp->near_z) {
                if (sp->opz > 0x4000 || sp->opz2 < -0x4000) {
                    div.d4.ndiv = 1;
                    if (sp->opz > 0x8000 || sp->opz2 < -0x8000) {
                        div.d4.ndiv = 2;
                        if (sp->opz > 0x10000 || sp->opz2 < -0x10000) {
                            div.d4.ndiv = 3;
                        }
                    }
                    gte_ldv3(sp->normals + face->n0, sp->normals + face->n1, sp->normals + face->n2);
                    div.d4.pih = 320;
                    div.d4.piv = 240;
                    gte_ldrgb(&sp->rgb);
                    if (!(MESH_MODE & MESH_NO_DEPTH_CUE)) {
                        gte_lddp((v0->p + v1->p + v2->p + v3->p) >> 2);
                        gte_ncdt();
                        div.d4.clut = MESH_WORD_HI(face->uv0_clut) + sp->clut_base;
                        div.d4.tpage = MESH_WORD_HI(face->uv1_tpage) | ((flags >> 16) & 0x60);
                        gte_strgb3(&div.d4.r0.c, &div.d4.r1.c, &div.d4.r2.c);
                        gte_ldv0(sp->normals + face->n3);
                        gte_ncds();
                        div.d4.rgbc = sp->rgb;
                        div.d4.ot = ot + z;
                        gte_strgb(&div.d4.r3.c);
                        if (MESH_MODE & (MESH_TINT_A | MESH_TINT_B)) {
                            tint(&div.d4.r0.c, v0->z);
                            tint(&div.d4.r1.c, v1->z);
                            tint(&div.d4.r2.c, v2->z);
                            tint(&div.d4.r3.c, v3->z);
                        }
                    } else {
                        gte_ncct();
                        div.d4.clut = MESH_WORD_HI(face->uv0_clut) + sp->clut_base;
                        div.d4.tpage = MESH_WORD_HI(face->uv1_tpage) | ((flags >> 16) & 0x60);
                        gte_strgb3(&div.d4.r0.c, &div.d4.r1.c, &div.d4.r2.c);
                        gte_ldv0(sp->normals + face->n3);
                        gte_nccs();
                        div.d4.rgbc = sp->rgb;
                        div.d4.ot = ot + z;
                        gte_strgb(&div.d4.r3.c);
                    }
                    div.d4.cr[0].r0 = &div.d4.r0;
                    div.d4.cr[0].r1 = &div.d4.r1;
                    div.d4.cr[0].r2 = &div.d4.r2;
                    div.d4.cr[0].r3 = &div.d4.r3;
                    div.d4.r0.sxy = v0->sxy;
                    div.d4.r1.sxy = v1->sxy;
                    div.d4.r2.sxy = v2->sxy;
                    div.d4.r3.sxy = v3->sxy;
                    div.d4.r0.sz = v0->z;
                    div.d4.r1.sz = v1->z;
                    div.d4.r2.sz = v2->z;
                    div.d4.r3.sz = v3->z;
                    div.d4.r0.v = *(SVECTOR *)(sp->local_vertices + face->i0);
                    div.d4.r1.v = *(SVECTOR *)(sp->local_vertices + face->i1);
                    div.d4.r2.v = *(SVECTOR *)(sp->local_vertices + face->i2);
                    div.d4.r3.v = *(SVECTOR *)(sp->local_vertices + face->i3);
                    *(u_long *)div.d4.r0.uv = face->uv0_clut;
                    *(u_long *)div.d4.r1.uv = face->uv1_tpage | (flags & 0x600000);
                    *(u_long *)div.d4.r2.uv = *(u_long *)&face->uv2;
                    *(u_long *)div.d4.r3.uv = *(u_long *)&face->uv3;
                    sp->prim = RCpolyGT4((POLY_GT4 *)sp->prim, &div.d4);
                    if (sp->prim_end < sp->prim) {
                        return;
                    }
                    face = next;
                    continue;
                } else {
                    gte_ldv3(sp->normals + face->n0, sp->normals + face->n1, sp->normals + face->n2);
                    *(u_long *)&p->u0 = face->uv0_clut;
                    gte_ldrgb(&sp->rgb);
                    if (!(MESH_MODE & MESH_NO_DEPTH_CUE)) {
                        gte_lddp((v0->p + v1->p + v2->p + v3->p) >> 2);
                        gte_ncdt();
                        *(u_long *)&p->u1 = face->uv1_tpage | (flags & 0x600000);
                        *(u_short *)&p->u2 = face->uv2;
                        gte_strgb3(&p->r0, &p->r1, &p->r2);
                        gte_ldv0(sp->normals + face->n3);
                        gte_ncds();
                        *(u_short *)&p->u3 = face->uv3;
                        mesh_add_prim(ot + z, p, 12);
                        gte_strgb(&p->r3);
                        if (MESH_MODE & (MESH_TINT_A | MESH_TINT_B)) {
                            tint(&p->r0, v0->z);
                            tint(&p->r1, v1->z);
                            tint(&p->r2, v2->z);
                            tint(&p->r3, v3->z);
                        }
                    } else {
                        gte_ncct();
                        *(u_long *)&p->u1 = face->uv1_tpage | (flags & 0x600000);
                        *(u_short *)&p->u2 = face->uv2;
                        gte_strgb3(&p->r0, &p->r1, &p->r2);
                        gte_ldv0(sp->normals + face->n3);
                        gte_nccs();
                        *(u_short *)&p->u3 = face->uv3;
                        mesh_add_prim(ot + z, p, 12);
                        gte_strgb(&p->r3);
                    }
                    sp->prim = (u_long *)((POLY_GT4 *)sp->prim + 1);
                }
            }
        }
        face = next;
    }
    sp->faces = (u8 *)face;
    /* 12 words per primitive, although a POLY_GT4 is 13. */
    sp->prim_count.count += (sp->prim - start) / 12;
}

ADDRESS(0x8001bf10, 0x1f0)
void mesh_div_draw_part(u_short part, u_int flags)
{
    ScratchPad *sp = SCRATCHPAD;
    int mode;

    mode = (flags >> 1) & 2;
    mode |= (flags & 3) << 21;
    sp->header = (MeshHeader *)(sp->model + 12) + part;
    sp->normals = sp->model + 12 + sp->header->normals;
    if (MESH_MODE & MESH_TINT_A) {
        if (MESH_MODE & MESH_TINT_SLOW) {
            sp->rgb.r = sp->rgb.g = sp->rgb.b = -120 - (MESH_TINT_LEVEL >> 13);
        } else {
            sp->rgb.r = sp->rgb.g = sp->rgb.b = -96 - (MESH_TINT_LEVEL >> 11);
        }
    } else {
        sp->rgb.r = sp->rgb.g = sp->rgb.b = 128;
    }
    if (MESH_MODE & MESH_TINT_A) {
        gte_SetFarColor(6, 4, 2);
    } else if (MESH_MODE & MESH_TINT_B) {
        gte_SetFarColor(2, 6, 6);
    } else {
        gte_SetFarColor(5, 5, 5);
    }
    sp->faces = sp->model + 12 + sp->header->faces;
    mesh_div_draw_ft3(sp->header->n_ft3, (MeshFaceFT3 *)sp->faces, sp->ot, mode);
    mesh_div_draw_ft4(sp->header->n_ft4, (MeshFaceFT4 *)sp->faces, sp->ot, mode);
    mesh_div_draw_gt3(sp->header->n_gt3, (MeshFaceGT3 *)sp->faces, sp->ot, mode);
    mesh_div_draw_gt4(sp->header->n_gt4, (MeshFaceGT4 *)sp->faces, sp->ot, mode);
}

ADDRESS(0x8001c100, 0x164)
void vis_build_visible(u8 *arg)
{
    u_long mask[16];
    u_long unused[4];
    u_long *bits;
    int i;
    int j;
    int k;
    u16 id;

    vis_lists.n_sources = 0;
    vis_collect_sources((VECTOR *)arg, 0, 0);
    fill_words(mask, 0, 16);
    i = vis_lists.n_sources;
    while (i--) {
        bits = vis_source_masks[vis_lists.sources[i]].bits;
        for (j = 0; j < 16; j++) {
            mask[j] |= *bits++;
        }
    }
    vis_lists.n_visible = 0;
    for (id = j = 0; j < 16; j++) {
        for (k = 0; k < 32; k++) {
            if (mask[j] & 1) {
                if (area_state.loading != 1 || area_state.loading_world == 0) {
                    vis_lists.visible[vis_lists.n_visible] = id;
                    vis_lists.n_visible++;
                }
            }
            mask[j] >>= 1;
            id++;
        }
    }
}

/*
 * Builds the viewer's two side planes for vis_measure_position: the view
 * apex sits behind the viewer along its yaw, and the half-angle widens with
 * the pitch (folded to 0..2048).
 */
ADDRESS(0x8001c264, 0x264)
void vis_build_planes(void)
{
    ScratchPad *sp = SCRATCHPAD;
    Viewer *cam = &viewer;
    int yaw;
    int pitch;
    int half;
    int c;
    int s;
    int cl;
    int sl;
    int cr;
    int sr;
    int cp;
    int back;
    int len;
    int near;
    short ox;
    short oz;
    short lx;
    short lz;
    short rx;
    short rz;

    yaw = cam->yaw + 1024;
    pitch = cam->pitch;
    if (pitch > 2048) {
        half = 4096 - pitch;
    } else {
        half = pitch;
    }
    half = half * 584 / 1024 + 512;
    c = rcos(yaw);
    s = rsin(yaw);
    cl = rcos(yaw - half);
    sl = rsin(yaw - half);
    cr = rcos(yaw + half);
    sr = rsin(yaw + half);
    cp = rcos(pitch);
    sp->pitch_sin = rsin(pitch);
    back = cp * -11 >> 12;
    len = cp * 16 >> 12;
    near = cp * -8 >> 12;
    ox = back * c >> 12;
    oz = back * s >> 12;
    lx = (len * cl + back * c) >> 12;
    lz = (len * sl + back * s) >> 12;
    rx = (len * cr + back * c) >> 12;
    rz = (len * sr + back * s) >> 12;
    sp->plane[0] = lz - oz;
    sp->plane[2] = lx - ox;
    sp->plane[1] = rz - oz;
    sp->plane[3] = rx - ox;
    sp->offset_x = near * c >> 12;
    sp->offset_z = near * s >> 12;
    sp->plane[4] = -sp->plane[0] * ox + sp->plane[2] * oz;
    sp->plane[5] = -sp->plane[1] * ox + sp->plane[3] * oz;
}

/*
 * Squared distance (in 2048ths) from the viewer to the scratchpad position,
 * or 256 when the position is outside the two view planes. The distance is
 * written out on both paths; the compiler merges the copies.
 */
ADDRESS(0x8001c4c8, 0x144)
int vis_measure_position(void)
{
    ScratchPad *sp = SCRATCHPAD;
    Viewer *cam = &viewer;
    int x;
    int z;
    int y;
    int a;
    int c;
    int ox;
    int oz;

    x = (sp->x - cam->position.vx) / 2048 + (ox = sp->offset_x);
    z = (sp->z - cam->position.vz) / 2048 + (oz = sp->offset_z);
    if (sp->no_plane_test) {
        sp->offset_y = -(sp->pitch_sin * 8) >> 12;
        y = (sp->y - cam->position.vy) / 2048;
        return (x - ox) * (x - ox) + (z - oz) * (z - oz) + y * y;
    }
    a = sp->plane[0] * x - sp->plane[2] * z + sp->plane[4];
    c = sp->plane[1] * x - sp->plane[3] * z + sp->plane[5];
    if (a > 0 || c < 0) {
        return 256;
    }
    sp->offset_y = -(sp->pitch_sin * 8) >> 12;
    y = (sp->y - cam->position.vy) / 2048;
    return (x - ox) * (x - ox) + (z - oz) * (z - oz) + y * y;
}

ADDRESS(0x8001c60c, 0x1f4)
void vis_cull_by_range(void)
{
    ScratchPad *sp = SCRATCHPAD;
    MapBlock *rec;
    int range_near;
    int range_mid;
    int range_far;
    int d;
    int i;
    int k;

    range_near = (world_state.range_near >> 11) * (world_state.range_near >> 11);
    range_mid = (world_state.range_mid >> 11) * (world_state.range_mid >> 11);
    i = vis_lists.n_visible;
    range_far = (world_state.light->depth_cue_far / 2048 + 2) *
                (world_state.light->depth_cue_far / 2048 + 2);
    while (i--) {
        rec = &vis_records[vis_lists.visible[i]];
        sp->x = rec->cell_x << 12;
        sp->y = (rec->cell_y << 12) + ((rec->y_offset - 128) << 7);
        sp->z = rec->cell_z << 12;
        sp->no_plane_test = world_state.no_plane_test;
        rec->draw_flags &= 1;
        d = vis_measure_position();
        if (d > range_far) {
            vis_lists.n_visible--;
            for (k = i; k < vis_lists.n_visible; k++) {
                vis_lists.visible[k] = vis_lists.visible[k + 1];
            }
        } else if (d >= range_near || d <= range_mid) {
            rec->draw_flags |= 2;
        }
    }
}

ADDRESS(0x8001c800, 0x8c)
int vis_record_is_visible(int a, int b, int c)
{
    MapBlock *rec;
    int i;

    i = vis_lists.n_visible;
    while (i--) {
        rec = &vis_records[vis_lists.visible[i]];
        if (rec->cell_x == a && rec->cell_y == b && rec->cell_z == c) {
            return 1;
        }
    }
    return 0;
}

ADDRESS(0x8001c88c, 0x54)
int vis_id_is_visible(u16 id)
{
    int i;

    i = vis_lists.n_visible;
    while (i--) {
        if (id == vis_lists.visible[i]) {
            return 1;
        }
    }
    return 0;
}

ADDRESS(0x8001c8e0, 0xe8)
void vis_collect_creatures(void)
{
    Creature *e;
    int i;

    vis_table_a.count = 0;
    e = vis_table_a.entries;
    i = 64;
    while (i--) {
        if (e->visibility != 2 && (e->state == 1 || e->state == 5)
            && (e->visibility == 1 || vis_id_is_visible(e->block))) {
            if (vis_table_a.count >= 12) {
                return;
            }
            vis_table_a.list[vis_table_a.count] = 63 - i;
            vis_table_a.count++;
        } else {
            e->draw_state = 0;
        }
        e++;
    }
}

ADDRESS(0x8001c9c8, 0xf0)
void vis_collect_objects(void)
{
    AreaObject *e;
    int i;

    e = vis_table_b.entries;
    vis_table_b.count = 0;
    i = 240;
    while (i--) {
        if (e->type < 280) {
            e->flags &= 0x7f;
            if (vis_id_is_visible(e->block) || e->kind == 236) {
                if (vis_table_b.count >= 96) {
                    return;
                }
                e->flags |= 0x80;
                vis_table_b.list[vis_table_b.count] = 239 - i;
                vis_table_b.count++;
            }
        } else {
            e->drawn = 0;
        }
        e++;
    }
}

ADDRESS(0x8001cab8, 0xd0)
void vis_collect_ground_items(void)
{
    GroundItem *e;
    int i;

    e = vis_table_c.entries;
    vis_table_c.count = 0;
    i = 64;
    while (i--) {
        if (e->item != 0xffff && e->area == area_state.area && vis_id_is_visible(e->block)) {
            vis_table_c.list[vis_table_c.count] = 63 - i;
            vis_table_c.count++;
        }
        e++;
    }
}

/*
 * Transforms each visible record's position into camera space for its
 * part's translation. The viewer-relative offset is halved until it fits
 * an SVECTOR, rotated with RTV0TR, and the result scaled back up.
 */
ADDRESS(0x8001cb88, 0x270)
void world_transform_parts(VECTOR *trans)
{
    ScratchPad *sp = SCRATCHPAD;
    MapBlock *rec;
    int i;

    i = vis_lists.n_visible;
    while (i--) {
        rec = &vis_records[vis_lists.visible[i]];
        sp->x = rec->cell_x << 12;
        sp->y = (rec->cell_y << 12) + ((rec->y_offset - 128) << 7);
        sp->z = rec->cell_z << 12;
        sp->x = sp->x - viewer.position.vx;
        sp->y = sp->y - (viewer.position.vy + viewer.bob_offset + viewer.unk2D2);
        sp->z = sp->z - viewer.position.vz;
        sp->part_shift = 0;
        while (sp->x > 32767 || sp->x < -32768 || sp->y > 32767 || sp->y < -32768
               || sp->z > 32767 || sp->z < -32768) {
            sp->x >>= 1;
            sp->y >>= 1;
            sp->z >>= 1;
            sp->part_shift++;
        }
        sp->part_offset.vx = sp->x;
        sp->part_offset.vy = sp->y;
        sp->part_offset.vz = sp->z;
        gte_ldv0(&sp->part_offset);
        gte_rtv0tr();
        gte_stlvnl(&sp->part_pos);
        gte_stflg(&sp->flag);
        trans[i].pad = sp->part_shift;
        while (--sp->part_shift != -1) {
            sp->part_pos.vx <<= 1;
            sp->part_pos.vy <<= 1;
            sp->part_pos.vz <<= 1;
        }
        trans[i].vx = sp->part_pos.vx;
        trans[i].vy = sp->part_pos.vy;
        trans[i].vz = sp->part_pos.vz;
    }
}

/* A transformed vertex of the current record's part. */
#define PART_VERTEX(sp, i) ((MeshVertex *)((u8 *)(sp)->vertex_out + (i)))

/*
 * Projects each visible record's part vertices (RTPT in threes, then RTPS)
 * into the shared transformed-vertex buffer, with a depth-cue value from
 * the lighting's near/far depths (ScratchPad.depth_cue_near/unk68).
 */
/*
 * Not exact (same size, same instructions): retail keeps the unk38 copy
 * (`lw v0,52(t0)` ... `sw v0,56(t0)`) behind the two vertex pointer stores;
 * here sched1 lifts it above the header->vertices load, which swaps v0/v1
 * in between. In the sched1 trace the copy's store has priority 4 (a load
 * at depth 3 plus its latency) against 5 for the vertex stores (their
 * chain starts with the rec->part load), so it is placed after them in
 * the backward pass; retail needs it at 5 or tied. Ruled out: every order
 * of the five header statements, scratch variables for the copied value,
 * `*(T *)&field` spellings, moving the n_vertices read ahead of the copy.
 * Retail's final order (`lw v0,52; sw 60; sw 64; sw v0,56`) is what sched2
 * makes of "vertex stores, then the copy" once sched1 leaves the copy
 * last; the copy's store is free of every other insn in sched1 (same
 * constant base, other offsets), so only its priority places it. A test
 * build whose scratch base sched1 cannot resolve adds the store's
 * dependence on the header->vertices load and still leaves it ahead of
 * the vertex stores, so the base is not the cause. With retail's sched1
 * order local-alloc gives every register by itself (offset v0, model and
 * sum v1, unk34 v0, header reload a0), but only while all of them are
 * single-death block locals: one variable shared by the copy and the
 * offset or the sum does order the copy last, yet has two deaths, goes to
 * global-alloc and takes a0/a1 (or pulls unk34 into v1).
 * Narrowed since: priority alone is not enough. With the copy's store at
 * 5 or more it is picked first, but its load is a boosted temporary that
 * is blocked behind the two vertex stores and then lands above the sum,
 * where the offset still holds v0. Retail needs the load itself to wait
 * for the sum (the header reload in a0 shows the offset and the model
 * are block locals there, so no variable is shared), and sched1 has no
 * memory or register rule that gives it for these addresses.
 */
ADDRESS(0x8001cdf8, 0x42c)
void world_project_parts(VECTOR *trans)
{
    ScratchPad *sp = SCRATCHPAD;
    MapBlock *rec;
    MeshVertex *out;
    SVECTOR *in;
    int i;
    int n;

    i = vis_lists.n_visible;
    while (i--) {
        rec = &vis_records[vis_lists.visible[i]];
        sp->turn = rec->turn & 3;
        gte_SetRotMatrix(&world_state.view_quarter_turns[sp->turn]);
        gte_SetTransVector(&trans[i]);
        sp->header = (MeshHeader *)(sp->model + 12) + rec->part;
        sp->vertices = sp->part_vertices = sp->model + 12 + sp->header->vertices;
        sp->unk38 = sp->vertex_out;
        n = sp->header->n_vertices / 3;
        out = (MeshVertex *)sp->vertex_out;
        in = (SVECTOR *)sp->part_vertices;
        while (n--) {
            gte_ldv3c(in);
            gte_rtpt();
            gte_stsxy3(&out[0].sxy, &out[1].sxy, &out[2].sxy);
            gte_stsz3(&out[0].z, &out[1].z, &out[2].z);
            if (sp->depth_cue_near != sp->depth_cue_far) {
                out[0].p = ((out[0].z - sp->depth_cue_near) << 12) / (sp->depth_cue_far - sp->depth_cue_near);
                out[1].p = ((out[1].z - sp->depth_cue_near) << 12) / (sp->depth_cue_far - sp->depth_cue_near);
                out[2].p = ((out[2].z - sp->depth_cue_near) << 12) / (sp->depth_cue_far - sp->depth_cue_near);
            } else {
                out[0].p = 0;
                out[1].p = 0;
                out[2].p = 0;
            }
            if (out[0].p < 0) {
                out[0].p = 0;
            }
            if (out[1].p < 0) {
                out[1].p = 0;
            }
            if (out[2].p < 0) {
                out[2].p = 0;
            }
            out += 3;
            in += 3;
        }
        n = sp->header->n_vertices % 3;
        while (n--) {
            gte_ldv0(in);
            gte_rtps();
            gte_stsxy(&out->sxy);
            gte_stsz(&out->z);
            if (sp->depth_cue_near != sp->depth_cue_far) {
                out->p = ((out->z - sp->depth_cue_near) << 12) / (sp->depth_cue_far - sp->depth_cue_near);
            } else {
                out->p = 0;
            }
            if (out->p > 4065) {
                out->p = 4065;
            }
            if (out->p < 0) {
                out->p = 0;
            }
            out++;
            in++;
        }
        sp->part_vertex_count += sp->header->n_vertices;
        sp->vertex_out = (u_long *)((u8 *)sp->vertex_out + sp->header->n_vertices * 8);
    }
}

ADDRESS(0x8001d224, 0x40)
void world_ft3_tint_a(CVECTOR *rgb)
{
    if (rgb->b > 85) {
        rgb->b = 85;
    }
    rgb->r = rgb->b * 3;
    rgb->g = rgb->b * 2;
}

ADDRESS(0x8001d264, 0x44)
void world_ft3_tint_b(CVECTOR *rgb)
{
    if (rgb->r > 85) {
        rgb->r = 85;
    }
    rgb->b = rgb->r * 3;
    rgb->g = rgb->r * 3;
}

ADDRESS(0x8001d2a8, 0x838)
void world_draw_ft3_near(VECTOR *trans)
{
    ScratchPad *sp = SCRATCHPAD;
    DIVPOLYGON3 div;
    void (*tint)(CVECTOR *);
    MapBlock *rec;
    MeshFaceFT3 *face;
    MeshVertex *v0;
    MeshVertex *v1;
    MeshVertex *v2;
    POLY_GT3 *p;
    int i;
    int n;
    int t;

    tint = 0;
    i = vis_lists.n_visible;
    if (MESH_MODE & MESH_TINT_A) {
        tint = world_ft3_tint_a;
    } else if (MESH_MODE & MESH_TINT_B) {
        tint = world_ft3_tint_b;
    }
    while (i--) {
        rec = &vis_records[vis_lists.visible[i]];
        if (!(rec->draw_flags & 3)) {
            sp->header = (MeshHeader *)(sp->model + 12) + rec->part;
            sp->vertex_out = (u_long *)((u8 *)sp->vertex_out + sp->header->n_vertices * 8);
        } else {
            sp->turn = rec->turn & 3;
            gte_SetLightMatrix(&sp->light->light[sp->turn]);
            sp->header = (MeshHeader *)(sp->model + 12) + rec->part;
            sp->part_vertices = sp->model + 12 + sp->header->vertices;
            sp->normals = sp->model + 12 + sp->header->normals;
            face = (MeshFaceFT3 *)(sp->model + 12 + sp->header->faces);
            n = sp->header->n_ft3;
            sp->part_faces += n;
            while ((t = n, n = n - 1, t)) {
                v0 = PART_VERTEX(sp, face->i0);
                v1 = PART_VERTEX(sp, face->i1);
                v2 = PART_VERTEX(sp, face->i2);
                gte_ldsxy3(*(u_long *)&v0->sxy, *(u_long *)&v1->sxy, *(u_long *)&v2->sxy);
                gte_nclip();
                sp->part_rgb.cd = face->code | 0x34;
                gte_stopz(&sp->opz);
                if (sp->opz <= 0) {
                    face++;
                    continue;
                }
                gte_stsxy3_gt3(sp->prim);
                sp->near_z = sp->z_max = v0->z;
                if (sp->z_max < v1->z) {
                    sp->z_max = v1->z;
                }
                if (v1->z < sp->near_z) {
                    sp->near_z = v1->z;
                }
                if (sp->z_max < v2->z) {
                    sp->z_max = v2->z;
                }
                if (v2->z < sp->near_z) {
                    sp->near_z = v2->z;
                }
                sp->z_ot = (sp->z_max >> 5) + face->depth_bias;
                if (sp->z_ot >= 4096 || sp->z_ot < sp->face_count) {
                    face++;
                    continue;
                }
                if (sp->opz > 0x4000) {
                    div.ndiv = 1;
                    if (sp->opz > 0x8000) {
                        div.ndiv = 2;
                        if (sp->opz > 0x10000) {
                            div.ndiv = 3;
                        }
                    }
                    div.pih = 320;
                    div.piv = 240;
                    div.clut = MESH_WORD_HI(face->uv0_clut);
                    div.tpage = MESH_WORD_HI(face->uv1_tpage);
                    gte_ldv0(sp->normals + face->normal);
                    gte_ldrgb(&sp->part_rgb);
                    gte_nccs();
                    gte_strgb(&sp->unk2C);
                    gte_ldrgb(&sp->unk2C);
                    gte_lddp(v0->p);
                    gte_dpcs();
                    gte_strgb(&div.r0.c);
                    gte_lddp(v1->p);
                    gte_dpcs();
                    gte_strgb(&div.r1.c);
                    gte_lddp(v2->p);
                    gte_dpcs();
                    gte_strgb(&div.r2.c);
                    if (MESH_MODE & (MESH_TINT_A | MESH_TINT_B)) {
                        tint(&div.r0.c);
                        tint(&div.r1.c);
                        tint(&div.r2.c);
                    }
                    gte_SetRotMatrix(&world_state.view_quarter_turns[sp->turn]);
                    gte_SetTransVector(&trans[i]);
                    div.rgbc = sp->part_rgb;
                    div.cr[0].r0 = &div.r0;
                    div.cr[0].r1 = &div.r1;
                    div.cr[0].r2 = &div.r2;
                    div.ot = sp->ot + sp->z_ot;
                    div.r0.sxy = v0->sxy;
                    div.r1.sxy = v1->sxy;
                    div.r2.sxy = v2->sxy;
                    div.r0.sz = v0->z;
                    div.r1.sz = v1->z;
                    div.r2.sz = v2->z;
                    div.r0.v = *(SVECTOR *)(sp->part_vertices + face->i0);
                    div.r1.v = *(SVECTOR *)(sp->part_vertices + face->i1);
                    div.r2.v = *(SVECTOR *)(sp->part_vertices + face->i2);
                    ((u_long *)&div.r0)[2] = face->uv0_clut;
                    ((u_long *)&div.r1)[2] = face->uv1_tpage;
                    ((u_long *)&div.r2)[2] = *(u_long *)&face->uv2;
                    face++;
                    sp->prim = RCpolyGT3((POLY_GT3 *)sp->prim, &div);
                    sp->part_div_prims += (1 << div.ndiv) << div.ndiv;
                } else {
                    p = (POLY_GT3 *)sp->prim;
                    if (sp->prim_end < (u_long *)(p + 1)) {
                        return;
                    }
                    sp->prim = (u_long *)(p + 1);
                    sp->part_prims++;
                    gte_ldv0(sp->normals + face->normal);
                    gte_ldrgb(&sp->part_rgb);
                    gte_nccs();
                    *(u_long *)&p->u0 = face->uv0_clut;
                    *(u_long *)&p->u1 = face->uv1_tpage;
                    gte_strgb(&sp->unk2C);
                    gte_ldrgb(&sp->unk2C);
                    gte_lddp(v0->p);
                    gte_dpcs();
                    *(u_short *)&p->u2 = face->uv2;
                    gte_strgb(&p->r0);
                    gte_lddp(v1->p);
                    gte_dpcs();
                    mesh_set_tag(sp->ot + sp->z_ot, p, 9);
                    gte_strgb(&p->r1);
                    gte_lddp(v2->p);
                    gte_dpcs();
                    mesh_set_ot(sp->ot + sp->z_ot, p);
                    gte_strgb(&p->r2);
                    if (MESH_MODE & (MESH_TINT_A | MESH_TINT_B)) {
                        tint((CVECTOR *)&p->r0);
                        tint((CVECTOR *)&p->r1);
                        tint((CVECTOR *)&p->r2);
                    }
                    face++;
                }
            }
            sp->vertex_out = (u_long *)((u8 *)sp->vertex_out + sp->header->n_vertices * 8);
        }
    }
}

ADDRESS(0x8001dae0, 0x40)
void world_ft4_tint_a(CVECTOR *rgb)
{
    if (rgb->b > 85) {
        rgb->b = 85;
    }
    rgb->r = rgb->b * 3;
    rgb->g = rgb->b * 2;
}

ADDRESS(0x8001db20, 0x44)
void world_ft4_tint_b(CVECTOR *rgb)
{
    if (rgb->r > 85) {
        rgb->r = 85;
    }
    rgb->b = rgb->r * 3;
    rgb->g = rgb->r * 3;
}

ADDRESS(0x8001db64, 0x9b8)
void world_draw_ft4_near(VECTOR *trans)
{
    ScratchPad *sp = SCRATCHPAD;
    DIVPOLYGON4 div;
    void (*tint)(CVECTOR *);
    MapBlock *rec;
    MeshFaceFT4 *face;
    MeshVertex *v0;
    MeshVertex *v1;
    MeshVertex *v2;
    MeshVertex *v3;
    POLY_GT4 *p;
    int i;
    int n;
    int t;

    tint = 0;
    i = vis_lists.n_visible;
    if (MESH_MODE & MESH_TINT_A) {
        tint = world_ft4_tint_a;
    } else if (MESH_MODE & MESH_TINT_B) {
        tint = world_ft4_tint_b;
    }
    while (i--) {
        rec = &vis_records[vis_lists.visible[i]];
        if (!(rec->draw_flags & 3)) {
            sp->header = (MeshHeader *)(sp->model + 12) + rec->part;
            sp->vertex_out = (u_long *)((u8 *)sp->vertex_out + sp->header->n_vertices * 8);
        } else {
            sp->turn = rec->turn & 3;
            gte_SetLightMatrix(&sp->light->light[sp->turn]);
            sp->header = (MeshHeader *)(sp->model + 12) + rec->part;
            sp->part_vertices = sp->model + 12 + sp->header->vertices;
            sp->normals = sp->model + 12 + sp->header->normals;
            face = (MeshFaceFT4 *)(sp->model + 12 + sp->header->faces);
            face = (MeshFaceFT4 *)((u8 *)face + sp->header->n_ft3 * sizeof(MeshFaceFT3));
            n = sp->header->n_ft4;
            sp->part_faces += n;
            while ((t = n, n = n - 1, t)) {
                v0 = PART_VERTEX(sp, face->i0);
                v1 = PART_VERTEX(sp, face->i1);
                v2 = PART_VERTEX(sp, face->i2);
                gte_ldsxy3(*(u_long *)&v0->sxy, *(u_long *)&v1->sxy, *(u_long *)&v2->sxy);
                gte_nclip();
                sp->part_rgb.cd = face->code | 0x3c;
                gte_stopz(&sp->opz);
                v3 = PART_VERTEX(sp, face->i3);
                gte_stsxy3_gt4(sp->prim);
                gte_ldsxy0(*(u_long *)&v3->sxy);
                gte_nclip();
                gte_stopz(&sp->opz2);
                if (sp->opz <= 0 && sp->opz2 >= 0) {
                    face++;
                    continue;
                }
                gte_stsxy0(&((POLY_GT4 *)sp->prim)->x3);
                sp->near_z = sp->z_max = v0->z;
                if (sp->z_max < v1->z) {
                    sp->z_max = v1->z;
                }
                if (v1->z < sp->near_z) {
                    sp->near_z = v1->z;
                }
                if (sp->z_max < v2->z) {
                    sp->z_max = v2->z;
                }
                if (v2->z < sp->near_z) {
                    sp->near_z = v2->z;
                }
                if (sp->z_max < v3->z) {
                    sp->z_max = v3->z;
                }
                if (v3->z < sp->near_z) {
                    sp->near_z = v3->z;
                }
                sp->z_ot = (sp->z_max >> 5) + face->depth_bias;
                if (sp->z_ot >= 4096 || sp->z_ot < sp->face_count) {
                    face++;
                    continue;
                }
                if (sp->opz > 0x4000 || sp->opz2 < -0x4000) {
                    div.ndiv = 1;
                    if (sp->opz > 0x8000 || sp->opz2 < -0x8000) {
                        div.ndiv = 2;
                    }
                    div.pih = 320;
                    div.piv = 240;
                    div.clut = MESH_WORD_HI(face->uv0_clut);
                    div.tpage = MESH_WORD_HI(face->uv1_tpage);
                    gte_ldv0(sp->normals + face->normal);
                    gte_ldrgb(&sp->part_rgb);
                    gte_nccs();
                    gte_strgb(&sp->unk2C);
                    gte_ldrgb(&sp->unk2C);
                    gte_lddp(v0->p);
                    gte_dpcs();
                    gte_strgb(&div.r0.c);
                    gte_lddp(v1->p);
                    gte_dpcs();
                    gte_strgb(&div.r1.c);
                    gte_lddp(v2->p);
                    gte_dpcs();
                    gte_strgb(&div.r2.c);
                    gte_lddp(v3->p);
                    gte_dpcs();
                    gte_strgb(&div.r3.c);
                    if (MESH_MODE & (MESH_TINT_A | MESH_TINT_B)) {
                        tint(&div.r0.c);
                        tint(&div.r1.c);
                        tint(&div.r2.c);
                        tint(&div.r3.c);
                    }
                    gte_SetRotMatrix(&world_state.view_quarter_turns[sp->turn]);
                    gte_SetTransVector(&trans[i]);
                    div.rgbc = sp->part_rgb;
                    div.cr[0].r0 = &div.r0;
                    div.cr[0].r1 = &div.r1;
                    div.cr[0].r2 = &div.r2;
                    div.cr[0].r3 = &div.r3;
                    div.ot = sp->ot + sp->z_ot;
                    div.r0.sxy = v0->sxy;
                    div.r1.sxy = v1->sxy;
                    div.r2.sxy = v2->sxy;
                    div.r3.sxy = v3->sxy;
                    div.r0.sz = v0->z;
                    div.r1.sz = v1->z;
                    div.r2.sz = v2->z;
                    div.r3.sz = v3->z;
                    div.r0.v = *(SVECTOR *)(sp->part_vertices + face->i0);
                    div.r1.v = *(SVECTOR *)(sp->part_vertices + face->i1);
                    div.r2.v = *(SVECTOR *)(sp->part_vertices + face->i2);
                    div.r3.v = *(SVECTOR *)(sp->part_vertices + face->i3);
                    ((u_long *)&div.r0)[2] = face->uv0_clut;
                    ((u_long *)&div.r1)[2] = face->uv1_tpage;
                    ((u_long *)&div.r2)[2] = *(u_long *)&face->uv2;
                    ((u_long *)&div.r3)[2] = *(u_long *)&face->uv3;
                    face++;
                    sp->prim = RCpolyGT4((POLY_GT4 *)sp->prim, &div);
                    sp->part_div_prims += (1 << div.ndiv) << div.ndiv;
                } else {
                    p = (POLY_GT4 *)sp->prim;
                    if (sp->prim_end < (u_long *)(p + 1)) {
                        return;
                    }
                    sp->prim = (u_long *)(p + 1);
                    sp->part_prims++;
                    gte_ldv0(sp->normals + face->normal);
                    gte_ldrgb(&sp->part_rgb);
                    gte_nccs();
                    *(u_long *)&p->u0 = face->uv0_clut;
                    *(u_long *)&p->u1 = face->uv1_tpage;
                    gte_strgb(&sp->unk2C);
                    gte_ldrgb(&sp->unk2C);
                    gte_lddp(v0->p);
                    gte_dpcs();
                    *(u_short *)&p->u2 = face->uv2;
                    gte_strgb(&p->r0);
                    gte_lddp(v1->p);
                    gte_dpcs();
                    *(u_short *)&p->u3 = face->uv3;
                    gte_strgb(&p->r1);
                    gte_lddp(v2->p);
                    gte_dpcs();
                    mesh_set_tag(sp->ot + sp->z_ot, p, 12);
                    gte_strgb(&p->r2);
                    gte_lddp(v3->p);
                    gte_dpcs();
                    mesh_set_ot(sp->ot + sp->z_ot, p);
                    gte_strgb(&p->r3);
                    if (MESH_MODE & (MESH_TINT_A | MESH_TINT_B)) {
                        tint((CVECTOR *)&p->r0);
                        tint((CVECTOR *)&p->r1);
                        tint((CVECTOR *)&p->r2);
                        tint((CVECTOR *)&p->r3);
                    }
                    face++;
                }
            }
            sp->vertex_out = (u_long *)((u8 *)sp->vertex_out + sp->header->n_vertices * 8);
        }
    }
}

ADDRESS(0x8001e51c, 0x40)
void world_gt3_tint_a(CVECTOR *rgb)
{
    if (rgb->b > 85) {
        rgb->b = 85;
    }
    rgb->r = rgb->b * 3;
    rgb->g = rgb->b * 2;
}

ADDRESS(0x8001e55c, 0x44)
void world_gt3_tint_b(CVECTOR *rgb)
{
    if (rgb->r > 85) {
        rgb->r = 85;
    }
    rgb->b = rgb->r * 3;
    rgb->g = rgb->r * 3;
}

ADDRESS(0x8001e5a0, 0x888)
void world_draw_gt3_near(VECTOR *trans)
{
    ScratchPad *sp = SCRATCHPAD;
    DIVPOLYGON3 div;
    void (*tint)(CVECTOR *);
    MapBlock *rec;
    MeshFaceGT3 *face;
    MeshVertex *v0;
    MeshVertex *v1;
    MeshVertex *v2;
    POLY_GT3 *p;
    int i;
    int n;

    tint = 0;
    i = vis_lists.n_visible;
    if (MESH_MODE & MESH_TINT_A) {
        tint = world_gt3_tint_a;
    } else if (MESH_MODE & MESH_TINT_B) {
        tint = world_gt3_tint_b;
    }
    while (i--) {
        rec = &vis_records[vis_lists.visible[i]];
        if (!(rec->draw_flags & 3)) {
            sp->header = (MeshHeader *)(sp->model + 12) + rec->part;
            sp->vertex_out = (u_long *)((u8 *)sp->vertex_out + sp->header->n_vertices * 8);
        } else {
            sp->turn = rec->turn & 3;
            gte_SetLightMatrix(&sp->light->light[sp->turn]);
            sp->header = (MeshHeader *)(sp->model + 12) + rec->part;
            sp->part_vertices = sp->model + 12 + sp->header->vertices;
            sp->normals = sp->model + 12 + sp->header->normals;
            face = (MeshFaceGT3 *)(sp->model + 12 + sp->header->faces);
            face = (MeshFaceGT3 *)((u8 *)face + sp->header->n_ft3 * sizeof(MeshFaceFT3));
            face = (MeshFaceGT3 *)((u8 *)face + sp->header->n_ft4 * sizeof(MeshFaceFT4));
            n = sp->header->n_gt3;
            sp->faces_left = n;
            sp->part_faces += n;
            while (--sp->faces_left != -1) {
                v0 = PART_VERTEX(sp, face->i0);
                v1 = PART_VERTEX(sp, face->i1);
                v2 = PART_VERTEX(sp, face->i2);
                gte_ldsxy3(*(u_long *)&v0->sxy, *(u_long *)&v1->sxy, *(u_long *)&v2->sxy);
                gte_nclip();
                sp->part_rgb.cd = face->code | 0x34;
                gte_stopz(&sp->opz);
                if (sp->opz <= 0) {
                    face++;
                    continue;
                }
                gte_stsxy3_gt3(sp->prim);
                sp->near_z = sp->z_max = v0->z;
                if (sp->z_max < v1->z) {
                    sp->z_max = v1->z;
                }
                if (v1->z < sp->near_z) {
                    sp->near_z = v1->z;
                }
                if (sp->z_max < v2->z) {
                    sp->z_max = v2->z;
                }
                if (v2->z < sp->near_z) {
                    sp->near_z = v2->z;
                }
                sp->z_ot = (sp->z_max >> 5) + face->depth_bias;
                if (sp->z_ot >= 4096 || sp->z_ot < sp->face_count) {
                    face++;
                    continue;
                }
                if (sp->opz > 0x4000) {
                    div.ndiv = 1;
                    if (sp->opz > 0x8000) {
                        div.ndiv = 2;
                        if (sp->opz > 0x10000) {
                            div.ndiv = 3;
                        }
                    }
                    div.pih = 320;
                    div.piv = 240;
                    div.clut = MESH_WORD_HI(face->uv0_clut);
                    div.tpage = MESH_WORD_HI(face->uv1_tpage);
                    gte_ldrgb(&sp->part_rgb);
                    gte_ldv0(sp->normals + face->n0);
                    gte_lddp(v0->p);
                    gte_ncds();
                    gte_strgb(&div.r0.c);
                    gte_ldv0(sp->normals + face->n1);
                    gte_lddp(v1->p);
                    gte_ncds();
                    gte_strgb(&div.r1.c);
                    gte_ldv0(sp->normals + face->n2);
                    gte_lddp(v2->p);
                    gte_ncds();
                    gte_strgb(&div.r2.c);
                    if (MESH_MODE & (MESH_TINT_A | MESH_TINT_B)) {
                        tint(&div.r0.c);
                        tint(&div.r1.c);
                        tint(&div.r2.c);
                    }
                    gte_SetRotMatrix(&world_state.view_quarter_turns[sp->turn]);
                    gte_SetTransVector(&trans[i]);
                    div.rgbc = sp->part_rgb;
                    div.cr[0].r0 = &div.r0;
                    div.cr[0].r1 = &div.r1;
                    div.cr[0].r2 = &div.r2;
                    div.ot = sp->ot + sp->z_ot;
                    div.r0.sxy = v0->sxy;
                    div.r1.sxy = v1->sxy;
                    div.r2.sxy = v2->sxy;
                    div.r0.sz = v0->z;
                    div.r1.sz = v1->z;
                    div.r2.sz = v2->z;
                    div.r0.v = *(SVECTOR *)(sp->part_vertices + face->i0);
                    div.r1.v = *(SVECTOR *)(sp->part_vertices + face->i1);
                    div.r2.v = *(SVECTOR *)(sp->part_vertices + face->i2);
                    ((u_long *)&div.r0)[2] = face->uv0_clut;
                    ((u_long *)&div.r1)[2] = face->uv1_tpage;
                    ((u_long *)&div.r2)[2] = *(u_long *)&face->uv2;
                    face++;
                    sp->prim = RCpolyGT3((POLY_GT3 *)sp->prim, &div);
                    sp->part_div_prims += (1 << div.ndiv) << div.ndiv;
                } else {
                    p = (POLY_GT3 *)sp->prim;
                    if (sp->prim_end < (u_long *)(p + 1)) {
                        return;
                    }
                    sp->prim = (u_long *)(p + 1);
                    sp->part_prims++;
                    gte_ldrgb(&sp->part_rgb);
                    gte_ldv0(sp->normals + face->n0);
                    gte_lddp(v0->p);
                    *(u_long *)&p->u0 = face->uv0_clut;
                    gte_ncds_b();
                    *(u_long *)&p->u1 = face->uv1_tpage;
                    *(u_short *)&p->u2 = face->uv2;
                    gte_strgb(&p->r0);
                    gte_ldv0(sp->normals + face->n1);
                    gte_lddp(v1->p);
                    gte_ncds();
                    mesh_set_tag(sp->ot + sp->z_ot, p, 9);
                    gte_strgb(&p->r1);
                    gte_ldv0(sp->normals + face->n2);
                    gte_lddp(v2->p);
                    gte_ncds();
                    mesh_set_ot(sp->ot + sp->z_ot, p);
                    gte_strgb(&p->r2);
                    if (MESH_MODE & (MESH_TINT_A | MESH_TINT_B)) {
                        tint((CVECTOR *)&p->r0);
                        tint((CVECTOR *)&p->r1);
                        tint((CVECTOR *)&p->r2);
                    }
                    face++;
                }
            }
            sp->vertex_out = (u_long *)((u8 *)sp->vertex_out + sp->header->n_vertices * 8);
        }
    }
}

ADDRESS(0x8001ee28, 0x40)
void world_gt4_tint_a(CVECTOR *rgb)
{
    if (rgb->b > 85) {
        rgb->b = 85;
    }
    rgb->r = rgb->b * 3;
    rgb->g = rgb->b * 2;
}

ADDRESS(0x8001ee68, 0x44)
void world_gt4_tint_b(CVECTOR *rgb)
{
    if (rgb->r > 85) {
        rgb->r = 85;
    }
    rgb->b = rgb->r * 3;
    rgb->g = rgb->r * 3;
}

ADDRESS(0x8001eeac, 0xaa8)
void world_draw_gt4_near(VECTOR *trans)
{
    ScratchPad *sp = SCRATCHPAD;
    DIVPOLYGON4 div;
    MapBlock *rec;
    MeshFaceGT4 *face;
    MeshVertex *v0;
    MeshVertex *v1;
    MeshVertex *v2;
    MeshVertex *v3;
    POLY_GT4 *p;
    int i;
    void (*tint)(CVECTOR *);
    int n;

    tint = 0;
    i = vis_lists.n_visible;
    if (MESH_MODE & MESH_TINT_A) {
        tint = world_gt4_tint_a;
    } else if (MESH_MODE & MESH_TINT_B) {
        tint = world_gt4_tint_b;
    }
    while (i--) {
        rec = &vis_records[vis_lists.visible[i]];
        if (!(rec->draw_flags & 3)) {
            sp->header = (MeshHeader *)(sp->model + 12) + rec->part;
            sp->vertex_out = (u_long *)((u8 *)sp->vertex_out + sp->header->n_vertices * 8);
        } else {
            sp->turn = rec->turn & 3;
            gte_SetLightMatrix(&sp->light->light[sp->turn]);
            sp->header = (MeshHeader *)(sp->model + 12) + rec->part;
            sp->part_vertices = sp->model + 12 + sp->header->vertices;
            sp->normals = sp->model + 12 + sp->header->normals;
            face = (MeshFaceGT4 *)(sp->model + 12 + sp->header->faces);
            face = (MeshFaceGT4 *)((u8 *)face + sp->header->n_ft3 * sizeof(MeshFaceFT3));
            face = (MeshFaceGT4 *)((u8 *)face + sp->header->n_ft4 * sizeof(MeshFaceFT4));
            face = (MeshFaceGT4 *)((u8 *)face + sp->header->n_gt3 * sizeof(MeshFaceGT3));
            n = sp->header->n_gt4;
            sp->faces_left = n;
            sp->part_faces += n;
            while (--sp->faces_left != -1) {
                v0 = PART_VERTEX(sp, face->i0);
                v1 = PART_VERTEX(sp, face->i1);
                v2 = PART_VERTEX(sp, face->i2);
                gte_ldsxy3(*(u_long *)&v0->sxy, *(u_long *)&v1->sxy, *(u_long *)&v2->sxy);
                gte_nclip();
                sp->part_rgb.cd = face->code | 0x3c;
                gte_stopz(&sp->opz);
                gte_stsxy3_gt4(sp->prim);
                v3 = PART_VERTEX(sp, face->i3);
                gte_ldsxy0(*(u_long *)&v3->sxy);
                gte_nclip();
                gte_stopz(&sp->opz2);
                if (sp->opz <= 0 && sp->opz2 >= 0) {
                    face++;
                    continue;
                }
                gte_stsxy0(&((POLY_GT4 *)sp->prim)->x3);
                sp->near_z = sp->z_max = v0->z;
                if (sp->z_max < v1->z) {
                    sp->z_max = v1->z;
                }
                if (v1->z < sp->near_z) {
                    sp->near_z = v1->z;
                }
                if (sp->z_max < v2->z) {
                    sp->z_max = v2->z;
                }
                if (v2->z < sp->near_z) {
                    sp->near_z = v2->z;
                }
                if (sp->z_max < v3->z) {
                    sp->z_max = v3->z;
                }
                if (v3->z < sp->near_z) {
                    sp->near_z = v3->z;
                }
                sp->z_ot = (sp->z_max >> 5) + face->depth_bias;
                if (sp->z_ot >= 4096 || sp->z_ot < sp->face_count) {
                    face++;
                    continue;
                }
                if (sp->opz > 0x4000 || sp->opz2 < -0x4000) {
                    div.ndiv = 1;
                    if (sp->opz > 0x8000 || sp->opz2 < -0x8000) {
                        div.ndiv = 2;
                        if (sp->opz > 0x10000 || sp->opz2 < -0x10000) {
                            div.ndiv = 3;
                        }
                    }
                    div.pih = 320;
                    div.piv = 240;
                    div.clut = MESH_WORD_HI(face->uv0_clut);
                    div.tpage = MESH_WORD_HI(face->uv1_tpage);
                    gte_ldrgb(&sp->part_rgb);
                    gte_ldv0(sp->normals + face->n0);
                    gte_lddp(v0->p);
                    gte_ncds();
                    gte_strgb(&div.r0.c);
                    gte_ldv0(sp->normals + face->n1);
                    gte_lddp(v1->p);
                    gte_ncds();
                    gte_strgb(&div.r1.c);
                    gte_ldv0(sp->normals + face->n2);
                    gte_lddp(v2->p);
                    gte_ncds();
                    gte_strgb(&div.r2.c);
                    gte_ldv0(sp->normals + face->n3);
                    gte_lddp(v3->p);
                    gte_ncds();
                    gte_strgb(&div.r3.c);
                    if (MESH_MODE & (MESH_TINT_A | MESH_TINT_B)) {
                        tint(&div.r0.c);
                        tint(&div.r1.c);
                        tint(&div.r2.c);
                        tint(&div.r3.c);
                    }
                    gte_SetRotMatrix(&world_state.view_quarter_turns[sp->turn]);
                    gte_SetTransVector(&trans[i]);
                    div.rgbc = sp->part_rgb;
                    div.cr[0].r0 = &div.r0;
                    div.cr[0].r1 = &div.r1;
                    div.cr[0].r2 = &div.r2;
                    div.cr[0].r3 = &div.r3;
                    div.ot = sp->ot + sp->z_ot;
                    div.r0.sxy = v0->sxy;
                    div.r1.sxy = v1->sxy;
                    div.r2.sxy = v2->sxy;
                    div.r3.sxy = v3->sxy;
                    div.r0.sz = v0->z;
                    div.r1.sz = v1->z;
                    div.r2.sz = v2->z;
                    div.r3.sz = v3->z;
                    div.r0.v = *(SVECTOR *)(sp->part_vertices + face->i0);
                    div.r1.v = *(SVECTOR *)(sp->part_vertices + face->i1);
                    div.r2.v = *(SVECTOR *)(sp->part_vertices + face->i2);
                    div.r3.v = *(SVECTOR *)(sp->part_vertices + face->i3);
                    ((u_long *)&div.r0)[2] = face->uv0_clut;
                    ((u_long *)&div.r1)[2] = face->uv1_tpage;
                    ((u_long *)&div.r2)[2] = *(u_long *)&face->uv2;
                    ((u_long *)&div.r3)[2] = *(u_long *)&face->uv3;
                    face++;
                    sp->prim = RCpolyGT4((POLY_GT4 *)sp->prim, &div);
                    sp->part_div_prims += (1 << div.ndiv) << div.ndiv;
                } else {
                    p = (POLY_GT4 *)sp->prim;
                    if (sp->prim_end < (u_long *)(p + 1)) {
                        return;
                    }
                    sp->prim = (u_long *)(p + 1);
                    sp->part_prims++;
                    gte_ldrgb(&sp->part_rgb);
                    gte_ldv0(sp->normals + face->n0);
                    gte_lddp(v0->p);
                    *(u_long *)&p->u0 = face->uv0_clut;
                    gte_ncds_b();
                    *(u_long *)&p->u1 = face->uv1_tpage;
                    *(u_short *)&p->u2 = face->uv2;
                    gte_strgb(&p->r0);
                    gte_ldv0(sp->normals + face->n1);
                    gte_lddp(v1->p);
                    gte_ncds();
                    *(u_short *)&p->u3 = face->uv3;
                    gte_strgb(&p->r1);
                    gte_ldv0(sp->normals + face->n2);
                    gte_lddp(v2->p);
                    gte_ncds();
                    mesh_set_tag(sp->ot + sp->z_ot, p, 12);
                    gte_strgb(&p->r2);
                    gte_ldv0(sp->normals + face->n3);
                    gte_lddp(v3->p);
                    gte_ncds();
                    mesh_set_ot(sp->ot + sp->z_ot, p);
                    gte_strgb(&p->r3);
                    if (MESH_MODE & (MESH_TINT_A | MESH_TINT_B)) {
                        tint((CVECTOR *)&p->r0);
                        tint((CVECTOR *)&p->r1);
                        tint((CVECTOR *)&p->r2);
                        tint((CVECTOR *)&p->r3);
                    }
                    face++;
                }
            }
            sp->vertex_out = (u_long *)((u8 *)sp->vertex_out + sp->header->n_vertices * 8);
        }
    }
}

ADDRESS(0x8001f954, 0x3ac)
void world_draw_ft3_far(VECTOR *trans)
{
    ScratchPad *sp = SCRATCHPAD;
    MapBlock *rec;
    MeshFaceFT3 *face;
    MeshVertex *v0;
    MeshVertex *v1;
    MeshVertex *v2;
    POLY_FT3 *p;
    int i;
    int n;
    int t;

    i = vis_lists.n_visible;
    while (i--) {
        rec = &vis_records[vis_lists.visible[i]];
        if (rec->draw_flags & 3) {
            sp->header = (MeshHeader *)(sp->model + 12) + rec->part;
            sp->vertex_out = (u_long *)((u8 *)sp->vertex_out + sp->header->n_vertices * 8);
        } else {
            sp->turn = rec->turn & 3;
            gte_SetLightMatrix(&sp->light->light[sp->turn]);
            sp->header = (MeshHeader *)(sp->model + 12) + rec->part;
            sp->part_vertices = sp->model + 12 + sp->header->vertices;
            sp->normals = sp->model + 12 + sp->header->normals;
            face = (MeshFaceFT3 *)(sp->model + 12 + sp->header->faces);
            n = sp->header->n_ft3;
            sp->part_faces += n;
            while ((t = n, n = n - 1, t)) {
                v0 = PART_VERTEX(sp, face->i0);
                v1 = PART_VERTEX(sp, face->i1);
                v2 = PART_VERTEX(sp, face->i2);
                gte_ldsxy3(*(u_long *)&v0->sxy, *(u_long *)&v1->sxy, *(u_long *)&v2->sxy);
                gte_nclip();
                sp->part_rgb.cd = face->code | 0x24;
                gte_stopz(&sp->opz);
                if (sp->opz <= 0) {
                    face++;
                    continue;
                }
                gte_stsxy3_ft3(sp->prim);
                sp->near_z = sp->z_max = v0->z;
                if (sp->z_max < v1->z) {
                    sp->z_max = v1->z;
                }
                if (v1->z < sp->near_z) {
                    sp->near_z = v1->z;
                }
                if (sp->z_max < v2->z) {
                    sp->z_max = v2->z;
                }
                if (v2->z < sp->near_z) {
                    sp->near_z = v2->z;
                }
                sp->z_ot = (sp->z_max >> 5) + face->depth_bias;
                if (sp->z_ot >= 4096 || sp->z_ot < sp->face_count) {
                    face++;
                    continue;
                }
                p = (POLY_FT3 *)sp->prim;
                if (sp->prim_end < (u_long *)(p + 1)) {
                    return;
                }
                sp->prim = (u_long *)(p + 1);
                sp->part_prims++;
                gte_ldv0(sp->normals + face->normal);
                gte_ldrgb(&sp->part_rgb);
                *(u_long *)&p->u0 = face->uv0_clut;
                *(u_long *)&p->u1 = face->uv1_tpage;
                gte_nccs_b();
                *(u_short *)&p->u2 = face->uv2;
                mesh_set_tag(sp->ot + sp->z_ot, p, 7);
                gte_strgb(&p->r0);
                mesh_set_ot(sp->ot + sp->z_ot, p);
                face++;
            }
            sp->vertex_out = (u_long *)((u8 *)sp->vertex_out + sp->header->n_vertices * 8);
        }
    }
}

ADDRESS(0x8001fd00, 0x464)
void world_draw_ft4_far(VECTOR *trans)
{
    ScratchPad *sp = SCRATCHPAD;
    MapBlock *rec;
    MeshFaceFT4 *face;
    MeshVertex *v0;
    MeshVertex *v1;
    MeshVertex *v2;
    MeshVertex *v3;
    POLY_FT4 *p;
    int i;
    int n;
    int t;

    i = vis_lists.n_visible;
    while (i--) {
        rec = &vis_records[vis_lists.visible[i]];
        if (rec->draw_flags & 3) {
            sp->header = (MeshHeader *)(sp->model + 12) + rec->part;
            sp->vertex_out = (u_long *)((u8 *)sp->vertex_out + sp->header->n_vertices * 8);
        } else {
            sp->turn = rec->turn & 3;
            gte_SetLightMatrix(&sp->light->light[sp->turn]);
            sp->header = (MeshHeader *)(sp->model + 12) + rec->part;
            sp->part_vertices = sp->model + 12 + sp->header->vertices;
            sp->normals = sp->model + 12 + sp->header->normals;
            face = (MeshFaceFT4 *)(sp->model + 12 + sp->header->faces);
            face = (MeshFaceFT4 *)((u8 *)face + sp->header->n_ft3 * sizeof(MeshFaceFT3));
            n = sp->header->n_ft4;
            sp->part_faces += n;
            while ((t = n, n = n - 1, t)) {
                v0 = PART_VERTEX(sp, face->i0);
                v1 = PART_VERTEX(sp, face->i1);
                v2 = PART_VERTEX(sp, face->i2);
                gte_ldsxy3(*(u_long *)&v0->sxy, *(u_long *)&v1->sxy, *(u_long *)&v2->sxy);
                gte_nclip();
                sp->part_rgb.cd = face->code | 0x2c;
                gte_stopz(&sp->opz);
                v3 = PART_VERTEX(sp, face->i3);
                gte_stsxy3_ft4(sp->prim);
                gte_ldsxy0(*(u_long *)&v3->sxy);
                gte_nclip();
                gte_stopz(&sp->opz2);
                if (sp->opz <= 0 && sp->opz2 >= 0) {
                    face++;
                    continue;
                }
                gte_stsxy0(&((POLY_FT4 *)sp->prim)->x3);
                sp->near_z = sp->z_max = v0->z;
                if (sp->z_max < v1->z) {
                    sp->z_max = v1->z;
                }
                if (v1->z < sp->near_z) {
                    sp->near_z = v1->z;
                }
                if (sp->z_max < v2->z) {
                    sp->z_max = v2->z;
                }
                if (v2->z < sp->near_z) {
                    sp->near_z = v2->z;
                }
                if (sp->z_max < v3->z) {
                    sp->z_max = v3->z;
                }
                if (v3->z < sp->near_z) {
                    sp->near_z = v3->z;
                }
                sp->z_ot = (sp->z_max >> 5) + face->depth_bias;
                if (sp->z_ot >= 4096 || sp->z_ot < sp->face_count) {
                    face++;
                    continue;
                }
                p = (POLY_FT4 *)sp->prim;
                if (sp->prim_end < (u_long *)(p + 1)) {
                    return;
                }
                sp->prim = (u_long *)(p + 1);
                sp->part_prims++;
                p->clut = MESH_WORD_HI(face->uv0_clut);
                p->tpage = MESH_WORD_HI(face->uv1_tpage);
                gte_ldv0(sp->normals + face->normal);
                gte_ldrgb(&sp->part_rgb);
                *(u_long *)&p->u0 = face->uv0_clut;
                *(u_long *)&p->u1 = face->uv1_tpage;
                gte_nccs_b();
                *(u_short *)&p->u2 = face->uv2;
                *(u_short *)&p->u3 = face->uv3;
                gte_strgb(&p->r0);
                face++;
                mesh_add_prim(sp->ot + sp->z_ot, p, 9);
            }
            sp->vertex_out = (u_long *)((u8 *)sp->vertex_out + sp->header->n_vertices * 8);
        }
    }
}

ADDRESS(0x80020164, 0x424)
void world_draw_gt3_far(VECTOR *trans)
{
    ScratchPad *sp = SCRATCHPAD;
    MapBlock *rec;
    MeshFaceGT3 *face;
    MeshVertex *v0;
    MeshVertex *v1;
    MeshVertex *v2;
    POLY_GT3 *p;
    int i;
    int n;

    i = vis_lists.n_visible;
    while (i--) {
        rec = &vis_records[vis_lists.visible[i]];
        if (rec->draw_flags & 3) {
            sp->header = (MeshHeader *)(sp->model + 12) + rec->part;
            sp->vertex_out = (u_long *)((u8 *)sp->vertex_out + sp->header->n_vertices * 8);
        } else {
            sp->turn = rec->turn & 3;
            gte_SetLightMatrix(&sp->light->light[sp->turn]);
            sp->header = (MeshHeader *)(sp->model + 12) + rec->part;
            sp->part_vertices = sp->model + 12 + sp->header->vertices;
            sp->normals = sp->model + 12 + sp->header->normals;
            face = (MeshFaceGT3 *)(sp->model + 12 + sp->header->faces);
            face = (MeshFaceGT3 *)((u8 *)face + sp->header->n_ft3 * sizeof(MeshFaceFT3));
            face = (MeshFaceGT3 *)((u8 *)face + sp->header->n_ft4 * sizeof(MeshFaceFT4));
            n = sp->header->n_gt3;
            sp->faces_left = n;
            sp->part_faces += n;
            while (--sp->faces_left != -1) {
                v0 = PART_VERTEX(sp, face->i0);
                v1 = PART_VERTEX(sp, face->i1);
                v2 = PART_VERTEX(sp, face->i2);
                gte_ldsxy3(*(u_long *)&v0->sxy, *(u_long *)&v1->sxy, *(u_long *)&v2->sxy);
                gte_nclip();
                sp->part_rgb.cd = face->code | 0x34;
                gte_stopz(&sp->opz);
                if (sp->opz <= 0) {
                    face++;
                    continue;
                }
                gte_stsxy3_gt3(sp->prim);
                sp->near_z = sp->z_max = v0->z;
                if (sp->z_max < v1->z) {
                    sp->z_max = v1->z;
                }
                if (v1->z < sp->near_z) {
                    sp->near_z = v1->z;
                }
                if (sp->z_max < v2->z) {
                    sp->z_max = v2->z;
                }
                if (v2->z < sp->near_z) {
                    sp->near_z = v2->z;
                }
                sp->z_ot = (sp->z_max >> 5) + face->depth_bias;
                if (sp->z_ot >= 4096 || sp->z_ot < sp->face_count) {
                    face++;
                    continue;
                }
                p = (POLY_GT3 *)sp->prim;
                if (sp->prim_end < (u_long *)(p + 1)) {
                    return;
                }
                sp->prim = (u_long *)(p + 1);
                sp->part_prims++;
                p->clut = MESH_WORD_HI(face->uv0_clut);
                p->tpage = MESH_WORD_HI(face->uv1_tpage);
                gte_ldv3(sp->normals + face->n0, sp->normals + face->n1, sp->normals + face->n2);
                gte_ldrgb(&sp->part_rgb);
                gte_ncct();
                gte_strgb3(&p->r0, &p->r1, &p->r2);
                *(u_long *)&p->u0 = face->uv0_clut;
                *(u_long *)&p->u1 = face->uv1_tpage;
                *(u_short *)&p->u2 = face->uv2;
                face++;
                mesh_add_prim(sp->ot + sp->z_ot, p, 9);
            }
            sp->vertex_out = (u_long *)((u8 *)sp->vertex_out + sp->header->n_vertices * 8);
        }
    }
}

ADDRESS(0x80020588, 0x4f8)
void world_draw_gt4_far(VECTOR *trans)
{
    ScratchPad *sp = SCRATCHPAD;
    MapBlock *rec;
    MeshFaceGT4 *face;
    MeshVertex *v0;
    MeshVertex *v1;
    MeshVertex *v2;
    MeshVertex *v3;
    POLY_GT4 *p;
    int i;
    int n;

    i = vis_lists.n_visible;
    while (i--) {
        rec = &vis_records[vis_lists.visible[i]];
        if (rec->draw_flags & 3) {
            sp->header = (MeshHeader *)(sp->model + 12) + rec->part;
            sp->vertex_out = (u_long *)((u8 *)sp->vertex_out + sp->header->n_vertices * 8);
        } else {
            sp->turn = rec->turn & 3;
            gte_SetLightMatrix(&sp->light->light[sp->turn]);
            sp->header = (MeshHeader *)(sp->model + 12) + rec->part;
            sp->part_vertices = sp->model + 12 + sp->header->vertices;
            sp->normals = sp->model + 12 + sp->header->normals;
            face = (MeshFaceGT4 *)(sp->model + 12 + sp->header->faces);
            face = (MeshFaceGT4 *)((u8 *)face + sp->header->n_ft3 * sizeof(MeshFaceFT3));
            face = (MeshFaceGT4 *)((u8 *)face + sp->header->n_ft4 * sizeof(MeshFaceFT4));
            face = (MeshFaceGT4 *)((u8 *)face + sp->header->n_gt3 * sizeof(MeshFaceGT3));
            n = sp->header->n_gt4;
            sp->faces_left = n;
            sp->part_faces += n;
            while (--sp->faces_left != -1) {
                v0 = PART_VERTEX(sp, face->i0);
                v1 = PART_VERTEX(sp, face->i1);
                v2 = PART_VERTEX(sp, face->i2);
                gte_ldsxy3(*(u_long *)&v0->sxy, *(u_long *)&v1->sxy, *(u_long *)&v2->sxy);
                gte_nclip();
                sp->part_rgb.cd = face->code | 0x3c;
                gte_stopz(&sp->opz);
                v3 = PART_VERTEX(sp, face->i3);
                gte_stsxy3_gt4(sp->prim);
                gte_ldsxy0(*(u_long *)&v3->sxy);
                gte_nclip();
                gte_stopz(&sp->opz2);
                if (sp->opz <= 0 && sp->opz2 >= 0) {
                    face++;
                    continue;
                }
                gte_stsxy0(&((POLY_GT4 *)sp->prim)->x3);
                sp->near_z = sp->z_max = v0->z;
                if (sp->z_max < v1->z) {
                    sp->z_max = v1->z;
                }
                if (v1->z < sp->near_z) {
                    sp->near_z = v1->z;
                }
                if (sp->z_max < v2->z) {
                    sp->z_max = v2->z;
                }
                if (v2->z < sp->near_z) {
                    sp->near_z = v2->z;
                }
                if (sp->z_max < v3->z) {
                    sp->z_max = v3->z;
                }
                if (v3->z < sp->near_z) {
                    sp->near_z = v3->z;
                }
                sp->z_ot = (sp->z_max >> 5) + face->depth_bias;
                if (sp->z_ot >= 4096 || sp->z_ot < sp->face_count) {
                    face++;
                    continue;
                }
                p = (POLY_GT4 *)sp->prim;
                if (sp->prim_end < (u_long *)(p + 1)) {
                    return;
                }
                sp->prim = (u_long *)(p + 1);
                sp->part_prims++;
                p->clut = MESH_WORD_HI(face->uv0_clut);
                p->tpage = MESH_WORD_HI(face->uv1_tpage);
                gte_ldv3(sp->normals + face->n0, sp->normals + face->n1, sp->normals + face->n2);
                gte_ldrgb(&sp->part_rgb);
                gte_ncct();
                gte_strgb3(&p->r0, &p->r1, &p->r2);
                gte_ldv0(sp->normals + face->n3);
                gte_nccs();
                gte_strgb(&p->r3);
                *(u_long *)&p->u0 = face->uv0_clut;
                *(u_long *)&p->u1 = face->uv1_tpage;
                *(u_short *)&p->u2 = face->uv2;
                *(u_short *)&p->u3 = face->uv3;
                face++;
                /* 12 words, although a POLY_GT4 is 13. */
                mesh_add_prim(sp->ot + sp->z_ot, p, 12);
            }
            sp->vertex_out = (u_long *)((u8 *)sp->vertex_out + sp->header->n_vertices * 8);
        }
    }
}

/*
 * Draws the world: sets up the scratchpad for the visible records, builds
 * and culls the visible list, transforms every record's part, then runs
 * the far and near renderers and saves the counters and the packet cursor
 * back.
 *
 * The 0x90/0x92 halves (st.mesh_draw*'s prim_count) hold 240 and 320 here.
 */
ADDRESS(0x80020a80, 0x428)
void world_draw(void)
{
    ScratchPad *sp = SCRATCHPAD;
    VECTOR trans[256];

    if (world_state.draw_collision) {
        sp->model = world_state.area_geometry[1];
    } else {
        sp->model = world_state.area_geometry[0];
    }
    sp->face_count = 4;
    sp->far_z = 4096;
    sp->ot = world_state.ot_cur;
    sp->prim = world_state.packet->cur;
    sp->prim_end = world_state.packet->end;
    sp->prim_count.halves[1] = 320;
    sp->prim_count.halves[0] = 240;
    sp->part_vertex_count = world_state.vertex_face_counts[0];
    sp->part_faces = world_state.vertex_face_counts[1];
    sp->unk1B8 = world_state.prim_count;
    sp->part_prims = world_state.prim_alloc_count;
    sp->part_div_prims = world_state.div_prim_count;
    sp->unk1C4 = world_state.unk8330;
    sp->unk1CC = world_state.unk8334;
    sp->unk1D0 = world_state.unk8338[0];
    sp->unk1D4 = world_state.unk8338[1];
    sp->unk1D8 = world_state.unk8338[2];
    sp->unk1DC = world_state.unk8338[3];
    sp->unk1E0 = world_state.unk8338[4];
    MESH_TINT_LEVEL = rand();
    if (MESH_MODE & MESH_TINT_A) {
        if (MESH_MODE & MESH_TINT_SLOW) {
            sp->part_rgb.r = sp->part_rgb.g = sp->part_rgb.b = -120 - (MESH_TINT_LEVEL >> 13);
        } else {
            sp->part_rgb.r = sp->part_rgb.g = sp->part_rgb.b = -96 - (MESH_TINT_LEVEL >> 11);
        }
    } else {
        sp->part_rgb.r = sp->part_rgb.g = sp->part_rgb.b = 128;
    }
    if (MESH_MODE & MESH_TINT_A) {
        gte_SetFarColor(6, 4, 2);
    } else if (MESH_MODE & MESH_TINT_B) {
        gte_SetFarColor(2, 6, 6);
    } else {
        gte_SetFarColor(5, 5, 5);
    }
    vis_build_visible((u8 *)&viewer.position.vx);
    vis_build_planes();
    vis_cull_by_range();
    gte_SetRotMatrix(&world_state.view_matrix);
    gte_SetTransMatrix(&world_state.view_matrix);
    world_transform_parts(trans);
    sp->light = world_state.light;
    gte_SetBackColor(sp->light->back.r, sp->light->back.g, sp->light->back.b);
    gte_SetColorMatrix(sp->light->color.m);
    sp->depth_cue_near = sp->light->depth_cue_near;
    sp->depth_cue_far = sp->light->depth_cue_far;
    sp->vertex_out = (u_long *)world_state.world_vertices;
    world_project_parts(trans);
    sp->vertex_out = (u_long *)world_state.world_vertices;
    world_draw_ft3_far(trans);
    sp->vertex_out = (u_long *)world_state.world_vertices;
    world_draw_ft4_far(trans);
    sp->vertex_out = (u_long *)world_state.world_vertices;
    world_draw_gt3_far(trans);
    sp->vertex_out = (u_long *)world_state.world_vertices;
    world_draw_gt4_far(trans);
    sp->vertex_out = (u_long *)world_state.world_vertices;
    world_draw_ft3_near(trans);
    sp->vertex_out = (u_long *)world_state.world_vertices;
    world_draw_ft4_near(trans);
    sp->vertex_out = (u_long *)world_state.world_vertices;
    world_draw_gt3_near(trans);
    sp->vertex_out = (u_long *)world_state.world_vertices;
    world_draw_gt4_near(trans);
    sp->vertex_out = (u_long *)world_state.world_vertices;
    world_state.vertex_face_counts[0] = sp->part_vertex_count;
    world_state.vertex_face_counts[1] = sp->part_faces;
    world_state.prim_alloc_count = sp->part_prims;
    world_state.prim_count = sp->unk1B8;
    world_state.div_prim_count = sp->part_div_prims;
    world_state.unk8330 = sp->unk1C4;
    world_state.unk8334 = sp->unk1CC;
    world_state.unk8338[0] = sp->unk1D0;
    world_state.unk8338[1] = sp->unk1D4;
    world_state.unk8338[2] = sp->unk1D8;
    world_state.unk8338[3] = sp->unk1DC;
    world_state.unk8338[4] = sp->unk1E0;
    world_state.packet->cur = sp->prim;
    world_state.packet->end = sp->prim_end;
}

/* The semi-transparency argument that leaves a quad opaque. */
#define OPAQUE 255

/* Builds a textured quad in the current frame's work area and draws it at once. */
ADDRESS(0x80020ea8, 0x124)
void quad_draw_now(int x0, int y0, int x1, int y1, u8 u, u8 v, u8 w, u8 h,
                   u8 semi, u16 tpage, u16 clut, u8 r, u8 g, u8 b)
{
    POLY_FT4 *p;

    if (world_state.frame >= 2)
        p = (POLY_FT4 *)world_state.packet_base + 4;
    else
        p = (POLY_FT4 *)world_state.packet_base + world_state.frame * 4;
    setRGB0(p, r, g, b);
    p->tpage = tpage;
    p->clut = clut;
    setUV4(p, u, v, u + w, v, u, v + h, u + w, v + h);
    p->x0 = p->x2 = x0;
    p->x1 = p->x3 = x1;
    p->y0 = p->y1 = y0;
    p->y2 = p->y3 = y1;
    setPolyFT4(p);
    if (semi != OPAQUE)
        setSemiTrans(p, 1);
    DrawPrim(p);
}

/* Allocates a textured quad from the primitive buffer and links it to the OT. */
ADDRESS(0x80020fcc, 0x128)
void quad_add(int x0, int y0, int x1, int y1, int u, int v, int w, int h,
                   u8 semi, int tpage, int clut, int r, int g, int b)
{
    POLY_FT4 *p;

    p = (POLY_FT4 *)world_state.packet->cur;
    world_state.packet->cur = (u_long *)(p + 1);
    setRGB0(p, r, g, b);
    p->tpage = tpage;
    p->clut = clut;
    setUV4(p, u, v, u + w, v, u, v + h, u + w, v + h);
    p->x0 = p->x2 = x0;
    p->x1 = p->x3 = x1;
    p->y0 = p->y1 = y0;
    p->y2 = p->y3 = y1;
    setPolyFT4(p);
    if (semi != OPAQUE)
        setSemiTrans(p, 1);
    addPrim(world_state.ot_cur, p);
}

ADDRESS(0x800210f4, 0x64)
void loading_screen_find_file(void)
{
    char name[64];

    strcpy(name, "\\LOAD02.TIM;1");
    while (CdSearchFile(&cd_state.loading_file, name) == NULL)
        ;
}

/*
 * Shows the loading screen: reads \LOAD02.TIM;1 (up to 50 attempts) while a
 * dark translucent quad covers the display, then hands the image to
 * tim_load_images and copies the lower half of VRAM to the upper.
 */
ADDRESS(0x80021158, 0x34c)
void loading_screen_show(void)
{
    u_char result[8];
    POLY_F4 poly;
    DR_MODE mode;
    int status;
    int i;

    world_state.draw[0].isbg = 0;
    world_state.draw[1].isbg = 0;
    world_state.loading_screen_active = 1;
    world_state.draw[1].dfe = 1;
    world_state.draw[0].dfe = 1;
    world_state.draw[1].isbg = 0;
    world_state.draw[0].isbg = 0;
    /* A block: retail forms the display flags' address before it copies the
     * rectangle, so the rectangle is declared after the flag stores. */
    {
        RECT clear = {0, 0, 320, 480};
        u_long ot[1];

        PutDrawEnv(&world_state.draw[world_state.frame]);
        PutDrawEnv(&world_state.draw[1 - world_state.frame]);
        ClearOTagR(ot, 1);
        setcode(&poly, 0x28);
        setSemiTrans(&poly, 1);
        poly.tag = (ot[0] & 0xffffff) | (5 << 24);
        poly.x0 = 0;
        poly.y0 = 0;
        poly.x1 = world_state.draw[1 - world_state.frame].clip.w;
        poly.y1 = 0;
        poly.x2 = 0;
        poly.y2 = world_state.draw[1 - world_state.frame].clip.h;
        poly.x3 = world_state.draw[1 - world_state.frame].clip.w;
        mesh_set_ot(ot, &poly);
        poly.y3 = world_state.draw[1 - world_state.frame].clip.h;
        poly.r0 = poly.g0 = poly.b0 = 8;
        SetDrawMode(&mode, 0, 0, GetTPage(0, 2, 640, 0), NULL);
        addPrim(ot, &mode);
        for (i = 0; i < 50; i++) {
            CdControl(CdlSetloc, (u_char *)&cd_state.loading_file.pos, NULL);
            CdRead((cd_state.loading_file.size + 2047) >> 11, world_state.packet_base, CdlModeSpeed);
            while ((status = CdReadSync(1, result)) > 0) {
                VSync(0);
                DrawOTag(ot);
            }
            if (status == 0)
                break;
        }
        ClearImage(&clear, 0, 0, 0);
        DrawSync(0);
        gfx_setup_320x240();
        tim_load_images(world_state.packet_base, cd_state.loading_file.size);
        {
            RECT lower = {0, 240, 320, 240};

            VSync(0);
            MoveImage(&lower, 0, 0);
        }
        DrawSync(0);
        SetDispMask(1);
    }
}

ADDRESS(0x800214a4, 0x28)
void loading_screen_end(void)
{
    world_state.draw[0].isbg = 1;
    world_state.draw[1].isbg = 1;
    world_state.loading_screen_active = 0;
}

ADDRESS(0x800214cc, 0x4c)
void loading_screen_step(void)
{
    if (world_state.loading_screen_active) {
        gfx_begin_frame();
        gfx_end_frame();
    }
}

/*
 * Allocates a textured quad, when the buffer has room, and adds it to OT
 * entry `depth`. An old-style definition: the narrow parameters arrive as
 * ints that the body loads whole and stores narrow, which a prototype would
 * not produce; callers outside this module need the promoted prototype.
 */
ADDRESS(0x80021518, 0x13c)
void quad_add_at_depth(x0, y0, x1, y1, u, v, w, h, semi, tpage, clut, r, g, b, depth)
int x0, y0, x1, y1;
u8 u, v, w, h;
u8 semi;
u16 tpage, clut;
u8 r, g, b;
int depth;
{
    StPacketBuffer *buffer = world_state.packet;
    POLY_FT4 *p = (POLY_FT4 *)buffer->cur;

    if ((u_long *)(p + 1) <= buffer->end) {
        buffer->cur = (u_long *)(p + 1);
        world_state.prim_alloc_count++;
        setRGB0(p, r, g, b);
        p->tpage = tpage;
        p->clut = clut;
        setUV4(p, u, v, u + w, v, u, v + h, u + w, v + h);
        p->x0 = p->x2 = x0;
        p->x1 = p->x3 = x1;
        p->y0 = p->y1 = y0;
        p->y2 = p->y3 = y1;
        SetPolyFT4(p);
        if (semi != OPAQUE)
            SetSemiTrans(p, 1);
        AddPrim(world_state.ot_cur + depth, p);
        world_state.prim_count++;
    }
}

/*
 * Fades the screen in from black over 120 ticks after area_state.entry_fade_start is
 * set, then out.
 */
ADDRESS(0x80021654, 0x1c8)
void area_draw_entry_fade(void)
{
    int elapsed = cd_state.vsync_count - area_state.entry_fade_start;
    int early;

    if (area_state.loading == 0 && area_state.entry_fade_start == -1)
        return;
    if ((s16)area_state.load_step < 2) {
        area_state.entry_fade_start = cd_state.vsync_count;
        return;
    }
    if (elapsed < 120) {
        u8 level = elapsed * 2;

        quad_add_at_depth(0, 0, 320, 240, 0, 224, 15, 15, 1, getTPage(0, 2, 896, 256),
                      getClut(560, 496), level, level, level, 0);
        return;
    }
    early = elapsed < 240;
    if (early || area_state.loading != 0) {
        u8 level = (elapsed - 120) * 2;

        quad_add_at_depth(32, 0, 288, 256, 0, 0, 255, 255, 1, getTPage(0, 1, 960, 256),
                      getClut(560, 495), level, level, level, 0);
        if (!early)
            area_state.entry_fade_start = cd_state.vsync_count - 240;
    } else if (elapsed < 360) {
        u8 level = (360 - elapsed) * 2;

        quad_add_at_depth(32, 0, 288, 256, 0, 0, 255, 255, 1, getTPage(0, 1, 960, 256),
                      getClut(560, 495), level, level, level, 0);
    } else {
        area_state.entry_fade_start = -1;
    }
}

/* Draws and clears fade channel 0. */
ADDRESS(0x8002181c, 0xbc)
void fade_draw_additive(void)
{
    if (world_state.fade.active[0]) {
        quad_add_at_depth(0, 0, 320, 240, 0, 224, 15, 15, 1,
                      getTPage(0, world_state.fade.mode[0], 896, 256), getClut(560, 496),
                      (u8)world_state.fade.r[0], (u8)world_state.fade.g[0],
                      (u8)world_state.fade.b[0], 0);
        world_state.fade.b[0] = 0;
        world_state.fade.g[0] = 0;
        world_state.fade.r[0] = 0;
        world_state.fade.active[0] = 0;
    }
}

/* Draws and clears fade channel 1. */
ADDRESS(0x800218d8, 0xbc)
void fade_draw_subtractive(void)
{
    if (world_state.fade.active[1]) {
        quad_add_at_depth(0, 0, 320, 240, 0, 224, 15, 15, 1,
                      getTPage(0, world_state.fade.mode[1], 896, 256), getClut(560, 496),
                      (u8)world_state.fade.r[1], (u8)world_state.fade.g[1],
                      (u8)world_state.fade.b[1], 0);
        world_state.fade.b[1] = 0;
        world_state.fade.g[1] = 0;
        world_state.fade.r[1] = 0;
        world_state.fade.active[1] = 0;
    }
}

/* Adds a colour scaled by scale/4096 to the fade channel of `mode`. */
ADDRESS(0x80021994, 0x130)
void fade_add_color(int r, int g, int b, int scale, int mode)
{
    switch (mode) {
    case 1:
    case 3:
        world_state.fade.active[0]++;
        world_state.fade.mode[0] = mode;
        world_state.fade.r[0] += (r * scale) >> 12;
        world_state.fade.g[0] += (g * scale) >> 12;
        world_state.fade.b[0] += (b * scale) >> 12;
        break;
    case 2:
        world_state.fade.active[1]++;
        world_state.fade.mode[1] = mode;
        world_state.fade.r[1] += (r * scale) >> 12;
        world_state.fade.g[1] += (g * scale) >> 12;
        world_state.fade.b[1] += (b * scale) >> 12;
        break;
    }
}

/* The model matrix (rotation and translation) the part renderers draw with. */
/* A camera-space vertex (RT result). */
#define MODEL_POS(sp) ((VECTOR *)&(sp)->vertex_pos)

/* A model block's part data. */
#define MODEL_DATA(block) ((u8 *)(block) + ((ModelFile *)(block))->model)

/*
 * Projects the current part's model-space vertices (RTPS) into the
 * transformed-vertex buffer, with a depth-cue value from the lighting's
 * near/far depths (ScratchPad.depth_cue_near/unk68).
 */
#define MODEL_PROJECT(sp, out, in, n)                                          \
    while (n--) {                                                              \
        gte_ldv0(in);                                                          \
        gte_rtps();                                                            \
        gte_stsxy(&(out)->sxy);                                                \
        gte_stsz(&(out)->z);                                                   \
        if ((sp)->depth_cue_near != (sp)->depth_cue_far) {                                      \
            (out)->p = (((out)->z - (sp)->depth_cue_near) << 12) / ((sp)->depth_cue_far - (sp)->depth_cue_near); \
        } else {                                                               \
            (out)->p = 0;                                                      \
        }                                                                      \
        if ((out)->p > 4065) {                                                 \
            (out)->p = 4065;                                                   \
        }                                                                      \
        if ((out)->p < 0) {                                                    \
            (out)->p = 0;                                                      \
        }                                                                      \
        out++;                                                                 \
        in++;                                                                  \
    }

/*
 * Transforms the current part's vertices to camera space (RT) without
 * projecting them: x and y become the screen word, the caller's z the
 * depth and the camera-space z the depth-cue slot.
 */
#define MODEL_TRANSFORM(sp, depth)                                             \
    {                                                                          \
        MeshVertex *tout;                                                      \
        SVECTOR *tin;                                                          \
        int tn;                                                                \
                                                                               \
        (sp)->z_ot += (sp)->header->n_vertices;                                \
        tout = (MeshVertex *)(sp)->vertices;                                   \
        tin = (SVECTOR *)(sp)->local_vertices;                                 \
        tn = (sp)->header->n_vertices;                                         \
        while (tn--) {                                                         \
            gte_ldv0(tin);                                                     \
            gte_rt();                                                          \
            gte_stlvnl(MODEL_POS(sp));                                         \
            tin++;                                                         \
            tout->sxy.vx = MODEL_POS(sp)->vx;                              \
            tout->sxy.vy = MODEL_POS(sp)->vy;                              \
            tout->p = MODEL_POS(sp)->vz;                                   \
            tout->z = depth;                                               \
            tout++;                                                            \
        }                                                                      \
    }

/* Draws viewer model slot `slot` (skipped while its unk130 is -1). */
ADDRESS(0x80021ac4, 0x54c)
void player_draw_equipment_model(u8 slot)
{
    ScratchPad *sp;
    MATRIX light;
    SVECTOR rot;
    MeshVertex *out;
    SVECTOR *in;
    int n;

    if (viewer.action_timers.halves[slot] == -1) {
        return;
    }
    lighting_load_gte(0, 0);
    sp = SCRATCHPAD;
    gte_MulMatrix0((MATRIX *)&sp->model_light->light[3], &world_state.view_matrix, &light);
    gte_SetLightMatrix(&light);
    sp->model_t[0] = viewer.slot_model_info[slot]->pos_x;
    sp->model_t[1] = viewer.slot_model_info[slot]->pos_y;
    sp->model_t[2] = viewer.slot_model_info[slot]->pos_z;
    if (viewer.equipment[EQUIP_SLOT_LEFT_HAND] != EQUIP_NONE && slot != 0) {
        rot.vx = viewer.slot_model_info[slot]->rotation.vx + viewer.hand_rot_x;
        rot.vy = viewer.slot_model_info[slot]->rotation.vy + viewer.hand_rot_y;
        rot.vz = viewer.slot_model_info[slot]->rotation.vz + viewer.hand_rot_z;
        RotMatrix(&rot, SCRATCH_MATRIX(sp));
    } else {
        RotMatrix(&viewer.slot_model_info[slot]->rotation, SCRATCH_MATRIX(sp));
    }
    sp->far_z = 64;
    sp->near_z = 4;
    sp->ot = world_state.ot2_cur;
    sp->z_ot = world_state.vertex_face_counts[0];
    sp->face_count = world_state.vertex_face_counts[1];
    sp->prim_count.count = world_state.prim_count;
    sp->prim_alloc_count = world_state.prim_alloc_count;
    sp->prim = world_state.packet->cur;
    sp->prim_end = world_state.packet->end;
    sp->vertices = world_state.world_vertices;
    sp->clut_base = 0;
    *(u_long *)&sp->rgb = equipment_model_rgb;
    if (((ModelFile *)viewer.slot_model_files[slot])->node_count == 0) {
        gte_SetRotMatrix(SCRATCH_MATRIX(sp));
        gte_SetTransMatrix(SCRATCH_MATRIX(sp));
        sp->model = MODEL_DATA(world_state.model_files[slot + 32]);
        sp->header = (MeshHeader *)(sp->model + 12);
        if (anim_prepare((StAnimSlot **)&viewer.slot_anims[slot], slot + 32, viewer.slot_anim_ids[slot],
                          viewer.action_timers.halves[slot])) {
            sp->z_ot += sp->header->n_vertices;
            out = (MeshVertex *)sp->vertices;
            in = (SVECTOR *)sp->local_vertices;
            n = sp->header->n_vertices;
            MODEL_PROJECT(sp, out, in, n);
            mesh_draw_part_nccs(0, 0x80);
        }
    } else {
        sp->model = MODEL_DATA(world_state.model_files[slot + 32]);
        if (anim_prepare_any(&viewer.slot_anims[slot], slot + 32, viewer.slot_anim_ids[slot],
                          viewer.action_timers.halves[slot])) {
            model_draw_list(0x80, 0);
        }
    }
    world_state.packet->cur = sp->prim;
    world_state.packet->end = sp->prim_end;
    world_state.vertex_face_counts[0] = sp->z_ot;
    world_state.vertex_face_counts[1] = sp->face_count;
    world_state.prim_alloc_count = sp->prim_alloc_count;
    world_state.prim_count = sp->prim_count.count;
}

/*
 * Draws each entry of the scratchpad's model list: kind 2 turns its joint
 * matrix to face the viewer first, kinds 1, 3 and 4 draw scaled and only on
 * the entries whose index matches the frame counter (every 6th, 4th, 8th).
 */
ADDRESS(0x80022010, 0x668)
void model_draw_list(u8 flags, SVECTOR *rot)
{
    ScratchPad *sp = SCRATCHPAD;
    ModelEntry *e = sp->list.models->entries;
    int i = sp->list.models->count;
    MATRIX work;         /* unused: retail reserves its frame slot */
    MATRIX scale = {{{ONE, 0, 0}, {0, ONE, 0}, {0, 0, ONE}}, {0, 0, 0}};
    MeshVertex *out;
    SVECTOR *in;
    int n;

    while (i--) {
        e->obj->composed = 0;
        e++;
    }
    e = sp->list.models->entries;
    i = sp->list.models->count;
    while (i--) {
        e->obj->composed = 0;
        if (e->key == -1 || e->part == 0xffff) {
            e++;
            continue;
        }
        if (e->kind == 2) {
            MATRIX face = {{{ONE, 0, 0}, {0, ONE, 0}, {0, 0, ONE}}, {0, 0, 0}};
            SVECTOR angle;

            angle.vx = -viewer.pitch;
            angle.vy = -viewer.yaw;
            angle.vz = -viewer.roll;
            RotMatrixX(angle.vx, &face);
            RotMatrixY(angle.vy + rot->vy, &face);
            RotMatrixZ(angle.vz, &face);
            if (e->obj->parent->relative_set == 0) {
                RotMatrix(&e->obj->parent->transform->rot, &e->obj->parent->local_matrix);
            }
            gte_MulMatrix0(&e->obj->parent->local_matrix, &face, &e->obj->parent->local_matrix);
            e->obj->parent->relative_set = 0;
        }
        anim_record_compose(e->obj);
        if (e->kind == 1) {
            if (i % 6 != world_state.drawn_frames % 6) {
                e++;
                continue;
            }
            ScaleMatrix(&scale, &e->obj->transform->scale);
            gte_SetRotMatrix(&scale);
        } else if (e->kind == 3) {
            if (i % 4 != world_state.drawn_frames % 4) {
                e++;
                continue;
            }
            ScaleMatrix(&scale, &e->obj->transform->scale);
            gte_SetRotMatrix(&scale);
        } else if (e->kind == 4) {
            if (i % 8 != world_state.drawn_frames % 8) {
                e++;
                continue;
            }
            ScaleMatrix(&scale, &e->obj->transform->scale);
            gte_SetRotMatrix(&scale);
        }
        sp->header = (MeshHeader *)(sp->model + 12) + e->part;
        sp->local_vertices = sp->model + 12 + sp->header->vertices;
        out = (MeshVertex *)sp->vertices;
        in = (SVECTOR *)sp->local_vertices;
        n = sp->header->n_vertices;
        MODEL_PROJECT(sp, out, in, n);
        if (flags & 0x40) {
            mesh_div_draw_part(e->part, flags);
        } else if (flags & 0x80) {
            mesh_draw_part(e->part, flags);
        } else {
            mesh_draw_part_nccs(e->part, flags);
        }
        e++;
    }
}

/*
 * Draws part `part` of model block `model` at `pos` with rotation `rot`
 * and an optional scale. With a view matrix, `pos` is a world position
 * (made viewer-relative and turned by the camera) and the vertices are
 * projected; without one, `pos` is already in camera space and the
 * vertices are only transformed, at depth `z`. Flag 0x40 selects the
 * subdividing renderers (dropped when the packet buffer runs low), 0x80
 * the depth-cued ones; part numbers from 128 select a part directly.
 */
ADDRESS(0x80022678, 0xb90)
void model_draw(int unused0, u16 model, VECTOR *pos, SVECTOR *rot, SVECTOR *scale, void *anim,
                MATRIX *view, u16 part, u16 arg8, short light, short unused10, u8 flags,
                int z)
{
    ScratchPad *sp = SCRATCHPAD;
    SVECTOR rel;
    VECTOR rel_l;        /* stored but never read */
    VECTOR s;
    MATRIX work0;        /* unused, like work1: retail reserves their frame slots */
    MATRIX lm;
    MATRIX work1;
    long flag;
    MeshVertex *out;
    SVECTOR *in;
    u_int i;
    int n;

    sp->vertices = world_state.world_vertices;
    /* 0x44 holds a second vertex cursor in this phase. */
    (&sp->vertices)[1] = world_state.world_vertices;
    sp->z_ot = world_state.vertex_face_counts[0];
    sp->face_count = world_state.vertex_face_counts[1];
    sp->prim_count.count = world_state.prim_count;
    sp->prim_alloc_count = world_state.prim_alloc_count;
    sp->prim = world_state.packet->cur;
    sp->prim_end = world_state.packet->end;
    if ((u8 *)sp->prim_end - 0x7800 < (u8 *)sp->prim) {
        flags &= ~0x40;
    }
    if (sp->prim_end < sp->prim) {
        player_load_equipment_model(0xffff, 0, 0);
        player_load_equipment_model(0xffff, 1, 0);
    }
    gte_SetRotMatrix(&world_state.view_matrix);
    gte_SetTransMatrix(&world_state.view_matrix);
    if (view) {
        rel_l.vx = pos->vx - viewer.position.vx;
        rel_l.vy = pos->vy - (viewer.position.vy + viewer.bob_offset + viewer.unk2D2);
        rel_l.vz = pos->vz - viewer.position.vz;
        rel.vx = pos->vx - viewer.position.vx;
        rel.vy = pos->vy - (viewer.position.vy + viewer.bob_offset + viewer.unk2D2);
        rel.vz = pos->vz - viewer.position.vz;
        gte_RotTrans(&rel, sp->model_t, &flag);
    } else {
        sp->model_t[0] = pos->vx;
        sp->model_t[1] = pos->vy;
        sp->model_t[2] = pos->vz;
    }
    matrix_rot_yxz(rot, SCRATCH_MATRIX(sp));
    lighting_load_gte(light, flags);
    gte_MulMatrix0((MATRIX *)sp->model_light, SCRATCH_MATRIX(sp), &lm);
    gte_SetLightMatrix(&lm);
    if (scale) {
        s.vx = scale->vx;
        s.vy = scale->vy;
        s.vz = scale->vz;
        ScaleMatrix(SCRATCH_MATRIX(sp), &s);
    }
    if (view) {
        gte_MulMatrix0(view, SCRATCH_MATRIX(sp), SCRATCH_MATRIX(sp));
    }
    gte_SetRotMatrix(SCRATCH_MATRIX(sp));
    gte_SetTransMatrix(SCRATCH_MATRIX(sp));
    if (((ModelFile *)world_state.model_files[model])->node_count == 0) {
        sp->model = MODEL_DATA(world_state.model_files[model]);
        if (part < 128) {
            i = 0;
            sp->header = (MeshHeader *)(sp->model + 12);
            if (anim_prepare(anim, model, part, arg8) == 0) {
                sp->header = (MeshHeader *)(sp->model + 12);
                sp->local_vertices = sp->model + 12 + sp->header->vertices;
            }
        } else {
            i = part & 0x7f;
            sp->header = (MeshHeader *)(sp->model + 12) + i;
            sp->local_vertices = sp->model + 12 + sp->header->vertices;
        }
    } else {
        sp->model = MODEL_DATA(world_state.model_files[model]);
        if (part < 128) {
            if (anim_prepare_any(anim, model, part, arg8) == 0) {
                sp->header = (MeshHeader *)(sp->model + 12);
                sp->local_vertices = sp->model + 12 + sp->header->vertices;
            } else {
                model_draw_list(flags, rot);
            }
            goto done;
        } else {
            i = part & 0x7f;
            sp->header = (MeshHeader *)(sp->model + 12) + i;
            sp->local_vertices = sp->model + 12 + sp->header->vertices;
        }
    }
    if (flags & 0x40) {
        sp->z_ot += sp->header->n_vertices;
        out = (MeshVertex *)sp->vertices;
        in = (SVECTOR *)sp->local_vertices;
        n = sp->header->n_vertices;
        MODEL_PROJECT(sp, out, in, n);
        if (view) {
            mesh_div_draw_part(i, flags);
        } else {
            MODEL_TRANSFORM(sp, z);
            mesh_draw_part_nccs(i, flags);
        }
    } else if (flags & 0x80) {
        sp->z_ot += sp->header->n_vertices;
        out = (MeshVertex *)sp->vertices;
        in = (SVECTOR *)sp->local_vertices;
        n = sp->header->n_vertices;
        MODEL_PROJECT(sp, out, in, n);
        if (view) {
            mesh_draw_part(i, flags);
        } else {
            MODEL_TRANSFORM(sp, z);
            mesh_draw_part_nccs(i, flags);
        }
    } else if (view) {
        sp->z_ot += sp->header->n_vertices;
        out = (MeshVertex *)sp->vertices;
        in = (SVECTOR *)sp->local_vertices;
        n = sp->header->n_vertices;
        MODEL_PROJECT(sp, out, in, n);
        mesh_draw_part_nccs(i, flags);
    } else {
        MODEL_TRANSFORM(sp, z);
        mesh_draw_part_nccs(i, flags);
    }
done:
    world_state.packet->cur = sp->prim;
    world_state.packet->end = sp->prim_end;
    world_state.vertex_face_counts[0] = sp->z_ot;
    world_state.vertex_face_counts[1] = sp->face_count;
    world_state.prim_count = sp->prim_count.count;
    world_state.prim_alloc_count = sp->prim_alloc_count;
}

ADDRESS(0x80023208, 0x80)
u16 object_model_slot(u16 id)
{
    if (id < 300)
        return id + 256;
    if (area_state.loading == 1 && area_state.tables_loaded == 1 && area_state.request_area != 0xff)
        return area_state.request_area * 32 + 256 + id;
    return area_state.loaded_area * 32 + 256 + id;
}

ADDRESS(0x80023288, 0x100)
void creature_model_loaded(void *data)
{
    DVECTOR a[4] = {{0, 0}, {64, 0}, {128, 0}, {192, 0}};
    DVECTOR b[4] = {{0, 0}, {0, 3}, {0, 6}, {0, 9}};
    u8 n = heap_get_tag(data);

    model_offset_textures((Model *)((u8 *)data + ((u32 *)data)[2]), &a[n], &b[n], 0);
    heap_set_state(data, ST_BLOCK_USED);
}

ADDRESS(0x80023388, 0xe8)
void creature_tim_loaded(void *data)
{
    DVECTOR a[4] = {{0, 0}, {64, 0}, {128, 0}, {192, 0}};
    DVECTOR b[4] = {{0, 0}, {0, 3}, {0, 6}, {0, 9}};
    u8 n = heap_get_tag(data);

    tim_load_images_at(data, &a[n], &b[n], 0);
}

ADDRESS(0x80023470, 0x194)
void item_model_loaded(void *data)
{
    DVECTOR a[16] = {
        {0, 0},   {0, 32},   {0, 64},   {0, 96},   {0, 128},  {0, 160},  {0, 192},  {0, 224},
        {64, 0},  {64, 32},  {64, 64},  {64, 96},  {64, 128}, {64, 160}, {64, 192}, {64, 224},
    };
    DVECTOR b[16] = {
        {0, 0}, {0, 1}, {0, 2},  {0, 3},  {0, 4},  {0, 5},  {0, 6},  {0, 7},
        {0, 8}, {0, 9}, {0, 10}, {0, 11}, {0, 12}, {0, 13}, {0, 14}, {0, 15},
    };
    u8 n = heap_get_tag(data);

    model_offset_textures((Model *)((u8 *)data + ((u32 *)data)[2]), &a[n], &b[n], 0);
    heap_set_state(data, ST_BLOCK_USED);
}

ADDRESS(0x80023604, 0x184)
void item_tim_loaded(void *data)
{
    DVECTOR a[16] = {
        {0, 0},   {0, 32},   {0, 64},   {0, 96},   {0, 128},  {0, 160},  {0, 192},  {0, 224},
        {64, 0},  {64, 32},  {64, 64},  {64, 96},  {64, 128}, {64, 160}, {64, 192}, {64, 224},
    };
    DVECTOR b[16] = {
        {0, 0}, {0, 1}, {0, 2},  {0, 3},  {0, 4},  {0, 5},  {0, 6},  {0, 7},
        {0, 8}, {0, 9}, {0, 10}, {0, 11}, {0, 12}, {0, 13}, {0, 14}, {0, 15},
    };
    u8 n = heap_get_tag(data);

    tim_load_images_at(data, &a[n], &b[n], 0);
}

ADDRESS(0x80023788, 0x20)
void object_model_loaded(void *data)
{
    heap_set_state(data, ST_BLOCK_USED);
}

/*
 * Loads members `index` and `index + 1` of `archive` into one heap block
 * (the larger size), then, when member `index + 2` exists, frees the other
 * slots of the bank's column and loads it as a sound bank.
 *
 * A skipped bank advances from self; visited slots advance from the cursor.
 * Keep these as distinct paths so no second VAB-field cursor spans the loop.
 */
ADDRESS(0x800237a8, 0x244)
void creature_model_load(archive, index, slot, tag)
int archive;
int index;
u16 slot;
u8 tag;
{
    StVabSlot *self;
    StVabSlot *p;
    u32 second;
    u32 size;
    void *data;
    int bank;
    int i;

    second = cd_archive_member_size(archive, index + 1);
    size = cd_archive_member_size(archive, index);
    if (size < second)
        size = second;
    data = heap_alloc(model_heap, size, &world_state.model_files[slot]);
    if (data == NULL)
        return;
    heap_set_state(data, ST_BLOCK_LOADING);
    heap_set_slot(data, slot);
    heap_set_tag(data, tag);
    if (second != 0)
        cd_queue_read_member(archive, index + 1, data, creature_tim_loaded);
    cd_queue_read_member(archive, index, data, creature_model_loaded);
    if (cd_archive_member_size(archive, index + 2) == 0)
        return;
    bank = index / 5;
    self = &sound_state.slots[8 + bank];
    p = &sound_state.slots[8 + bank % 4];
    for (i = 4; i--;) {
        if (p == self) {
            p = self + 4;
            continue;
        }
        if (p->block != NULL) {
            while (p->block[2] == ST_BLOCK_LOADING)
                cd_wait_frame();
            SsVabClose(p->vab);
            p->vab = -1;
            p->block = NULL;
        }
        p += 4;
    }
    if (self->block == NULL)
        sound_bank_load(archive, index + 2, bank + 8);
    else if (self->block[2] == 1)
        self->block[2] = 2;
}

/*
 * Loads member `index + 4` of `archive` into a heap block owned by
 * world_state.model_files[slot - 16].
 */
ADDRESS(0x800239ec, 0xd0)
void creature_second_model_load(archive, index, slot, tag)
int archive;
int index;
u16 slot;
u8 tag;
{
    int member;
    u32 size;
    void *data;

    member = index + 4;
    size = cd_archive_member_size(archive, member);
    if (size == 0)
        return;
    data = heap_alloc(model_heap, size, &world_state.model_files[slot - 16]);
    if (data == NULL)
        return;
    heap_set_state(data, ST_BLOCK_LOADING);
    heap_set_slot(data, slot - 16);
    heap_set_tag(data, tag);
    cd_queue_read_member(archive, member, data, creature_model_loaded);
}

/* Loads members `index` and `index + 1` of `archive` into one heap block (the larger size). */
ADDRESS(0x80023abc, 0x108)
void item_model_load(archive, index, slot, tag)
int archive;
int index;
u16 slot;
u8 tag;
{
    u32 second;
    u32 size;
    void *data;

    second = cd_archive_member_size(archive, index + 1);
    size = cd_archive_member_size(archive, index);
    if (size < second)
        size = second;
    data = heap_alloc(model_heap, size, &world_state.model_files[slot]);
    if (data == NULL)
        return;
    heap_set_state(data, ST_BLOCK_LOADING);
    heap_set_slot(data, slot);
    heap_set_tag(data, tag);
    cd_queue_read_member(archive, index + 1, data, item_tim_loaded);
    cd_queue_read_member(archive, index, data, item_model_loaded);
}

ADDRESS(0x80023bc4, 0xa8)
void object_model_load(int archive, int index, int slot)
{
    void *data;

    data = heap_alloc(model_heap, cd_archive_member_size(archive, index), &world_state.model_files[slot]);
    if (data != NULL) {
        heap_set_state(data, ST_BLOCK_LOADING);
        heap_set_slot(data, slot);
        cd_queue_read_member(archive, index, data, object_model_loaded);
    }
}

ADDRESS(0x80023c6c, 0x26c)
void creature_models_sync(int archive, int first, int slot, int count)
{
    ScratchPad *sp = SCRATCHPAD;
    u8 *flags = sp->model_needed_cur;
    int end = count + first;
    int i;
    u16 id;
    void *a;
    void *b;
    StVabSlot *vslot;
    s16 *vab;

    for (i = first; i < end; i++, slot++) {
        id = slot;
        a = world_state.model_files[id];
        b = world_state.model_files[id - 16];
        if (*flags++) {
            if (a == NULL) {
                creature_model_load(archive, i * 5, id, sp->vram_tags[slot & 0xf]);
            } else if (heap_get_state(a) == ST_BLOCK_RELEASABLE) {
                if (heap_get_tag(a) != sp->vram_tags[slot & 0xf]) {
                    heap_free(a);
                    creature_model_load(archive, i * 5, id, sp->vram_tags[slot & 0xf]);
                } else {
                    heap_set_state(a, ST_BLOCK_USED);
                }
            }
            if (b == NULL) {
                creature_second_model_load(archive, i * 5, (u16)slot, sp->vram_tags[slot & 0xf]);
            } else if (heap_get_state(b) == ST_BLOCK_RELEASABLE) {
                if (heap_get_tag(b) != sp->vram_tags[slot & 0xf]) {
                    heap_free(b);
                    creature_second_model_load(archive, i * 5, (u16)slot, sp->vram_tags[slot & 0xf]);
                } else {
                    heap_set_state(b, ST_BLOCK_USED);
                }
            }
        } else {
            if (a != NULL && heap_get_state(a) != ST_BLOCK_LOADING)
                heap_set_state(a, ST_BLOCK_RELEASABLE);
            if (b != NULL && heap_get_state(b) != ST_BLOCK_LOADING)
                heap_set_state(b, ST_BLOCK_RELEASABLE);
            vslot = &sound_state.slots[i + 8];
            vab = vslot->block;
            if (vab != NULL && vab[2] == 1)
                vab[2] = 2;
        }
    }
}

ADDRESS(0x80023ed8, 0x12c)
void item_models_sync(int archive, int first, int slot, int count)
{
    u8 *flags = SCRATCHPAD->model_needed_cur;
    int end = count + first;
    int i;
    void *p;

    for (i = first; i < end; i++, slot++) {
        p = world_state.model_files[slot];
        if (*flags++) {
            if (p == NULL)
                item_model_load(archive, i * 2, (u16)slot, vis_table_c.ground_item_counts[slot - 440]);
            else if (heap_get_state(p) == ST_BLOCK_RELEASABLE)
                heap_set_state(p, ST_BLOCK_USED);
        } else if (p != NULL && heap_get_state(p) != ST_BLOCK_LOADING) {
            heap_set_state(p, ST_BLOCK_RELEASABLE);
        }
    }
}

ADDRESS(0x80024004, 0x1cc)
void object_models_sync(int archive, int first, int slot, int count)
{
    u8 *flags = SCRATCHPAD->model_needed_cur;
    int end = count + first;
    int i;
    void *p;
    StVabSlot *vslot;
    s16 *vab;

    for (i = first; i < end; i++, slot++) {
        p = world_state.model_files[slot];
        if (*flags++) {
            if (p == NULL)
                object_model_load(archive, i * 3, slot);
            else if (heap_get_state(p) == ST_BLOCK_RELEASABLE)
                heap_set_state(p, ST_BLOCK_USED);
            if (cd_archive_member_size(archive, i * 3 + 1)) {
                vslot = &sound_state.slots[i + 24];
                if (vslot->block == NULL) {
                    sound_bank_load(archive, i * 3 + 1, i + 24);
                } else {
                    vab = vslot->block;
                    if (vab[2] == 2)
                        vab[2] = 1;
                }
            }
        } else {
            if (p != NULL && heap_get_state(p) != ST_BLOCK_LOADING)
                heap_set_state(p, ST_BLOCK_RELEASABLE);
            vslot = &sound_state.slots[i + 24];
            if (vslot->block != NULL) {
                vab = vslot->block;
                if (vab[2] == 1)
                    vab[2] = 2;
            }
        }
    }
}

/* Table A model slots start at 128 (table B: 256, table C: 588). */
#define SLOT_A 128

ADDRESS(0x800241d0, 0x514)
void world_draw_creatures(void)
{
    ScratchPad *sp = SCRATCHPAD;
    Creature *e;
    CreatureType *t;
    VECTOR unused;
    StAnimFile *model;
    VECTOR *at;
    u8 flags;
    int depth;
    u32 bit;
    int i;

    sp->model_needed_cur = sp->model_needed;
    sp->unkF4 = sp->unk244;
    fill_words((u32 *)sp->model_needed, 0, 32);
    fill_words(sp->unk244, 0, 4);
    i = vis_table_a.count;
    sp->clut_base = 0;
    while (--i != -1) {
        e = &vis_table_a.entries[vis_table_a.list[i]];
        t = &vis_table_a.types[e->type];
        anim_get_file(e->model_slot + SLOT_A, &model);
        e->draw_state = 0;
        flags = e->status.bytes[2] & 0x3f;
        e->status.bytes[2] = flags;
        if (e->player_dist_sq <= world_state.range_mid * world_state.range_mid)
            e->status.bytes[2] = flags | 0x40;
        else if (e->player_dist_sq >= world_state.range_near * world_state.range_near)
            e->status.bytes[2] = flags | 0x80;
        if (e->status.word & 0x300000)
            depth = e->hurt_timer;
        else
            depth = e->fade;
        if (t->kind != 0xffff) {
            bit = t->kind;
            game_progress.creature_bits_and_vs_data[bit >> 5] |= 1 << (bit & 0x1f);
        }
        if (model != NULL) {
        at = &e->position;
        if (e->flags & 0x20) {
            sp->model_rot.vx = sp->model_rot.vy = sp->model_rot.vz = 0;
            sp->ot = world_state.ot_cur + e->depth_bias;
            sp->near_z = 4 - e->depth_bias;
            sp->far_z = 4096 - e->depth_bias;
            model_draw(0, e->model_slot + SLOT_A, at, &sp->model_rot, &e->scale, &e->anim_slot,
                          &identity_matrix, e->anim, e->anim_frame, 4096 - depth, e->unk24,
                          e->status.bytes[2], 0);
        } else {
            VECTOR pos;
            SVECTOR scale = {4096, 4096, 4096, 0};

            e->draw_state = 1;
            sp->model_rot.vx = e->rot_x;
            sp->model_rot.vy = e->facing;
            sp->model_rot.vz = e->rot_z;
            if (e->visibility != 3) {
                sp->ot = world_state.ot_cur + e->depth_bias;
                sp->near_z = 4 - e->depth_bias;
                sp->far_z = 4096 - e->depth_bias;
                model_draw(0, e->model_slot + SLOT_A, at, &sp->model_rot, &e->scale, &e->anim_slot,
                              &world_state.view_matrix, e->anim, e->anim_frame, 4096 - depth, e->unk24,
                              e->status.bytes[2], 0);
            }
            if (e->shadow_size != 0) {
                pos = e->position;
                scale.vx = e->shadow_size * 2 + 1024;
                scale.vz = e->shadow_size * 2 + 1024;
                sp->ot = world_state.ot_cur + e->depth_bias - 4;
                sp->near_z = 8 - e->depth_bias;
                sp->far_z = 4100 - e->depth_bias;
                model_draw(0, 44, &pos, &sp->model_rot, &scale, NULL, &world_state.view_matrix, 128, 0,
                              0, 0, 4, 0);
            }
        }
        } else {
            e->draw_state = 2;
        }
        if (e->voice != -1) {
            SpuGetAllKeysStatus(sound_state.keys);
            if (sound_state.keys[e->voice] == 0)
                e->voice = -1;
        }
        sp->model_needed[e->model_slot] = 1;
        sp->vram_tags[e->model_slot] = e->vram_tag;
    }
    if (area_state.skip_model_sync == 0)
        creature_models_sync(ARCHIVE_CHR, 0, 128, 16);
}

/* Steps the visible table B entries: the two kinds of area sound (over a
 * floor rectangle, around a point) and the models with their shadows. */
ADDRESS(0x800246e4, 0xae4)
void world_draw_objects(void)
{
    ScratchPad *sp = SCRATCHPAD;
    AreaObject *e;
    AreaObjectType *r;
    VECTOR unused;
    StAnimFile *model;
    SVECTOR *size;
    u32 now;
    u8 flags;
    u16 id;
    int volume, range, d, dz;
    int i;

    sp->model_needed_cur = sp->model_needed;
    sp->unkF4 = sp->unk244;
    fill_words((u32 *)sp->model_needed, 0, 83);
    fill_words(sp->unk244, 0, 75);
    sp->clut_base = 0;
    i = vis_table_b.count;
    now = cd_state.vsync_count;
    while (--i != -1) {
        e = &vis_table_b.entries[vis_table_b.list[i]];
        e->drawn = 0;
        if (e->visibility == 2)
            continue;
        flags = e->draw_flags & 0x3f;
        e->draw_flags = flags;
        if (e->player_dist_sq <= world_state.range_mid * world_state.range_mid)
            e->draw_flags = flags | 0x40;
        else if (e->player_dist_sq >= world_state.range_near * world_state.range_near)
            e->draw_flags = flags | 0x80;
        switch (e->kind) {
        case 37:
        case 226:
        case 229:
        case 233:
            continue;
        case 31:
            /* A sound over a rectangle of the floor. */
            if (player_in_cell_rect(e->position.vx >> 12, e->position.vz >> 12, e->unk48, e->unk49, 0x8000)) {
                if ((s32)(e->unk50 - now) >= 0)
                    continue;
                if (e->params.snd.sound_flags & 2)
                    e->unk50 = now + (e->params.snd.period * 6 + (rand() >> 8));
                if (e->params.snd.sound_flags & 4)
                    e->unk50 = now + (e->params.snd.period * 6 + (rand() >> 6));
                else
                    e->unk50 = now + e->params.snd.period * 6;
                range = e->unk48 << 11;
                d = viewer.position.vx - (e->position.vx + range);
                if (d < 0)
                    d = -d;
                d = range - d;
                range = e->unk49 << 11;
                dz = viewer.position.vz - (e->position.vz + range);
                if (dz < 0)
                    dz = -dz;
                dz = range - dz;
                if (dz < d)
                    d = dz;
                volume = e->params.snd.fade_range << 12;
                if (d >= volume) {
                    volume = e->params.snd.volume;
                } else {
                    if (volume == 0)
                        continue;
                    volume = e->params.snd.volume * d / volume;
                }
                if (e->params.snd.sound_flags & 1) {
                    int dy = viewer.position.vy - e->position.vy;

                    if (dy < 0)
                        dy = -dy;
                    dy = e->params.snd.volume * dy >> 13;
                    volume -= dy;
                }
                if (volume < 20)
                    continue;
                if (e->unk54 != -1) {
                    sound_stop(e->unk54, e->params.snd.sound + 208);
                    e->unk54 = -1;
                }
                e->unk54 = sound_play(e->params.snd.sound + 208, volume);
            } else {
                e->unk50 = now + e->params.snd.period * 6;
                if (e->unk54 == -1)
                    continue;
                sound_stop(e->unk54, e->params.snd.sound + 208);
                e->unk54 = -1;
            }
            break;
        case 236:
            /* A sound around a point. */
            if (player_in_centred_rect(e->position.vx, e->position.vz, e->unk48 << 8, e->unk49 << 8, 0x8000)) {
                if ((s32)(e->unk50 - now) >= 0)
                    continue;
                if (e->params.snd.sound_flags & 2)
                    e->unk50 = now + (e->params.snd.period * 60 + (rand() >> 11));
                if (e->params.snd.sound_flags & 4)
                    e->unk50 = now + (e->params.snd.period * 60 + (rand() >> 9));
                else
                    e->unk50 = now + e->params.snd.period * 60;
                {
                    int inner = (e->unk48 << 7) - (e->params.snd.fade_range << 7);

                    range = e->unk48 << 7;
                    if (e->player_dist_xz >= inner && range != inner)
                        volume = e->params.snd.volume * (range - e->player_dist_xz) / (range - inner);
                    else
                        volume = e->params.snd.volume;
                }
                if (e->params.snd.sound_flags & 1) {
                    int dy = viewer.position.vy - e->position.vy;

                    if (dy < 0)
                        dy = -dy;
                    dy = e->params.snd.volume * dy >> 13;
                    volume -= dy;
                }
                if (volume < 20)
                    continue;
                if (e->unk54 != -1) {
                    sound_stop(e->unk54, e->params.snd.sound + 1108);
                    e->unk54 = -1;
                }
                e->unk54 = sound_play(e->params.snd.sound + 1108, volume);
            } else {
                e->unk50 = now + e->params.snd.period * 60;
                if (e->unk54 == -1)
                    continue;
                sound_stop(e->unk54, e->params.snd.sound + 1108);
                e->unk54 = -1;
            }
            break;
        case 38:
            sp->model_needed[e->type] = 1;
            size = &e->scale;
            id = object_model_slot(e->type);
            anim_get_file(id, &model);
            if (model == NULL)
                continue;
            sp->model_rot.vx = e->rot_x;
            sp->model_rot.vy = e->facing;
            sp->model_rot.vz = e->rot_z;
            sp->ot = world_state.ot_cur + e->depth_bias;
            sp->near_z = 4 - e->depth_bias;
            sp->far_z = 4096 - e->depth_bias;
            model_draw(0, id, &e->position, &sp->model_rot, size, e->anim_slot,
                          &world_state.view_matrix, e->anim, e->anim_frame, 0, e->light, e->draw_flags, 0);
            break;
        default:
            r = &vis_table_b.types[e->type];
            sp->model_needed[e->type] = 1;
            id = object_model_slot(e->type);
            anim_get_file(id, &model);
            size = &e->scale;
            if (model == NULL)
                continue;
            sp->model_rot.vx = e->rot_x;
            sp->model_rot.vy = e->facing;
            sp->model_rot.vz = e->rot_z;
            if (e->flags & 0x40) {
                sp->model_rot.vx = e->rot_x;
                sp->model_rot.vy = 0;
                sp->model_rot.vz = e->rot_z;
                sp->ot = world_state.ot_cur + e->depth_bias;
                sp->near_z = 4 - e->depth_bias;
                sp->far_z = 4096 - e->depth_bias;
                model_draw(0, id, &e->position, &sp->model_rot, size, e->anim_slot,
                              &world_state.pitch_matrix, e->anim, e->anim_frame, e->light, e->light,
                              e->draw_flags, 0);
                break;
            }
            {
                VECTOR pos;
                SVECTOR scale = {4096, 4096, 4096, 0};

                e->drawn = 1;
                sp->ot = world_state.ot_cur + e->depth_bias;
                sp->near_z = 4 - e->depth_bias;
                sp->far_z = 4096 - e->depth_bias;
                model_draw(0, id, &e->position, &sp->model_rot, size, e->anim_slot,
                              &world_state.view_matrix, e->anim, e->anim_frame, e->light, e->light, e->draw_flags, 0);
                if (e->flags & 1)
                    continue;
                pos = e->position;
                if (e->flags & 4) {
                    scale.vx = r->shadow_x * 2 + (r->shadow_x >> 1);
                    scale.vz = r->shadow_z * 2 + (r->shadow_z >> 1);
                } else {
                    scale.vx = r->radius * 2 + (r->radius >> 1);
                    scale.vz = r->radius * 2 + (r->radius >> 1);
                }
                sp->ot = world_state.ot_cur + e->depth_bias - 5;
                sp->near_z = 9 - e->depth_bias;
                sp->far_z = 4101 - e->depth_bias;
                model_draw(0, (r->flags >> 2 & 1) | 44, &pos, &sp->model_rot, &scale, e->anim_slot,
                              &world_state.view_matrix, 128, 0, e->light, 4096, 4, 0);
            }
            break;
        }
    }
    if (area_state.skip_model_sync == 0)
        object_models_sync(ARCHIVE_MO, 0, 256, 249);
    sp->model_needed_cur = &sp->model_needed[300];
}

#define SLOT_C 588

ADDRESS(0x800251c8, 0x428)
void world_draw_ground_items(void)
{
    ScratchPad *sp = SCRATCHPAD;
    GroundItem *e;
    ItemType *r;
    VECTOR unused;
    StAnimFile *model;
    int i;

    sp->model_needed_cur = sp->model_needed;
    sp->unkF4 = sp->unk244;
    fill_words((u32 *)sp->model_needed, 0, 32);
    i = vis_table_c.count;
    sp->clut_base = 0;
    while (--i != -1) {
        e = &vis_table_c.entries[vis_table_c.list[i]];
        r = &vis_table_c.types[e->item];
        if (e->player_dist_xz >= (s32)world_state.range_near)
            e->draw_flags |= 0x80;
        switch (e->state) {
        case 0:
        case 4:
            continue;
        case 1:
        case 2:
        case 3:
        case 5:
            break;
        }
        {
            SVECTOR scale = {4096, 4096, 4096};

            sp->model_needed[r->model] = 1;
            anim_get_file(r->model + SLOT_C, &model);
            if (model == NULL)
                continue;
            sp->model_rot.vx = e->rot_x;
            sp->model_rot.vy = e->facing;
            sp->model_rot.vz = e->rot_z;
            if (r->flags & 0x40) {
                MATRIX m;

                sp->ot = world_state.ot_cur + e->depth_bias;
                sp->near_z = 4 - e->depth_bias;
                sp->far_z = 4096 - e->depth_bias;
                model_draw(0, r->model + SLOT_C, &e->position, &sp->model_rot, &scale, e->anim_slot,
                              &world_state.view_matrix, 128, 0, 0, 4096, e->draw_flags, 0);
                sp->model_rot.vx = sp->model_rot.vy = sp->model_rot.vz = 0;
                sp->ot = world_state.ot_cur + e->depth_bias;
                sp->near_z = 4 - e->depth_bias;
                sp->far_z = 4096 - e->depth_bias;
                m = world_state.pitch_matrix;
                if (e->state != 2)
                    RotMatrixX(e->rot_x, &m);
                model_draw(0, r->model + SLOT_C, &e->position, &sp->model_rot, &scale, e->anim_slot, &m, 129,
                              0, 0, 4096, e->draw_flags, 0);
            } else {
                sp->ot = world_state.ot_cur + e->depth_bias;
                sp->near_z = 4 - e->depth_bias;
                sp->far_z = 4096 - e->depth_bias;
                model_draw(0, r->model + SLOT_C, &e->position, &sp->model_rot, &scale, e->anim_slot,
                              &world_state.view_matrix, 128, 0, 0, 4096, e->draw_flags, 0);
            }
        }
    }
    if (area_state.skip_model_sync == 0)
        item_models_sync(ARCHIVE_ITEM, 0, SLOT_C, 128);
    sp->model_needed_cur = &sp->model_needed[128];
}

ADDRESS(0x800255f0, 0x634)
void world_draw_effects_placements(void)
{
    ScratchPad *sp = SCRATCHPAD;
    StEffect *o;
    PlacedObject *f;
    StAnimFile *model;
    VECTOR *at;
    int mode;
    int kind;
    int i;

    for (o = effects.slots, i = 127; i != -1; o++, i--) {
        if (o->flags == 0xff)
            continue;
        mode = o->draw_mode & 3;
        if (mode == 0)
            continue;
        *(VECTOR *)&sp->point_x = o->position;
        at = &o->position;
        if (mode != 2 && !view_point_in_near_cone())
            continue;
        sp->clut_base = o->clut_base;
        anim_get_file(o->model + 96, &model);
        if (model == NULL)
            continue;
        switch (o->draw_mode & 0xc) {
        case 0:
            sp->model_rot.vx = o->rotation.vx;
            sp->model_rot.vy = o->rotation.vy;
            sp->model_rot.vz = o->rotation.vz;
            sp->ot = world_state.ot_cur - 7;
            sp->near_z = 11;
            sp->far_z = 4103;
            model_draw(0, o->model + 96, at, &sp->model_rot, &o->scale, o->anim_slot,
                          &world_state.view_matrix, o->part, o->anim_frame, o->light, o->light, o->draw_flags, 0);
            break;
        case 4:
            sp->near_z = 11;
            sp->far_z = 4103;
            sp->ot = world_state.ot_cur - 7;
            model_draw(0, o->model + 96, at, &o->rotation, &o->scale, o->anim_slot,
                          &identity_matrix, o->part, o->anim_frame, o->light, o->light, o->draw_flags, 0);
            break;
        case 8:
            sp->near_z = 11;
            sp->far_z = 4103;
            sp->ot = world_state.ot_cur - 7;
            model_draw(0, o->model + 96, at, &o->rotation, &o->scale, o->anim_slot,
                          &world_state.pitch_matrix, o->part, o->anim_frame, o->light, o->light, o->draw_flags, 0);
            break;
        case 12:
            sp->near_z = -16;
            sp->far_z = 4076;
            sp->ot = world_state.ot_cur + 20;
            model_draw(0, o->model + 96, at, &o->rotation, &o->scale, o->anim_slot,
                          NULL, o->part, o->anim_frame, o->light, o->light, o->draw_flags, 0);
            break;
        }
    }
    sp->model_rot.vx = sp->model_rot.vy = sp->model_rot.vz = 0;
    sp->clut_base = 0;
    for (f = placement_objects.objects, i = 127; i != -1; f++, i--) {
        if (f->model == 0xffff)
            continue;
        if (f->model & 0x8000) {
            sp->ot = world_state.ot_cur;
            sp->near_z = 4;
            sp->far_z = 4096;
        } else {
            *(VECTOR *)&sp->point_x = f->position;
            at = &f->position;
            if (view_point_in_near_cone()) {
                kind = f->kind & 0x3f;
                if (kind == 1) {
                    sp->model_rot.vy = (f->kind & 0xc0) << 4;
                    sp->near_z = 24;
                    sp->far_z = 4116;
                    sp->ot = world_state.ot_cur - 20;
                    model_draw(3, f->model + 40, at, &sp->model_rot, NULL, NULL,
                                  &world_state.view_matrix, f->frame + 128, 0, 0, 4096, 7, 0);
                    sp->model_rot.vy = 0;
                    model_draw(3, f->model + 40, at, &sp->model_rot, NULL, NULL,
                                  &world_state.pitch_matrix, f->frame + 128, 0, 0, 4096, 7, 0);
                } else if (kind == 2) {
                    sp->near_z = 24;
                    sp->far_z = 4116;
                    sp->ot = world_state.ot_cur - 20;
                    model_draw(3, f->model + 40, at, &sp->model_rot, NULL, NULL,
                                  &world_state.pitch_matrix, f->frame + 128, 0, 0, 4096, 7, 0);
                } else if (kind == 3) {
                    sp->near_z = 24;
                    sp->far_z = 4116;
                    sp->ot = world_state.ot_cur - 20;
                    model_draw(3, f->model + 40, at, &sp->model_rot, NULL, NULL,
                                  &world_state.view_matrix, f->frame + 128, 0, 0, 4096, 7, 0);
                } else {
                    sp->near_z = 4;
                    sp->far_z = 4096;
                    sp->ot = world_state.ot_cur;
                    model_draw(3, f->model + 40, at, &sp->model_rot, NULL, NULL,
                                  &world_state.pitch_matrix, f->frame + 128, 0, 0, 4096, 5, 0);
                }
            }
        }
        if (f->frame_period != 0 && placement_objects.frame % f->frame_period == 0) {
            if (++f->frame >= f->frame_count)
                f->frame = 0;
        }
    }
    placement_objects.frame++;
}

#define HUD_END 0xff
/* A record whose width is HUD_SPRITE_SIZE is drawn at its texture size. */
#define HUD_SPRITE_SIZE 0xff
#define HUD_OPAQUE 0xff

/*
 * Takes the next primitive from the current packet buffer, or returns.
 * A statement sequence, not a do-while block.
 */
#define NEW_PRIM(buf, p, type)            \
    (buf) = world_state.packet;           \
    (p) = (type *)(buf)->cur;             \
    if ((buf)->end < (u_long *)((p) + 1)) \
        return;                           \
    (buf)->cur = (u_long *)((p) + 1);     \
    world_state.prim_alloc_count++

/* A sprite at its texture size, behind its draw mode. */
ADDRESS(0x80025c24, 0x1dc)
void hud_draw_sprite(rec, abr, color)
StHudSprite *rec;
u16 abr;
u8 *color;
{
    StPacketBuffer *buf;
    DR_MODE *mode;
    SPRT *p;

    NEW_PRIM(buf, mode, DR_MODE);
    NEW_PRIM(buf, p, SPRT);
    p->x0 = rec->r.x;
    p->y0 = rec->r.y;
    p->clut = rec->clut;
    p->u0 = rec->u;
    p->v0 = rec->v;
    p->w = rec->uw;
    p->h = rec->vh;
    p->r0 = color[0];
    p->g0 = color[1];
    p->b0 = color[2];
    setSprt(p);
    if (abr != HUD_OPAQUE) {
        SetDrawMode(mode, 0, 1, rec->tpage | (abr & 3) << 5, NULL);
        setSemiTrans(p, 1);
    } else {
        SetDrawMode(mode, 0, 1, rec->tpage, NULL);
        setSemiTrans(p, 0);
    }
    AddPrim(world_state.ot_cur + rec->otz, p);
    AddPrim(world_state.ot_cur + rec->otz, mode);
    world_state.prim_count += 2;
}

/* A sprite scaled to its rectangle. */
ADDRESS(0x80025e00, 0x220)
void hud_draw_sprite_scaled(rec, abr, color)
StHudSprite *rec;
u16 abr;
u8 *color;
{
    StPacketBuffer *buf;
    POLY_FT4 *p;

    NEW_PRIM(buf, p, POLY_FT4);
    p->x0 = rec->r.x;
    p->y0 = rec->r.y;
    p->x1 = rec->r.x + rec->r.w;
    p->y1 = rec->r.y;
    p->x2 = rec->r.x;
    p->y2 = rec->r.y + rec->r.h;
    p->x3 = rec->r.x + rec->r.w;
    p->y3 = rec->r.y + rec->r.h;
    p->clut = rec->clut;
    p->u0 = rec->u;
    p->v0 = rec->v;
    p->u1 = rec->u + rec->uw;
    p->v1 = rec->v;
    p->u2 = rec->u;
    p->v2 = rec->v + rec->vh;
    p->u3 = rec->u + rec->uw;
    p->v3 = rec->v + rec->vh;
    p->r0 = color[0];
    p->g0 = color[1];
    p->b0 = color[2];
    setPolyFT4(p);
    if (abr != HUD_OPAQUE) {
        p->tpage = rec->tpage | (abr & 3) << 5;
        setSemiTrans(p, 1);
    } else {
        p->tpage = rec->tpage;
        setSemiTrans(p, 0);
    }
    AddPrim(world_state.ot_cur + rec->otz, p);
    world_state.prim_count++;
}

/* An untextured rectangle, behind its draw mode. */
ADDRESS(0x80026020, 0x208)
void hud_draw_rect(rec, abr, color)
StHudSprite *rec;
u16 abr;
u8 *color;
{
    StPacketBuffer *buf;
    DR_MODE *mode;
    POLY_F4 *p;

    NEW_PRIM(buf, mode, DR_MODE);
    NEW_PRIM(buf, p, POLY_F4);
    p->x0 = rec->r.x;
    p->y0 = rec->r.y;
    p->x1 = rec->r.x + rec->r.w;
    p->y1 = rec->r.y;
    p->x2 = rec->r.x;
    p->y2 = rec->r.y + rec->r.h;
    p->x3 = rec->r.x + rec->r.w;
    p->y3 = rec->r.y + rec->r.h;
    p->r0 = color[0];
    p->g0 = color[1];
    p->b0 = color[2];
    setPolyF4(p);
    if (abr != HUD_OPAQUE) {
        SetDrawMode(mode, 0, 1, rec->tpage | (abr & 3) << 5, NULL);
        setSemiTrans(p, 1);
    } else {
        SetDrawMode(mode, 0, 1, rec->tpage, NULL);
        setSemiTrans(p, 0);
    }
    AddPrim(world_state.ot_cur + rec->otz, p);
    AddPrim(world_state.ot_cur + rec->otz, mode);
    world_state.prim_count += 2;
}

/*
 * Shows `value` (at most 9999) in the four digit records from `first`:
 * leading zeros are hidden and each digit picks its glyph `width` texels
 * apart from u = 96.
 */
ADDRESS(0x80026228, 0x2e0)
void hud_set_number(int value, int first, u8 width)
{
    short n;
    short d;
    int third;

    n = 9999;
    if (value < 10000)
        n = value;
    if (n < 10) {
        hud_sprites[first].flags = 0;
        hud_sprites[first + 1].flags = 0;
        hud_sprites[first + 2].flags = 0;
        hud_sprites[first + 3].flags |= 1;
    } else if (n < 100) {
        hud_sprites[first].flags = 0;
        hud_sprites[first + 1].flags = 0;
        hud_sprites[first + 2].flags |= 1;
        hud_sprites[first + 3].flags |= 1;
    } else if (n < 1000) {
        hud_sprites[first].flags = 0;
        hud_sprites[first + 1].flags |= 1;
        hud_sprites[first + 2].flags |= 1;
        hud_sprites[first + 3].flags |= 1;
    } else {
        hud_sprites[first].flags |= 1;
        hud_sprites[first + 1].flags |= 1;
        hud_sprites[first + 2].flags |= 1;
        hud_sprites[first + 3].flags |= 1;
    }
    d = n / 1000;
    n -= d * 1000;
    hud_sprites[first].u = d * width + 96;
    d = n / 100;
    n -= d * 100;
    hud_sprites[first + 1].u = d * width + 96;
    d = n / 10;
    third = first + 2;
    n -= d * 10;
    hud_sprites[third].u = d * width + 96;
    hud_sprites[first + 3].u = n * width + 96;
}

ADDRESS(0x80026508, 0x45c)
void hud_update(void)
{
    if (viewer.hp != viewer.hud_hp || viewer.mp != viewer.hud_mp) {
        viewer.hud_redraw = 2;
        viewer.hud_hp = viewer.hp;
        viewer.hud_mp = viewer.mp;
    }
    if (viewer.hud_redraw != 0) {
        if (viewer.hud_redraw == 2) {
            hud_sprites[23].flags = HUD_END;
            hud_sprites[0].flags |= 1;
            hud_sprites[1].flags |= 1;
            hud_sprites[2].flags |= 2;
            hud_sprites[3].flags |= 2;
            hud_sprites[13].flags |= 1;
            hud_set_number(viewer.hp, 14, 16);
            hud_set_number(viewer.mp, 19, 16);
            viewer.hud_redraw = 0;
        }
    }
    {
        Viewer *v = &viewer;

        if (v->hud_redraw == 0) {
            hud_sprites[0].r.w = v->weapon_gauge * 112 / 5120;
            hud_sprites[1].r.w = v->magic_gauge * 112 / 5120;
            hud_sprites[1].r.x = 307 - hud_sprites[1].r.w;
        }
        if (v->status_flags & STATUS_POISON) {
            hud_sprites[4].flags = 1;
            if (v->status_flags & STATUS_POISON_3)
                hud_sprites[4].flags = 9;
            else if (v->status_flags & STATUS_POISON_2)
                hud_sprites[4].flags = 5;
        } else {
            hud_sprites[4].flags = 0;
        }
    }
    if (viewer.status_flags & STATUS_PARALYZE) {
        hud_sprites[7].flags = 1;
        if (viewer.status_flags & STATUS_PARALYZE_2)
            hud_sprites[7].flags = 9;
    } else {
        hud_sprites[7].flags = 0;
    }
    if (viewer.status_flags & STATUS_DARK) {
        hud_sprites[9].flags = 1;
        if (viewer.status_flags & 0x40)
            hud_sprites[9].flags = 9;
    } else {
        hud_sprites[9].flags = 0;
    }
    if (viewer.status_flags & STATUS_SLOW)
        hud_sprites[11].flags |= 1;
    else
        hud_sprites[11].flags = 0;
    if (viewer.status_flags & STATUS_CURSED)
        hud_sprites[12].flags |= 1;
    else
        hud_sprites[12].flags = 0;
    if (*(s32 *)viewer.unk190[5] == 0)
        hud_sprites[37].flags = 0;
    else
        hud_sprites[37].flags |= 1;
    if (*(s32 *)viewer.unk190[12] == 0)
        hud_sprites[38].flags = 0;
    else
        hud_sprites[38].flags |= 1;
    if (*(s32 *)viewer.unk190[7] == 0)
        hud_sprites[39].flags = 0;
    else
        hud_sprites[39].flags |= 1;
    if (*(s32 *)viewer.unk190[8] == 0)
        hud_sprites[40].flags = 0;
    else
        hud_sprites[40].flags |= 1;
    if (*(s32 *)viewer.unk190[9] == 0)
        hud_sprites[41].flags = 0;
    else
        hud_sprites[41].flags |= 1;
    if (*(s32 *)viewer.unk190[10] == 0)
        hud_sprites[42].flags = 0;
    else
        hud_sprites[42].flags |= 1;
    if (*(s32 *)viewer.unk190[11] == 0)
        hud_sprites[43].flags = 0;
    else
        hud_sprites[43].flags |= 1;
    if (viewer.status_flags & STATUS_DURABILITY_WARNING)
        hud_sprites[44].flags |= 1;
    else
        hud_sprites[44].flags = 0;
}

/* Draws the HUD records from `first` up to the next end marker. */
ADDRESS(0x80026964, 0x124)
void hud_draw_records(u8 first)
{
    CVECTOR color = {96, 96, 96, 0};
    StHudSprite *rec;

    for (rec = &hud_sprites[first]; rec->flags != HUD_END; rec++) {
        if (rec->flags & 1) {
            if (rec->flags & 4) {
                color.b = 0;
            } else if (rec->flags & 8) {
                color.g = 0;
                color.b = 0;
            } else {
                color.g = 96;
                color.b = 96;
            }
            if ((u16)rec->r.w != HUD_SPRITE_SIZE)
                hud_draw_sprite_scaled(rec, rec->abr, &color);
            else
                hud_draw_sprite(rec, rec->abr, &color);
        } else if (rec->flags & 2) {
            hud_draw_rect(rec, rec->abr, &color);
        }
    }
}

/* Queues `value` in a free one of the five HUD number slots for 60 frames. */
ADDRESS(0x80026a88, 0x74)
void hud_queue_damage_number(short value)
{
    int i;

    if (value != -1 && game_settings.show_status != 0) {
        for (i = 4; i != -1; i--) {
            if (viewer.damage_number_timers[i] == 0) {
                viewer.damage_number_timers[i] = 60;
                viewer.damage_numbers[i] = value;
                break;
            }
        }
    }
}

/* Blink phase of world_render_frame's selection frame (0..31). */
DATA(0x80087f18, 0x4, ".sdata")
static s32 hud_selection_blink = 0;

ADDRESS(0x80026afc, 0x2d8)
void world_render_frame(VECTOR *pos, SVECTOR *rot)
{
    RECT unused;
    CVECTOR color;
    StHudSprite *frame;
    int level;
    int i;

    pad_update_vibration();
    gfx_set_camera(pos, rot);
    texanim_update();
    if (viewer.state == VIEWER_STATE_NORMAL || viewer.state == VIEWER_STATE_SCRIPTED
        || viewer.state == VIEWER_STATE_SHOP || viewer.state == VIEWER_STATE_QUICK_ITEM)
        lighting_update();
    gfx_begin_frame();
    anim_slots_retire();
    anim_sets_retire();
    world_state.vertex_face_counts[1] = 0;
    if (viewer.state == VIEWER_STATE_QUICK_ITEM) {
        frame = &hud_sprites[26 + viewer.quick_item_index];
        hud_selection_blink = (hud_selection_blink + 1) % 32;
        if (hud_selection_blink < 16)
            level = 8 - (u16)hud_selection_blink;
        else
            level = (u16)hud_selection_blink - 24;
        color.b = color.g = color.r = level * 4 + 96;
        ui_draw_rect_outline(&frame->r, &color, 257, 1);
        hud_draw_records(26);
    }
    if (game_settings.show_status != 0 && area_state.area < 44) {
        hud_update();
        hud_draw_records(0);
        for (i = 4; i != -1; i--) {
            if (viewer.damage_number_timers[i] != 0) {
                hud_sprites[35].r.y = i * 16 + 144;
                hud_sprites[34].r.y = i * 16 + 144;
                hud_sprites[33].r.y = i * 16 + 144;
                hud_sprites[32].r.y = i * 16 + 144;
                hud_set_number(viewer.damage_numbers[i], 32, 8);
                hud_draw_records(32);
                viewer.damage_number_timers[i]--;
            }
        }
        hud_draw_records(37);
    }
    fade_draw_additive();
    fade_draw_subtractive();
    area_draw_entry_fade();
    player_draw_equipment_model(0);
    player_draw_equipment_model(1);
    world_draw();
    vis_collect_creatures();
    vis_collect_objects();
    vis_collect_ground_items();
    world_draw_creatures();
    world_draw_objects();
    world_draw_ground_items();
    world_draw_effects_placements();
    if (world_state.talk_overlay_state != 0)
        talk_draw_overlay();
    gfx_end_frame();
    vsync_wait_tick();
    world_state.drawn_frames++;
}

/* One frame of the world and the player's status; no value is returned
 * (see the prototype). */
ADDRESS(0x80026dd4, 0x30)
int world_run_frame(VECTOR *pos, SVECTOR *rot, int arg)
{
    world_render_frame(pos, rot);
    player_update_status(arg);
}

/*
 * Moves the second packet buffer up to make room below it and stores the
 * current drawing area there (the screen behind a menu); both drawing
 * environments then clear their background.
 */
ADDRESS(0x80026e04, 0x8c)
void gfx_store_screen(void)
{
    DrawSync(0);
    world_state.packets[1].start = world_state.packets[0].end = world_state.packets[0].start + 0x3500;
    world_state.packets[1].end = world_state.packets[1].start + 0x3500;
    StoreImage(&world_state.draw[world_state.frame].clip, world_state.packets[1].end);
    world_state.draw[0].isbg = 1;
    world_state.draw[1].isbg = 1;
}

/* Restores the two packet buffers to their full size. */
ADDRESS(0x80026e90, 0x3c)
void gfx_restore_buffers(void)
{
    world_state.draw[0].isbg = 1;
    world_state.draw[1].isbg = 1;
    world_state.packets[1].start = world_state.packets[0].end = world_state.packets[0].start + 0x8000;
    world_state.packets[1].end = world_state.packets[1].start + 0x8000;
}

/* Unreferenced: the halfword before the text clip window. */
DATA(0x800881b8, 0x2, ".sbss")
static s16 DAT_800881b8;
/* The text clip window. */
DATA(0x800881ba, 0x2, ".sbss")
static s16 text_clip_left;
DATA(0x800881bc, 0x2, ".sbss")
static s16 text_clip_right;
DATA(0x800881c0, 0x4, ".sbss")
static s32 text_clip_advance;
DATA(0x800881c4, 0x4, ".sbss")
static s32 text_clip_on;
DATA(0x800881c8, 0x4, ".sbss")
static s32 text_clip_dropped;

/*
 * Font glyphs, text strings, numbers and 2D menu primitives. Glyphs are
 * textured quads cut from the font pages in VRAM; the scratchpad's font mode
 * picks the 6x12 or 7x10 half-width cell layout.
 */

/*
 * Takes the next primitive from the current packet buffer, or returns.
 * A statement sequence, not a do-while block: the block changes retail's
 * code.
 */
#define NEW_PRIM(buf, p, type)            \
    (buf) = world_state.packet;           \
    (p) = (type *)(buf)->cur;             \
    if ((buf)->end < (u_long *)((p) + 1)) \
        return;                           \
    (buf)->cur = (u_long *)((p) + 1);     \
    world_state.prim_alloc_count++

/*
 * NEW_PRIM with the end of the new primitive kept in the caller's `next`:
 * a function that takes a primitive in two branches uses one variable for
 * both.
 */
#define NEW_PRIM_NEXT(buf, p, next, type) \
    (buf) = world_state.packet;           \
    (p) = (type *)(buf)->cur;             \
    (next) = (u_long *)((p) + 1);         \
    if ((buf)->end < (next))              \
        return;                           \
    (buf)->cur = (next);                  \
    world_state.prim_alloc_count++

/* Draws glyph `c` at (x, y): half-width codes below the split, full-width
 * codes after it. */
ADDRESS(0x80026ecc, 0x460)
void ui_draw_glyph(u16 c, u16 x, u16 y, CVECTOR *color)
{
    StPacketBuffer *buf;
    POLY_FT4 *p;
    u_long *next;
    CVECTOR rgb;
    u16 half, cols, qw, qh, fw, ch;
    u16 split;
    int u, v;
    u16 q, r;
    int n;

    if (MENU_SCRATCH->font_mode == MENU_FONT_LOW_RES) {
        ch = 12;
        fw = ch;
        qh = fw;
        qw = fw;
        half = 6;
        cols = 21;
        split = 64;
    } else {
        half = 7;
        cols = 18;
        split = 4;
        qw = 12;
        qh = 10;
        fw = qw;
        ch = qh;
    }
    if (c < split) {
        r = c % (cols * 2);
        q = c / (cols * 2);
        u = r * half;
        v = ch * q;
        NEW_PRIM_NEXT(buf, p, next, POLY_FT4);
        setUV4(p, u, v, half + u, v, u, ch + v, half + u, ch + v);
        if (text_clip_on) {
            if (text_scroll_mode == 1)
                x -= text_scroll_list * (half + 1);
            else
                x -= DAT_800881a2 * (half + 1);
            text_clip_advance += half + 1;
            if (text_clip_right < x) {
                text_clip_dropped++;
                return;
            }
            if (x < text_clip_left)
                return;
        }
        p->x1 = p->x3 = x + half;
        p->x0 = p->x2 = x;
        p->y0 = p->y1 = y;
        p->y2 = p->y3 = y + qh;
        rgb = *color;
        rgb.r += 8;
        rgb.g += 8;
        rgb.b += 8;
    } else {
        n = c - split / 2;
        r = n % cols;
        q = n / cols;
        u = fw * r;
        v = ch * q;
        NEW_PRIM_NEXT(buf, p, next, POLY_FT4);
        setUV4(p, u, v, fw + u, v, u, ch + v, fw + u, ch + v);
        if (text_clip_on) {
            if (text_scroll_mode == 1)
                x -= text_scroll_list * (half + 1);
            else
                x -= DAT_800881a2 * (half + 1);
            text_clip_advance += fw;
            if (text_clip_right < x) {
                text_clip_dropped++;
                return;
            }
            if (x < text_clip_left)
                return;
        }
        p->x1 = p->x3 = x + qw;
        p->x0 = p->x2 = x;
        p->y0 = p->y1 = y;
        p->y2 = p->y3 = y + qh;
        rgb = *color;
        rgb.r += 8;
        rgb.g += 8;
        rgb.b += 8;
    }
    setRGB0(p, rgb.r, rgb.g, rgb.b);
    if (color->cd & 0x80) {
        p->tpage = GetTPage(0, color->cd & 3, 896, 256);
        p->clut = GetClut(560, 497);
    } else {
        p->tpage = GetTPage(0, color->cd & 3, 960, 256);
        p->clut = GetClut(560, 495);
    }
    setlen(p, 9);
    setcode(p, 0x2e);
    addPrim(world_state.ot_cur, p);
}

/* Draws a 0xffff-terminated glyph string; code 0 starts a new line. */
ADDRESS(0x8002732c, 0x114)
void ui_draw_text(u16 *s, u16 x, u16 y, CVECTOR *color)
{
    u16 left = x;
    u_int space = (color->cd & 0x7c) >> 2;
    int line;
    u_int split;
    u_int half;

    if (MENU_SCRATCH->font_mode == MENU_FONT_LOW_RES) {
        line = 14;
        split = 64;
    } else {
        space = 11;
        line = 10;
        split = 4;
    }
    half = space >> 1;
    while (*s != 0xffff) {
        if (*s == 0) {
            x = left;
            y += line;
            s++;
            s++;
        } else {
            ui_draw_glyph(*s, x, y, color);
            if (*s < split)
                x += half;
            else
                x += space;
            s++;
        }
    }
}

/*
 * Number words: the value in the low halfword (signed), the digit count in
 * bits 24-27 and, in bits 28-31, flags that ask for a plus sign. Leading
 * zeros are blank; a value too wide for the digit count shows as nines.
 */
#define NUMBER_DIGITS(num) (((num) >> 24) & 0xf)
#define NUMBER_SIGNED 0xf0000000

/* Draws a number with the proportional font: digit glyphs follow the
 * half-width codes. */
ADDRESS(0x80027440, 0x27c)
void ui_draw_number(u32 num, u16 x, u16 y, CVECTOR *color)
{
    u16 digit[8];
    int end;
    u8 leading = 1;
    u8 neg = 0;
    u16 width = color->cd >> 2;
    u16 base;
    u8 n;
    u32 sign;
    s16 value;
    int pow;
    u32 div;
    int i;

    if (number_narrow)
        width -= 4;
    if (MENU_SCRATCH->font_mode == MENU_FONT_LOW_RES)
        base = 64;
    else
        base = 4;
    n = NUMBER_DIGITS(num);
    value = num;
    sign = num & NUMBER_SIGNED;
    end = -1;
    if ((s16)num < 0) {
        value = -num;
        neg = 1;
    }
    if (value == 0) {
        ui_draw_glyph(base, x + width * (n - 1), y, color);
        return;
    }
    pow = 1;
    i = n - 1;
    if (i != end) {
        do {
            pow *= 10;
        } while (--i != -1);
    }
    if (value < pow) {
        div = pow;
        i = n;
        while (i--) {
            div /= 10;
            digit[i] = value / div;
            value -= digit[i] * div;
        }
    } else {
        i = n;
        while (i--)
            digit[i] = 9;
    }
    if (neg)
        ui_draw_glyph(base + 13, x - width, y, color);
    else if (sign)
        ui_draw_glyph(base + 12, x - width, y, color);
    i = n;
    while (i--) {
        if (digit[i] != 0)
            leading = 0;
        if (!leading)
            ui_draw_glyph(base + digit[i], x + width * (n - i - 1), y, color);
    }
}

/* Draws a number with the fixed 7-pixel digit glyphs (codes 2-11). */
ADDRESS(0x800276bc, 0x244)
void ui_draw_number_fixed(u32 num, u16 x, u16 y, CVECTOR *color)
{
    u16 digit[8];
    u8 leading = 1;
    u8 neg = 0;
    u8 n = NUMBER_DIGITS(num);
    u16 width = 7;
    int end;
    u32 sign;
    s16 value;
    u32 pow;
    u32 div;
    int i;

    value = num;
    sign = num & NUMBER_SIGNED;
    end = -1;
    if ((s16)num < 0) {
        value = -num;
        neg = 1;
    }
    if (value == 0) {
        ui_draw_glyph(2, x + width * (n - 1), y, color);
        return;
    }
    pow = 1;
    i = n - 1;
    if (i != end) {
        do {
            pow *= 10;
        } while (--i != -1);
    }
    if (value < pow) {
        div = pow;
        i = n;
        while (i--) {
            div /= 10;
            digit[i] = value / div;
            value -= digit[i] * div;
        }
    } else {
        i = n;
        while (i--)
            digit[i] = 9;
    }
    if (neg)
        ui_draw_glyph(13, x - width, y, color);
    else if (sign)
        ui_draw_glyph(12, x - width, y, color);
    i = n;
    while (i--) {
        if (digit[i] != 0)
            leading = 0;
        if (!leading)
            ui_draw_glyph(digit[i] + 2, x + width * (n - i - 1), y, color);
    }
}

/* Draws a small arrow pointing along `dir` (0, 1024, 2048 or 3072) at
 * `pos`, over a copy offset by one pixel. Nothing calls it. */
ADDRESS(0x80027900, 0x418)
void ui_draw_small_arrow(DVECTOR *pos, CVECTOR *color, u16 dir)
{
    POLY_F3 *p;
    StPacketBuffer *buf;
    DR_TPAGE *t;
    short x0 = 0, x1 = 0, x2 = 0, y0 = 0, y1 = 0, y2 = 0;

    switch (dir) {
    case 0:
        x0 = pos->vx - 4;
        x1 = pos->vx + 4;
        x2 = x0;
        y0 = pos->vy - 8;
        y1 = pos->vy;
        y2 = y1 + 8;
        break;
    case 1024:
        x0 = pos->vx - 8;
        x1 = pos->vx;
        x2 = x1 + 8;
        y0 = pos->vy - 4;
        y1 = pos->vy + 4;
        y2 = y0;
        break;
    case 2048:
        x0 = pos->vx + 4;
        x1 = pos->vx - 4;
        x2 = x0;
        y0 = pos->vy - 8;
        y1 = pos->vy;
        y2 = y1 + 8;
        break;
    case 3072:
        x0 = pos->vx - 8;
        x1 = pos->vx;
        x2 = x1 + 8;
        y0 = pos->vy + 4;
        y1 = pos->vy - 4;
        y2 = y0;
        break;
    }
    NEW_PRIM(buf, p, POLY_F3);
    setlen(p, 4);
    setcode(p, 0x22);
    setXY3(p, x0, y0, x1, y1, x2, y2);
    setRGB0(p, color->r, color->g, color->b);
    addPrim(&world_state.ot_cur[1], p);
    NEW_PRIM(buf, t, DR_TPAGE);
    SetDrawTPage(t, 0, 1, GetTPage(0, 3, 0, 0));
    addPrim(&world_state.ot_cur[1], t);
    NEW_PRIM(buf, p, POLY_F3);
    setlen(p, 4);
    setcode(p, 0x22);
    setXY3(p, x0 + 1, y0 + 1, x1 + 1, y1 + 1, x2 + 1, y2 + 1);
    setRGB0(p, color->r, color->g, color->b);
    addPrim(&world_state.ot_cur[1], p);
    NEW_PRIM(buf, t, DR_TPAGE);
    SetDrawTPage(t, 0, 1, GetTPage(0, 3, 0, 0));
    addPrim(&world_state.ot_cur[1], t);
}

/* The body of a flat rectangle: the code (semi-transparent when `semi` is
 * 1), the tag linked in front of the ordering table's head, the corners
 * and the colour. */
#define FILL_F4(p, semi, x0, y0, x1, y1, x2, y2, x3, y3, color)    \
    (p)->code = (semi) == 1 ? 0x2a : 0x28;                     \
    (p)->tag = (*world_state.ot_cur & 0xffffff) | 0x05000000;  \
    setXY4(p, x0, y0, x1, y1, x2, y2, x3, y3);                 \
    setRGB0(p, (color)->r, (color)->g, (color)->b);            \
    *world_state.ot_cur = (u_long)(p) & 0xffffff

/* FILL_F4 over the rectangle with top left (x, y), width w and height h. */
#define FILL_RECT_F4(p, semi, x, y, w, h, color) \
    FILL_F4(p, semi, x, y, (x) + (w), y, x, (y) + (h), (x) + (w), (y) + (h), color)

/* Draws a rectangle outline: bars `border & 0xff` high at the top and
 * bottom and `border >> 8` wide at the sides. */
ADDRESS(0x80027d18, 0x508)
void ui_draw_rect_outline(RECT *r, CVECTOR *color, u16 border, u16 semi)
{
    POLY_F4 *p;
    StPacketBuffer *buf;
    short w = border >> 8;
    short h = border & 0xff;

    NEW_PRIM(buf, p, POLY_F4);
    FILL_RECT_F4(p, semi, r->x, r->y, r->w, h, color);
    NEW_PRIM(buf, p, POLY_F4);
    FILL_RECT_F4(p, semi, r->x, r->y + r->h - h, r->w, h, color);
    NEW_PRIM(buf, p, POLY_F4);
    FILL_RECT_F4(p, semi, r->x, r->y + h, w, r->h - h * 2, color);
    NEW_PRIM(buf, p, POLY_F4);
    FILL_RECT_F4(p, semi, r->x + r->w - w, r->y + h, w, r->h - h * 2, color);
}

/* Fills a rectangle (semi-transparent when `semi` is 1). */
ADDRESS(0x80028220, 0x144)
void ui_fill_rect(RECT *r, CVECTOR *color, u16 semi)
{
    POLY_F4 *p;
    StPacketBuffer *buf;

    NEW_PRIM(buf, p, POLY_F4);
    FILL_F4(p, semi, r->x, r->y, r->x + r->w, r->y, r->x, r->y + r->h, r->x + r->w, r->y + r->h,
            color);
}

/*
 * Draws the texture rectangle `tex` (VRAM page from its x/y, u/v from the
 * low bits) over `r` with CLUT `clut`, tinted by `color` (grey when NULL).
 * `mode`: bit 0 semi-transparent, bits 1-2 blend rate, bits 3-4 colour
 * depth, bits 5-6 flip (1 horizontal, 2 vertical).
 */
ADDRESS(0x80028364, 0x344)
void ui_draw_image(RECT *r, DVECTOR *tex, DVECTOR *clut, CVECTOR *color, int mode)
{
    POLY_FT4 *p;
    StPacketBuffer *buf;
    int semi = mode & 1;
    int abr = (mode >> 1) & 3;
    int tp = (mode >> 3) & 3;
    int flip = (mode >> 5) & 3;
    short tx, ty;
    u8 u0, v0, u1, v1;

    NEW_PRIM(buf, p, POLY_FT4);
    tx = tex->vx & 0xffc0;
    ty = tex->vy & 0xff00;
    u0 = tex->vx & 0x3f;
    v0 = tex->vy;
    if (tp == 0) {
        if (u0 < 64)
            u0 *= 4;
        else
            u0 = 255;
    } else if (tp == 1) {
        if (u0 < 128)
            u0 *= 2;
        else
            u0 = 255;
    }
    u1 = 255;
    if (u0 + r->w < 256)
        u1 = u0 + r->w;
    v1 = 255;
    if (v0 + r->h < 256)
        v1 = v0 + r->h;
    p->tpage = GetTPage(tp, abr, tx, ty);
    p->clut = GetClut(clut->vx, clut->vy);
    if (color == NULL) {
        setRGB0(p, 128, 128, 128);
    } else {
        setRGB0(p, color->r, color->g, color->b);
    }
    p->tag = (*world_state.ot_cur & 0xffffff) | 0x09000000;
    setXY4(p, r->x, r->y, r->x + r->w, r->y, r->x, r->y + r->h, r->x + r->w, r->y + r->h);
    if (flip == 1)
        setUV4(p, u1 - 1, v0, u0 - 1, v0, u1 - 1, v1, u0 - 1, v1);
    else if (flip == 2)
        setUV4(p, u0, v1 - 1, u1, v1 - 1, u0, v0 - 1, u1, v0 - 1);
    else
        setUV4(p, u0, v0, u1, v0, u0, v1, u1, v1);
    p->code = semi ? 0x2e : 0x2c;
    *world_state.ot_cur = (u_long)p & 0xffffff;
}

/* Adds a draw-mode primitive selecting the texture page at (x, y) with
 * blend rate `abr`. */
ADDRESS(0x800286a8, 0xdc)
void ui_add_draw_mode(int abr, int x, int y)
{
    DR_MODE *p;
    StPacketBuffer *buf;

    NEW_PRIM(buf, p, DR_MODE);
    SetDrawMode(p, 0, 0, GetTPage(0, abr, x, y), NULL);
    addPrim(world_state.ot_cur, p);
}

/* Card fade states in world_state.talk_overlay_state. */
#define CARD_FADE_IN 1
#define CARD_SHOW_IN 2
#define CARD_SHOW 3
#define CARD_SHOW_OUT 4
#define CARD_DONE 5
#define CARD_FADE_OUT 6

/*
 * Steps the talk picture overlay one frame: fades the screen, fades the
 * TALKMSG.T picture talk_load_image uploads in and out, and counts the
 * frames in unk1EE19 (unk1EE17 is the picture mode, unk1EE18 the fade
 * length).
 */
ADDRESS(0x80028784, 0x4a8)
void talk_draw_overlay(void)
{
    RECT screen = {0, 0, 320, 240};
    RECT card = {32, 132, 255, 255};
    CVECTOR fill;
    CVECTOR tint;
    DVECTOR tex = {960, 256};
    DVECTOR clut = {560, 495};

    if (world_state.talk_overlay_mode == 1) {
        tex.vy = 0;
        clut.vx = 0;
        clut.vy = 511;
    }
    if (world_state.talk_fade_length == 0)
        world_state.talk_fade_length = 1;
    switch (world_state.talk_overlay_state) {
    case CARD_FADE_IN:
        if (world_state.talk_overlay_mode != 3)
            fill.r = fill.g = fill.b = (world_state.talk_frame << 4) / world_state.talk_fade_length;
        else
            fill.r = fill.g = fill.b = 255;
        ui_fill_rect(&screen, &fill, 1);
        ui_add_draw_mode(2, 0, 0);
        break;
    case CARD_SHOW_IN:
        if (game_settings.subtitles == 1 || world_state.talk_overlay_mode == 2) {
            tint.r = tint.g = tint.b = world_state.talk_frame << 3;
            ui_draw_image(&card, &tex, &clut, &tint, 3);
            if (world_state.talk_frame >= 16)
                world_state.talk_overlay_state = CARD_SHOW;
        } else {
            world_state.talk_overlay_state = CARD_SHOW;
        }
        break;
    case CARD_SHOW:
        if (game_settings.subtitles == 1 || world_state.talk_overlay_mode == 2) {
            tint.r = tint.g = tint.b = 128;
            if (talk_stream_file == 25 || talk_stream_file == 30 || talk_stream_file == 5)
                ui_draw_image(&card, &tex, &clut, &tint, 2);
            else
                ui_draw_image(&card, &tex, &clut, &tint, 3);
        }
        break;
    case CARD_SHOW_OUT:
        if (game_settings.subtitles == 1 || world_state.talk_overlay_mode == 2) {
            tint.r = tint.g = tint.b = (16 - world_state.talk_frame) << 3;
            ui_draw_image(&card, &tex, &clut, &tint, 3);
            if (world_state.talk_frame >= 16)
                world_state.talk_overlay_state = CARD_DONE;
        } else {
            world_state.talk_overlay_state = CARD_DONE;
        }
        break;
    case CARD_FADE_OUT:
        if (world_state.talk_overlay_mode != 3)
            fill.r = fill.g = fill.b =
                ((world_state.talk_fade_length - world_state.talk_frame) << 4) / world_state.talk_fade_length;
        else
            fill.r = fill.g = fill.b = 255;
        ui_fill_rect(&screen, &fill, 1);
        ui_add_draw_mode(2, 0, 0);
        break;
    default:
        return;
    }
    if (world_state.talk_overlay_mode == 0 && world_state.talk_overlay_state >= CARD_SHOW_IN &&
        world_state.talk_overlay_state < CARD_FADE_OUT) {
        fill.r = fill.g = fill.b = 16;
        ui_fill_rect(&screen, &fill, 1);
        ui_add_draw_mode(2, 0, 0);
    }
    if (world_state.talk_overlay_mode == 3) {
        fill.r = fill.g = fill.b = 255;
        ui_fill_rect(&screen, &fill, 1);
        ui_add_draw_mode(2, 0, 0);
    }
    world_state.talk_frame++;
}

/* Draws a Gouraud quad: corner positions and colours in setXY4 order. */
ADDRESS(0x80028c2c, 0x18c)
void ui_draw_gouraud_quad(DVECTOR *xy, CVECTOR *rgb, int semi)
{
    StPacketBuffer *buf;
    POLY_G4 *p;

    NEW_PRIM(buf, p, POLY_G4);
    p->tag = (*world_state.ot_cur & 0xffffff) | 0x08000000;
    setRGB0(p, rgb[0].r, rgb[0].g, rgb[0].b);
    setRGB1(p, rgb[1].r, rgb[1].g, rgb[1].b);
    setRGB2(p, rgb[2].r, rgb[2].g, rgb[2].b);
    setRGB3(p, rgb[3].r, rgb[3].g, rgb[3].b);
    setXY4(p, xy[0].vx, xy[0].vy, xy[1].vx, xy[1].vy, xy[2].vx, xy[2].vy, xy[3].vx, xy[3].vy);
    p->code = semi == 0 ? 0x38 : 0x3a;
    *world_state.ot_cur = (u_long)p & 0xffffff;
}

/* Fills `r` with a vertical gradient from `edge` at the top and bottom to
 * `mid` across the middle. */
ADDRESS(0x80028db8, 0x3b4)
void ui_fill_rect_gradient(RECT *r, CVECTOR *mid, CVECTOR *edge, int semi)
{
    StPacketBuffer *buf;
    StPacketBuffer *lower;
    POLY_G4 *p;
    RECT b;

    b = *r;
    b.h = b.h >> 1;
    NEW_PRIM(buf, p, POLY_G4);
    p->code = semi == 0 ? 0x38 : 0x3a;
    p->tag = (*world_state.ot_cur & 0xffffff) | 0x08000000;
    setRGB0(p, edge->r, edge->g, edge->b);
    setRGB1(p, edge->r, edge->g, edge->b);
    setRGB2(p, mid->r, mid->g, mid->b);
    setRGB3(p, mid->r, mid->g, mid->b);
    setXY4(p, b.x, b.y, b.x + b.w, b.y, b.x, b.y + b.h, b.x + b.w, b.y + b.h);
    *world_state.ot_cur = (u_long)p & 0xffffff;
    b = *r;
    b.h = b.h >> 1;
    b.y += b.h;
    NEW_PRIM(lower, p, POLY_G4);
    p->code = semi == 0 ? 0x38 : 0x3a;
    p->tag = (*world_state.ot_cur & 0xffffff) | 0x08000000;
    setRGB0(p, mid->r, mid->g, mid->b);
    setRGB1(p, mid->r, mid->g, mid->b);
    setRGB2(p, edge->r, edge->g, edge->b);
    setRGB3(p, edge->r, edge->g, edge->b);
    setXY4(p, b.x, b.y, b.x + b.w, b.y, b.x, b.y + b.h, b.x + b.w, b.y + b.h);
    *world_state.ot_cur = (u_long)p & 0xffffff;
}

/* Draws a triangle filling `r` that points along `dir` (0, 1024, 2048 or
 * 3072), over a copy offset by one pixel. */
ADDRESS(0x8002916c, 0x320)
void ui_draw_arrow(RECT *r, CVECTOR *color, u16 dir)
{
    short x0 = 0, x1 = 0, x2 = 0, y0 = 0, y1 = 0, y2 = 0;
    StPacketBuffer *buf;
    POLY_F3 *p;

    switch (dir) {
    case 0:
        x0 = r->x - r->h / 2;
        x1 = r->x + r->h / 2;
        x2 = x0;
        y0 = r->y - r->w / 2;
        y1 = r->y;
        y2 = y1 + r->w / 2;
        break;
    case 1024:
        x0 = r->x - r->w / 2;
        x1 = r->x;
        x2 = x1 + r->w / 2;
        y0 = r->y - r->h / 2;
        y1 = r->y + r->h / 2;
        y2 = y0;
        break;
    case 2048:
        x0 = r->x + r->h / 2;
        x1 = r->x - r->h / 2;
        x2 = x0;
        y0 = r->y - r->w / 2;
        y1 = r->y;
        y2 = y1 + r->w / 2;
        break;
    case 3072:
        x0 = r->x - r->w / 2;
        x1 = r->x;
        x2 = x1 + r->w / 2;
        y0 = r->y + r->h / 2;
        y1 = r->y - r->h / 2;
        y2 = y0;
        break;
    }
    NEW_PRIM(buf, p, POLY_F3);
    setlen(p, 4);
    setcode(p, 0x22);
    setXY3(p, x0, y0, x1, y1, x2, y2);
    setRGB0(p, color->r, color->g, color->b);
    addPrim(world_state.ot_cur, p);
    NEW_PRIM(buf, p, POLY_F3);
    setlen(p, 4);
    setcode(p, 0x22);
    setXY3(p, x0 + 1, y0 + 1, x1 + 1, y1 + 1, x2 + 1, y2 + 1);
    setRGB0(p, color->r, color->g, color->b);
    addPrim(world_state.ot_cur, p);
}

/* Copies the screen into the spare buffer's image and darkens it. */
ADDRESS(0x8002948c, 0xbc)
void menu_draw_backdrop(void)
{
    RECT screen = {0, 0, 320, 240};
    CVECTOR shade = {32, 32, 32};

    LoadImage(&world_state.draw[world_state.frame].clip, world_state.packets[1].end);
    ui_fill_rect(&screen, &shade, 1);
    ui_add_draw_mode(2, 0, 0);
}

/* Draws a menu panel: a dark fill over a lighter border, both blended. */
ADDRESS(0x80029548, 0xd0)
void ui_draw_panel(int x, int y, int w, int h)
{
    MENU_SCRATCH->border_color.g = 80;
    MENU_SCRATCH->border_color.r = 80;
    MENU_SCRATCH->border_color.b = 96;
    MENU_SCRATCH->fill_color.g = 40;
    MENU_SCRATCH->fill_color.r = 40;
    MENU_SCRATCH->rect.x = x;
    MENU_SCRATCH->rect.y = y;
    MENU_SCRATCH->rect.w = w;
    MENU_SCRATCH->rect.h = h;
    MENU_SCRATCH->fill_color.b = 48;
    MENU_SCRATCH->inner_rect.x = x + 2;
    MENU_SCRATCH->inner_rect.y = y + 1;
    MENU_SCRATCH->inner_rect.w = w - 4;
    MENU_SCRATCH->inner_rect.h = h - 2;
    ui_fill_rect(&MENU_SCRATCH->inner_rect, &MENU_SCRATCH->fill_color, 1);
    ui_add_draw_mode(2, 0, 0);
    ui_fill_rect(&MENU_SCRATCH->rect, &MENU_SCRATCH->border_color, 1);
    ui_add_draw_mode(0, 0, 0);
}

/* Tile modes for ui_draw_image: semi-transparent, optionally flipped. */
#define TILE (1)
#define TILE_HFLIP (1 | 1 << 5)
#define TILE_VFLIP (1 | 2 << 5)

/*
 * Draws a window frame from 16x16 tiles of the menu texture page: corners,
 * edges, an inner bevel and the filled middle, then a grey blended wash.
 */
ADDRESS(0x80029618, 0x55c)
void ui_draw_window(short x, short y, short w, short h)
{
    MenuScratch *sp = MENU_SCRATCH;
    RECT r;
    CVECTOR wash = {48, 48, 48};
    int rows, columns;

    sp->rect.w = 16;
    sp->rect.h = 16;
    sp->clut_pos.vx = 544;
    sp->clut_pos.vy = 504;
    sp->tex_pos.vx = 612;
    sp->rect.x = x;
    sp->rect.y = y;
    sp->tex_pos.vy = 400;
    ui_draw_image(&sp->rect, &sp->tex_pos, &sp->clut_pos, NULL, TILE);
    sp->rect.y = y + h - 16;
    ui_draw_image(&sp->rect, &sp->tex_pos, &sp->clut_pos, NULL, TILE_VFLIP);
    sp->rect.x = x + w - 16;
    sp->rect.y = y;
    sp->tex_pos.vx = 620;
    ui_draw_image(&sp->rect, &sp->tex_pos, &sp->clut_pos, NULL, TILE);
    sp->rect.y = y + h - 16;
    ui_draw_image(&sp->rect, &sp->tex_pos, &sp->clut_pos, NULL, TILE_VFLIP);
    sp->rect.x = x;
    sp->rect.y = y + 16;
    sp->tex_pos.vx = 604;
    ui_draw_image(&sp->rect, &sp->tex_pos, &sp->clut_pos, NULL, TILE);
    sp->rect.y = y + h - 32;
    ui_draw_image(&sp->rect, &sp->tex_pos, &sp->clut_pos, NULL, TILE_VFLIP);
    sp->rect.x = x + w - 16;
    sp->rect.y = y + 16;
    sp->tex_pos.vx = 596;
    ui_draw_image(&sp->rect, &sp->tex_pos, &sp->clut_pos, NULL, TILE);
    sp->rect.y = y + h - 32;
    ui_draw_image(&sp->rect, &sp->tex_pos, &sp->clut_pos, NULL, TILE_VFLIP);

    sp->rect.y = y + 32;
    sp->rect.x = x;
    sp->tex_pos.vx = 624;
    rows = h / 16 - 4;
    while (rows--) {
        sp->rect.x = x;
        ui_draw_image(&sp->rect, &sp->tex_pos, &sp->clut_pos, NULL, TILE_HFLIP);
        sp->rect.x = x + w - 16;
        ui_draw_image(&sp->rect, &sp->tex_pos, &sp->clut_pos, NULL, TILE);
        sp->rect.y += 16;
    }
    sp->rect.x = x + 16;
    sp->tex_pos.vx = 616;
    columns = w / 16 - 2;
    while (columns--) {
        sp->rect.y = y;
        ui_draw_image(&sp->rect, &sp->tex_pos, &sp->clut_pos, NULL, TILE);
        sp->rect.y = y + h - 16;
        ui_draw_image(&sp->rect, &sp->tex_pos, &sp->clut_pos, NULL, TILE_VFLIP);
        sp->rect.x += 16;
    }
    sp->rect.x = x + 16;
    sp->tex_pos.vx = 608;
    columns = w / 16 - 2;
    while (columns--) {
        sp->rect.y = y + 16;
        ui_draw_image(&sp->rect, &sp->tex_pos, &sp->clut_pos, NULL, TILE);
        sp->rect.y = y + h - 32;
        ui_draw_image(&sp->rect, &sp->tex_pos, &sp->clut_pos, NULL, TILE_VFLIP);
        sp->rect.x += 16;
    }
    sp->rect.y = y + 32;
    sp->tex_pos.vx = 600;
    rows = h / 16 - 4;
    while (rows--) {
        sp->rect.x = x + 16;
        columns = w / 16 - 2;
        while (columns--) {
            ui_draw_image(&sp->rect, &sp->tex_pos, &sp->clut_pos, NULL, TILE);
            sp->rect.x += 16;
        }
        sp->rect.y += 16;
    }
    r.x = x;
    r.y = y;
    r.w = w;
    r.h = h;
    ui_fill_rect(&r, &wash, 1);
    ui_add_draw_mode(2, 0, 0);
}

/*
 * The pause menu pages. Each page restarts the frame,
 * draws its text, numbers and icons with the text primitives above,
 * frames itself with ui_draw_window and darkens the world behind it.
 */

/* Blink phase of the area list page's arrows (0..31). */
DATA(0x80087f34, 0x4, ".sdata")
static s32 area_info_blink = 0;

/* ui_draw_number/ui_draw_number_fixed take the digit count in the top byte. */
#define DIGITS(n) ((n) << 24)

/* Closes a page: its frame, the end of the frame and the darkened world. */
#define PAGE_END()                     \
    ui_draw_window(16, 36, 288, 192);   \
    gfx_flip();                   \
    menu_draw_backdrop();                   \
    gfx_draw_ot()

/* The pause menu's main page. */
ADDRESS(0x80029b74, 0x65c)
void menu_main_draw(void)
{
    u16 label_x[8] = {37, 47, 20, 20, 30, 41, 34, 37};
    DVECTOR row[4] = {{196, 94}, {274, 94}, {172, 104}, {298, 104}};
    CVECTOR lit[4] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {128, 64, 0, 0}, {128, 64, 0, 0}};
    CVECTOR dim[4] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 64, 128, 0}, {0, 64, 128, 0}};
    MenuScratch *sp;
    int i;
    int kind;
    int n;

    gfx_begin_frame();
    sp = MENU_SCRATCH;
    sp->style[0].r = 72;
    sp->style[0].g = 72;
    sp->style[0].b = 72;
    sp->style[2].r = 98;
    sp->style[2].g = 66;
    sp->style[2].b = 48;
    sp->style[2].cd = 57;
    sp->style[0].cd = 57;
    for (i = 0; i < 11; i++)
        ui_draw_text(MESSAGE(i), 24, i * 16 + 48, &sp->style[0]);
    if (viewer.status_flags & STATUS_POISON)
        ui_draw_text(MESSAGE(MSG_STATUS_POISON), 106, 112, &sp->style[0]);
    else if (viewer.status_flags & STATUS_PARALYZE)
        ui_draw_text(MESSAGE(MSG_STATUS_PARALYZE), 93, 112, &sp->style[0]);
    else if (viewer.status_flags & STATUS_DARK)
        ui_draw_text(MESSAGE(MSG_STATUS_DARK), 119, 112, &sp->style[0]);
    else if (viewer.status_flags & STATUS_SLOW)
        ui_draw_text(MESSAGE(MSG_STATUS_SLOW), 119, 112, &sp->style[0]);
    else if (viewer.status_flags & STATUS_CURSED)
        ui_draw_text(MESSAGE(MSG_STATUS_CURSED), 106, 112, &sp->style[0]);
    else
        ui_draw_text(MESSAGE(MSG_STATUS_NORMAL), 106, 112, &sp->style[0]);
    if (viewer.equipment[EQUIP_SLOT_LEFT_HAND] != EQUIP_NONE
        && vis_table_c.types[viewer.equipment[EQUIP_SLOT_LEFT_HAND]].unk5[0x1d] >= 9) {
        kind = vis_table_c.types[viewer.equipment[EQUIP_SLOT_LEFT_HAND]].unk5[0x1d];
        n = 0;
        if (kind - 9 == 2)
            n = 14;
        ui_draw_text(MESSAGE(MSG_ELEMENTS - 9 + kind), 119 - n, 208, &sp->style[0]);
    } else {
        ui_draw_text(MESSAGE(MSG_ELEMENT_NONE), 119, 208, &sp->style[0]);
    }
    for (i = 0; i < 8; i++)
        ui_draw_text(MESSAGE(MSG_PAUSE_ROWS + i), label_x[i] + 172, i * 16 + 90, &sp->style[2]);
    ui_draw_number_fixed(viewer.hp | DIGITS(4), 78, 48, &sp->style[0]);
    i = 0;
    ui_draw_glyph(15, 106, 48, &sp->style[0]);
    ui_draw_number_fixed(viewer.max_hp | DIGITS(4), 120, 48, &sp->style[0]);
    ui_draw_number_fixed(viewer.mp | DIGITS(4), 78, 64, &sp->style[0]);
    ui_draw_glyph(15, 106, 64, &sp->style[0]);
    ui_draw_number_fixed(viewer.max_mp | DIGITS(4), 120, 64, &sp->style[0]);
    ui_draw_number_fixed(viewer.sp_total | DIGITS(4), 78, 80, &sp->style[0]);
    ui_draw_glyph(15, 106, 80, &sp->style[0]);
    ui_draw_number_fixed(4921 | DIGITS(4), 120, 80, &sp->style[0]);
    ui_draw_number_fixed(inventory.counts[0x10a].count | DIGITS(3), 127, 96, &sp->style[0]);
    ui_draw_number_fixed(viewer.magic_attack_left | DIGITS(3), 127, 128, &sp->style[0]);
    ui_draw_number_fixed(viewer.magic_attack_right | DIGITS(3), 127, 144, &sp->style[0]);
    ui_draw_number_fixed(viewer.weapon_attack[0] | DIGITS(3), 127, 160, &sp->style[0]);
    ui_draw_number_fixed(viewer.weapon_attack[1] | DIGITS(3), 127, 176, &sp->style[0]);
    ui_draw_number_fixed(viewer.weapon_attack[2] | DIGITS(3), 127, 192, &sp->style[0]);
    sp->tex_pos.vx = 622;
    sp->tex_pos.vy = 416;
    sp->clut_pos.vx = 544;
    sp->clut_pos.vy = 511;
    sp->rect.x = 196;
    sp->rect.y = 54;
    sp->rect.w = 72;
    sp->rect.h = 24;
    ui_draw_image(&sp->rect, &sp->tex_pos, &sp->clut_pos, NULL, 1);
    for (; i < 8; i++) {
        if (i == (s16)sp->main_row)
            ui_draw_gouraud_quad(row, lit, 1);
        else
            ui_draw_gouraud_quad(row, dim, 1);
        row[0].vy += 16;
        row[1].vy += 16;
        row[2].vy += 16;
        row[3].vy += 16;
    }
    ui_add_draw_mode(0, 0, 0);
    PAGE_END();
}

/* A scrolling list page (five rows of icon, name and count). */
ADDRESS(0x8002a1d0, 0x3d4)
void menu_use_item_draw(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    int id = sp->state.page_list.entries[sp->scroll + sp->cursor];
    int entry;
    int rows;
    int i;
    int red, green;

    gfx_begin_frame();
    sp->style[0].r = sp->style[4].r = 72;
    sp->style[0].g = sp->style[4].g = 72;
    sp->style[0].b = sp->style[4].b = 72;
    sp->style[2].r = 98;
    sp->style[2].g = 66;
    sp->style[2].b = 48;
    sp->style[2].cd = 57;
    sp->style[0].cd = 57;
    sp->style[4].cd = 41;
    if (sp->blink_phase < 16)
        sp->blink_level = 8 - sp->blink_phase;
    else
        sp->blink_level = sp->blink_phase - 24;
    sp->cursor_rect.x = 23;
    sp->cursor_rect.w = 276;
    red = (u8)sp->blink_level * 4 + 96;
    sp->cursor_color.b = 0;
    green = (u8)sp->blink_level * 2 + 48;
    sp->cursor_rect.h = 26;
    sp->cursor_color.r = red;
    sp->cursor_color.g = green;
    sp->cursor_rect.y = sp->cursor * 26 + 69;
    ui_draw_rect_outline(&sp->cursor_rect, &sp->cursor_color, 257, 1);
    ui_add_draw_mode(1, 0, 0);
    sp->image_rect.x = 24;
    sp->image_rect.y = 70;
    sp->image_rect.w = 24;
    sp->image_rect.h = 24;
    rows = 5;
    if (sp->state.page_list.count < 6)
        rows = sp->state.page_list.count;
    sp->style[0].cd |= 0x80;
    if (sp->cursor < rows)
        ui_draw_text(MESSAGE_IN(sp->state.page_list.messages, 0x176 + id), 32, 208, &sp->style[0]);
    for (i = 0; i < rows; i++) {
        entry = sp->state.page_list.entries[sp->scroll + i];
        ui_draw_text(MESSAGE_IN(sp->state.page_list.messages, entry), 54, i * 26 + 76, &sp->style[0]);
        ui_draw_number(inventory.counts[entry].count | DIGITS(2), 274, i * 26 + 76, &sp->style[4]);
        ui_draw_image(&sp->image_rect, &sp->state.icons[(sp->scroll + i) % 5 + 1], NULL, NULL, 16);
        sp->image_rect.y += 26;
    }
    sp->clut_pos.vx = 544;
    sp->clut_pos.vy = 511;
    sp->rect.x = 70;
    sp->rect.y = 44;
    sp->rect.w = 35;
    sp->rect.h = 16;
    sp->tex_pos.vx = 624;
    sp->tex_pos.vy = 448;
    sp->style[0].cd &= 0x7f;
    ui_draw_image(&sp->rect, &sp->tex_pos, &sp->clut_pos, NULL, 3);
    sp->tex_pos.vx = 633;
    sp->rect.x = 32;
    sp->tex_pos.vy = 448;
    sp->rect.w = 30;
    ui_draw_image(&sp->rect, &sp->tex_pos, &sp->clut_pos, NULL, 3);
    PAGE_END();
}

/* The attribute point screen: seven rows of two columns. */
ADDRESS(0x8002a5a4, 0x3b0)
void menu_attribute_points_draw(void)
{
    MenuScratch *sp;
    CVECTOR *left, *left_num, *right, *right_num;
    int i;

    gfx_begin_frame();
    sp = MENU_SCRATCH;
    sp->style[0].r = sp->style[4].r = 42;
    sp->style[0].g = sp->style[4].g = 42;
    sp->style[0].b = sp->style[4].b = 42;
    sp->style[2].r = sp->style[6].r = 98;
    sp->style[2].g = sp->style[6].g = 66;
    sp->style[2].b = sp->style[6].b = 48;
    sp->style[2].cd = 57;
    sp->style[0].cd = 57;
    sp->style[6].cd = 41;
    sp->style[4].cd = 41;
    sp->rect.y = 70;
    sp->rect.w = 10;
    sp->rect.h = 10;
    sp->tex_pos.vx = 597;
    sp->tex_pos.vy = 433;
    sp->clut_pos.vx = 544;
    sp->clut_pos.vy = 502;
    for (i = 0; i < 7; i++) {
        if (sp->state.stats.selected == i) {
            left = &sp->style[2];
            left_num = &sp->style[6];
        } else {
            left = &sp->style[0];
            left_num = &sp->style[4];
        }
        if (sp->state.stats.selected == i + 7) {
            right = &sp->style[2];
            right_num = &sp->style[6];
        } else {
            right = &sp->style[0];
            right_num = &sp->style[4];
        }
        ui_draw_text(MESSAGE(MSG_ATTR_STRENGTH + i), 46, i * 20 + 68, left);
        ui_draw_text(MESSAGE(MSG_ATTR_SPIRIT + i), 170, i * 20 + 68, right);
        ui_draw_number((sp->state.stats.base[0][i] + sp->state.stats.bonus[0][i]) | DIGITS(3), 108, i * 20 + 68, left_num);
        ui_draw_number((sp->state.stats.base[1][i] + sp->state.stats.bonus[1][i]) | DIGITS(3), 232, i * 20 + 68, right_num);
        sp->rect.x = 141;
        ui_draw_image(&sp->rect, &sp->tex_pos, &sp->clut_pos, left, 3);
        sp->rect.x = 265;
        ui_draw_image(&sp->rect, &sp->tex_pos, &sp->clut_pos, right, 3);
        sp->rect.y += 20;
    }
    sp->style[0].r = sp->style[4].r = 72;
    sp->style[0].g = sp->style[4].g = 72;
    sp->style[0].b = sp->style[4].b = 72;
    ui_draw_text(MESSAGE(MSG_REMAINING_SP), 56, 208, &sp->style[0]);
    ui_draw_number(sp->state.stats.value | DIGITS(2), 242, 208, &sp->style[4]);
    sp->clut_pos.vx = 544;
    sp->clut_pos.vy = 511;
    sp->rect.x = 70;
    sp->rect.y = 44;
    sp->rect.w = 76;
    sp->rect.h = 16;
    sp->tex_pos.vx = 621;
    sp->tex_pos.vy = 480;
    ui_draw_image(&sp->rect, &sp->tex_pos, &sp->clut_pos, NULL, 3);
    sp->tex_pos.vx = 633;
    sp->tex_pos.vy = 448;
    sp->rect.x = 32;
    sp->rect.w = 30;
    ui_draw_image(&sp->rect, &sp->tex_pos, &sp->clut_pos, NULL, 3);
    PAGE_END();
}

/* The area list page: visited areas and their completion, or the area summary. */
ADDRESS(0x8002a954, 0x994)
void menu_area_info_draw(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    u8 total[48] = {
        255, 18, 25, 21, 20, 19, 20, 255, 25, 27, 29, 32, 28, 24, 26, 23,
        255, 19, 25, 255, 27, 13, 255, 22, 24, 23, 22, 32, 24, 255, 255, 255,
        32, 12, 255, 255, 255, 11, 4, 255, 255, 255, 255, 255, 255, 255, 255, 0,
    };
    CVECTOR arrow;
    RECT unused;
    RECT up = {159, 130, 16, 8};
    RECT next = {296, 130, 16, 8};
    RECT prev = {23, 130, 16, 8};
    RECT down = {160, 130, 16, 8};
    s16 name_x[42] = {
        0,  42, 35, 42, 35, 28, 21, 21, 42, 21, 35, 35, 14, 14, 28, 0,  35, 42, 0,  35, 28,
        0,  28, 42, 21, 42, 7,  42, 0,  0,  0,  35, 21, 0,  0,  0,  35, 42, 0,  42, 0,  0,
    };
    u32 time;
    int hours;
    int minutes;
    int i;
    int id;
    int level;

    gfx_begin_frame();
    if (area_info_blink < 16)
        sp->blink_level = 8 - area_info_blink;
    else
        sp->blink_level = area_info_blink - 24;
    sp->style[0].r = sp->style[4].r = 72;
    sp->style[0].g = sp->style[4].g = 72;
    sp->style[0].b = sp->style[4].b = 72;
    sp->style[2].r = sp->style[6].r = 98;
    sp->style[2].g = sp->style[6].g = 66;
    sp->style[2].b = sp->style[6].b = 48;
    sp->style[2].cd = 57;
    sp->style[0].cd = 57;
    sp->style[6].cd = 41;
    sp->style[4].cd = 41;
    sp->cursor_color.b = 0;
    sp->cursor_color.r = (u8)sp->blink_level * 4 + 96;
    sp->cursor_color.g = (u8)sp->blink_level * 4 + 48;
    if (area_info_list_shown != 0) {
        for (i = 0; i < 10; i++) {
            id = sp->state.map.ids[i];
            if (id >= 90) {
                if (id == 99)
                    ui_draw_text(MESSAGE(MSG_WORLD_UNKNOWN), 53, i * 16 + 54, &sp->style[0]);
                else if (id == sp->state.map.current)
                    ui_draw_text(MESSAGE(MSG_WORLD_NAMES - 90 + id), 53, i * 16 + 54, &sp->style[2]);
                else
                    ui_draw_text(MESSAGE(MSG_WORLD_NAMES - 90 + id), 53, i * 16 + 54, &sp->style[0]);
            } else if (id == 89) {
                ui_draw_text(MESSAGE(MSG_AREA_UNKNOWN), 67, i * 16 + 54, &sp->style[0]);
            } else if (id == area_state.area) {
                ui_draw_text(MESSAGE(MSG_AREA_NAMES + id), 67, i * 16 + 54, &sp->style[2]);
                ui_draw_number(game_progress.area_kills[sp->state.map.ids[i]] * 100 / total[sp->state.map.ids[i]] | DIGITS(3),
                              223, i * 16 + 54, &sp->style[6]);
                ui_draw_glyph(80, 256, i * 16 + 54, &sp->style[2]);
            } else {
                ui_draw_text(MESSAGE(MSG_AREA_NAMES + id), 67, i * 16 + 54, &sp->style[0]);
                ui_draw_number(game_progress.area_kills[sp->state.map.ids[i]] * 100 / total[sp->state.map.ids[i]] | DIGITS(3),
                              223, i * 16 + 54, &sp->style[4]);
                ui_draw_glyph(80, 256, i * 16 + 54, &sp->style[0]);
            }
        }
    } else {
        ui_draw_text(MESSAGE(MSG_AREA_INFO_LABELS), 72, 53, &sp->style[2]);
        for (i = 1; i < 5; i++)
            ui_draw_text(MESSAGE(MSG_AREA_INFO_LABELS + i), 72, i * 30 + 63, &sp->style[2]);
        if (sp->state.map.current != -1) {
            ui_draw_text(MESSAGE(MSG_WORLD_NAMES - 90 + sp->state.map.current), 86, 66, &sp->style[0]);
            ui_draw_text(MESSAGE(MSG_AREA_NAMES + area_state.area), name_x[area_state.area] + 100, 79,
                          &sp->style[0]);
        } else {
            ui_draw_text(MESSAGE(MSG_AREA_NAMES + area_state.area), 100, 73, &sp->style[0]);
        }
        ui_draw_number(viewer.cunes_found | DIGITS(3), 179, 107, &sp->style[4]);
        ui_draw_glyph(15, 213, 107, &sp->style[0]);
        ui_draw_number(99 | DIGITS(3), 221, 107, &sp->style[4]);
        ui_draw_number(viewer.items_found | DIGITS(3), 179, 138, &sp->style[4]);
        ui_draw_glyph(15, 213, 138, &sp->style[0]);
        ui_draw_number(265 | DIGITS(3), 221, 138, &sp->style[4]);
        ui_draw_number(viewer.creatures_killed | DIGITS(3), 179, 168, &sp->style[4]);
        ui_draw_glyph(15, 213, 168, &sp->style[0]);
        ui_draw_number(630 | DIGITS(3), 221, 168, &sp->style[4]);
        time = (u32)viewer.play_seconds / 60;
        minutes = time / 60;
        hours = minutes;
        minutes = time - hours * 60;
        ui_draw_number((u16)hours | DIGITS(2), 199, 198, &sp->style[4]);
        ui_draw_glyph(27, 224, 198, &sp->style[0]);
        ui_draw_number((u16)(minutes / 10) | DIGITS(1), 231, 198, &sp->style[4]);
        ui_draw_number((u16)(minutes % 10) | DIGITS(1), 241, 198, &sp->style[4]);
    }
    area_info_blink = (area_info_blink + 1) % 32;
    if (area_info_blink < 16)
        level = 8 - (u16)area_info_blink;
    else
        level = (u16)area_info_blink - 24;
    arrow.b = arrow.g = arrow.r = level * 4 + 96;
    if (area_info_list_shown != 0) {
        if (DAT_8008818c != 0) {
            sp->rect.x = 160;
            sp->rect.y = 43;
            sp->rect.w = 16;
            sp->rect.h = 8;
            ui_draw_arrow(&sp->rect, &sp->cursor_color, 3072);
        }
        if (DAT_8008818c != 25) {
            sp->rect.x = 160;
            sp->rect.y = 220;
            sp->rect.w = 16;
            sp->rect.h = 8;
            ui_draw_arrow(&sp->rect, &sp->cursor_color, 1024);
        }
    }
    sp->rect.x = 28;
    sp->rect.y = 48;
    sp->rect.w = 264;
    sp->rect.h = 168;
    sp->border_color.r = 80;
    sp->border_color.g = 88;
    sp->border_color.b = 96;
    if (area_info_list_shown != 0)
        ui_draw_arrow(&prev, &sp->cursor_color, 2048);
    else
        ui_draw_arrow(&next, &sp->cursor_color, 0);
    ui_add_draw_mode(1, 0, 0);
    ui_draw_rect_outline(&sp->rect, &sp->border_color, 257, 1);
    PAGE_END();
}

/*
 * The option page. `wash` is menu_draw_backdrop's {32, 32, 32}: GCC merged the
 * two constants.
 */
ADDRESS(0x8002b2e8, 0x564)
void menu_options_draw(void)
{
    CVECTOR black = {0};
    CVECTOR shadow = {24, 24, 24};
    CVECTOR light = {96, 96, 96};
    CVECTOR wash = {32, 32, 32};
    DVECTOR row[4] = {{48, 84}, {114, 84}, {32, 94}, {130, 94}};
    CVECTOR lit[4] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {128, 64, 0, 0}, {128, 64, 0, 0}};
    CVECTOR dim[4] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 64, 128, 0}, {0, 64, 128, 0}};
    u16 label_x[6] = {15, 12, 12, 19, 19, 5};
    MenuScratch *sp;
    u8 *on;
    int i;
    int y;

    gfx_begin_frame();
    y = 80;
    sp = MENU_SCRATCH;
    sp->style[0].r = 72;
    sp->style[0].g = 72;
    sp->style[0].b = 72;
    sp->style[2].r = 98;
    sp->style[2].g = 66;
    sp->style[2].b = 48;
    sp->style[3].r = 24;
    sp->style[3].g = 24;
    sp->style[3].b = 24;
    sp->style[3].cd = 57;
    sp->style[2].cd = 57;
    sp->style[0].cd = 57;
    sp->tex_pos.vx = 600;
    sp->tex_pos.vy = 416;
    sp->clut_pos.vx = 544;
    sp->clut_pos.vy = 511;
    sp->rect.y = 83;
    sp->rect.w = 5;
    sp->rect.h = 5;
    sp->rect.x = game_settings.sfx_volume * 83 / 63 + 180;
    ui_draw_image(&sp->rect, &sp->tex_pos, &sp->clut_pos, NULL, 0);
    sp->tex_pos.vx = 576;
    sp->rect.x = 176;
    sp->rect.y = 78;
    sp->rect.w = 96;
    sp->rect.h = 14;
    sp->tex_pos.vy = 416;
    ui_draw_image(&sp->rect, &sp->tex_pos, &sp->clut_pos, NULL, 1);
    for (i = 0; i < 6; y += 20, i++)
        ui_draw_text(MESSAGE(MSG_OPTION_LABELS + i), label_x[i] + 32, y, &sp->style[2]);
    for (on = &game_settings.walk_bob, i = 1; i < 5; on++, i++) {
        if (*on == 1) {
            ui_draw_text(MESSAGE(MSG_ON), 193, i * 20 + 80, &sp->style[2]);
            ui_draw_text(MESSAGE(MSG_OFF), 241, i * 20 + 80, &sp->style[3]);
        } else {
            ui_draw_text(MESSAGE(MSG_ON), 193, i * 20 + 80, &sp->style[3]);
            ui_draw_text(MESSAGE(MSG_OFF), 241, i * 20 + 80, &sp->style[2]);
        }
    }
    ui_draw_number_fixed(game_settings.sfx_volume | DIGITS(2), 280, 80, &sp->style[0]);
    ui_draw_text(MESSAGE(MSG_OPTION_HELP + sp->cursor), 64, 208, &sp->style[0]);
    sp->tex_pos.vx = 596;
    sp->tex_pos.vy = 448;
    sp->clut_pos.vx = 544;
    sp->clut_pos.vy = 511;
    sp->rect.x = 32;
    sp->rect.y = 44;
    sp->rect.w = 70;
    sp->rect.h = 16;
    ui_draw_image(&sp->rect, &sp->tex_pos, &sp->clut_pos, NULL, 3);
    for (i = 0; i < 6; i++) {
        if (i == sp->cursor)
            ui_draw_gouraud_quad(row, lit, 1);
        else
            ui_draw_gouraud_quad(row, dim, 1);
        row[0].vy += 20;
        row[1].vy += 20;
        row[2].vy += 20;
        row[3].vy += 20;
    }
    ui_add_draw_mode(0, 0, 0);
    PAGE_END();
}

/*
 * A settings page: two columns of six labels (lit while their pad_state.buttons
 * bit is set in the page flags), twelve level gauges and two buttons
 * below them.
 * The button masks are read through a pointer set before each column's
 * loop (retail walks it; indexing the array directly keeps the label
 * positions from being strength-reduced).
 */
ADDRESS(0x8002b84c, 0x804)
void menu_button_config_draw(void)
{
    DVECTOR row[4] = {{48, 84}, {114, 84}, {32, 94}, {130, 94}};
    CVECTOR lit[4] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {128, 64, 0, 0}, {128, 64, 0, 0}};
    CVECTOR dim[4] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 64, 128, 0}, {0, 64, 128, 0}};
    u16 label_x[14] = {26, 36, 20, 16, 20, 16, 27, 20, 17, 20, 34, 16, 10, 37};
    MenuScratch *sp;
    int i;
    u16 *mask;

    gfx_begin_frame();
    sp = MENU_SCRATCH;
    sp->style[0].b = sp->style[0].g = sp->style[0].r = 72;
    sp->style[2].r = 98;
    sp->style[2].g = 66;
    sp->style[2].b = 48;
    sp->style[3].r = 96;
    sp->style[3].b = sp->style[3].g = 32;
    sp->style[0].cd = sp->style[2].cd = sp->style[3].cd = 57;
    if (sp->blink_phase < 16)
        sp->blink_level = 8 - sp->blink_phase;
    else
        sp->blink_level = sp->blink_phase - 24;
    if (sp->scroll == 1) {
        sp->cursor_color.b = 0;
        lit[2].r += (u8)sp->blink_level * 8;
        lit[2].g += (u8)sp->blink_level * 4;
        lit[3].r += (u8)sp->blink_level * 8;
        lit[3].g += (u8)sp->blink_level * 4;
        sp->cursor_color.r = (u8)sp->blink_level * 4 + 96;
        sp->cursor_color.g = (u8)sp->blink_level * 2 + 48;
    } else {
        sp->cursor_color = sp->style[2];
    }
    mask = pad_state.buttons;
    for (i = 0; i < 6; i++) {
        if (mask[i] & sp->state.levels.flags)
            ui_draw_text(MESSAGE(MSG_BUTTON_FORWARD + i), label_x[i] + 32, i * 18 + 80, &sp->style[3]);
        else
            ui_draw_text(MESSAGE(MSG_BUTTON_FORWARD + i), label_x[i] + 32, i * 18 + 80, &sp->style[2]);
    }
    mask = &pad_state.buttons[PAD_ACTION_LOOK_UP];
    for (i = 0; i < 6; i++) {
        if (mask[i] & sp->state.levels.flags)
            ui_draw_text(MESSAGE(MSG_BUTTON_LOOK_UP + i), label_x[i + 6] + 168, i * 18 + 80, &sp->style[3]);
        else
            ui_draw_text(MESSAGE(MSG_BUTTON_LOOK_UP + i), label_x[i + 6] + 168, i * 18 + 80, &sp->style[2]);
    }
    sp->tex_pos.vy = 384;
    sp->clut_pos.vx = 544;
    for (i = 0; i < 12; i++) {
        sp->rect.x = i < 6 ? 138 : 274;
        sp->rect.y = i % 6 * 18 + 79;
        if (sp->state.levels.level[i] >= 8) {
            sp->tex_pos.vx = sp->state.levels.level[i] * 8 + 544;
            sp->rect.w = 32;
            sp->clut_pos.vy = 506;
            sp->rect.x -= 8;
        } else {
            sp->tex_pos.vx = sp->state.levels.level[i] * 4 + 576;
            sp->rect.w = 16;
            if (sp->state.levels.level[i] < 4)
                sp->clut_pos.vy = 510 - sp->state.levels.level[i];
            else
                sp->clut_pos.vy = 506;
        }
        sp->border_color.r = sp->border_color.g = sp->border_color.b = (u8)sp->blink_level * 4 - 128;
        if (i == sp->cursor && sp->scroll == 1)
            ui_draw_image(&sp->rect, &sp->tex_pos, &sp->clut_pos, &sp->border_color, 0);
        else
            ui_draw_image(&sp->rect, &sp->tex_pos, &sp->clut_pos, NULL, 0);
    }
    sp->tex_pos.vx = 576;
    sp->tex_pos.vy = 432;
    sp->clut_pos.vx = 544;
    sp->clut_pos.vy = 511;
    sp->rect.x = 32;
    sp->rect.y = 44;
    sp->rect.w = 60;
    sp->rect.h = 16;
    ui_draw_image(&sp->rect, &sp->tex_pos, &sp->clut_pos, NULL, 3);
    sp->tex_pos.vx = 590;
    sp->tex_pos.vy = 480;
    sp->clut_pos.vx = 544;
    sp->clut_pos.vy = 511;
    sp->rect.x = 96;
    sp->rect.y = 44;
    sp->rect.w = 120;
    sp->rect.h = 16;
    ui_draw_image(&sp->rect, &sp->tex_pos, &sp->clut_pos, NULL, 3);
    ui_draw_text(MESSAGE(MSG_STANDARD_SET), label_x[12] + 48, 204, &sp->style[2]);
    ui_draw_text(MESSAGE(MSG_DONE), label_x[13] + 152, 204, &sp->style[2]);
    for (i = 0; i < 12; i++) {
        if (i == 6) {
            row[0].vx = 184;
            row[1].vx = 250;
            row[2].vx = 168;
            row[0].vy = row[1].vy = 84;
            row[3].vx = 266;
            row[2].vy = row[3].vy = 94;
        }
        if (i == sp->cursor)
            ui_draw_gouraud_quad(row, lit, 1);
        else
            ui_draw_gouraud_quad(row, dim, 1);
        row[0].vy += 18;
        row[1].vy += 18;
        row[2].vy += 18;
        row[3].vy += 18;
    }
    row[0].vx = 64;
    row[1].vx = 130;
    row[0].vy = row[1].vy = 208;
    row[2].vx = 48;
    row[3].vx = 146;
    row[2].vy = row[3].vy = 218;
    if (sp->cursor == 12)
        ui_draw_gouraud_quad(row, lit, 1);
    else
        ui_draw_gouraud_quad(row, dim, 1);
    ui_add_draw_mode(0, 0, 0);
    row[0].vx = 168;
    row[1].vx = 234;
    row[0].vy = row[1].vy = 208;
    row[2].vx = 152;
    row[3].vx = 250;
    row[2].vy = row[3].vy = 218;
    if (sp->cursor == 13)
        ui_draw_gouraud_quad(row, lit, 1);
    else
        ui_draw_gouraud_quad(row, dim, 1);
    ui_add_draw_mode(0, 0, 0);
    PAGE_END();
}

/*
 * The first menu page bodies; card_screen_draw calls one
 * per page bit.
 */

ADDRESS(0x8002c050, 0x40)
void card_draw_checking(void)
{
    MenuScratch *sp = MENU_SCRATCH;

    ui_draw_text(MESSAGE(MSG_CARD_CHECKING), 32, 74, &sp->style[0]);
}

ADDRESS(0x8002c090, 0x114)
void card_draw_port_choice(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    short i;

    sp->cursor_rect.x = 94;
    sp->cursor_rect.w = 138;
    sp->cursor_rect.h = 17;
    if (sp->port == 0)
        sp->cursor_rect.y = 108;
    else
        sp->cursor_rect.y = 132;
    ui_draw_rect_outline(&sp->cursor_rect, &sp->cursor_color, 257, 1);
    ui_add_draw_mode(1, 0, 0);
    for (i = 0; i < 7; i++) {
        file_row_clipped[i] = 0;
        DAT_800881f0[i] = 0;
    }
    ui_draw_text(MESSAGE(MSG_CARD_SLOT_1), 102, 110, &sp->style[0]);
    ui_draw_text(MESSAGE(MSG_CARD_SLOT_2), 102, 134, &sp->style[0]);
}

ADDRESS(0x8002c1a4, 0x114)
void card_draw_format_prompt(void)
{
    MenuScratch *sp = MENU_SCRATCH;

    if (sp->state.half[1] == 0) {
        ui_draw_text(MESSAGE(MSG_CARD_UNFORMATTED), 32, 74, &sp->style[0]);
        if (sp->dialog_choice) {
            ui_draw_text(MESSAGE(MSG_CARD_YES), 121, 160, &sp->style[2]);
            ui_draw_text(MESSAGE(MSG_CARD_NO), 191, 160, &sp->style[0]);
        } else {
            ui_draw_text(MESSAGE(MSG_CARD_YES), 121, 160, &sp->style[0]);
            ui_draw_text(MESSAGE(MSG_CARD_NO), 191, 160, &sp->style[2]);
        }
    } else {
        ui_draw_text(MESSAGE(MSG_CARD_FORMATTING), 32, 74, &sp->style[0]);
        ui_draw_text(MESSAGE(MSG_CARD_DO_NOT_REMOVE), 40, 100, &sp->style[0]);
    }
}

/*
 * The record list page: a two-choice header while the first state half is
 * 1, scroll arrows, the cursor bar and three rows of record number, title,
 * kind name and figure. Each row's glyph overflow count goes to
 * file_row_clipped.
 */
ADDRESS(0x8002c2b8, 0x604)
void card_draw_file_list(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    int titles[48] = {
        -1, 0, 5, 2, 0, 0, 0, -1, 4, 4, 3, 1, 1, 4, 6, 1,
        -1, 5, 2, -1, 2, 3, -1, 6, 2, 6, 6, 1, 1, -1, -1, -1,
        3, 1, -1, -1, -1, 3, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    };
    RECT unused;
    int id;
    int n, i;

    sp->border_color.r = sp->border_color.g = sp->border_color.b = 64;
    if (sp->state.half[1] == 1) {
        id = (sp->mode & 0xff) ? MSG_CARD_LOAD_PROMPT : MSG_CARD_SAVE_PROMPT;
        ui_draw_text(MESSAGE(id), 68, 128, &sp->style[0]);
        sp->style[0].b = sp->style[0].g = sp->style[0].r = 24;
        if (sp->dialog_choice) {
            ui_draw_text(MESSAGE(MSG_CARD_YES), 120, 150, &sp->style[2]);
            ui_draw_text(MESSAGE(MSG_CARD_NO), 190, 150, &sp->style[0]);
        } else {
            ui_draw_text(MESSAGE(MSG_CARD_YES), 120, 150, &sp->style[0]);
            ui_draw_text(MESSAGE(MSG_CARD_NO), 190, 150, &sp->style[2]);
        }
        sp->style[0].b = sp->style[0].g = sp->style[0].r = 72;
        sp->rect.x = 60;
        sp->rect.y = 120;
        sp->rect.w = 200;
        sp->rect.h = 50;
        sp->inner_rect.x = 61;
        sp->inner_rect.y = 121;
        sp->inner_rect.w = 198;
        sp->inner_rect.h = 48;
        sp->fill_color.r = sp->fill_color.g = sp->fill_color.b = 32;
        ui_fill_rect(&sp->inner_rect, &sp->fill_color, 1);
        ui_add_draw_mode(2, 0, 0);
        ui_fill_rect(&sp->rect, &sp->border_color, 0);
    }
    if (sp->row_count >= 4) {
        if (sp->first_row > 0) {
            sp->rect.x = 156;
            sp->rect.y = 68;
            sp->rect.w = 16;
            sp->rect.h = 8;
            ui_draw_arrow(&sp->rect, &sp->cursor_color, 3072);
            ui_add_draw_mode(1, 0, 0);
        }
        if (sp->first_row < sp->row_count - 3) {
            sp->rect.x = 156;
            sp->rect.y = 215;
            sp->rect.w = 16;
            sp->rect.h = 8;
            ui_draw_arrow(&sp->rect, &sp->cursor_color, 1024);
            ui_add_draw_mode(1, 0, 0);
        }
    }
    sp->cursor_rect.x = 32;
    sp->cursor_rect.w = 256;
    sp->cursor_rect.h = 40;
    sp->cursor_rect.y = sp->file_row * 48 + 74;
    ui_draw_rect_outline(&sp->cursor_rect, &sp->cursor_color, 257, 1);
    n = 3;
    ui_add_draw_mode(1, 0, 0);
    sp->cursor_rect.y = 74;
    if (sp->row_count < 3)
        n = sp->row_count;
    for (i = 0; i < n; i++) {
        if (sp->has_new_row == 1 && i + sp->first_row == sp->row_count - 1) {
            ui_draw_text(MESSAGE(MSG_CARD_NEW_DATA), 64, sp->cursor_rect.y + 6, &sp->style[0]);
        } else {
            ui_draw_number_fixed((u16)(sp->first_row + i + 1) | DIGITS(2), 38, sp->cursor_rect.y + 6,
                          &sp->style[0]);
            ui_draw_text(MESSAGE(MSG_CARD_SP), 218, sp->cursor_rect.y + 6, &sp->style[0]);
            text_scroll_mode = 1;
            text_clip_right = 280;
            text_clip_on = 0;
            text_clip_left = 0;
            if (titles[sp->card->files[i + sp->first_row].area] != -1)
                ui_draw_text(MESSAGE(MSG_WORLD_NAMES + titles[sp->card->files[i + sp->first_row].area]), 64,
                              sp->cursor_rect.y + 6, &sp->style[0]);
            if (titles[sp->card->files[i + sp->first_row].area] != -1)
                ui_draw_text(MESSAGE(MSG_AREA_NAMES + sp->card->files[i + sp->first_row].area), 64,
                              sp->cursor_rect.y + 22, &sp->style[0]);
            else
                ui_draw_text(MESSAGE(MSG_AREA_NAMES + sp->card->files[i + sp->first_row].area), 64,
                              sp->cursor_rect.y + 6, &sp->style[0]);
            ui_draw_number_fixed(sp->card->files[i + sp->first_row].sp_total | DIGITS(4), 248, sp->cursor_rect.y + 6,
                          &sp->style[0]);
        }
        file_row_clipped[i + sp->first_row] = text_clip_dropped;
        ui_draw_rect_outline(&sp->cursor_rect, &sp->border_color, 257, 1);
        sp->cursor_rect.y += 48;
    }
    ui_add_draw_mode(1, 0, 0);
}

/*
 * The remaining menu page bodies, the page frame that dispatches
 * them by page bit, and three two-choice dialogs.
 */

ADDRESS(0x8002c8bc, 0x9c)
void card_draw_transfer(void)
{
    MenuScratch *sp = MENU_SCRATCH;

    if (sp->mode & 0xff)
        ui_draw_text(MESSAGE(MSG_CARD_LOADING), 32, 74, &sp->style[0]);
    else
        ui_draw_text(MESSAGE(MSG_CARD_SAVING), 32, 74, &sp->style[0]);
    ui_draw_text(MESSAGE(MSG_CARD_DO_NOT_REMOVE), 40, 100, &sp->style[0]);
}

ADDRESS(0x8002c958, 0x40)
void card_draw_finished(void)
{
    MenuScratch *sp = MENU_SCRATCH;

    ui_draw_text(MESSAGE(MSG_CARD_FINISHED), 32, 74, &sp->style[0]);
}

ADDRESS(0x8002c998, 0x50)
void card_draw_message(void)
{
    MenuScratch *sp = MENU_SCRATCH;

    ui_draw_text(MESSAGE(sp->state.half[0]), 60, 114, &sp->style[0]);
}

ADDRESS(0x8002c9e8, 0x9c)
void menu_draw_background(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    int n;

    sp->image_rect.w = 128;
    sp->image_rect.h = 240;
    sp->tex_pos.vx = 768;
    sp->image_rect.y = 0;
    sp->image_rect.x = 0;
    sp->tex_pos.vy = 256;
    n = 5;
    while (n--) {
        ui_draw_image(&sp->image_rect, &sp->tex_pos, 0, 0, 16);
        sp->image_rect.x += 128;
    }
}

ADDRESS(0x8002ca84, 0x314)
void card_screen_draw(void)
{
    MenuScratch *sp = MENU_SCRATCH;

    gfx_begin_frame();
    sp->style[0].b = sp->style[0].g = sp->style[0].r = 72;
    sp->style[2].r = 98;
    sp->style[2].g = 66;
    sp->style[2].b = 48;
    sp->style[3].r = 96;
    sp->style[3].b = sp->style[3].g = 32;
    sp->style[0].cd = sp->style[2].cd = sp->style[3].cd = 57;
    if (sp->blink_phase < 16)
        sp->blink_level = 8 - sp->blink_phase;
    else
        sp->blink_level = sp->blink_phase - 24;
    sp->tex_pos.vx = 576;
    sp->tex_pos.vy = 448;
    sp->clut_pos.vx = 544;
    sp->clut_pos.vy = 511;
    sp->rect.x = 81;
    sp->rect.y = 45;
    sp->rect.w = 40;
    sp->rect.h = 16;
    sp->cursor_color.b = 0;
    sp->cursor_color.r = (u8)sp->blink_level * 4 + 96;
    sp->cursor_color.g = (u8)sp->blink_level * 2 + 48;
    ui_draw_image(&sp->rect, &sp->tex_pos, &sp->clut_pos, 0, 3);
    sp->rect.y = 44;
    if (sp->mode & 0xff)
        sp->tex_pos.vx = 586;
    else
        sp->tex_pos.vx = 613;
    sp->rect.x = 32;
    sp->rect.w = 40;
    ui_draw_image(&sp->rect, &sp->tex_pos, &sp->clut_pos, 0, 3);
    switch (sp->mode) {
    case 0x2:
    case 0x200:
        card_draw_port_choice();
        break;
    case 0x4:
    case 0x400:
        card_draw_checking();
        break;
    case 0x8:
    case 0x800:
        card_draw_format_prompt();
        break;
    case 0x10:
    case 0x1000:
        card_draw_file_list();
        break;
    case 0x20:
    case 0x2000:
        card_draw_transfer();
        break;
    case 0x40:
    case 0x4000:
        card_draw_finished();
        break;
    case 0x80:
    case 0x8000:
        card_draw_message();
        break;
    }
    ui_draw_panel(16, 36, 288, 192);
    gfx_flip();
    if (sp->show_backdrop == 1)
        menu_draw_backdrop();
    gfx_draw_ot();
}

ADDRESS(0x8002cd98, 0xec)
void menu_end_game_draw_dialog(void)
{
    MenuScratch *sp = MENU_SCRATCH;

    ui_draw_text(MESSAGE(MSG_END_GAME_PROMPT), 76, 92, &sp->style[0]);
    sp->style[0].b = sp->style[0].g = sp->style[0].r = 24;
    if (sp->cursor == 0) {
        ui_draw_text(MESSAGE(MSG_END_GAME_YES), 121, 136, &sp->style[2]);
        ui_draw_text(MESSAGE(MSG_END_GAME_NO), 189, 136, &sp->style[0]);
    } else {
        ui_draw_text(MESSAGE(MSG_END_GAME_YES), 121, 136, &sp->style[0]);
        ui_draw_text(MESSAGE(MSG_END_GAME_NO), 189, 136, &sp->style[2]);
    }
    ui_draw_window(40, 80, 240, 80);
}

ADDRESS(0x8002ce84, 0x74)
void menu_end_game_draw(void)
{
    MenuScratch *sp = MENU_SCRATCH;

    gfx_begin_frame();
    sp->style[0].b = sp->style[0].g = sp->style[0].r = 96;
    sp->style[2].r = 98;
    sp->style[2].g = 66;
    sp->style[2].b = 48;
    sp->style[0].cd = sp->style[2].cd = 57;
    menu_end_game_draw_dialog();
    gfx_flip();
    menu_draw_backdrop();
    gfx_draw_ot();
}

ADDRESS(0x8002cef8, 0x1b8)
void menu_game_over_draw(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    short x[2] = {16, 10};

    gfx_begin_frame();
    sp->style[0].b = sp->style[0].g = sp->style[0].r = 96;
    sp->style[0].cd = 49;
    if (sp->blink_phase < 16)
        sp->blink_level = 8 - sp->blink_phase;
    else
        sp->blink_level = sp->blink_phase - 24;
    sp->cursor_color.r = (u8)sp->blink_level * 4 + 96;
    sp->cursor_color.g = (u8)sp->blink_level * 2 + 48;
    sp->cursor_color.b = 0;
    sp->cursor_rect.x = 93;
    sp->cursor_rect.y = sp->state.word * 24 + 118;
    sp->cursor_rect.w = 134;
    sp->cursor_rect.h = 16;
    ui_draw_rect_outline(&sp->cursor_rect, &sp->cursor_color, 257, 1);
    ui_add_draw_mode(1, 0, 0);
    ui_draw_text(MESSAGE(MSG_NEW_GAME), x[0] + 97, 120, &sp->style[0]);
    ui_draw_text(MESSAGE(MSG_LOAD_GAME), x[1] + 97, 144, &sp->style[0]);
    ui_draw_window(72, 100, 176, 80);
    gfx_flip();
    ClearImage(&world_state.draw[world_state.frame].clip, 0, 0, 0);
    gfx_draw_ot();
}

ADDRESS(0x8002d0b0, 0x12c)
void menu_save_prompt_draw(void)
{
    MenuScratch *sp = MENU_SCRATCH;

    gfx_begin_frame();
    sp->style[0].b = sp->style[0].g = sp->style[0].r = 96;
    sp->style[2].r = 98;
    sp->style[2].g = 66;
    sp->style[2].b = 48;
    sp->style[0].cd = sp->style[2].cd = 57;
    ui_draw_text(MESSAGE(MSG_SAVE_PROMPT), 92, 120, &sp->style[0]);
    if (sp->main_row == 0) {
        ui_draw_text(MESSAGE(MSG_SAVE_YES), 127, 152, &sp->style[2]);
        ui_draw_text(MESSAGE(MSG_SAVE_NO), 183, 152, &sp->style[0]);
    } else {
        ui_draw_text(MESSAGE(MSG_SAVE_YES), 127, 152, &sp->style[0]);
        ui_draw_text(MESSAGE(MSG_SAVE_NO), 183, 152, &sp->style[2]);
    }
    ui_draw_window(72, 100, 176, 80);
    gfx_flip();
    gfx_draw_ot();
}

/*
 * The header tabs of the item list screens for page bits 0x1, 0x2, 0x4 and
 * 0x8, the window, and the darkened screen while bit 0x1 is set.
 */
ADDRESS(0x8002d1dc, 0x398)
void shop_draw_header(void)
{
    MenuScratch *sp = MENU_SCRATCH;

    sp->rect.x = 34;
    sp->rect.y = 24;
    sp->rect.h = 16;
    sp->clut_pos.vx = 544;
    sp->clut_pos.vy = 511;
    if (sp->mode & 0x1) {
        sp->rect.w = 30;
        sp->tex_pos.vx = 631;
        sp->tex_pos.vy = 464;
        ui_draw_image(&sp->rect, &sp->tex_pos, &sp->clut_pos, NULL, 3);
        sp->rect.w = 35;
        sp->tex_pos.vx = 624;
        sp->tex_pos.vy = 448;
        sp->rect.x += 37;
        ui_draw_image(&sp->rect, &sp->tex_pos, &sp->clut_pos, NULL, 3);
    } else if (sp->mode & 0x2) {
        sp->rect.w = 90;
        sp->tex_pos.vx = 588;
        sp->tex_pos.vy = 464;
        ui_draw_image(&sp->rect, &sp->tex_pos, &sp->clut_pos, NULL, 3);
        sp->rect.x = 153;
        sp->rect.w = 54;
        sp->tex_pos.vx = 617;
        sp->tex_pos.vy = 464;
        ui_draw_image(&sp->rect, &sp->tex_pos, &sp->clut_pos, NULL, 3);
        sp->rect.x = 131;
        sp->rect.w = 20;
        sp->tex_pos.vx = 612;
        sp->tex_pos.vy = 464;
        ui_draw_image(&sp->rect, &sp->tex_pos, &sp->clut_pos, NULL, 3);
    } else if (sp->mode & 0x4) {
        sp->rect.w = 90;
        sp->tex_pos.vx = 588;
        sp->tex_pos.vy = 464;
        ui_draw_image(&sp->rect, &sp->tex_pos, &sp->clut_pos, NULL, 3);
        sp->rect.x = 153;
        sp->rect.w = 54;
        sp->tex_pos.vx = 617;
        sp->tex_pos.vy = 464;
        ui_draw_image(&sp->rect, &sp->tex_pos, &sp->clut_pos, NULL, 3);
        sp->rect.x = 130;
        sp->rect.w = 20;
        sp->tex_pos.vx = 591;
        sp->tex_pos.vy = 432;
        ui_draw_image(&sp->rect, &sp->tex_pos, &sp->clut_pos, NULL, 3);
    } else if (sp->mode & 0x8) {
        sp->rect.w = 60;
        sp->tex_pos.vx = 576;
        sp->tex_pos.vy = 480;
        ui_draw_image(&sp->rect, &sp->tex_pos, &sp->clut_pos, NULL, 3);
        sp->rect.w = 36;
        sp->tex_pos.vx = 624;
        sp->tex_pos.vy = 448;
        sp->rect.x += 63;
        ui_draw_image(&sp->rect, &sp->tex_pos, &sp->clut_pos, NULL, 3);
    }
    ui_draw_window(16, 16, 288, 208);
    if (sp->mode & 0x1) {
        RECT screen = {0, 0, 320, 240};
        /* Four initializers: GCC keeps it apart from menu_draw_backdrop's {32, 32, 32}. */
        CVECTOR shade = {32, 32, 32, 0};

        ui_fill_rect(&screen, &shade, 1);
        ui_add_draw_mode(2, 0, 0);
    }
}

/*
 * The item list screens: a cursor bar, three list
 * layouts chosen by the page bits (with carried counts, with prices, plain),
 * the selected item's detail panel and the screen frame that picks them.
 */

/* The list rows shown at once. */
#define LIST_ROWS 5

#define LIST_TEXT(sp, id) \
    ((u16 *)((sp)->state.list.messages->text + (sp)->state.list.messages->offsets[id]))

ADDRESS(0x8002d574, 0x98)
void shop_draw_cursor(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    int x, w;

    if (sp->mode & 0x6) {
        x = 54;
        sp->cursor_rect.w = 218;
    } else {
        x = 28;
        sp->cursor_rect.w = 272;
    }
    sp->cursor_rect.x = x - 4;
    sp->cursor_rect.h = 34;
    sp->cursor_rect.y = sp->cursor * 36 + 44;
    ui_draw_rect_outline(&sp->cursor_rect, &sp->cursor_color, 257, 1);
    ui_add_draw_mode(1, 0, 0);
}

ADDRESS(0x8002d60c, 0x2dc)
void shop_draw_buy_list(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    int rows;
    int i;
    int id;
    CVECTOR *text, *figure;

    rows = LIST_ROWS;
    if (sp->state.list.count < LIST_ROWS + 1)
        rows = sp->state.list.count;
    ui_draw_number(inventory.counts[266].count | 0x3000000, 244, 27, &sp->style[6]);
    ui_draw_text(MESSAGE(MSG_SHOP_CUNES), 160, 27, &sp->style[2]);
    for (i = 0; i < rows; i++) {
        id = inventory.shop_ids[sp->state.list.ids[sp->scroll + i]];
        if (inventory.counts[266].count >= vis_table_c.types[id].price) {
            text = &sp->style[0];
            figure = &sp->style[4];
        } else {
            text = &sp->style[3];
            figure = &sp->style[7];
        }
        sp->style[0].cd |= 0x80;
        sp->style[3].cd |= 0x80;
        ui_draw_text(LIST_TEXT(sp, id), 28, i * 36 + 48, text);
        sp->style[0].cd &= 0x7f;
        sp->style[3].cd &= 0x7f;
        if (vis_table_c.types[id].max_durability) {
            ui_draw_text(MESSAGE(MSG_DURABILITY), 72, i * 36 + 62, text);
            ui_draw_number(vis_table_c.types[id].durability | 0x3000000, 128, i * 36 + 62, figure);
            ui_draw_glyph(74, 160, i * 36 + 62, text);
            ui_draw_number(vis_table_c.types[id].max_durability | 0x3000000, 172, i * 36 + 62, figure);
        }
        ui_draw_number(vis_table_c.types[id].price | 0x3000000, 244, i * 36 + 62, figure);
        ui_draw_text(MESSAGE(MSG_CUNES_SIGN), 216, i * 36 + 62, text);
    }
}

ADDRESS(0x8002d8e8, 0x1b8)
void shop_draw_trade_list(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    int rows;
    int i;
    int id;

    rows = LIST_ROWS;
    if (sp->state.list.count < LIST_ROWS + 1)
        rows = sp->state.list.count;
    for (i = 0; i < rows; i++) {
        id = sp->state.list.ids[sp->scroll + i];
        sp->style[0].cd |= 0x80;
        ui_draw_text(LIST_TEXT(sp, id), 54, i * 36 + 48, &sp->style[0]);
        sp->style[0].cd &= 0x7f;
        if (vis_table_c.types[id].max_durability) {
            ui_draw_text(MESSAGE(MSG_DURABILITY), 131, i * 36 + 62, &sp->style[0]);
            ui_draw_number(vis_table_c.types[id].durability | 0x3000000, 187, i * 36 + 62,
                          &sp->style[4]);
            ui_draw_glyph(74, 219, i * 36 + 62, &sp->style[0]);
            ui_draw_number(vis_table_c.types[id].max_durability | 0x3000000, 231, i * 36 + 62,
                          &sp->style[4]);
        }
    }
}

ADDRESS(0x8002daa0, 0x2d0)
void shop_draw_repair_list(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    int rows;
    int i;
    int id;
    int price;
    CVECTOR *text, *figure;

    ui_draw_number(viewer.hp | 0x4000000, 244, 27, &sp->style[6]);
    ui_draw_text(MESSAGE(MSG_SHOP_HP_LEFT), 176, 27, &sp->style[2]);
    rows = LIST_ROWS;
    if (sp->state.list.count < LIST_ROWS + 1)
        rows = sp->state.list.count;
    for (i = 0; i < rows; i++) {
        id = sp->state.list.ids[sp->scroll + i];
        price = vis_table_c.types[id].repair_price * 2 / 3 *
                (vis_table_c.types[id].max_durability - vis_table_c.types[id].durability);
        if (vis_table_c.types[id].durability == 0)
            price *= 2;
        if (price < viewer.hp) {
            text = &sp->style[0];
            figure = &sp->style[4];
        } else {
            text = &sp->style[3];
            figure = &sp->style[7];
        }
        sp->style[0].cd |= 0x80;
        sp->style[3].cd |= 0x80;
        ui_draw_text(LIST_TEXT(sp, id), 28, i * 36 + 48, text);
        sp->style[0].cd &= 0x7f;
        sp->style[3].cd &= 0x7f;
        if (vis_table_c.types[id].max_durability) {
            ui_draw_text(MESSAGE(MSG_DURABILITY), 77, i * 36 + 62, text);
            ui_draw_number(vis_table_c.types[id].durability | 0x3000000, 133, i * 36 + 62, figure);
            ui_draw_glyph(74, 165, i * 36 + 62, text);
            ui_draw_number(vis_table_c.types[id].max_durability | 0x3000000, 177, i * 36 + 62, figure);
        }
        ui_draw_number((u16)price | 0x4000000, 244, i * 36 + 62, figure);
    }
}

ADDRESS(0x8002dd70, 0x5a0)
void shop_draw_confirm(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    short id;
    int price;
    int i;
    int y;
    int flag;

    flag = sp->state.list.confirm_choice;
    if (flag == 0)
        sp->rect.x = 167;
    else
        sp->rect.x = 105;
    sp->rect.y = 206;
    sp->rect.w = 50;
    sp->rect.h = 16;
    ui_draw_rect_outline(&sp->rect, &sp->cursor_color, 257, 1);
    ui_add_draw_mode(1, 0, 0);
    if (sp->mode & 0x1) {
        id = inventory.shop_ids[sp->state.list.ids[sp->scroll + sp->cursor]];
        ui_draw_text(MESSAGE(MSG_SHOP_BUY), 120, 208, &sp->style[0]);
    } else if (sp->mode & 0x8) {
        id = sp->state.list.ids[sp->scroll + sp->cursor];
        ui_draw_text(MESSAGE(MSG_SHOP_REPAIR), 110, 208, &sp->style[0]);
    } else {
        id = sp->state.list.ids[sp->scroll + sp->cursor];
        ui_draw_text(MESSAGE(MSG_SHOP_TRADE), 113, 208, &sp->style[0]);
    }
    ui_draw_text(MESSAGE(MSG_SHOP_CANCEL), 172, 208, &sp->style[0]);
    sp->style[0].cd |= 0x80;
    ui_draw_text(LIST_TEXT(sp, id), 24, 176, &sp->style[0]);
    sp->style[0].cd &= 0x7f;
    if (vis_table_c.types[id].max_durability) {
        ui_draw_text(MESSAGE(MSG_DURABILITY), 48, 190, &sp->style[0]);
        ui_draw_number(vis_table_c.types[id].durability | 0x3000000, 104, 190, &sp->style[4]);
        ui_draw_glyph(74, 136, 190, &sp->style[0]);
        ui_draw_number(vis_table_c.types[id].max_durability | 0x3000000, 148, 190, &sp->style[4]);
    }
    if (sp->mode & 0x1) {
        ui_draw_number(vis_table_c.types[id].price | 0x3000000, 244, 190, &sp->style[4]);
        ui_draw_text(MESSAGE(MSG_CUNES_SIGN), 216, 190, &sp->style[0]);
    } else if (sp->mode & 0x8) {
        price = vis_table_c.types[id].repair_price * 2 / 3 *
                (vis_table_c.types[id].max_durability - vis_table_c.types[id].durability);
        if (vis_table_c.types[id].durability == 0)
            price *= 2;
        ui_draw_number((u16)price | 0x4000000, 244, 190, &sp->style[4]);
    }
    ui_draw_text(MESSAGE(MSG_ATTRIBUTE_COLUMNS), 165, 46, &sp->style[0]);
    y = 46;
    ui_draw_text(MESSAGE(MSG_ATTRIBUTE_COLUMNS), 165, y, &sp->style[0]);
    for (i = 0; i < 7; i++) {
        ui_draw_number_fixed(vis_table_c.types[id].attributes[i] | 0x13000000, 207, y, &sp->style[4]);
        ui_draw_number_fixed(vis_table_c.types[id].attributes[i + 7] | 0x13000000, 273, y, &sp->style[4]);
        y += 14;
    }
    if (vis_table_c.types[id].weight) {
        menu_figure_whole = vis_table_c.types[id].weight * 100 / 454;
        DAT_80088188 = vis_table_c.types[id].weight * 1000 / 454;
        ui_draw_text(MESSAGE(MSG_WEIGHT), 220, 156, &sp->style[0]);
        ui_draw_number_fixed(menu_figure_whole | 0x3000000, 248, 156, &sp->style[4]);
        ui_draw_glyph(78, 269, 156, &sp->style[0]);
        ui_draw_number_fixed((u16)(DAT_80088188 - menu_figure_whole * 10) | 0x1000000, 275, 156,
                      &sp->style[4]);
        ui_draw_glyph(108, 281, 156, &sp->style[0]);
    }
    sp->image_rect.x = 20;
    sp->image_rect.y = 40;
    sp->image_rect.w = 128;
    sp->image_rect.h = 128;
    sp->tex_pos.vx = 448;
    sp->tex_pos.vy = 256;
    ui_draw_image(&sp->image_rect, &sp->tex_pos, 0, 0, 16);
}

ADDRESS(0x8002e310, 0x180)
void shop_draw(void)
{
    MenuScratch *sp = MENU_SCRATCH;

    gfx_begin_frame();
    sp->style[3].r = sp->style[7].r = 96;
    sp->style[3].g = sp->style[7].g = sp->style[3].b = sp->style[7].b = 48;
    sp->style[0].b = sp->style[4].b = sp->style[0].g = sp->style[4].g = sp->style[0].r =
        sp->style[4].r = 96;
    sp->style[2].r = sp->style[6].r = 98;
    sp->style[2].g = sp->style[6].g = 66;
    sp->style[2].b = sp->style[6].b = 48;
    sp->style[0].cd = sp->style[2].cd = sp->style[3].cd = 57;
    sp->style[4].cd = sp->style[6].cd = sp->style[7].cd = 41;
    if (sp->blink_phase < 16)
        sp->blink_level = 8 - sp->blink_phase;
    else
        sp->blink_level = sp->blink_phase - 24;
    sp->cursor_color.b = 0;
    sp->cursor_color.r = (u8)sp->blink_level * 4 + 96;
    sp->cursor_color.g = (u8)sp->blink_level * 2 + 48;
    if (!(sp->mode & 0x8000))
        shop_draw_cursor();
    if (sp->mode & 0x8000)
        shop_draw_confirm();
    else if (sp->mode & 0x1)
        shop_draw_buy_list();
    else if (sp->mode & 0x8)
        shop_draw_repair_list();
    else
        shop_draw_trade_list();
    shop_draw_header();
    gfx_flip();
    menu_draw_backdrop();
    gfx_draw_ot();
}

/*
 * The status screens: the two window frames, the
 * equipment slot grid and the attribute table with its bars.
 */

/* Attribute rows. */
#define STATUS_ROWS 14

ADDRESS(0x8002e490, 0x40)
void menu_equip_info_draw_panels(void)
{
    ui_draw_panel(8, 8, 300, 224);
    ui_draw_panel(308, 8, 324, 224);
}

/*
 * Draws the equipment slots from their VRAM images, highlights the slot
 * under the cursor and, while +0x2c6 is 1, prints each equipped item's
 * +0x28 figure with a shadow.
 */
ADDRESS(0x8002e4d0, 0x47c)
void menu_equip_info_draw_slots(void)
{
    MenuScratch *sp;
    RECT rects[EQUIP_SLOTS] = {
        {14, 11, 56, 28},   {14, 40, 56, 28},   {14, 69, 56, 28},   {14, 98, 92, 73},
        {14, 172, 56, 28},  {14, 201, 56, 28},  {72, 15, 48, 24},   {130, 11, 56, 28},
        {130, 40, 56, 28},  {108, 69, 100, 65}, {108, 135, 100, 65}, {130, 201, 56, 28},
        {196, 15, 48, 24},  {246, 11, 56, 28},  {246, 40, 56, 28},  {246, 69, 56, 28},
        {210, 98, 92, 73},  {246, 172, 56, 28}, {246, 201, 56, 28}, {72, 201, 56, 28},
        {188, 201, 56, 28},
    };
    DVECTOR tex[EQUIP_SLOTS] = {
        {740, 152}, {796, 152}, {852, 152}, {932, 0},   {908, 152}, {964, 152}, {740, 124},
        {740, 180}, {796, 180}, {640, 124}, {640, 189}, {852, 180}, {788, 124}, {908, 180},
        {964, 180}, {852, 152}, {932, 73},  {796, 208}, {852, 208}, {908, 208}, {964, 208},
    };
    DVECTOR clut = {0, 511};
    CVECTOR unused = {0};
    RECT unused2;
    int i, slot, mode;
    u16 *equip;

    sp = MENU_SCRATCH;
    if (sp->cursor < 10)
        slot = sp->cursor;
    else
        slot = sp->cursor - 1;
    sp->cursor_rect.x = rects[slot].x - 2;
    sp->cursor_rect.y = rects[slot].y - 1;
    sp->cursor_rect.w = rects[slot].w + 4;
    sp->cursor_rect.h = rects[slot].h + 2;
    ui_draw_rect_outline(&sp->cursor_rect, &sp->cursor_color, 513, 1);
    ui_add_draw_mode(1, 0, 0);
    sp->style[7].r = 192;
    sp->style[7].g = 158;
    sp->style[7].b = 96;
    sp->style[7].cd = 48;
    equip = viewer.equipment;
    for (i = 0; i < EQUIP_SLOTS; equip++, i++) {
        if (i == 2)
            mode = 48;
        else if ((i == 3 && vis_table_c.types[viewer.equipment[EQUIP_SLOT_LEFT_HAND]].category == 9) ||
                 (i == 4 && vis_table_c.types[viewer.equipment[EQUIP_SLOT_LEFT_RESERVE]].category == 9))
            mode = 48;
        else
            mode = 16;
        if (i != 6 && i != 12 && sp->show_item_values == 1 && *equip != 0xffff &&
            vis_table_c.types[*equip].max_durability) {
            sp->style[7].r = 200;
            sp->style[7].g = 164;
            sp->style[7].b = 96;
            ui_draw_number(vis_table_c.types[*equip].durability | 0x2000000, rects[i].x + 8,
                          rects[i].y + 4, &sp->style[7]);
            sp->style[7].g = sp->style[7].r = 8;
            sp->style[7].b = 0;
            ui_draw_number(vis_table_c.types[*equip].durability | 0x2000000, rects[i].x + 9,
                          rects[i].y + 5, &sp->style[7]);
        }
        ui_draw_image(&rects[i], &tex[i], &clut, 0, mode);
    }
}

/*
 * The attribute table: names, current values (with the change a candidate
 * item would make on page 2) and bars scaled to +0x2c4.
 */
ADDRESS(0x8002e94c, 0x44c)
void menu_equip_info_draw_attributes(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    CVECTOR bright = {128, 128, 32, 0};
    CVECTOR dark = {0};
    RECT unused;
    RECT bar;
    short xoff[STATUS_ROWS] = {0};
    int i;
    u8 *count;
    s16 *figure;

    sp->style[2].r = 40;
    sp->style[3].r = 120;
    sp->style[2].g = 120;
    sp->style[3].g = 60;
    sp->style[2].b = 40;
    sp->style[3].b = 40;
    sp->style[3].cd = 73;
    sp->style[2].cd = 73;
    sp->style[6].r = 40;
    sp->style[7].r = 120;
    sp->style[6].g = 120;
    sp->style[7].g = 60;
    sp->style[6].b = 40;
    sp->style[7].b = 40;
    sp->style[7].cd = 49;
    sp->style[6].cd = 49;
    if (sp->mode == 1) {
        sp->value_style = sp->increase_style = sp->decrease_style = &sp->style[0];
    } else {
        sp->value_style = &sp->style[0];
        sp->increase_style = &sp->style[2];
        sp->decrease_style = &sp->style[3];
    }
    for (i = 0; i < STATUS_ROWS; i++)
        ui_draw_text(MESSAGE(MSG_HI_ATTRIBUTES + i), xoff[i] + 316, i * 15 + 16, sp->value_style);
    if (sp->mode == 1) {
        sp->value_style = sp->increase_style = sp->decrease_style = &sp->style[4];
    } else {
        sp->value_style = &sp->style[4];
        sp->increase_style = &sp->style[6];
        sp->decrease_style = &sp->style[7];
    }
    if (sp->mode == 2 && sp->show_item_values) {
        if (sp->choice_index != -1) {
            count = vis_table_c.types[sp->choice_id].attributes;
            for (i = 0; i < STATUS_ROWS; count++, i++)
                ui_draw_number(*count | 0x13000000, 590, i * 15 + 16, sp->value_style);
        }
        return;
    }
    figure = viewer.attributes;
    for (i = 0; i < STATUS_ROWS; figure++, i++) {
        if (sp->mode == 2) {
            if (sp->bonus_changes[i] > 0) {
                ui_draw_number(sp->attributes_clamped[i] | 0x3000000, 590, i * 15 + 16, sp->value_style);
                ui_draw_glyph(19, 544, i * 15 + 16, sp->increase_style);
                ui_draw_number(sp->bonus_changes[i] | 0x3000000, 508, i * 15 + 16, sp->increase_style);
            } else if (sp->bonus_changes[i] < 0) {
                ui_draw_number(sp->attributes_clamped[i] | 0x3000000, 590, i * 15 + 16, sp->value_style);
                ui_draw_glyph(20, 544, i * 15 + 16, sp->decrease_style);
                ui_draw_number(-sp->bonus_changes[i] | 0x3000000, 508, i * 15 + 16, sp->decrease_style);
            } else {
                ui_draw_number(sp->attributes_clamped[i] | 0x3000000, 590, i * 15 + 16, sp->value_style);
            }
        } else if (*figure < 0) {
            ui_draw_number(0x3000000, 590, i * 15 + 16, sp->value_style);
        } else {
            ui_draw_number((u16)*figure | 0x3000000, 590, i * 15 + 16, sp->value_style);
        }
    }
    bar.h = 4;
    for (i = 0; i < STATUS_ROWS; i++) {
        bar.w = sp->attributes_clamped[i] * 50 / sp->bar_scale;
        if (bar.w > 100)
            bar.w = 100;
        bar.y = i * 15 + 27;
        bar.x = 626 - bar.w;
        ui_fill_rect_gradient(&bar, &bright, &dark, 1);
    }
    ui_add_draw_mode(1, 832, 256);
}

/*
 * The status figure page, the cell grid drawer and the item and
 * cell pages.
 */

/* Attribute rows. */

/* Unreferenced: eight zero bytes before this unit's constants. */
DATA(0x80087f68, 0x8, ".sdata")
static s32 DAT_80087f68[2] = {0, 0};

/*
 * The figure page: two current/maximum pairs coloured by how low the
 * current value is, the other figures and two weights in tenths.
 */
ADDRESS(0x8002ed98, 0x460)
void menu_equip_info_draw_stats(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    s16 *p;
    int i;
    s16 w;

    sp->style[6].r = 96;
    sp->style[7].r = 96;
    sp->style[6].g = 48;
    sp->style[7].g = 96;
    sp->style[6].b = 48;
    sp->style[7].b = 32;
    sp->style[7].cd = 49;
    sp->style[6].cd = 49;
    for (i = 0; i < STATUS_ROWS; i++)
        ui_draw_text(MESSAGE(i), 316, i * 15 + 16, &sp->style[0]);
    p = (s16 *)&viewer;
    for (i = 0; i < 2; i++) {
        if (p[1] < p[0] / 2)
            sp->value_style = &sp->style[7];
        else if (p[1] < (short)(p[0] / 5))
            sp->value_style = &sp->style[6];
        else
            sp->value_style = &sp->style[4];
        ui_draw_number((u16)*p++ | 0x5000000, 566, i * 15 + 16, sp->value_style);
        ui_draw_glyph(14, 554, i * 15 + 16, sp->value_style);
        ui_draw_number((u16)*p++ | 0x5000000, 494, i * 15 + 16, sp->value_style);
    }
    p++;
    ui_draw_number((u16)*p++ | 0x3000000, 542, 46, &sp->style[4]);
    ui_draw_glyph(15, 578, 46, &sp->style[4]);
    ui_draw_number((u16)*p++ | 0x3000000, 590, 46, &sp->style[4]);
    for (i = 3; i < 13; i++)
        ui_draw_number((u16)*p++ | 0x3000000, 590, i * 15 + 16, &sp->style[4]);
    if ((short)(p[0] / 10) > (short)(p[1] / 10))
        sp->value_style = &sp->style[6];
    else
        sp->value_style = &sp->style[4];
    w = *p++;
    menu_figure_whole = w * 100 / 454;
    DAT_80088188 = w * 1000 / 454;
    ui_draw_number(menu_figure_whole | 0x3000000, 494, 211, sp->value_style);
    ui_draw_glyph(18, 534, 211, sp->value_style);
    ui_draw_number((u16)(DAT_80088188 - menu_figure_whole * 10) | 0x1000000, 542, 211, sp->value_style);
    ui_draw_glyph(14, 554, 211, sp->value_style);
    w = *p;
    menu_figure_whole = w * 100 / 454;
    DAT_80088188 = w * 1000 / 454;
    ui_draw_number(menu_figure_whole | 0x3000000, 566, 211, sp->value_style);
    ui_draw_glyph(18, 606, 211, sp->value_style);
    ui_draw_number((u16)(DAT_80088188 - menu_figure_whole * 10) | 0x1000000, 614, 211, sp->value_style);
}

/*
 * Draws a 25 x 5 grid of 10 x 10 texture cells from tex at pos, 11 pixels
 * apart horizontally and 12 vertically, semi-transparent in color.
 */
ADDRESS(0x8002f1f8, 0x2a0)
void ui_draw_cell_grid(DVECTOR *pos, DVECTOR *tex, DVECTOR *clut, CVECTOR *color, int arg4)
{
    int row, col;
    int cols, rows; /* cells across and down */
    int w, h;       /* a cell on screen */
    int tw, th;     /* a cell in the texture */
    int dx, dy;     /* cell pitch on screen */
    int u0, v0;
    int u, v;
    StPacketBuffer *buf;
    POLY_FT4 *p;
    CVECTOR c;

    cols = 25;
    rows = 5;
    w = 10;
    h = 10;
    tw = 10;
    th = 10;
    dx = 11;
    dy = 12;
    u0 = (short)(tex->vx % 64) * 4;
    v0 = (short)(tex->vy % 256);
    for (row = 0; row < rows; row++) {
        for (col = 0; col < cols; col++) {
            u = u0 + col * tw;
            v = v0 + row * th;
            NEW_PRIM(buf, p, POLY_FT4);
            setUV4(p, u, v, u + tw, v, u, v + th, u + tw, v + th);
            p->x0 = p->x2 = pos->vx + col * dx;
            p->x1 = p->x3 = pos->vx + col * dx + w;
            p->y0 = p->y1 = pos->vy + row * dy;
            p->y2 = p->y3 = pos->vy + row * dy + h;
            c = *color;
            setRGB0(p, c.r, c.g, c.b);
            p->tpage = GetTPage(0, color->cd & 3, tex->vx, tex->vy);
            p->clut = GetClut(clut->vx, clut->vy);
            setPolyFT4(p);
            setSemiTrans(p, 1);
            addPrim(world_state.ot_cur, p);
        }
    }
}

/* The item page: its arrows, panels, name, cell grid, weight and figures. */
ADDRESS(0x8002f498, 0x4e4)
void menu_equip_info_draw_item(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    DVECTOR pos = {20, 152};
    RECT up = {16, 72, 16, 8};
    RECT down = {300, 72, 16, 8};

    sp->style[2].r = sp->style[6].r = 96;
    sp->style[2].b = sp->style[6].b = 48;
    sp->style[2].g = sp->style[6].g = 48;
    sp->style[2].cd = 73;
    sp->style[6].cd = 49;
    ui_draw_arrow(&down, &sp->cursor_color, 0);
    ui_draw_arrow(&up, &sp->cursor_color, 2048);
    ui_add_draw_mode(1, 0, 0);
    sp->image_rect.x = 12;
    sp->image_rect.y = 10;
    sp->image_rect.w = 192;
    sp->image_rect.h = 124;
    sp->tex_pos.vx = 640;
    sp->tex_pos.vy = 0;
    ui_draw_image(&sp->image_rect, &sp->tex_pos, 0, 0, 16);
    sp->image_rect.x = 204;
    sp->image_rect.w = 100;
    sp->tex_pos.vx = 832;
    ui_draw_image(&sp->image_rect, &sp->tex_pos, 0, 0, 16);
    if (sp->choice_index == -1)
        return;
    sp->style[0].cd |= 0x80;
    ui_draw_text((u16 *)(sp->item_messages->text + sp->item_messages->offsets[sp->choice_id]), 14, 136,
                  &sp->style[0]);
    sp->style[0].cd &= 0x7f;
    ui_draw_cell_grid(&pos, &sp->state.vram[0], &sp->state.vram[1], &sp->style[0], 0xf0004);
    if (vis_table_c.types[sp->choice_id].weight) {
        menu_figure_whole = vis_table_c.types[sp->choice_id].weight * 100 / 454;
        DAT_80088188 = vis_table_c.types[sp->choice_id].weight * 1000 / 454;
        ui_draw_text(MESSAGE(MSG_HI_WEIGHT), 188, 217, &sp->style[0]);
        ui_draw_number(menu_figure_whole | 0x3000000, 224, 217, &sp->style[4]);
        ui_draw_glyph(18, 264, 217, &sp->style[0]);
        ui_draw_number((u16)(DAT_80088188 - menu_figure_whole * 10) | 0x1000000, 272, 217,
                      &sp->style[4]);
        ui_draw_glyph(53, 287, 217, &sp->style[0]);
    }
    if (vis_table_c.types[sp->choice_id].max_durability) {
        ui_draw_text(MESSAGE(MSG_HI_DURABILITY), 16, 216, &sp->style[0]);
        if (vis_table_c.types[sp->choice_id].durability < 4) {
            ui_draw_number(vis_table_c.types[sp->choice_id].durability | 0x3000000, 80, 217,
                          &sp->style[6]);
            ui_draw_glyph(14, 120, 217, &sp->style[2]);
            ui_draw_number(vis_table_c.types[sp->choice_id].max_durability | 0x3000000, 136, 217,
                          &sp->style[6]);
        } else {
            ui_draw_number(vis_table_c.types[sp->choice_id].durability | 0x3000000, 80, 217,
                          &sp->style[4]);
            ui_draw_glyph(14, 120, 217, &sp->style[0]);
            ui_draw_number(vis_table_c.types[sp->choice_id].max_durability | 0x3000000, 136, 217,
                          &sp->style[4]);
        }
    }
}

/* The cell page: its name and grid, the cursor and the cells not yet set. */
ADDRESS(0x8002f97c, 0x2d0)
void menu_equip_info_draw_magic(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    CVECTOR grey = {64, 64, 64, 0};
    DVECTOR pos = {20, 152};
    int row, col, bit;

    sp->style[0].cd |= 0x80;
    ui_draw_text((u16 *)(sp->item_messages->text + sp->item_messages->offsets[512 + sp->choice_index]), 14, 136,
                  &sp->style[0]);
    sp->style[0].cd &= 0x7f;
    ui_draw_cell_grid(&pos, &sp->state.vram[0], &sp->state.vram[1], &sp->style[0], 0xf0004);
    sp->cursor_rect.w = 48;
    sp->cursor_rect.h = 24;
    sp->cursor_rect.x = (short)(sp->choice_index / 3) * 49 + 11;
    sp->cursor_rect.y = (short)(sp->choice_index % 3) * 25 + 12;
    ui_draw_rect_outline(&sp->cursor_rect, &grey, 513, 0);
    sp->cursor_rect.x += 2;
    sp->cursor_rect.y += 1;
    sp->cursor_rect.w -= 4;
    sp->cursor_rect.h -= 2;
    ui_fill_rect(&sp->cursor_rect, &grey, 1);
    ui_add_draw_mode(1, 0, 0);
    sp->image_rect.w = 48;
    sp->image_rect.h = 24;
    sp->image_rect.y = 12;
    sp->tex_pos.vy = 0;
    for (row = 0; row < 2; row++) {
        CVECTOR dim = {64, 64, 64, 0};

        sp->image_rect.x = 11;
        sp->tex_pos.vx = 640;
        for (col = 0, bit = row; col < 6; bit += 3, col++) {
            if (!(*sp->ring_magic & (1 << bit))) {
                ui_fill_rect(&sp->image_rect, &dim, 1);
                ui_add_draw_mode(2, 0, 0);
            }
            ui_draw_image(&sp->image_rect, &sp->tex_pos, 0, 0, 16);
            sp->image_rect.x += 49;
            sp->tex_pos.vx += 48;
        }
        sp->image_rect.y += 25;
        sp->tex_pos.vy += 24;
    }
    ui_add_draw_mode(1, 0, 0);
}

/*
 * The status screen frame.
 */

/* Unreferenced: four zero bytes before this unit's constants. */
DATA(0x80087f84, 0x4, ".sdata")
static s32 DAT_80087f84 = 0;

/* The status screen frame: page toggles, colours, arrows and pages. */
ADDRESS(0x8002fc4c, 0x200)
void menu_equip_info_draw(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    RECT unused[2];
    RECT left = {310, 119, 16, 8};
    RECT right = {630, 119, 16, 8};

    gfx_begin_frame();
    if (pad_state.pressed & PAD_BUTTON_R1)
        menu_second_page = 1;
    else if (pad_state.pressed & PAD_BUTTON_L1)
        menu_second_page = 0;
    pad_state.pressed = 0;
    sp->style[0].r = sp->style[4].r = 80;
    sp->style[0].g = sp->style[4].g = 80;
    sp->style[0].b = sp->style[4].b = 80;
    sp->style[0].cd = 73;
    sp->style[4].cd = 49;
    if (sp->blink_phase < 16)
        sp->blink_level = 8 - sp->blink_phase;
    else
        sp->blink_level = sp->blink_phase - 24;
    sp->cursor_color.b = 0;
    sp->cursor_color.r = (u8)sp->blink_level * 4 + 96;
    sp->cursor_color.g = (u8)sp->blink_level * 2 + 48;
    if (DAT_80088192) {
        if (menu_second_page == 0)
            ui_draw_arrow(&right, &sp->cursor_color, 0);
        else
            ui_draw_arrow(&left, &sp->cursor_color, 2048);
    }
    if (sp->mode == 2) {
        if (sp->list_kind != 10)
            menu_equip_info_draw_item();
        else
            menu_equip_info_draw_magic();
    } else {
        menu_equip_info_draw_slots();
    }
    if (menu_second_page == 0)
        menu_equip_info_draw_stats();
    else
        menu_equip_info_draw_attributes();
    menu_equip_info_draw_panels();
    menu_draw_background();
    gfx_end_frame();
}

/*
 * The creature book's grid page: five cells per row,
 * three rows shown, the selected entry's number and name, and the cell
 * images.
 */

/* The cells per grid row and the rows shown at once. */
#define STORE_COLUMNS 5
#define STORE_ROWS 3

ADDRESS(0x8002fe4c, 0x410)
void menu_creature_book_draw_grid(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    RECT up = {320, 16, 32, 12};
    RECT down = {320, 208, 32, 12};
    int index, kind;
    int row, col, cols;

    menu_second_page = 0;
    sp->cursor_rect.w = 96;
    sp->cursor_rect.h = 48;
    sp->cursor_rect.x = (short)(sp->cursor % STORE_COLUMNS) * 112 + 48;
    sp->cursor_rect.y = (short)(sp->cursor / STORE_COLUMNS) * 56 + 32;
    ui_draw_rect_outline(&sp->cursor_rect, &sp->cursor_color, 513, 1);
    ui_add_draw_mode(1, 0, 0);
    if (sp->scroll > 0)
        ui_draw_arrow(&up, &sp->cursor_color, 3072);
    if (sp->scroll <= sp->state.book.rows - 3)
        ui_draw_arrow(&down, &sp->cursor_color, 1024);
    ui_add_draw_mode(1, 0, 0);
    index = sp->scroll * STORE_COLUMNS + sp->cursor;
    kind = sp->state.book.kinds[index];
    ui_draw_number((u16)(index + 1) | 0x3000000, 486, 214, &sp->style[4]);
    ui_draw_glyph(14, 531, 214, &sp->style[0]);
    ui_draw_number(150 | 0x3000000, 551, 214, &sp->style[4]);
    if (index < sp->state.book.count) {
        sp->style[0].cd |= 0x80;
        ui_draw_text((u16 *)(sp->state.book.names->text +
                              sp->state.book.names->offsets[kind]),
                      40, 214, &sp->style[0]);
        sp->style[0].cd &= 0x7f;
    }
    ui_draw_panel(32, 210, 576, 22);
    sp->image_rect.w = 96;
    sp->image_rect.h = 48;
    sp->image_rect.y = 32;
    index = sp->scroll * STORE_COLUMNS;
    for (row = 0; row < STORE_ROWS; row++) {
        if (sp->scroll + row == sp->state.book.rows)
            cols = sp->state.book.last;
        else
            cols = STORE_COLUMNS;
        sp->image_rect.x = 48;
        for (col = 0; col < cols; index++, col++) {
            if (index < sp->state.book.count) {
                ui_draw_rect_outline(&sp->image_rect, &sp->style[0], 513, 1);
                ui_draw_image(&sp->image_rect, &sp->state.book.cell_tex[index % 15], 0, 0, 16);
            }
            sp->image_rect.x += 112;
        }
        sp->image_rect.y += 56;
    }
    ui_add_draw_mode(0, 0, 0);
    ui_draw_panel(32, 24, 576, 176);
}

/*
 * The creature book's detail page: the kind's name and image, then either its
 * fourteen attribute figures (page toggle on) or its labels, weight, size,
 * paired figures and up to three trait names.
 */
ADDRESS(0x8003025c, 0x85c)
void menu_creature_book_draw_entry(void)
{
    DVECTOR tex = {960, 446};
    DVECTOR clut = {560, 498};
    DVECTOR pos = {34, 154};
    RECT left = {319, 119, 16, 8};
    RECT right = {621, 119, 16, 8};
    MenuScratch *sp = MENU_SCRATCH;
    u16 kind_w[17] = {113, 58, 36, 36, 58, 135, 168, 157, 69, 168, 80, 0, 58, 0, 69, 113, 0};
    CreatureBookEntry *item;
    int kind;
    int i;

    kind = sp->state.book.kinds[sp->scroll * 5 + sp->cursor];
    item = &sp->state.book.entries[kind];
    if (pad_state.pressed & PAD_BUTTON_R1)
        menu_second_page = 1;
    if (pad_state.pressed & PAD_BUTTON_L1)
        menu_second_page = 0;
    pad_state.pressed = 0;
    if (menu_second_page == 0)
        ui_draw_arrow(&right, &sp->cursor_color, 0);
    else
        ui_draw_arrow(&left, &sp->cursor_color, 2048);
    ui_add_draw_mode(1, 0, 0);
    sp->style[0].cd |= 0x80;
    ui_draw_text(MESSAGE_IN(sp->state.book.names, kind), 32, 16, &sp->style[0]);
    sp->style[0].cd &= 0x7f;
    ui_draw_cell_grid(&pos, &tex, &clut, &sp->style[0], 0xf0005);
    if (menu_second_page) {
        for (i = 0; i < 14; i++) {
            ui_draw_text(MESSAGE(MSG_HI_BOOK_ATTRIBUTES + i), 336, i * 15 + 16, &sp->style[0]);
            ui_draw_number((u16)item->attributes[i] | DIGITS(2), 582, i * 15 + 16, &sp->style[4]);
        }
    } else {
        for (i = 0; i < 10; i++)
            ui_draw_text(MESSAGE(MSG_HI_BOOK_LABELS + i), 336, i * 15 + 16, &sp->style[0]);
        if (item->category < 17)
            ui_draw_text(MESSAGE(MSG_HI_CREATURE_TYPES + item->category), 606 - kind_w[item->category], 16,
                          &sp->style[0]);
        ui_draw_number(item->hp | DIGITS(4), 558, 31, &sp->style[4]);
        ui_draw_number(item->sp_gain | DIGITS(2), 582, 46, &sp->style[4]);
        menu_figure_whole = item->height * 10 / 3048;
        ui_draw_number(menu_figure_whole | DIGITS(3), 522, 61, &sp->style[4]);
        ui_draw_glyph(55, 559, 61, &sp->style[4]);
        ui_draw_number((u16)((u16)(item->height * 10 - menu_figure_whole * 3048) / 254) | DIGITS(2), 570,
                      61, &sp->style[4]);
        ui_draw_glyph(54, 595, 61, &sp->style[4]);
        menu_figure_whole = item->weight * 100 / 454;
        DAT_80088188 = item->weight * 1000 / 454;
        ui_draw_number(menu_figure_whole | DIGITS(4), 522, 76, &sp->style[4]);
        ui_draw_glyph(18, 574, 76, &sp->style[4]);
        ui_draw_number((u16)(DAT_80088188 - menu_figure_whole * 10) | DIGITS(1), 582, 76, &sp->style[4]);
        ui_draw_glyph(53, 594, 76, &sp->style[4]);
        if (item->pairs[0][1]) {
            ui_draw_number((u16)item->pairs[0][1] | DIGITS(3), 522, 91, &sp->style[4]);
            ui_draw_glyph(15, 558, 91, &sp->style[4]);
        }
        ui_draw_number((u16)item->pairs[0][0] | DIGITS(3), 570, 91, &sp->style[4]);
        if (item->pairs[1][1]) {
            ui_draw_number((u16)item->pairs[1][1] | DIGITS(3), 522, 106, &sp->style[4]);
            ui_draw_glyph(15, 558, 106, &sp->style[4]);
        }
        ui_draw_number((u16)item->pairs[1][0] | DIGITS(3), 570, 106, &sp->style[4]);
        if (item->pairs[2][1]) {
            ui_draw_number((u16)item->pairs[2][1] | DIGITS(3), 522, 121, &sp->style[4]);
            ui_draw_glyph(15, 558, 121, &sp->style[4]);
        }
        ui_draw_number((u16)item->pairs[2][0] | DIGITS(3), 570, 121, &sp->style[4]);
        if (item->pairs[3][1]) {
            ui_draw_number((u16)item->pairs[3][1] | DIGITS(3), 522, 136, &sp->style[4]);
            ui_draw_glyph(15, 558, 136, &sp->style[4]);
        }
        ui_draw_number((u16)item->pairs[3][0] | DIGITS(3), 570, 136, &sp->style[4]);
        if (item->abilities & 0xf)
            ui_draw_text(MESSAGE(MSG_HI_ABILITIES + (item->abilities & 0xf)), 336, 181, &sp->style[0]);
        if (item->abilities >> 8 & 0xf)
            ui_draw_text(MESSAGE(MSG_HI_ABILITIES + (item->abilities >> 8 & 0xf)), 336, 196, &sp->style[0]);
        if (item->abilities >> 16 & 0xf)
            ui_draw_text(MESSAGE(MSG_HI_ABILITIES + (item->abilities >> 16 & 0xf)), 336, 211, &sp->style[0]);
    }
    sp->image_rect.x = 55;
    sp->image_rect.y = 32;
    sp->image_rect.w = 230;
    sp->image_rect.h = 115;
    ui_draw_rect_outline(&sp->image_rect, &sp->style[0], 513, 1);
    ui_add_draw_mode(0, 0, 0);
    ui_draw_image(&sp->image_rect, &sp->state.book.image, NULL, NULL, 16);
    ui_draw_panel(24, 12, 292, 216);
    ui_draw_panel(324, 12, 292, 216);
}

/*
 * The creature book's frame: colours, cursor blink,
 * then the grid page or the page menu_creature_book_draw_entry draws.
 */

ADDRESS(0x80030ab8, 0x12c)
void menu_creature_book_draw(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    RECT unused;
    RECT left = {314, 119, 16, 8};
    RECT right = {625, 119, 16, 8};

    gfx_begin_frame();
    sp->style[0].r = sp->style[4].r = 80;
    sp->style[0].g = sp->style[4].g = 80;
    sp->style[0].b = sp->style[4].b = 80;
    sp->style[0].cd = 73;
    sp->style[4].cd = 49;
    if (sp->blink_phase < 16)
        sp->blink_level = 8 - sp->blink_phase;
    else
        sp->blink_level = sp->blink_phase - 24;
    sp->cursor_color.b = 0;
    sp->cursor_color.r = (u8)sp->blink_level * 4 + 96;
    sp->cursor_color.g = (u8)sp->blink_level * 2 + 48;
    if (sp->mode == 2)
        menu_creature_book_draw_entry();
    else
        menu_creature_book_draw_grid();
    menu_draw_background();
    gfx_end_frame();
}

/*
 * The id grid's left block: the cursor cell, scroll arrows and a position
 * bar, the header, and five rows of four cell images from the first row
 * shown, wrapping after twenty.
 */
ADDRESS(0x80030be4, 0x3bc)
void menu_arrange_icons_draw_hand(void)
{
    RECT unused;
    CVECTOR light = {96, 96, 96};
    RECT up = {26, 40, 24, 8};
    RECT down = {26, 188, 24, 8};
    RECT bar = {20, 47, 16, 3};
    CVECTOR shade = {40, 40, 48};
    RECT edge;
    RECT unused2;
    MenuScratch *sp = MENU_SCRATCH;
    int step_x = 64;
    int step_y = 32;
    short column;
    int cell;
    int row, col;

    column = sp->cursor % 8;
    if (column < 4) {
        sp->cursor_rect.x = column * 64 + 50;
        sp->cursor_rect.y = sp->cursor / 8 * 32 + 35;
        sp->cursor_rect.w = 56;
        sp->cursor_rect.h = 28;
        ui_draw_rect_outline(&sp->cursor_rect, &sp->cursor_color, 513, 1);
    }
    if (sp->state.grid.hand_row > 0)
        ui_draw_arrow(&up, &sp->cursor_color, 3072);
    if (sp->state.grid.hand_row < 73)
        ui_draw_arrow(&down, &sp->cursor_color, 1024);
    bar.y = sp->state.grid.hand_row * 136 / 74 + 47;
    ui_fill_rect(&bar, &shade, 1);
    ui_add_draw_mode(1, 0, 0);
    edge.x = bar.x - 2;
    edge.y = bar.y - 1;
    edge.w = bar.w;
    edge.h = bar.h;
    ui_fill_rect(&edge, &shade, 1);
    ui_add_draw_mode(2, 0, 0);
    sp->rect.x = 60;
    sp->rect.y = 12;
    sp->rect.w = 196;
    sp->rect.h = 16;
    sp->tex_pos.vx = 896;
    sp->tex_pos.vy = 448;
    sp->clut_pos.vx = 544;
    sp->clut_pos.vy = 502;
    ui_draw_image(&sp->rect, &sp->tex_pos, &sp->clut_pos, NULL, 3);
    cell = sp->state.grid.hand_row * 4 % 20;
    sp->image_rect.y = 35;
    sp->image_rect.w = 56;
    sp->image_rect.h = 28;
    for (row = 0; row < 5; row++) {
        sp->image_rect.x = 50;
        for (col = 0; col < 4; col++) {
            ui_draw_image(&sp->image_rect, &sp->state.grid.cell_tex[cell], NULL, NULL, 16);
            cell = (cell + 1) % 20;
            sp->image_rect.x += step_x;
        }
        sp->image_rect.y += step_y;
    }
    ui_draw_panel(10, 9, 296, 188);
}

/* The id grid's right block, as menu_arrange_icons_draw_hand for the second id table. */
ADDRESS(0x80030fa0, 0x3d8)
void menu_arrange_icons_draw_bag(void)
{
    RECT unused;
    CVECTOR light = {96, 96, 96};
    RECT up = {612, 40, 24, 8};
    RECT down = {612, 188, 24, 8};
    RECT bar = {606, 47, 16, 3};
    CVECTOR shade = {40, 40, 48};
    RECT edge;
    RECT unused2;
    MenuScratch *sp = MENU_SCRATCH;
    int step_x = 64;
    int step_y = 32;
    short column;
    int cell;
    int row, col;

    column = sp->cursor % 8;
    if (column >= 4) {
        sp->cursor_rect.x = column * 64 + 82;
        sp->cursor_rect.y = sp->cursor / 8 * 32 + 35;
        sp->cursor_rect.w = 56;
        sp->cursor_rect.h = 28;
        ui_draw_rect_outline(&sp->cursor_rect, &sp->cursor_color, 513, 1);
    }
    if (sp->state.grid.bag_row > 0)
        ui_draw_arrow(&up, &sp->cursor_color, 3072);
    if (sp->state.grid.bag_row < 73)
        ui_draw_arrow(&down, &sp->cursor_color, 1024);
    bar.y = sp->state.grid.bag_row * 136 / 74 + 47;
    ui_fill_rect(&bar, &shade, 1);
    ui_add_draw_mode(1, 0, 0);
    edge.x = bar.x - 2;
    edge.y = bar.y - 1;
    edge.w = bar.w;
    edge.h = bar.h;
    ui_fill_rect(&edge, &shade, 1);
    ui_add_draw_mode(2, 0, 0);
    sp->rect.x = 384;
    sp->rect.y = 12;
    sp->rect.w = 196;
    sp->rect.h = 16;
    sp->tex_pos.vx = 896;
    sp->tex_pos.vy = 464;
    sp->clut_pos.vx = 544;
    sp->clut_pos.vy = 502;
    ui_draw_image(&sp->rect, &sp->tex_pos, &sp->clut_pos, NULL, 3);
    cell = sp->state.grid.bag_row * 4 % 20;
    sp->image_rect.y = 35;
    sp->image_rect.w = 56;
    sp->image_rect.h = 28;
    for (row = 0; row < 5; row++) {
        sp->image_rect.x = 338;
        for (col = 0; col < 4; col++) {
            ui_draw_image(&sp->image_rect, &sp->state.grid.cell_tex[cell + 20], NULL, NULL, 16);
            cell = (cell + 1) % 20;
            sp->image_rect.x += step_x;
        }
        sp->image_rect.y += step_y;
    }
    ui_draw_panel(334, 9, 296, 188);
}

/*
 * The id grid's detail panels: the name and figures
 * of the id under the grid cursor, and its table C counts.
 */

ADDRESS(0x80031378, 0x2fc)
void menu_arrange_icons_draw_caption(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    int id;
    short column;

    column = sp->cursor % 8;
    if (column < 4)
        id = sp->state.grid.hand_ids[(sp->state.grid.hand_row + sp->cursor / 8) * 4 + column];
    else
        id = sp->state.grid.bag_ids[(sp->state.grid.bag_row + sp->cursor / 8) * 4 + column - 4];
    sp->style[0].cd |= 0x80;
    if (id != -1)
        ui_draw_text((u16 *)(sp->state.grid.messages->text + sp->state.grid.messages->offsets[id]),
                      18, 202, &sp->style[0]);
    sp->style[0].cd &= 0x7f;
    if (id != -1) {
        if (vis_table_c.types[id].max_durability) {
            ui_draw_text(MESSAGE(MSG_HI_DURABILITY), 74, 215, &sp->style[0]);
            ui_draw_number(vis_table_c.types[id].durability | 0x3000000, 138, 215, &sp->style[4]);
            ui_draw_glyph(14, 174, 215, &sp->style[0]);
            ui_draw_number(vis_table_c.types[id].max_durability | 0x3000000, 190, 215, &sp->style[4]);
        }
        if (vis_table_c.types[id].weight) {
            ui_draw_text(MESSAGE(MSG_HI_WEIGHT), 250, 215, &sp->style[0]);
            menu_figure_whole = vis_table_c.types[id].weight * 100 / 454;
            DAT_80088188 = vis_table_c.types[id].weight * 1000 / 454;
            ui_draw_number(menu_figure_whole | 0x3000000, 300, 215, &sp->style[4]);
            ui_draw_glyph(18, 339, 215, &sp->style[0]);
            ui_draw_number((u16)(DAT_80088188 - menu_figure_whole * 10) | 0x1000000, 346, 215,
                          &sp->style[4]);
            ui_draw_glyph(53, 362, 215, &sp->style[0]);
        }
    }
    ui_draw_panel(10, 198, 620, 34);
}

/*
 * The id grid's detail panel: the panel sits on the side away from the
 * selected column, and lists the selected id's fourteen counts in columns
 * of four.
 */
ADDRESS(0x80031674, 0x334)
void menu_arrange_icons_draw_details(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    int text_y;
    int i;
    short x, y;
    int name_x;
    int count_x;
    short column;
    short index;
    short id;
    int none;
    u8 *count;
    int n, k, j;

    if ((short)(sp->cursor % 8) < 4) {
        x = 334;
        y = 9;
        text_y = 135;
        count_x = 342;
        sp->image_rect.x = x;
        sp->image_rect.y = y;
        sp->image_rect.w = 192;
        sp->image_rect.h = 124;
    } else {
        x = 10;
        y = 9;
        text_y = 135;
        count_x = 18;
        sp->image_rect.x = x;
        sp->image_rect.y = y;
        sp->image_rect.w = 192;
        sp->image_rect.h = 124;
    }
    none = 0;
    column = sp->cursor % 8;
    if (column < 4) {
        index = (sp->state.grid.hand_row + sp->cursor / 8) * 4;
        index += column;
        id = sp->state.grid.hand_ids[index];
        if (sp->state.grid.hand_ids[index] == -1)
            none = 1;
    } else {
        index = (sp->state.grid.bag_row + sp->cursor / 8) * 4 - 4;
        index += column;
        id = sp->state.grid.bag_ids[index];
        if (sp->state.grid.bag_ids[index] == -1)
            none = 1;
    }
    /* The flag is tested as a short (retail copies it to a temporary first). */
    if (!(short)none) {
        sp->tex_pos.vx = 640;
        sp->tex_pos.vy = 0;
        ui_draw_image(&sp->image_rect, &sp->tex_pos, 0, 0, 16);
        sp->image_rect.w = 100;
        sp->tex_pos.vx = 832;
        sp->image_rect.x += 192;
        ui_draw_image(&sp->image_rect, &sp->tex_pos, 0, 0, 16);
        n = 0;
        k = 2;
        count = vis_table_c.types[id].attributes;
        for (i = 0, name_x = count_x; i < 4; i++) {
            int label_x;
            int value_x;
            for (j = 0, label_x = name_x, value_x = name_x + 42; j < 4; j++) {
                if (n == 14)
                    goto done;
                n++;
                if (*count) {
                    ui_draw_text(MESSAGE(MSG_HI_HAND + k), label_x, text_y + j * 15, &sp->style[0]);
                    ui_draw_number(*count | 0x13000000, value_x, text_y + j * 15, &sp->style[4]);
                }
                count++;
                k++;
            }
            if (n == 14)
                break;
            name_x += 70;
        }
    }
done:
    ui_draw_panel(x, y, 296, 188);
}

/* A two-tile emblem with its caption in a framed panel. */
ADDRESS(0x800319a8, 0x144)
void menu_draw_button_prompt(void)
{
    CVECTOR light = {96, 96, 96};
    CVECTOR grey = {80, 80, 80};
    MenuScratch *sp = MENU_SCRATCH;

    sp->tex_pos.vx = 896;
    sp->tex_pos.vy = 480;
    sp->clut_pos.vx = 544;
    sp->clut_pos.vy = 503;
    sp->image_rect.w = 32;
    sp->image_rect.h = 16;
    sp->image_rect.x = 353;
    sp->image_rect.y = 168;
    ui_draw_image(&sp->image_rect, &sp->tex_pos, &sp->clut_pos, NULL, 0);
    sp->tex_pos.vx = 904;
    sp->clut_pos.vy = 504;
    sp->image_rect.x = 232;
    sp->image_rect.y = 168;
    ui_draw_image(&sp->image_rect, &sp->tex_pos, &sp->clut_pos, NULL, 0);
    ui_draw_text(MESSAGE(MSG_HI_MOVE_PROMPT), 188, 150, &sp->style[0]);
    ui_draw_panel(168, 144, 319, 44);
}

/*
 * The id grid screen frame, a VRAM clear and a framed panel.
 */

/*
 * Draws the id grid: both column blocks while the 0x1 page bit is set,
 * otherwise the block under the cursor with its detail panel.
 */
ADDRESS(0x80031aec, 0x210)
void menu_arrange_icons_draw(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    RECT arrow = {319, 110, 24, 16};
    CVECTOR color = {16, 48, 16, 0};

    gfx_begin_frame();
    sp->style[0].r = sp->style[4].r = 96;
    sp->style[0].g = sp->style[4].g = 96;
    sp->style[0].b = sp->style[4].b = 96;
    sp->style[0].cd = 73;
    sp->style[4].cd = 49;
    if (sp->blink_phase < 16)
        sp->blink_level = 8 - sp->blink_phase;
    else
        sp->blink_level = sp->blink_phase - 24;
    sp->cursor_color.b = 0;
    sp->cursor_color.r = (u8)sp->blink_level * 4 + 96;
    sp->cursor_color.g = (u8)sp->blink_level * 2 + 48;
    if (sp->mode & 0x8000)
        menu_draw_button_prompt();
    number_narrow = 1;
    if (sp->mode & 0x1) {
        menu_arrange_icons_draw_hand();
        menu_arrange_icons_draw_bag();
    } else {
        if ((short)(sp->cursor % 8) < 4)
            menu_arrange_icons_draw_hand();
        else
            menu_arrange_icons_draw_bag();
        menu_arrange_icons_draw_details();
    }
    number_narrow = 0;
    if ((short)(sp->cursor % 8) < 4)
        ui_draw_arrow(&arrow, &color, 0);
    else
        ui_draw_arrow(&arrow, &color, 2048);
    ui_add_draw_mode(1, 0, 0);
    menu_arrange_icons_draw_caption();
    menu_draw_background();
    gfx_end_frame();
}

/* Clears the 640x480 VRAM area from the origin to black. */
ADDRESS(0x80031cfc, 0x54)
void clear_vram(void)
{
    RECT r = {0, 0, 640, 480};

    ClearImage(&r, 0, 0, 0);
}

ADDRESS(0x80031d50, 0x94)
void pause_screen_draw(void)
{
    gfx_begin_frame();
    gfx_flip();
    menu_draw_backdrop();
    quad_add_at_depth(120, 120, 200, 136, 0, 144, 80, 16, 255, 25, 32738, 96, 96, 96, 0);
    gfx_draw_ot();
}

/*
 * Copies 128 placement records into the table: the id, the flag bytes,
 * each coordinate as block * 4096 + offset and a random start below
 * unk3. An id of 0xffff marks an empty record.
 */
ADDRESS(0x80031de4, 0x118)
void placements_expand(StPlacement *src)
{
    PlacedObject *dst = placement_objects.objects;
    int i = 128;
    u16 id;

    while (i--) {
        id = src->id;
        if (id != 0xffff) {
            dst->model = id;
            dst->kind = src->kind | (src->turn << 6);
            dst->frame_count = src->frame_count;
            dst->frame_period = src->frame_period;
            dst->position.vx = (src->bx << 12) + src->x;
            dst->position.vy = (src->by << 12) + src->y;
            dst->position.vz = (src->bz << 12) + src->z;
            dst->frame = (rand() * dst->frame_count) >> 15;
        } else {
            dst->model = 0xffff;
        }
        dst++;
        src++;
    }
}

/*
 * Uploads the TIM images packed in `buf` (each its 8-byte header, an
 * optional CLUT block and the pixel block) and returns the bytes of the
 * complete images; it stops at the first truncated or inconsistent one.
 */
ADDRESS(0x80031efc, 0x15c)
int tim_load_images(u_long *buf, u_long size)
{
    u_char *p = (u_char *)buf;
    u_char *done = (u_char *)buf;

    for (;;) {
        TimHeader *tim;
        TimBlock *block;
        u_long *pixels;
        int bytes;

        if (size < p - (u_char *)buf + sizeof(TimHeader))
            break;
        tim = (TimHeader *)p;
        p += sizeof(TimHeader);
        if (tim->flag & 8) {
            u_long *clut;

            block = (TimBlock *)p;
            if (size < p - (u_char *)buf + sizeof(TimBlock))
                break;
            p += sizeof(TimBlock);
            bytes = block->w * block->h * 2;
            if (block->bnum != bytes + sizeof(TimBlock))
                break;
            if (size < p - (u_char *)buf + bytes)
                break;
            clut = (u_long *)p;
            p += bytes;
            VSync(0);
            LoadImage((RECT *)&block->x, clut);
        }
        block = (TimBlock *)p;
        if (size < p - (u_char *)buf + sizeof(TimBlock))
            break;
        p += sizeof(TimBlock);
        bytes = block->w * block->h * 2;
        if (block->bnum != bytes + sizeof(TimBlock))
            break;
        if (size < p - (u_char *)buf + bytes)
            break;
        pixels = (u_long *)p;
        p += bytes;
        VSync(0);
        LoadImage((RECT *)&block->x, pixels);
        done = p;
    }
    return done - (u_char *)buf;
}

/* Fills the environments from the defaults. */
ADDRESS(0x80032058, 0x114)
void lighting_load_defaults(void)
{
    StLightInit *init = lighting_defaults;
    StLight *light = lighting.light;
    int i = 64;

    while (i--) {
        light->color = init->color;
        light->light[0] = init->light;
        light->back = init->back;
        light->depth_cue_near = init->depth_cue_near;
        light->depth_cue_far = init->depth_cue_far;
        init++;
        light++;
    }
    lighting.rebuild_matrices = 1;
}

/* Derives every environment's light matrices 0..3 from matrix 0. */
ADDRESS(0x8003216c, 0xa4)
void lighting_build_matrices(void)
{
    StLight *light;
    int i;

    if (lighting.rebuild_matrices) {
        light = lighting.light;
        i = 64;
        while (i--) {
            lighting_quarter_turn_matrix(light, &light->light[0], 0);
            lighting_quarter_turn_matrix(light, &light->light[1], 1);
            lighting_quarter_turn_matrix(light, &light->light[2], 2);
            lighting_quarter_turn_matrix(light, &light->light[3], 3);
            light++;
        }
        lighting.rebuild_matrices = 0;
    }
}

ADDRESS(0x80032210, 0x34)
void lighting_blend_matrix(StLightInit *init, StLight *light, short t)
{
    matrix_lerp(&light->light[0], &init->light, &light->light[0], t);
}

ADDRESS(0x80032244, 0x38)
void lighting_blend_color(StLightInit *init, StLight *light, short t)
{
    matrix_lerp(&light->color, &init->color, &light->color, t);
}

ADDRESS(0x8003227c, 0x7c)
void lighting_blend_back(StLightInit *init, StLight *light, short t)
{
    light->back.r = math_lerp(light->back.r, init->back.r, t);
    light->back.g = math_lerp(light->back.g, init->back.g, t);
    light->back.b = math_lerp(light->back.b, init->back.b, t);
}

/* Blends unk68, keeping unk6A at least 4096 beyond it. */
ADDRESS(0x800322f8, 0x60)
void lighting_blend_near(StLightInit *init, StLight *light, short t)
{
    light->depth_cue_near = math_lerp(light->depth_cue_near, init->depth_cue_near, t);
    if (light->depth_cue_near > light->depth_cue_far - 4096)
        light->depth_cue_far = light->depth_cue_near + 4096;
}

/* Blends unk6A, keeping unk68 at least 4096 below it. */
ADDRESS(0x80032358, 0x60)
void lighting_blend_far(StLightInit *init, StLight *light, short t)
{
    light->depth_cue_far = math_lerp(light->depth_cue_far, init->depth_cue_far, t);
    if (light->depth_cue_far < light->depth_cue_near + 4096)
        light->depth_cue_near = light->depth_cue_far - 4096;
}

/*
 * Blends the parts of `init` that `flags` selects into the first 48
 * environments: 1 matrix 0, 2 the colour matrix, 4 the back colour,
 * 8 unk68, 0x10 unk6A.
 */
ADDRESS(0x800323b8, 0x178)
void lighting_blend(int flags, StLightInit *init, short t)
{
    StLight *light;
    int i;

    if (flags & 1) {
        light = lighting.light;
        for (i = 0; i < 48; i++) {
            lighting_blend_matrix(init, light, t);
            light++;
        }
        lighting.rebuild_matrices = 1;
    }
    if (flags & 2) {
        light = lighting.light;
        for (i = 0; i < 48; i++) {
            lighting_blend_color(init, light, t);
            light++;
        }
    }
    if (flags & 4) {
        light = lighting.light;
        for (i = 0; i < 48; i++) {
            lighting_blend_back(init, light, t);
            light++;
        }
    }
    if (flags & 8) {
        light = lighting.light;
        for (i = 0; i < 48; i++) {
            lighting_blend_near(init, light, t);
            light++;
        }
    }
    if (flags & 0x10) {
        light = lighting.light;
        for (i = 0; i < 48; i++) {
            lighting_blend_far(init, light, t);
            light++;
        }
    }
}

/* Blends a back colour (no part when any component is 255) and the two
 * halfwords (none when 0xffff) into the environments. */
ADDRESS(0x80032530, 0x94)
void lighting_blend_back_ranges(r, g, b, unk68, unk6A, t)
u8 r;
u8 g;
u8 b;
int unk68;
int unk6A;
short t;
{
    StLightInit init;
    int flags = 0;

    if (r != 255 && g != 255 && b != 255) {
        flags = 4;
        init.back.r = r;
        init.back.g = g;
        init.back.b = b;
    }
    if (unk68 != 0xffff) {
        init.depth_cue_near = unk68;
        flags |= 8;
    }
    if (unk6A != 0xffff) {
        init.depth_cue_far = unk6A;
        flags |= 0x10;
    }
    lighting_blend(flags, &init, t);
}

/* Blends a colour matrix and a light matrix (none when any element is
 * 0x7fff) into the environments. */
ADDRESS(0x800325c4, 0x1b0)
void lighting_blend_matrices(StLightMatrix color, StLightMatrix matrix, short t)
{
    StLightInit init;
    int flags = 0;

    if (color.m[0][0] != 0x7fff && color.m[1][0] != 0x7fff && color.m[2][0] != 0x7fff &&
        color.m[0][1] != 0x7fff && color.m[1][1] != 0x7fff && color.m[2][1] != 0x7fff &&
        color.m[0][2] != 0x7fff && color.m[1][2] != 0x7fff && color.m[2][2] != 0x7fff) {
        flags = 2;
        init.color.m[0][0] = color.m[0][0];
        init.color.m[1][0] = color.m[1][0];
        init.color.m[2][0] = color.m[2][0];
        init.color.m[0][1] = color.m[0][1];
        init.color.m[1][1] = color.m[1][1];
        init.color.m[2][1] = color.m[2][1];
        init.color.m[0][2] = color.m[0][2];
        init.color.m[1][2] = color.m[1][2];
        init.color.m[2][2] = color.m[2][2];
    }
    if (matrix.m[0][0] != 0x7fff && matrix.m[0][1] != 0x7fff && matrix.m[0][2] != 0x7fff &&
        matrix.m[1][0] != 0x7fff && matrix.m[1][1] != 0x7fff && matrix.m[1][2] != 0x7fff &&
        matrix.m[2][0] != 0x7fff && matrix.m[2][1] != 0x7fff && matrix.m[2][2] != 0x7fff) {
        flags |= 1;
        init.light.m[0][0] = matrix.m[0][0];
        init.light.m[0][1] = matrix.m[0][1];
        init.light.m[0][2] = matrix.m[0][2];
        init.light.m[1][0] = matrix.m[1][0];
        init.light.m[1][1] = matrix.m[1][1];
        init.light.m[1][2] = matrix.m[1][2];
        init.light.m[2][0] = matrix.m[2][0];
        init.light.m[2][1] = matrix.m[2][1];
        init.light.m[2][2] = matrix.m[2][2];
    }
    lighting_blend(flags, &init, t);
}

/* The grid environment of the lowest row flag (0x10..0x80) and the lowest
 * column flag (1..4). */
ADDRESS(0x80032774, 0x8c)
int lighting_grid_index(flags)
u8 flags;
{
    int row = 0;
    int column = 0;

    if (flags & 0x10)
        row = 0;
    else if (flags & 0x20)
        row = 1;
    else if (flags & 0x40)
        row = 2;
    else if (flags & 0x80)
        row = 3;
    if (flags & 1)
        column = 0;
    else if (flags & 2)
        column = 1;
    else if (flags & 4)
        column = 2;
    return row * 3 + column;
}

/*
 * Selects the environment of the row and column flags. Without `blend` it
 * only records the selection; with it the current environment becomes the
 * blend start and lighting_update blends toward the selection.
 */
ADDRESS(0x80032800, 0x86c)
void lighting_select(rows, columns, blend)
u8 rows;
u8 columns;
int blend;
{
    int row;
    int column;
    u8 bits;
    u8 flags;

    row = 0;
    for (bits = rows; !(bits & 0x10); row++)
        bits >>= 1;
    column = 0;
    for (bits = columns; !(bits & 1); column++)
        bits >>= 1;
    world_state.next_render_mode = rows | columns;
    if (!blend) {
        world_state.render_mode = rows | columns;
        world_state.selected_light = row * 3 + column;
        return;
    }
    if (!(world_state.light_blend & 0x80)) {
        if (world_state.render_mode & 2) {
            if (world_state.render_mode & 0x10)
                *world_state.light = lighting.light[12];
            else if (world_state.render_mode & 0x20)
                *world_state.light = lighting.light[13];
            else if (world_state.render_mode & 0x40)
                *world_state.light = lighting.light[15];
            else if (world_state.render_mode & 0x80)
                *world_state.light = lighting.light[17];
        } else if (world_state.render_mode & 4) {
            if (world_state.render_mode & 0x20)
                *world_state.light = lighting.light[14];
            else if (world_state.render_mode & 0x40)
                *world_state.light = lighting.light[16];
            else if (world_state.render_mode & 0x80)
                *world_state.light = lighting.light[18];
        } else {
            *world_state.light = lighting.light[lighting_grid_index(world_state.render_mode)];
        }
    }
    world_state.selected_light = row * 3 + column;
    world_state.light_to = &lighting.light[world_state.selected_light];
    flags = world_state.next_render_mode;
    if (flags & 2) {
        if (flags & 0x10)
            world_state.light_to = &lighting.light[12];
        else if (flags & 0x20)
            world_state.light_to = &lighting.light[13];
        else if (flags & 0x40)
            world_state.light_to = &lighting.light[15];
        else if (flags & 0x80)
            world_state.light_to = &lighting.light[17];
    } else if (flags & 4) {
        if (flags & 0x20)
            world_state.light_to = &lighting.light[14];
        else if (flags & 0x40)
            world_state.light_to = &lighting.light[16];
        else if (flags & 0x80)
            world_state.light_to = &lighting.light[18];
    }
    *world_state.light_from = *world_state.light;
    world_state.render_mode = 0;
    world_state.light_blend = 0x80;
}

/* Points the current and blend-start environments at their table slots
 * and selects the current area's environment without blending. */
ADDRESS(0x8003306c, 0x84)
void lighting_start(void)
{
    u8 mode;

    world_state.light = &lighting.light[43];
    world_state.light_from = &lighting.light[44];
    lighting.row_request = 0;
    lighting.rows = 0;
    lighting.column_request = 0;
    lighting.columns = 0;
    lighting.row_request = 0;
    lighting.column_request = 0;
    lighting.last_area = 0;
    mode = lighting.area_mode[area_state.area];
    lighting_select(mode & 0xf0, mode & 0xf, 0);
}

ADDRESS(0x800330f0, 0x20)
void lighting_clear_requests(void)
{
    lighting.force_first_column = 0;
    lighting.row_request = 0;
    lighting.column_request = 0;
    lighting.rows = 0;
    lighting.columns = 0;
}

/* An environment field blended from unk1EE0C to unk1EE10 by t / 64 with
 * weight w for the start (unsigned for the colours, signed for the
 * ranges). */
#define LIGHT_BLEND(field, w) \
    ((world_state.light_from->field * (w) + world_state.light_to->field * t) >> 6)

/*
 * Per frame: picks the row and column flags for the current area and
 * state, starts a blend when they change and advances the blend of the
 * current environment over 64 steps.
 */
ADDRESS(0x80033110, 0x8b4)
void lighting_update(void)
{
    u8 mode;
    u8 columns;
    int rows;
    u8 t;
    u32 s;

    mode = lighting.area_mode[area_state.area];
    columns = mode & 0xf;
    rows = mode & 0xf0;
    if (area_state.area == 25 || area_state.area == 26 || area_state.area == 14 ||
        area_state.area == 23) {
        lighting.force_first_column = 0;
        lighting.row_request &= ~0x20;
    }
    if (viewer.equipment_light < 0) {
        lighting.column_request |= 1;
        lighting.row_request |= 4;
    } else if (viewer.equipment_light > 0) {
        lighting.column_request |= 1;
        lighting.row_request |= 0x10;
    }
    if (lighting.paused == 1)
        return;
    if (lighting.paused == 2) {
        lighting.paused = 0;
        return;
    }
    if (area_state.area != lighting.last_area) {
        lighting.entry_step = 1;
        lighting.last_area = area_state.area;
        world_state.light_blend = 0;
    }
    if (lighting.entry_step == 2 && viewer.state == VIEWER_STATE_SHOP) {
        lighting.column_request = 0;
        lighting.row_request = 0;
    }
    if (lighting.force_first_column) {
        columns = 1;
        lighting.force_first_column = 0;
    } else if (lighting.area_mode[area_state.area] == 17) {
        columns = 1;
    } else if ((columns | lighting.column_request) & 4) {
        columns = 4;
    } else if ((columns | lighting.column_request) & 2) {
        columns = 2;
    } else {
        columns = 1;
    }
    if (lighting.row_request) {
        if (lighting.row_request & 0xe0)
            rows = 16;
        else if (lighting.row_request & 0xf)
            rows = 64;
        else if (lighting.area_mode[area_state.area] & 0x10)
            rows = 16;
        else
            rows = 32;
    }
    if (columns != lighting.columns || rows != lighting.rows) {
        if (lighting.entry_step == 2) {
            lighting_select(rows, columns, 0);
            lighting.columns = columns;
            lighting.rows = rows;
        } else if (lighting.entry_step == 0) {
            lighting_select(rows, columns, 1);
            lighting.columns = columns;
            lighting.rows = rows;
        }
    }
    if (lighting.entry_step == 1 && (short)area_state.load_step >= 7)
        lighting.entry_step = 2;
    else if (lighting.entry_step == 2)
        lighting.entry_step = 0;
    lighting.row_request = 0;
    lighting.column_request = 0;
    if (world_state.light_blend & 0x80) {
        t = world_state.light_blend & 0x7f;
        if (t >= 64) {
            world_state.light_blend = 0;
            world_state.render_mode = world_state.next_render_mode;
            *world_state.light = lighting.light[world_state.selected_light];
        }
        world_state.light->depth_cue_near = LIGHT_BLEND(depth_cue_near, 64 - t);
        world_state.light->depth_cue_far = LIGHT_BLEND(depth_cue_far, 64 - t);
        s = 64 - t;
        world_state.light->color.m[0][0] = LIGHT_BLEND(color.m[0][0], s);
        world_state.light->color.m[0][1] = LIGHT_BLEND(color.m[0][1], s);
        world_state.light->color.m[0][2] = LIGHT_BLEND(color.m[0][2], s);
        world_state.light->color.m[1][0] = LIGHT_BLEND(color.m[1][0], s);
        world_state.light->color.m[1][1] = LIGHT_BLEND(color.m[1][1], s);
        world_state.light->color.m[1][2] = LIGHT_BLEND(color.m[1][2], s);
        world_state.light->color.m[2][0] = LIGHT_BLEND(color.m[2][0], s);
        world_state.light->color.m[2][1] = LIGHT_BLEND(color.m[2][1], s);
        world_state.light->color.m[2][2] = LIGHT_BLEND(color.m[2][2], s);
        world_state.light->back.r = LIGHT_BLEND(back.r, s);
        world_state.light->back.g = LIGHT_BLEND(back.g, s);
        world_state.light->back.b = LIGHT_BLEND(back.b, s);
        world_state.light_blend++;
    } else {
        *world_state.light = lighting.light[world_state.selected_light];
    }
    world_state.range_near = world_state.light->depth_cue_near - 1024;
    world_state.range_mid = 4096;
}

/* The file's mesh model, part headers and part vertex arrays. */
#define ANIM_MODEL(file) ((u8 *)(file) + (file)->model)
#define MODEL_PARTS(model) ((MeshHeader *)((model) + 12))
#define PART_VERTICES(model, part) ((SVECTOR *)((model) + (MODEL_PARTS(model)[part].vertices + 12)))

/*
 * Registers the files of a pack (a count, then the files back to back)
 * under consecutive ids from `id`; files of 4 bytes or less are skipped.
 */
ADDRESS(0x800339c4, 0x84)
void anim_register_pack(u16 id, u8 *pack)
{
    ScratchPad *sp = SCRATCHPAD;
    u16 count = *(u16 *)pack;
    StAnimFile *file;
    u_long size;

    pack += 4;
    while (count--) {
        file = (StAnimFile *)pack;
        size = file->size;
        if (size >= 5) {
            world_state.model_files[id] = file;
            sp->model = ANIM_MODEL(file);
        }
        pack += size;
        id++;
    }
}

ADDRESS(0x80033a48, 0x3c)
void anim_register(u16 id, StAnimFile *file)
{
    world_state.model_files[id] = file;
    SCRATCHPAD->model = ANIM_MODEL(file);
}

/* dst = to - from, per vertex. */
ADDRESS(0x80033a84, 0x7c)
void vertices_sub(SVECTOR *dst, SVECTOR *from, SVECTOR *to, int count)
{
    while (count--) {
        dst->vx = to->vx - from->vx;
        dst->vy = to->vy - from->vy;
        dst->vz = to->vz - from->vz;
        dst++;
        from++;
        to++;
    }
}

#include "../lib/anim_vertices.inc"

#include "../lib/anim_vertex_list.inc"

/* dst += the list's vertices scaled by p / 4096 (LIBGTE gteMIMefunc). */
ADDRESS(0x80033cd4, 0x28)
void vertex_list_scale_add(SVECTOR *dst, short *src, long p)
{
    long count = *src;

    src += 2;
    gteMIMefunc(dst, (SVECTOR *)src, count, p);
}

#include "../lib/anim_find_key.inc"

/* The file registered under `id`; a heap-loaded one (id 112 and up) only
 * while its block is releasable or in use. */
ADDRESS(0x80033db8, 0x78)
void anim_get_file(u16 id, StAnimFile **out)
{
    StAnimFile *file = world_state.model_files[id];
    u8 state;

    *out = file;
    if (id >= 112) {
        if (file == NULL) {
            *out = NULL;
        } else {
            state = heap_get_state(file);
            if (state != ST_BLOCK_RELEASABLE && state != ST_BLOCK_USED)
                *out = NULL;
        }
    }
}

/*
 * Vertex `index` of model `id`: of part `part` & 0x7f when `part` is 128
 * or more, else of part 0 posed at `frame` of clip `part`. Returns 1 (and
 * a zero vector) when the model is missing. An old-style definition:
 * callers pass `id` unconverted.
 */
ADDRESS(0x80033e30, 0x22c)
int anim_get_vertex(id, part, frame, index, out)
    u16 id;
    int part;
    int frame;
    int index;
    SVECTOR *out;
{
    StAnimFile *found;
    StAnimFile *file;
    u8 *model;
    SVECTOR *verts;

    anim_get_file(id, &found);
    file = found;
    if (file == NULL) {
        out->vz = 0;
        out->vy = 0;
        out->vx = 0;
        return 1;
    }
    model = ANIM_MODEL(file);
    if (part >= 128) {
    part_vertex:
        verts = PART_VERTICES(model, part & 0x7f);
        *out = verts[index];
    } else if (file->animated == 0) {
        part = 0;
        goto part_vertex;
    } else {
        SVECTOR *from;
        SVECTOR *to;
        StAnimKey *k;
        SVECTOR a;
        SVECTOR d;
        int key;
        int frac;
        u_long *frames;

        verts = PART_VERTICES(model, 0);
        k = anim_find_key(file, part, frame, &key, &frac);
        frames = (u_long *)((u8 *)file + file->frames);
        from = (SVECTOR *)((u8 *)file + frames[k->from]);
        to = (SVECTOR *)((u8 *)file + frames[k->to]);
        a.vx = verts[index].vx + from[index].vx;
        a.vy = verts[index].vy + from[index].vy;
        a.vz = verts[index].vz + from[index].vz;
        d.vx = to[index].vx - from[index].vx;
        d.vy = to[index].vy - from[index].vy;
        d.vz = to[index].vz - from[index].vz;
        out->vx = a.vx + ((d.vx * frac) >> 12);
        out->vy = a.vy + ((d.vy * frac) >> 12);
        out->vz = a.vz + ((d.vz * frac) >> 12);
    }
    return 0;
}

/* The vertex count of model `id`'s part 0 (or of part `part` & 0x7f when
 * `part` is 128 or more); 0 when the model is missing. */
ADDRESS(0x8003405c, 0x68)
u_long anim_vertex_count(int id, int part)
{
    StAnimFile *file = world_state.model_files[id];
    u8 *model;
    u_long count;

    if (file == NULL)
        return 0;
    model = ANIM_MODEL(file);
    if (part < 128)
        count = MODEL_PARTS(model)[0].n_vertices;
    else
        count = MODEL_PARTS(model)[part & 0x7f].n_vertices;
    return count;
}

ADDRESS(0x800340c4, 0x34)
void anim_slots_init(void)
{
    StAnimSlot *slot = world_state.anim_slots;
    u16 i;

    i = 16;
    do {
        slot->state = 4;
        slot->deltas = NULL;
        slot++;
    } while (--i);
}

ADDRESS(0x800340f8, 0x44)
void anim_slots_retire(void)
{
    StAnimSlot *slot = world_state.anim_slots;
    u16 i;

    i = 16;
    do {
        if (slot->state & ~4)
            slot->state = 5;
        slot++;
    } while (--i);
}

/* Frees a slot: clears its owner's pointer and its delta buffer. */
ADDRESS(0x8003413c, 0x64)
void anim_slot_free(StAnimSlot *slot)
{
    if (slot != NULL && (slot->state & 4)) {
        slot->state = 4;
        *slot->owner = NULL;
        if (slot->deltas != NULL) {
            mem_free(slot->deltas);
            slot->deltas = NULL;
        }
    }
}

/* Frees the slots in state 1. */
ADDRESS(0x800341a0, 0x84)
void anim_slots_free_idle(void)
{
    StAnimSlot *slot = world_state.anim_slots;
    u16 i;

    i = 16;
    do {
        if ((slot->state & ~4) == 1)
            anim_slot_free(slot);
        slot++;
    } while (--i);
}

/* Frees every slot in use. */
ADDRESS(0x80034224, 0x70)
void anim_slots_free_all(void)
{
    StAnimSlot *slot = world_state.anim_slots;
    s16 i;

    for (i = 16; i != 0; i--) {
        if (slot->state & ~4)
            anim_slot_free(slot);
        slot++;
    }
}

/* Finds a free slot (freeing the state-1 slots when none is), or NULL. */
ADDRESS(0x80034294, 0xd0)
void anim_slot_alloc(StAnimSlot **out)
{
    u16 i;

    *out = world_state.anim_slots;
    i = 16;
    do {
        if (!((*out)->state & ~4)) {
            (*out)->clip = 255;
            return;
        }
        (*out)++;
    } while (--i);
    anim_slots_free_idle();
    *out = world_state.anim_slots;
    i = 16;
    do {
        if (!((*out)->state & ~4)) {
            (*out)->clip = 255;
            return;
        }
        (*out)++;
    } while (--i);
    *out = NULL;
}

/*
 * Poses model `id` at `frame` of `clip` through the slot at *slotp
 * (claiming one first): the posed vertices go to world_state.posed_vertices and
 * the scratchpad's model-space vertex pointer. Returns 0 when no slot or
 * delta buffer is available.
 */
ADDRESS(0x80034364, 0x2a0)
int anim_pose(StAnimSlot **slotp, int id, int clip, int frame)
{
    ScratchPad *sp = SCRATCHPAD;
    StAnimFile *file = world_state.model_files[id];
    StAnimSlot *slot = *slotp;
    StAnimKey *k;
    int key;
    int frac;
    u8 *model;
    u_long *frames;

    if (slot == NULL) {
        anim_slot_alloc(&slot);
        if (slot == NULL)
            return 0;
    claim:
        slot->model_id = id;
        slot->owner = slotp;
        if (slot->deltas != NULL) {
            mem_free(slot->deltas);
            slot->deltas = NULL;
        }
        slot->deltas = mem_alloc(sp->header->n_vertices * sizeof(SVECTOR));
        if (slot->deltas == NULL) {
            anim_slots_free_idle();
            anim_sets_free_idle();
            anim_slots_free_all();
            anim_sets_free_all();
            slot->owner = NULL;
            slot->model_id = 0xffff;
            return 0;
        }
        *slotp = slot;
    } else if (slot->model_id != id) {
        anim_slot_free(slot);
        slot->clip = 255;
        goto claim;
    }
    k = anim_find_key(file, clip, frame, &key, &frac);
    if (slot->clip != clip || slot->key != key) {
        model = sp->model;
        sp->header = MODEL_PARTS(model);
        sp->local_vertices = model + (MODEL_PARTS(model)->vertices + 12);
        frames = (u_long *)((u8 *)file + file->frames);
        slot->from_keyframe = (SVECTOR *)((u8 *)file + frames[k->from]);
        slot->to_keyframe = (SVECTOR *)((u8 *)file + frames[k->to]);
        vertices_sub(slot->deltas, slot->from_keyframe, slot->to_keyframe, sp->header->n_vertices);
        slot->clip = clip;
        slot->key = key;
    }
    frames = (u_long *)((u8 *)file + file->frames);
    slot->from_keyframe = (SVECTOR *)((u8 *)file + frames[k->from]);
    vertices_add(world_state.posed_vertices, (SVECTOR *)(sp->model + 12 + sp->header->vertices),
                 slot->from_keyframe, sp->header->n_vertices);
    gteMIMefunc(world_state.posed_vertices, slot->deltas, sp->header->n_vertices, frac);
    sp->local_vertices = (u8 *)world_state.posed_vertices;
    slot->state = 6;
    return (int)slot;
}

/*
 * Prepares model `id` for drawing: a model without animation is set up
 * directly (freeing the slot at *slotp) and gives 1; otherwise the result
 * of the posing function for its kind.
 */
ADDRESS(0x80034604, 0xd8)
int anim_prepare(StAnimSlot **slotp, int id, int clip, int frame)
{
    ScratchPad *sp = SCRATCHPAD;
    StAnimSlot *slot = *slotp;
    StAnimFile *file;
    u8 *model;

    if (((StAnimFile *)world_state.model_files[id])->animated == 0) {
        if (slot != NULL) {
            anim_slot_free(slot);
            anim_set_free((StAnimSet *)slot);
        }
        file = world_state.model_files[id];
        model = ANIM_MODEL(file);
        sp->model = model;
        sp->header = MODEL_PARTS(model);
        sp->local_vertices = model + (MODEL_PARTS(model)->vertices + 12);
        return 1;
    }
    if (((StAnimFile *)world_state.model_files[id])->node_count == 0)
        return anim_pose(slotp, id, clip, frame);
    return anim_set_pose((StAnimSet **)slotp, id, clip, frame);
}

#include "../lib/anim_list_init.inc"

#include "../lib/anim_list_find_id.inc"

#include "../lib/anim_list.inc"

/* Unreferenced: the module's last .sdata word (0x80088014), after the menu
 * text's literals and before the effect module's. */
DATA(0x80088014, 0x4, ".sdata")
static u32 DAT_80088014 = 0;
