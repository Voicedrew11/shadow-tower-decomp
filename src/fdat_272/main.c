/*
 * FDAT_272.BIN: the handler table and the common functions (WIP module).
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

void func_801ed4d0();
void func_801ed760();
void func_801edad8();
void func_801edd24();
void func_801ee43c();
void func_801ee544();

const FdatHeader fdat_header = {
    0x20,
    {
        fdat_801ed498, func_801ed4d0, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, func_801ed760, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, func_801ee43c, fdat_801ed498, fdat_801ed498,
        func_801ee544, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        func_801edad8, func_801edd24, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
    },
    0x53,
};

#include "../lib/fdat_common.inc"
