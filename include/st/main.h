/*
 * ST.EXE main, shutdown and FN. table reader (src/st/main.c) and the game
 * loop (src/st/loop.c).
 */
#ifndef ST_MAIN_H
#define ST_MAIN_H

#include <kf/lib/types.h>

void func_80014410(void);
int func_80014450(u8 **cursor, int digits);
void func_80014544(void);
void func_80014a30(void);

#endif
