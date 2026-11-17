/*
 * FDAT_272.BIN: the palette blend (WIP module; a unit of its own until its neighbours in
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

RODATA(0x801eb7d4, 0x8)

/* A palette word: two 15-bit colours and their mask bits. */
typedef struct {
    u32 r0 : 5, g0 : 5, b0 : 5, s0 : 1;
    u32 r1 : 5, g1 : 5, b1 : 5, s1 : 1;
} ClutPair;

/* Steps of the blend left; at 32 the two rows are read first. */
DATA(0x801ef51c, 0x4, ".bss")
static int fdat_801ef51c;

DATA(0x801ef520, 0x200, ".bss")
static ClutPair fdat_801ef520[128];

DATA(0x801ef720, 0x200, ".bss")
static ClutPair fdat_801ef720[128];

/*
 * Blends palette row 489 from row 487 by fdat_801ef51c / 32, a step a
 * call; at 0 loads row 487's copy and ends.
 */
ADDRESS(0x801ef1c0, 0x344)
void func_801ef1c0(void)
{
    RECT dst = {0, 489, 256, 1};
    RECT src = {0, 487, 256, 1};
    ClutPair buf[128];
    int i;
    int t;
    int u;

    if (fdat_801ef51c == 32) {
        StoreImage(&dst, (u_long *)fdat_801ef520);
        StoreImage(&src, (u_long *)fdat_801ef720);
    }
    t = fdat_801ef51c;
    u = 32 - t;
    for (i = 0; i < 128; i++) {
        buf[i].b0 = (fdat_801ef520[i].b0 * t + fdat_801ef720[i].b0 * u) >> 5;
        buf[i].g0 = (fdat_801ef520[i].g0 * t + fdat_801ef720[i].g0 * u) >> 5;
        buf[i].r0 = (fdat_801ef520[i].r0 * t + fdat_801ef720[i].r0 * u) >> 5;
        buf[i].b1 = (fdat_801ef520[i].b1 * t + fdat_801ef720[i].b1 * u) >> 5;
        buf[i].g1 = (fdat_801ef520[i].g1 * t + fdat_801ef720[i].g1 * u) >> 5;
        buf[i].r1 = (fdat_801ef520[i].r1 * t + fdat_801ef720[i].r1 * u) >> 5;
        buf[i].s0 = fdat_801ef520[i].s0;
        buf[i].s1 = fdat_801ef520[i].s1;
    }
    LoadImage(&dst, (u_long *)buf);
    if (fdat_801ef51c == 0) {
        LoadImage(&dst, (u_long *)fdat_801ef720);
        fdat_801ef930 = -1;
        vis_lists.unk47E0 |= 0x10;
    } else {
        fdat_801ef51c--;
    }
}
