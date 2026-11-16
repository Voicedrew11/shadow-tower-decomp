/*
 * ST.EXE pause menu entry and loop (src/st/menu_main.c) and the equipment
 * pictures and attribute totals of the status screens (src/st/menu_pics.c),
 * WIP modules. The scratchpad they use is st/menu.h's MenuScratch.
 */
#ifndef ST_MENU_MAIN_H
#define ST_MENU_MAIN_H

#include <kf/lib/types.h>
#include <sys/types.h>

/* The menu pages func_80058ef4 runs (not reconstructed yet). */
void func_80060018(void);
int func_8005ec10(void);
void func_80064ff0(void);
void func_8005d25c(void);
void func_800675f4(void);
void func_8005c7fc(void);
void func_8005ad00(void);
void func_80060334(void);
int func_80063078(void);

/* src/st/menu_main.c */
void func_80058c04(void);
void func_80058c24(void);
void func_80058c4c(void);
void func_80058cc8(void);
void func_80058d30(void);
void func_80058ef4(void);

/* src/st/menu_pics.c */
void func_80059204(int slot);
void func_8005975c(int redraw);
void func_80059cec(void);
void func_80059dc0(void);

#endif
