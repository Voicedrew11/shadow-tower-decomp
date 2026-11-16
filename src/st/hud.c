/*
 * ST.EXE: the HUD sprite table and the per-frame world update (WIP module).
 * HUD elements are 20-byte sprite records drawn in runs that end at a
 * record whose flags byte is 0xff; func_80026508 refreshes the bars, the
 * numbers and the status icons from the viewer block, and func_80026afc
 * runs one frame of the world and draws its 2D overlays.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/data.h>
#include <st/gfx.h>
#include <st/hud.h>
#include <st/vis.h>
#include <st/text.h>
#include <st/funcs.h>

SDATA(0x80087f14, 0x4)

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
    world_state.unk831C[3]++

/* A sprite at its texture size, behind its draw mode. */
ADDRESS(0x80025c24, 0x1dc)
void func_80025c24(rec, abr, color)
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
    world_state.unk831C[2] += 2;
}

/* A sprite scaled to its rectangle. */
ADDRESS(0x80025e00, 0x220)
void func_80025e00(rec, abr, color)
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
    world_state.unk831C[2]++;
}

/* An untextured rectangle, behind its draw mode. */
ADDRESS(0x80026020, 0x208)
void func_80026020(rec, abr, color)
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
    world_state.unk831C[2] += 2;
}

/*
 * Shows `value` (at most 9999) in the four digit records from `first`:
 * leading zeros are hidden and each digit picks its glyph `width` texels
 * apart from u = 96.
 */
/* Not exact: retail forms the third digit record's index (first + 2) in $a0
 * before the tens subtraction; this build forms it in $v1 after it. */
ADDRESS(0x80026228, 0x2e0)
void func_80026228(int value, int first, u8 width)
{
    short n;
    short d;

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
    n -= d * 10;
    hud_sprites[first + 2].u = d * width + 96;
    hud_sprites[first + 3].u = n * width + 96;
}

/*
 * Not exact: after the redraw block retail rematerializes &viewer in $a3 and
 * branches to the lui in the delay slot; this build keeps $s0 and reloads
 * the base piecewise.
 */
ADDRESS(0x80026508, 0x45c)
void func_80026508(void)
{
    if (viewer.unk2 != viewer.unk18A || viewer.unk6 != viewer.unk18C) {
        viewer.unk186 = 2;
        viewer.unk18A = viewer.unk2;
        viewer.unk18C = viewer.unk6;
    }
    if (viewer.unk186 != 0) {
        if (viewer.unk186 == 2) {
            hud_sprites[23].flags = HUD_END;
            hud_sprites[0].flags |= 1;
            hud_sprites[1].flags |= 1;
            hud_sprites[2].flags |= 2;
            hud_sprites[3].flags |= 2;
            hud_sprites[13].flags |= 1;
            func_80026228(viewer.unk2, 14, 16);
            func_80026228(viewer.unk6, 19, 16);
            viewer.unk186 = 0;
        }
    }
    if (viewer.unk186 == 0) {
        hud_sprites[0].r.w = viewer.unk17E * 112 / 5120;
        hud_sprites[1].r.w = viewer.unk182 * 112 / 5120;
        hud_sprites[1].r.x = 307 - hud_sprites[1].r.w;
    }
    if (viewer.unk2C & 7) {
        hud_sprites[4].flags = 1;
        if (viewer.unk2C & 4)
            hud_sprites[4].flags = 9;
        else if (viewer.unk2C & 2)
            hud_sprites[4].flags = 5;
    } else {
        hud_sprites[4].flags = 0;
    }
    if (viewer.unk2C & 0x18) {
        hud_sprites[7].flags = 1;
        if (viewer.unk2C & 0x10)
            hud_sprites[7].flags = 9;
    } else {
        hud_sprites[7].flags = 0;
    }
    if (viewer.unk2C & 0x60) {
        hud_sprites[9].flags = 1;
        if (viewer.unk2C & 0x40)
            hud_sprites[9].flags = 9;
    } else {
        hud_sprites[9].flags = 0;
    }
    if (viewer.unk2C & 0x80)
        hud_sprites[11].flags |= 1;
    else
        hud_sprites[11].flags = 0;
    if (viewer.unk2C & 0x100)
        hud_sprites[12].flags |= 1;
    else
        hud_sprites[12].flags = 0;
    if (viewer.unk1A4[0] == 0)
        hud_sprites[37].flags = 0;
    else
        hud_sprites[37].flags |= 1;
    if (viewer.unk1A4[7] == 0)
        hud_sprites[38].flags = 0;
    else
        hud_sprites[38].flags |= 1;
    if (viewer.unk1A4[2] == 0)
        hud_sprites[39].flags = 0;
    else
        hud_sprites[39].flags |= 1;
    if (viewer.unk1A4[3] == 0)
        hud_sprites[40].flags = 0;
    else
        hud_sprites[40].flags |= 1;
    if (viewer.unk1A4[4] == 0)
        hud_sprites[41].flags = 0;
    else
        hud_sprites[41].flags |= 1;
    if (viewer.unk1A4[5] == 0)
        hud_sprites[42].flags = 0;
    else
        hud_sprites[42].flags |= 1;
    if (viewer.unk1A4[6] == 0)
        hud_sprites[43].flags = 0;
    else
        hud_sprites[43].flags |= 1;
    if (viewer.unk2C & 0x1000000)
        hud_sprites[44].flags |= 1;
    else
        hud_sprites[44].flags = 0;
}

