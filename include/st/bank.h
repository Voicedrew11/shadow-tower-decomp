/*
 * ST.EXE archive member loaders, their completion callbacks and the sound
 * bank slots (src/st/bank.c, src/st/bankload.c).
 */
#ifndef ST_BANK_H
#define ST_BANK_H

#include <kf/lib/types.h>

u16 func_80023208(u16 id);
void func_80023288(void *data);
void func_80023388(void *data);
void func_80023470(void *data);
void func_80023604(void *data);
void func_80023788(void *data);
/* Old-style definitions; these are their promoted prototypes. */
void func_800237a8(int archive, int index, int slot, int tag);
void func_800239ec(int archive, int index, int slot, int tag);
void func_80023abc(int archive, int index, int slot, int tag);

#endif
