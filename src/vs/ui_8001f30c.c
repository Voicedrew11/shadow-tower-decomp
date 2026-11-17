/*
 * VS.EXE: the menu window frame (WIP module), VS's copy of ST.EXE's
 * func_80029618 (src/st/ui.c) without the closing wash.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <vs/menu.h>

/* func_8001e530 modes: a semi-transparent tile, flipped. */
#define TILE (1)
#define TILE_HFLIP (1 | 1 << 5)
#define TILE_VFLIP (1 | 2 << 5)

/*
 * Draws a window frame from 16x16 tiles of the menu texture page: corners,
 * edges, an inner bevel and the filled middle.
 */
/* Not exact: register allocation, as ST.EXE's copy: retail keeps h in $s6
 * and spills only w; this build spills both. */
ADDRESS(0x8001f30c, 0x51c)
void func_8001f30c(short x, short y, short w, short h)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    short right;
    short bottom;
    int i, j;

    sp->unk224.w = 16;
    sp->unk224.h = 16;
    sp->unk244.vx = 544;
    sp->unk244.vy = 504;
    sp->unk240.vx = 612;
    sp->unk224.x = x;
    sp->unk224.y = y;
    sp->unk240.vy = 400;
    func_8001e530(&sp->unk224, &sp->unk240, &sp->unk244, NULL, TILE);
    sp->unk224.y = y + h - 16;
    func_8001e530(&sp->unk224, &sp->unk240, &sp->unk244, NULL, TILE_VFLIP);
    right = x + w - 16;
    sp->unk224.y = y;
    sp->unk240.vx = 620;
    sp->unk224.x = right;
    func_8001e530(&sp->unk224, &sp->unk240, &sp->unk244, NULL, TILE);
    sp->unk224.y = y + h - 16;
    func_8001e530(&sp->unk224, &sp->unk240, &sp->unk244, NULL, TILE_VFLIP);
    sp->unk224.x = x;
    sp->unk224.y = y + 16;
    sp->unk240.vx = 604;
    func_8001e530(&sp->unk224, &sp->unk240, &sp->unk244, NULL, TILE);
    sp->unk224.y = y + h - 32;
    func_8001e530(&sp->unk224, &sp->unk240, &sp->unk244, NULL, TILE_VFLIP);
    sp->unk224.y = y + 16;
    sp->unk240.vx = 596;
    sp->unk224.x = right;
    func_8001e530(&sp->unk224, &sp->unk240, &sp->unk244, NULL, TILE);
    sp->unk224.y = y + h - 32;
    func_8001e530(&sp->unk224, &sp->unk240, &sp->unk244, NULL, TILE_VFLIP);

    sp->unk224.y = y + 32;
    sp->unk224.x = x;
    sp->unk240.vx = 624;
    i = h / 16 - 4;
    while (i--) {
        sp->unk224.x = x;
        func_8001e530(&sp->unk224, &sp->unk240, &sp->unk244, NULL, TILE_HFLIP);
        sp->unk224.x = right;
        func_8001e530(&sp->unk224, &sp->unk240, &sp->unk244, NULL, TILE);
        sp->unk224.y += 16;
    }
    sp->unk224.x = x + 16;
    sp->unk240.vx = 616;
    i = w / 16 - 2;
    while (i--) {
        sp->unk224.y = y;
        func_8001e530(&sp->unk224, &sp->unk240, &sp->unk244, NULL, TILE);
        sp->unk224.y = y + h - 16;
        func_8001e530(&sp->unk224, &sp->unk240, &sp->unk244, NULL, TILE_VFLIP);
        sp->unk224.x += 16;
    }
    sp->unk224.x = x + 16;
    sp->unk240.vx = 608;
    i = w / 16 - 2;
    while (i--) {
        sp->unk224.y = y + 16;
        func_8001e530(&sp->unk224, &sp->unk240, &sp->unk244, NULL, TILE);
        sp->unk224.y = y + h - 32;
        func_8001e530(&sp->unk224, &sp->unk240, &sp->unk244, NULL, TILE_VFLIP);
        sp->unk224.x += 16;
    }
    sp->unk224.y = y + 32;
    sp->unk240.vx = 600;
    j = h / 16 - 4;
    while (j--) {
        sp->unk224.x = x + 16;
        i = w / 16 - 2;
        while (i--) {
            func_8001e530(&sp->unk224, &sp->unk240, &sp->unk244, NULL, TILE);
            sp->unk224.x += 16;
        }
        sp->unk224.y += 16;
    }
}
