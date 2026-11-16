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
#include <st/state.h>
#include <st/data.h>
#include <st/gfx.h>
#include <st/funcs.h>

/* The semi-transparency argument that leaves a quad opaque. */
#define OPAQUE 255

/*
 * Links a primitive of `len` words into an OT entry, writing the tag and the
 * entry whole (band C's mesh_add_prim in include/st/mesh.h).
 */
#define gfx_add_prim(ot, p, len)                                                   (*(u_long *)(p) = (*(ot) & 0xffffff) | ((len) << 24),                          *(ot) = (u_long)(p) & 0xffffff)

RODATA(0x80011000, 0xe)
SDATA(0x80087ef4, 0x10)

/* Builds a textured quad in the current frame's work area and draws it at once. */
ADDRESS(0x80020ea8, 0x124)
void func_80020ea8(int x0, int y0, int x1, int y1, u8 u, u8 v, u8 w, u8 h,
                   u8 semi, u16 tpage, u16 clut, u8 r, u8 g, u8 b)
{
    POLY_FT4 *p;

    if (DAT_8015d090.frame >= 2)
        p = (POLY_FT4 *)DAT_8015d090.unkE0 + 4;
    else
        p = (POLY_FT4 *)DAT_8015d090.unkE0 + DAT_8015d090.frame * 4;
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

    p = (POLY_FT4 *)DAT_8015d090.prims->next;
    DAT_8015d090.prims->next = (u8 *)(p + 1);
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
    addPrim(DAT_8015d090.ot, p);
}

ADDRESS(0x800210f4, 0x64)
void func_800210f4(void)
{
    char name[64];

    strcpy(name, "\\LOAD02.TIM;1");
    while (CdSearchFile(&DAT_80088200.unkA6C, name) == NULL)
        ;
}

/*
 * Shows the loading screen: reads \LOAD02.TIM;1 (up to 50 attempts) while a
 * dark translucent quad covers the display, then hands the image to
 * func_80031efc and copies the lower half of VRAM to the upper.
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

    DAT_8015d090.draw[0].isbg = 0;
    DAT_8015d090.draw[1].isbg = 0;
    DAT_8015d090.unk1ECC4 = 1;
    DAT_8015d090.draw[1].dfe = 1;
    DAT_8015d090.draw[0].dfe = 1;
    DAT_8015d090.draw[1].isbg = 0;
    DAT_8015d090.draw[0].isbg = 0;
    PutDrawEnv(&DAT_8015d090.draw[DAT_8015d090.frame]);
    PutDrawEnv(&DAT_8015d090.draw[1 - DAT_8015d090.frame]);
    ClearOTagR(ot, 1);
    setcode(&poly, 0x28);
    setSemiTrans(&poly, 1);
    setXY4(&poly, 0, 0, DAT_8015d090.draw[1 - DAT_8015d090.frame].clip.w, 0,
           0, DAT_8015d090.draw[1 - DAT_8015d090.frame].clip.h,
           DAT_8015d090.draw[1 - DAT_8015d090.frame].clip.w,
           DAT_8015d090.draw[1 - DAT_8015d090.frame].clip.h);
    gfx_add_prim(ot, &poly, 5);
    poly.r0 = poly.g0 = poly.b0 = 8;
    SetDrawMode(&mode, 0, 0, GetTPage(0, 2, 640, 0), NULL);
    addPrim(ot, &mode);
    for (i = 0; i < 50; i++) {
        CdControl(CdlSetloc, (u_char *)&DAT_80088200.unkA6C.pos, NULL);
        CdRead((DAT_80088200.unkA6C.size + 2047) >> 11, DAT_8015d090.unkE0, CdlModeSpeed);
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
    func_80031efc(DAT_8015d090.unkE0, DAT_80088200.unkA6C.size);
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
    DAT_8015d090.draw[0].isbg = 1;
    DAT_8015d090.draw[1].isbg = 1;
    DAT_8015d090.unk1ECC4 = 0;
}

ADDRESS(0x800214cc, 0x4c)
void func_800214cc(void)
{
    if (DAT_8015d090.unk1ECC4) {
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
    StPrimBuffer *buffer = DAT_8015d090.prims;
    POLY_FT4 *p = (POLY_FT4 *)buffer->next;

    if ((u8 *)(p + 1) <= buffer->end) {
        buffer->next = (u8 *)(p + 1);
        DAT_8015d090.unk8328++;
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
        AddPrim(DAT_8015d090.ot + depth, p);
        DAT_8015d090.unk8324++;
    }
}

/*
 * Fades the screen in from black over 120 ticks after DAT_801b5920.unk18 is
 * set, then out.
 *
 * Not exact (size-exact): retail keeps the start tick in $v1 and the elapsed
 * count in $a0, this source the reverse, which shifts the schedule of the
 * level computations.
 */
ADDRESS(0x80021654, 0x1c8)
void func_80021654(void)
{
    int elapsed = DAT_80088200.unk4 - DAT_801b5920.unk18;
    int early;
    int level;

    if (DAT_801b5920.unk0 == 0 && DAT_801b5920.unk18 == -1)
        return;
    if ((s16)DAT_801b5920.unk2 < 2) {
        DAT_801b5920.unk18 = DAT_80088200.unk4;
        return;
    }
    if (elapsed < 120) {
        level = (u8)(elapsed * 2);
        func_80021518(0, 0, 320, 240, 0, 224, 15, 15, 1, getTPage(0, 2, 896, 256),
                      getClut(560, 496), level, level, level, 0);
        return;
    }
    early = elapsed < 240;
    if (early || DAT_801b5920.unk0 != 0) {
        level = (u8)((elapsed - 120) * 2);
        func_80021518(32, 0, 288, 256, 0, 0, 255, 255, 1, getTPage(0, 1, 960, 256),
                      getClut(560, 495), level, level, level, 0);
        if (!early)
            DAT_801b5920.unk18 = DAT_80088200.unk4 - 240;
    } else if (elapsed < 360) {
        level = (u8)((360 - elapsed) * 2);
        func_80021518(32, 0, 288, 256, 0, 0, 255, 255, 1, getTPage(0, 1, 960, 256),
                      getClut(560, 495), level, level, level, 0);
    } else {
        DAT_801b5920.unk18 = -1;
    }
}
