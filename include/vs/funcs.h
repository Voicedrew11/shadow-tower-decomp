/*
 * VS.EXE functions called across reconstructed units. Most are VS's own
 * copies of ST.EXE functions; each comment names the ST twin.
 */
#ifndef VS_FUNCS_H
#define VS_FUNCS_H

#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBCD.H>
#include <st/cd.h>

/* src/vs/cd_8001cb34.c */
void func_8001cb34(void);                 /* ST func_8001531c */
void func_8001d1b8(void);                 /* ST func_800174c4 */
void func_8001d374(void);                 /* ST func_80017680 */

/* src/vs/cd_8002e5e4.c */
void func_8002e5e4(void);                 /* ST func_800214cc */
void *func_8002e690(u32 size);            /* ST func_8001528c */
void *func_8002e6d4(u32 size);            /* ST func_800152d0 */
void func_8002e6fc(void *p);              /* ST func_800152f8 */
void func_8002e720(u_char status, u_char *result); /* ST func_800154e8 */
void func_8002e868(u_char status, u_char *result); /* ST func_8001561c */
void func_8002ea94(StCdRequest *request); /* ST func_80015848 */
u32 func_8002eb9c(CdlLOC *pos);           /* ST func_80015950 */
void func_8002ece0(CdlLOC *pos, int offset, CdlLOC *out); /* ST func_80015a94 */
int func_8002ed20(u32 *data, int sectors); /* ST func_80015b78 */

/* Second-half VS functions called from the first half. */
void *func_8002398c(int size);            /* returns a buffer the caller passes to free() */

#endif
