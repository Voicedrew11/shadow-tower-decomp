/*
 * FDAT_242.BIN: the handler table and the common functions (WIP module).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <psyq/libc.h>
#include <st/cd.h>
#include <st/gfx.h>
#include <st/anim.h>
#include <st/sound.h>
#include <st/data.h>
#include <st/hud.h>
#include <st/xa.h>
#include <st/vis.h>
#include <st/fdat.h>

RODATA(0x801eb560, 0x88)

void func_801ed4e0();
void func_801ed71c();
void func_801edaa4();
void func_801edb10();
void func_801edd54();
void func_801ede3c();

const FdatHeader fdat_header = {
    0x1d,
    {
        fdat_801ed498, func_801ed71c, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, func_801ed4e0, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        func_801edaa4, func_801ede3c, fdat_801ed498, fdat_801ed498,
        func_801edb10, func_801edd54, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
    },
    0x50,
};

#include "../lib/fdat_common.inc"
