/*
 * ST.EXE game functions called across band B's units (and from them into
 * other bands). Address-derived names stay until behaviour is proven.
 */
#ifndef ST_GAME_H
#define ST_GAME_H

#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBCD.H>
#include <st/state.h>

extern void func_80014d2c(u16 *dst, u16 *src, int count);
extern void *func_800152d0(int size);
extern void func_800152f8(void *p);
extern void func_800153b4(void);
extern void func_8001561c(u_char status, u_char *result);
extern void func_80015a94(CdlLOC *base, int sector, CdlLOC *loc);
extern void func_80015ad4(void);
extern void func_80015b2c(StCdRequest *req);
extern void func_80016878(void);
extern void func_800214cc(void);
extern void func_800340c4(void);
extern void func_80034f48(void);
extern void func_800498cc(MATRIX *m, MATRIX *out);
extern void func_80049cd4(int angle, MATRIX *out);

#endif
