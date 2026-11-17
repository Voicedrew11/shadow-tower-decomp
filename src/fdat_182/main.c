/*
 * FDAT_182.BIN: the handler table and the common functions (WIP module).
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
void func_801ed4b4();
void func_801ed604();
void func_801ed65c();
void func_801ed8a0();
void func_801ed950();

const FdatHeader fdat_header = {
    0x17,
    {
        fdat_801ed498, func_801ed4b4, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, func_801ed4a0, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        func_801ed604, func_801ed950, fdat_801ed498, fdat_801ed498,
        func_801ed65c, func_801ed8a0, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
        fdat_801ed498, fdat_801ed498, fdat_801ed498, fdat_801ed498,
    },
    0x4a,
};

#include "../lib/fdat_common.inc"
