/*
 * VS.EXE: the round-win markers, the round timer, the message banners and
 * two screen overlays, all drawn near the front of the frame's ordering
 * table (WIP module).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <vs/data.h>

/* func_8002a360's switch table. */
JUMPTABLES(0x80011524, 0x50)

/*
 * One pulsing red marker per round won (DAT_80063370) for each fighter,
 * left to right for the first and right to left for the second; `phase`
 * (0..95) cycles the gradient, `scale` its brightness.
 */
ADDRESS(0x80029f28, 0x1fc)
void func_80029f28(int phase, int scale)
{
    POLY_G4 *p;
    int i;
    int j;
    int left;
    int top;
    int bottom;

    if (phase < 32) {
        bottom = phase * 4 + 64;
        top = 64;
    } else if (phase < 64) {
        bottom = 192 - (phase - 32) * 4;
        top = (phase - 32) * 4 + 64;
    } else if (phase < 96) {
        bottom = 64;
        top = 192 - (phase - 64) * 4;
    }
    for (j = 0; j < 2; j++) {
        for (i = 0; i < DAT_80063370[j]; i++) {
            p = (POLY_G4 *)vs_gfx_state.packet->cur;
            vs_gfx_state.packet->cur = (u_long *)(p + 1);
            setlen(p, 8);
            setcode(p, 58);
            if (j == 0)
                left = i * 15 + 20;
            else
                left = 290 - i * 15;
            setXY4(p, left, 210, left + 10, 210, left, 220, left + 10, 220);
            setRGB0(p, top * scale / 32, 0, 0);
            setRGB1(p, top * scale / 32, 0, 0);
            setRGB2(p, bottom * scale / 32, 0, 0);
            setRGB3(p, bottom * scale / 32, 0, 0);
            addPrim(&vs_gfx_state.ot_cur[1], p);
        }
    }
}

/*
 * The two-digit round timer `value` at level `level` (0..16); the digit
 * cases for value - 1 are unused.
 */
ADDRESS(0x8002a124, 0x23c)
void func_8002a124(int value, int level)
{
    POLY_FT4 *p;
    int i;
    int digit;

    if (level > 16)
        level = 16;
    if (level < 0)
        level = 0;
    for (i = 0; i < 2; i++) {
        p = (POLY_FT4 *)vs_gfx_state.packet->cur;
        vs_gfx_state.packet->cur = (u_long *)(p + 1);
        setlen(p, 9);
        setcode(p, 46);
        setXY4(p, 128 + i * 32, 16, 160 + i * 32, 16, 128 + i * 32, 48, 160 + i * 32, 48);
        switch (i) {
        case 0:
            digit = value / 10;
            break;
        case 1:
            digit = value % 10;
            break;
        case 2:
            digit = (value - 1) / 10;
            break;
        case 3:
            digit = (value - 1) % 10;
            break;
        }
        if (digit < 5) {
            setUV4(p, digit * 32, 0, digit * 32 + 32, 0, digit * 32, 32, digit * 32 + 32, 32);
        } else {
            digit -= 5;
            setUV4(p, digit * 32, 32, digit * 32 + 32, 32, digit * 32, 64, digit * 32 + 32, 64);
        }
        p->clut = 32290;
        p->tpage = 61;
        setRGB0(p, level * 8, level * 8, level * 8);
        addPrim(&vs_gfx_state.ot_cur[3], p);
    }
}

/*
 * Message banner `kind` (0..19; 6 and 8 set no rectangle):
 * its screen rectangle and texture rectangle, the first nine from texture
 * page 62, the rest from 63, at level `level` (0..15; white for kinds 17
 * and 18, else red).
 */
