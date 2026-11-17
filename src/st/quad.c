/*
 * ST.EXE: textured-quad helpers, the loading display and the timed screen
 * fade (WIP module). Its unit is -G8: func_80021158 copies its two RECTs
 * from .sdata literals. The module ends at func_80021654, which is not
 * exact, so that the fade channel functions after it keep their retail
 * offsets in their own unit (src/st/fade.c).
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBETC.H>
#include <LIBCD.H>
#include <psyq/libc.h>
#include <st/cd.h>
#include <st/data.h>
#include <st/gfx.h>
#include <st/mesh.h>
#include <st/funcs.h>
#include <st/tim.h>

/* The semi-transparency argument that leaves a quad opaque. */
#define OPAQUE 255

RODATA(0x80011000, 0x10)
SDATA(0x80087ef4, 0x10)

/* Builds a textured quad in the current frame's work area and draws it at once. */
ADDRESS(0x80020ea8, 0x124)
void func_80020ea8(int x0, int y0, int x1, int y1, u8 u, u8 v, u8 w, u8 h,
                   u8 semi, u16 tpage, u16 clut, u8 r, u8 g, u8 b)
{
    POLY_FT4 *p;

    if (world_state.frame >= 2)
        p = (POLY_FT4 *)world_state.unkE0 + 4;
    else
        p = (POLY_FT4 *)world_state.unkE0 + world_state.frame * 4;
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
void func_80020fcc(int x0, int y0, int x1, int y1, int u, int v, int w, int h,
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
void func_800210f4(void)
{
    char name[64];

    strcpy(name, "\\LOAD02.TIM;1");
    while (CdSearchFile(&cd_state.unkA6C, name) == NULL)
        ;
}

/*
 * Shows the loading screen: reads \LOAD02.TIM;1 (up to 50 attempts) while a
 * dark translucent quad covers the display, then hands the image to
 * tim_load_images and copies the lower half of VRAM to the upper.
 *
 * Not exact (size-exact): retail sets up the 0x18000 field offset in $s1
 * before the RECT copy and builds the POLY_F4 tag before its XY stores; this
 * source does both later.
 */
ADDRESS(0x80021158, 0x34c)
void func_80021158(void)
{
    u_char result[8];
    POLY_F4 poly;
    DR_MODE mode;
    RECT clear = {0, 0, 320, 480};
    u_long ot[1];
    int status;
    int i;

    world_state.draw[0].isbg = 0;
    world_state.draw[1].isbg = 0;
    world_state.unk1ECC4 = 1;
    world_state.draw[1].dfe = 1;
    world_state.draw[0].dfe = 1;
    world_state.draw[1].isbg = 0;
    world_state.draw[0].isbg = 0;
    PutDrawEnv(&world_state.draw[world_state.frame]);
    PutDrawEnv(&world_state.draw[1 - world_state.frame]);
    ClearOTagR(ot, 1);
    setcode(&poly, 0x28);
    setSemiTrans(&poly, 1);
    setXY4(&poly, 0, 0, world_state.draw[1 - world_state.frame].clip.w, 0,
           0, world_state.draw[1 - world_state.frame].clip.h,
           world_state.draw[1 - world_state.frame].clip.w,
           world_state.draw[1 - world_state.frame].clip.h);
    mesh_add_prim(ot, &poly, 5);
    poly.r0 = poly.g0 = poly.b0 = 8;
    SetDrawMode(&mode, 0, 0, GetTPage(0, 2, 640, 0), NULL);
    addPrim(ot, &mode);
    for (i = 0; i < 50; i++) {
        CdControl(CdlSetloc, (u_char *)&cd_state.unkA6C.pos, NULL);
        CdRead((cd_state.unkA6C.size + 2047) >> 11, world_state.unkE0, CdlModeSpeed);
        while ((status = CdReadSync(1, result)) > 0) {
            VSync(0);
            DrawOTag(ot);
        }
        if (status == 0)
            break;
    }
    ClearImage(&clear, 0, 0, 0);
    DrawSync(0);
    func_800173a8();
    tim_load_images(world_state.unkE0, cd_state.unkA6C.size);
    {
        RECT lower = {0, 240, 320, 240};

        VSync(0);
        MoveImage(&lower, 0, 0);
    }
    DrawSync(0);
    SetDispMask(1);
}

ADDRESS(0x800214a4, 0x28)
void func_800214a4(void)
{
    world_state.draw[0].isbg = 1;
    world_state.draw[1].isbg = 1;
    world_state.unk1ECC4 = 0;
}

ADDRESS(0x800214cc, 0x4c)
void func_800214cc(void)
{
    if (world_state.unk1ECC4) {
        func_800174c4();
        func_80017680();
    }
}

/*
 * Allocates a textured quad, when the buffer has room, and adds it to OT
 * entry `depth`. An old-style definition: the narrow parameters arrive as
 * ints that the body loads whole and stores narrow, which a prototype would
 * not produce; callers outside this module need the promoted prototype.
 */
ADDRESS(0x80021518, 0x13c)
void func_80021518(x0, y0, x1, y1, u, v, w, h, semi, tpage, clut, r, g, b, depth)
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
        world_state.unk8328++;
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
        world_state.unk8324++;
    }
}

/*
 * Fades the screen in from black over 120 ticks after area_state.unk18 is
 * set, then out.
 *
 * Not exact: retail keeps start tick in v1 and elapsed in a0; this
 * source uses the reverse. Color is narrowed earlier, and the 360-tick
 * test does not fill the preceding branch delay slot.
 */
ADDRESS(0x80021654, 0x1c8)
void func_80021654(void)
{
    int elapsed = cd_state.unk4 - area_state.unk18;
    int early;
    int level;

    if (area_state.unk0 == 0 && area_state.unk18 == -1)
        return;
    if ((s16)area_state.unk2 < 2) {
        area_state.unk18 = cd_state.unk4;
        return;
    }
    if (elapsed < 120) {
        level = (u8)(elapsed * 2);
        func_80021518(0, 0, 320, 240, 0, 224, 15, 15, 1, getTPage(0, 2, 896, 256),
                      getClut(560, 496), level, level, level, 0);
        return;
    }
    early = elapsed < 240;
    if (early || area_state.unk0 != 0) {
        level = (u8)((elapsed - 120) * 2);
        func_80021518(32, 0, 288, 256, 0, 0, 255, 255, 1, getTPage(0, 1, 960, 256),
                      getClut(560, 495), level, level, level, 0);
        if (!early)
            area_state.unk18 = cd_state.unk4 - 240;
    } else if (elapsed < 360) {
        level = (u8)((360 - elapsed) * 2);
        func_80021518(32, 0, 288, 256, 0, 0, 255, 255, 1, getTPage(0, 1, 960, 256),
                      getClut(560, 495), level, level, level, 0);
    } else {
        area_state.unk18 = -1;
    }
}
