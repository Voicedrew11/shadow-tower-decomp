/*
 * VS.EXE's large buffer at 0x800633b0 (bss, the first .bss object): two
 * 319720-byte sound bank body buffers, also used as the card read buffer
 * of a save record, and the two fighters (VsFighter, vs/data.h: each
 * 0x4e0e8-byte half holds a fighter's model file and state). It ends where
 * the menu sound bank's body begins.
 */
#ifndef VS_BUFFER_H
#define VS_BUFFER_H

#include <kf/lib/types.h>
#include <sys/types.h>
#include <vs/card.h>
#include <vs/data.h>

#define VS_BANK_BODY_SIZE 0x4e0e8

typedef union {
    VsSave save;                      /* card_load_game's card read */
    u8 bank[2][VS_BANK_BODY_SIZE];    /* sound_bank_open's VAB bodies */
    VsFighter fighter[2];             /* the two fighters (src/vs/fighter_*.c) */
} VsBuffer;

extern VsBuffer DAT_800633b0;

#endif
