/*
 * ST.EXE pause menu entry and loop (src/st/menu.c) and the equipment
 * pictures and attribute totals of the status screens (src/st/menu.c),
 * WIP modules. The scratchpad they use is st/menu.h's MenuScratch.
 */
#ifndef ST_MENU_MAIN_H
#define ST_MENU_MAIN_H

#include <kf/lib/types.h>
#include <sys/types.h>

/* The menu pages menu_main_run runs (not reconstructed yet). */
void menu_area_info_run(void);
int menu_use_item_run(void);
void menu_equip_run(void);
void menu_arrange_icons_run(void);
void menu_arrange_list_run(void);
void menu_creature_book_run(void);
void menu_equip_info_run(void);
void menu_options_run(void);
int menu_end_game_run(void);

/* src/st/menu.c */
void menu_store_screen(void);
void menu_restore_screen(void);
void menu_load_main_messages(void);
void menu_show_loading_screen(void);
void menu_reload_assets(void);
void menu_main_run(void);

/* src/st/menu.c */
void equip_load_slot_picture(int slot);
void equip_apply_slot_change(int redraw);
void equip_restore_slots(void);
void player_total_attributes(void);

#endif