/* Draws the HUD records from `first` up to the next end marker. */
ADDRESS(0x80026964, 0x124)
void func_80026964(u8 first)
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
                func_80025e00(rec, rec->abr, &color);
            else
                func_80025c24(rec, rec->abr, &color);
        } else if (rec->flags & 2) {
            func_80026020(rec, rec->abr, &color);
        }
    }
}

/* Queues `value` in a free one of the five HUD number slots for 60 frames. */
/* Not exact: retail keeps value in $a3 and -1 in $a2 and fills the first
 * beq delay slot with the lui of DAT_801cd123; this build swaps the two
 * registers and leaves a nop. */
ADDRESS(0x80026a88, 0x74)
void func_80026a88(short value)
{
    int i;

    if (value != -1 && DAT_801cd123 != 0) {
        for (i = 4; i != -1; i--) {
            if (viewer.unk1EC[i] == 0) {
                viewer.unk1EC[i] = 60;
                viewer.unk20C[i] = value;
                break;
            }
        }
    }
}

/* Blink phase of func_80026afc's selection frame (0..31). */
DATA(0x80087f18, 0x4, ".sdata")
static s32 DAT_80087f18 = 0;

/* Not exact: retail fills the delay slot of the unk218 == 1 test with the
 * lui of DAT_801cd123 from the branch target; this build leaves a nop. */
ADDRESS(0x80026afc, 0x2d8)
void func_80026afc(VECTOR *pos, SVECTOR *rot)
{
    RECT unused;
    CVECTOR color;
    StHudSprite *frame;
    int level;
    int i;

    func_80057b10();
    func_800176a8(pos, rot);
    func_80016a14();
    if (viewer.unk218 == 0 || viewer.unk218 == 5 || viewer.unk218 == 6 || viewer.unk218 == 1)
        func_80033110();
    func_800174c4();
    func_800340f8();
    func_80034fc4();
    world_state.unk831C[1] = 0;
    if (viewer.unk218 == 1) {
        frame = &hud_sprites[26 + viewer.unk15E];
        DAT_80087f18 = (DAT_80087f18 + 1) % 32;
        if (DAT_80087f18 < 16)
            level = 8 - (u16)DAT_80087f18;
        else
            level = (u16)DAT_80087f18 - 24;
        color.b = color.g = color.r = level * 4 + 96;
        func_80027d18(&frame->r, &color, 257, 1);
        func_80026964(26);
    }
    if (DAT_801cd123 != 0 && area_state.area < 44) {
        func_80026508();
        func_80026964(0);
        for (i = 4; i != -1; i--) {
            if (viewer.unk1EC[i] != 0) {
                hud_sprites[35].r.y = i * 16 + 144;
                hud_sprites[34].r.y = i * 16 + 144;
                hud_sprites[33].r.y = i * 16 + 144;
                hud_sprites[32].r.y = i * 16 + 144;
                func_80026228(viewer.unk20C[i], 32, 8);
                func_80026964(32);
                viewer.unk1EC[i]--;
            }
        }
        func_80026964(37);
    }
    func_8002181c();
    func_800218d8();
    func_80021654();
    func_80021ac4(0);
    func_80021ac4(1);
    func_80020a80();
    vis_collect_table_a();
    vis_collect_table_b();
    vis_collect_table_c();
    func_800241d0();
    func_800246e4();
    func_800251c8();
    func_800255f0();
    if (world_state.unk1EE16 != 0)
        func_80028784();
    func_80017680();
    func_8001535c();
    world_state.unk1ECC0++;
}

ADDRESS(0x80026dd4, 0x30)
void func_80026dd4(VECTOR *pos, SVECTOR *rot, int arg)
{
    func_80026afc(pos, rot);
    func_8003c24c(arg);
}

/*
 * Moves the second packet buffer up to make room below it and stores the
 * current drawing area there (the screen behind a menu); both drawing
 * environments then clear their background.
 */
ADDRESS(0x80026e04, 0x8c)
void func_80026e04(void)
{
    DrawSync(0);
    world_state.packets[1].start = world_state.packets[0].end = world_state.packets[0].start + 0x3500;
    world_state.packets[1].end = world_state.packets[1].start + 0x3500;
    StoreImage(&world_state.draw[world_state.id].clip, world_state.packets[1].end);
    world_state.draw[0].isbg = 1;
    world_state.draw[1].isbg = 1;
}

/* Restores the two packet buffers to their full size. */
ADDRESS(0x80026e90, 0x3c)
void func_80026e90(void)
{
    world_state.draw[0].isbg = 1;
    world_state.draw[1].isbg = 1;
    world_state.packets[1].start = world_state.packets[0].end = world_state.packets[0].start + 0x8000;
    world_state.packets[1].end = world_state.packets[1].start + 0x8000;
}
