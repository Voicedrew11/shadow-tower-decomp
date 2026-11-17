/*
 * FDAT_152.BIN: the handler table and the common functions (WIP module).
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

void func_801ed4a0();
void func_801ed530();
void func_801ed5e0();
void func_801ed5e8();
void func_801ed6b0();
void func_801ed8f4();

const FdatHeader fdat_header = {
    0x14,
    {
        fdat_801ed498, func_801ed530, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, func_801ed4a0, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, func_801ed5e0, fdat_801ed498, fdat_801ed498,
        func_801ed5e8, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        func_801ed6b0, func_801ed8f4, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
    },
    0x47,
};

#include "../lib/fdat_common.inc"
