/*
 * ST.EXE controller (LIBPAD direct connection) state and the functions that
 * read it (src/st/pad.c). Field names stay address-derived until their
 * behaviour is proven; the state, the vibration bytes and the button masks
 * live in DAT_801cd180 (st/data.h).
 */
#ifndef ST_PAD_H
#define ST_PAD_H

#include <kf/lib/types.h>
#include <sys/types.h>

/*
 * The LIBPAD direct-connection receive buffers and actuator data at
 * 0x801cd130. LIBPAD's interrupt handler fills the buffers, so the object
 * is volatile.
 */
typedef struct {
    u8 unk0;             /* 0x00: receive status, 0 when the data is valid */
    u8 unk1;
    u16 unk2;            /* 0x02: button bits, active low */
    u8 unk4[0x22 - 0x4];
    u8 unk22[0x22];      /* 0x22: port 2 receive buffer */
    u8 unk44[4];         /* 0x44: actuator data PadSetAct reads (+1 small motor, +3 large) */
    u8 unk48[4];
} StPadData;

extern volatile StPadData DAT_801cd130;

void func_8005738c(void);
void func_800573bc(void);
void func_80057428(void);
void func_800574c0(void);
void func_8005755c(void);
void func_800575a0(void);
void func_80057684(void);
void func_80057744(void);
void func_80057a28(void);
void func_80057a3c(void);
void func_80057a68(int arg0);
void func_80057b10(void);

#endif
