/*
 * ST.EXE main, shutdown and FN. table reader (src/st/main.c) and the game
 * loop (src/st/loop.c).
 */
#ifndef ST_MAIN_H
#define ST_MAIN_H

#include <kf/lib/types.h>

void game_shutdown(void);
int text_read_number(u8 **cursor, int digits);
void game_load_start_file(void);
void game_loop(void);

#endif
