/*
 * FDAT_372.BIN: its handler table, then fdat_801ebec4 (whose switch
 * table follows the handler table) and fdat_801ec6a4 (WIP module).
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

RODATA(0x801eb560, 0x1cc)

void func_801ed4a0();
void func_801ed510();
void func_801ed688();
void func_801ed904();
void func_801ed9b4();

const FdatHeader fdat_header = {
    0x2a,
    {
        fdat_801ed498, func_801ed510, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, func_801ed4a0, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, func_801ed9b4, fdat_801ed498, fdat_801ed498,
        func_801ed688, func_801ed904, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
    },
    0x5d,
};

#include "../lib/fdat_common.inc"
