/*
 * VS.EXE's large buffer at 0x800633b0 (bss, the first .bss object): two
 * 319720-byte sound bank body buffers, also used as the card read buffer
 * of a save record. It ends where the menu sound bank's body begins.
 */
#ifndef VS_BUFFER_H
#define VS_BUFFER_H

#include <kf/lib/types.h>
#include <sys/types.h>
#include <vs/card.h>

#define VS_BANK_BODY_SIZE 0x4e0e8

typedef union {
    VsSave save;                      /* func_8001b504's card read */
    u8 bank[2][VS_BANK_BODY_SIZE];    /* func_80012a70's VAB bodies */
} VsBuffer;

extern VsBuffer DAT_800633b0;

#endif
