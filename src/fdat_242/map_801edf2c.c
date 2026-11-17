/*
 * FDAT_242.BIN: the texture page move and the exit scene word fdat_801ee0b4 (WIP module; a unit of its own until its neighbours in
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

RODATA(0x801eb7bc, 0x30)

/* The exit scene state: 3 once the animation ends, 4 played. */
DATA(0x801ee0b4, 0x4, ".bss")
int fdat_801ee0b4;

/* Not exact (one attempt, retries off; 98.3%): first divergence retail `lui a1,0x0`, this source `lui a1,0x0`. */
/* Moves the finished texture animation's pages and palette rows into
 * place and ends the exit scene. */
ADDRESS(0x801edf2c, 0x184)
void func_801edf2c(void)
{
    RECT a = {496, 0, 16, 64};
    RECT b = {480, 160, 16, 96};
    RECT c = {800, 224, 32, 32};
    RECT d = {512, 485, 16, 1};
    RECT e = {512, 486, 16, 1};
    RECT f = {0, 490, 256, 1};

    MoveImage(&a, 480, 0);
    MoveImage(&b, 480, 64);
    MoveImage(&c, 768, 224);
    MoveImage(&d, 512, 483);
    MoveImage(&e, 512, 484);
    MoveImage(&f, 0, 490);
    fdat_801ee0b4 = 4;
    vis_lists.unk47E0 |= 8;
}
