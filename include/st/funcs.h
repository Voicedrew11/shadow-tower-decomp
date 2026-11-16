/*
 * Prototypes of ST.EXE game functions that no source reconstructs yet. Each
 * signature is the WIP one its callers' argument registers support; the
 * function moves to its module's header once that module exists.
 */
#ifndef ST_FUNCS_H
#define ST_FUNCS_H

#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>

void func_80016374(void);
void func_80016454(void);
void func_800164b4(void);
void func_80016508(void);
void func_8001618c(void *buf, char *name, int size);
void func_80016fb8(void);
int func_80015b78(u8 *buf, s32 sectors);
void func_80026afc(VECTOR *a, SVECTOR *b);
void func_80032058(void);
void func_8003216c(void);
void func_8003306c(void);
void func_800341a0(void);
void func_80034224(void);
void func_8003509c(void);
void func_80035120(void);
void func_8003bc10(void);
void func_8003bc40(void);
void func_8003c0e0(void);
void func_8003cedc(void);
void func_8003e734(VECTOR *a, SVECTOR *b);
void func_8003eaf8(void);
void func_8003ee38(u16 id, int slot);
void func_8003f12c(u16 id, int slot, int arg2);
void func_80042d74(void);
void func_80044674(void);
void func_80044ee8(void);
void func_800456f8(void);
void func_80045ffc(void);
void func_8004afdc(void);
void func_8004e324(void);
void func_8004eda8(void);
void func_800500ac(int arg0, int arg1, int arg2, int arg3);
void func_80050768(void);
void func_80052ca4(void);
void func_80053aa4(void);
void func_800559bc(void);
void func_8005735c(void);
void func_80057684(void);
void func_80057744(void);
void func_80057a28(void);
void func_80057a3c(void);
void func_80057da0(void);
void func_80057f3c(void);
void func_80058288(VECTOR *a, SVECTOR *b);
void func_80058620(void);
void func_80058dc4(int arg0, int arg1);
void func_80058ed4(void);
int func_80062c28(int arg0);
void func_800633d4(void);

/* Band D's callees. */
void func_800173a8(void);
void func_800174c4(void);
void func_80017680(void);
void func_80031efc(u_long *buf, u_long size);
int func_80015d54(int archive, int index, void *dest, void (*callback)(void *));
u32 func_80015ec8(u16 archive, u16 index);
void func_800177a0(u8 *data, DVECTOR *a, DVECTOR *b, u8 count);
void func_8003ec5c(void *data, DVECTOR *a, DVECTOR *b, int arg3);
void func_80058a0c(int archive, int index, int slot);

#endif
