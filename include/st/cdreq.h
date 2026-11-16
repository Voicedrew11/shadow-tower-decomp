/*
 * ST.EXE frame wait, vsync counters and the CD read-request ring
 * callbacks (src/st/cdreq.c).
 */
#ifndef ST_CDREQ_H
#define ST_CDREQ_H

#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBCD.H>
#include <st/state.h>

void func_8001531c(void);
void func_8001535c(void);
void func_800153b4(void);
void func_8001542c(u_char status, u_char *result);
void func_800154e8(u_char status, u_char *result);
void func_8001561c(u_char status, u_char *result);
void func_80015848(StCdRequest *request);
int func_800158f4(u8 bcd);
int func_80015914(u8 value);
u32 func_80015950(CdlLOC *pos);
void func_800159d4(CdlLOC *pos, u32 sector);
void func_80015a94(CdlLOC *pos, int offset, CdlLOC *out);
void func_80015ad4(void);
void func_80015b2c(u8 *flag);

#endif
