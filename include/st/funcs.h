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

void func_800210f4(void);
void func_80021158(void);
void func_800214a4(void);
void func_800214cc(void);
void func_80026afc(VECTOR *a, SVECTOR *b);
void func_80032058(void);
void func_8003216c(void);
void func_8003306c(void);
void func_800340c4(void);
void func_800341a0(void);
void func_80034224(void);
void func_80034f48(void);
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
void func_800498cc(MATRIX *m, MATRIX *out);
void func_80049cd4(int angle, MATRIX *out);
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

#endif