ADDRESS(0x8002a360, 0x660)
void func_8002a360(int kind, int level)
{
    POLY_FT4 *p;

    if (level > 15)
        level = 15;
    if (level < 0)
        level = 0;
    p = (POLY_FT4 *)vs_gfx_state.packet->cur;
    vs_gfx_state.packet->cur = (u_long *)(p + 1);
    setlen(p, 9);
    setcode(p, 46);
    switch (kind) {
    case 0:
        setXY4(p, 96, 80, 223, 80, 96, 135, 223, 135);
        setUV4(p, 0, 0, 127, 0, 0, 55, 126, 55);
        break;
    case 1:
        setXY4(p, 32, 80, 159, 80, 32, 135, 159, 135);
        setUV4(p, 128, 0, 255, 0, 128, 55, 255, 55);
        break;
    case 2:
        setXY4(p, 112, 80, 207, 80, 112, 135, 207, 135);
        setUV4(p, 0, 56, 95, 56, 0, 111, 95, 111);
        break;
    case 3:
        setXY4(p, 128, 80, 191, 80, 128, 135, 191, 135);
        setUV4(p, 96, 56, 159, 56, 96, 111, 159, 111);
        break;
    case 4:
        setXY4(p, 176, 80, 271, 80, 176, 135, 271, 135);
        setUV4(p, 160, 56, 255, 56, 160, 111, 255, 111);
        break;
    case 5:
        setXY4(p, 32, 80, 287, 80, 32, 135, 287, 135);
        setUV4(p, 0, 112, 255, 112, 0, 167, 255, 167);
        break;
    case 7:
        setXY4(p, 80, 80, 239, 80, 80, 135, 239, 135);
        setUV4(p, 0, 168, 159, 168, 0, 223, 159, 223);
        break;
    case 9:
        setXY4(p, 74, 99, 154, 99, 74, 115, 154, 115);
        setUV4(p, 124, 1, 204, 1, 124, 15, 204, 15);
        break;
    case 10:
        setXY4(p, 102, 100, 172, 100, 102, 115, 172, 115);
        setUV4(p, 130, 33, 200, 33, 130, 47, 200, 47);
        break;
    case 11:
        setXY4(p, 175, 99, 273, 99, 175, 115, 273, 115);
        setUV4(p, 3, 1, 111, 1, 3, 15, 111, 15);
        break;
    case 12:
        setXY4(p, 120, 90, 199, 90, 120, 105, 199, 105);
        setUV4(p, 106, 198, 186, 198, 106, 214, 186, 214);
        break;
    case 13:
        setXY4(p, 43, 100, 101, 100, 43, 115, 101, 115);
        setUV4(p, 0, 17, 58, 17, 0, 31, 58, 31);
        break;
    case 14:
        setXY4(p, 110, 130, 224, 130, 110, 145, 224, 145);
        setUV4(p, 60, 17, 174, 17, 60, 31, 174, 31);
        break;
    case 15:
        setXY4(p, 51, 100, 172, 100, 51, 115, 172, 115);
        setUV4(p, 0, 34, 121, 34, 0, 48, 121, 48);
        break;
    case 16:
        setXY4(p, 173, 99, 273, 99, 173, 115, 273, 115);
        setUV4(p, 3, 1, 111, 1, 3, 15, 111, 15);
        break;
    case 17:
        setXY4(p, 110, 130, 209, 130, 110, 146, 209, 146);
        setUV4(p, 0, 199, 100, 199, 0, 216, 100, 216);
        break;
    case 18:
        setXY4(p, 110, 155, 209, 155, 110, 171, 209, 171);
        setUV4(p, 0, 218, 99, 218, 0, 234, 99, 234);
        break;
    case 19:
        setXY4(p, 154, 99, 252, 99, 154, 115, 252, 115);
        setUV4(p, 3, 1, 111, 1, 3, 15, 111, 15);
        break;
    }
    if (kind < 9) {
        p->clut = 32162;
        p->tpage = 62;
    } else {
        p->clut = 32226;
        p->tpage = 63;
    }
    if (kind == 17 || kind == 18)
        setRGB0(p, level * 8, level * 8, level * 8);
    else
        setRGB0(p, level * 16, 0, 0);
    addPrim(&vs_gfx_state.ot_cur[5], p);
}

/*
 * A textured banner of half-height `size` centred at (160, 107), red at
 * `level` (0..15); `kind` 16, 17 or 18 picks its texture row.
 */
ADDRESS(0x8002a9c0, 0x198)
void func_8002a9c0(int kind, int level, int size)
{
    POLY_FT4 *p;

    if (level > 15)
        level = 15;
    if (level < 0)
        level = 0;
    p = (POLY_FT4 *)vs_gfx_state.packet->cur;
    vs_gfx_state.packet->cur = (u_long *)(p + 1);
    setlen(p, 9);
    setcode(p, 46);
    switch (kind) {
    case 16:
        setUV4(p, 0, 48, 209, 48, 0, 97, 209, 97);
        break;
    case 17:
        setUV4(p, 0, 98, 209, 98, 0, 147, 209, 147);
        break;
    case 18:
        setUV4(p, 0, 148, 209, 148, 0, 197, 209, 197);
        break;
    }
    setXY4(p, 160 - size * 4, 107 - size, 160 + size * 4, 107 - size, 160 - size * 4, 107 + size,
           160 + size * 4, 107 + size);
    p->clut = 32226;
    p->tpage = 63;
    setRGB0(p, level * 16, 0, 0);
    addPrim(&vs_gfx_state.ot_cur[5], p);
}

/*
 * A semi-transparent blue box (wide or narrow) at `level` (0..15), then an
 * empty textured triangle that sets the draw mode for it.
 */
ADDRESS(0x8002ab58, 0x1e8)
void func_8002ab58(int level, int wide)
{
    POLY_F4 *p;
    POLY_FT3 *t;

    if (level > 15)
        level = 15;
    if (level < 0)
        level = 0;
    p = (POLY_F4 *)vs_gfx_state.packet->cur;
    vs_gfx_state.packet->cur = (u_long *)(p + 1);
    setlen(p, 5);
    setcode(p, 42);
    if (wide)
        setXY4(p, 10, 80, 309, 80, 10, 185, 309, 185);
    else
        setXY4(p, 80, 80, 239, 80, 80, 185, 239, 185);
    setRGB0(p, 0, 0, level * 4);
    addPrim(&vs_gfx_state.ot_cur[5], p);
    t = (POLY_FT3 *)vs_gfx_state.packet->cur;
    vs_gfx_state.packet->cur = (u_long *)(t + 1);
    SetPolyFT3(t);
    t->clut = 32641;
    setXY3(t, 0, 0, 0, 0, 0, 0);
    setUV3(t, 0, 0, 0, 0, 0, 0);
    t->tpage = 24;
    setRGB0(t, 0, 0, 0);
    addPrim(&vs_gfx_state.ot_cur[5], t);
}
