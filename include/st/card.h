/*
 * ST.EXE memory card access (src/st/menu.c). The save screens keep their
 * card state in the menu scratchpad block (MenuCardScratch, st/menu.h).
 */
#ifndef ST_CARD_H
#define ST_CARD_H

#include <kf/lib/types.h>

/*
 * The 512-byte header of a save file (the memory card block format): the
 * "SC" magic, the icon type and block count, the Shift-JIS title, the
 * game's own bytes in the title's tail (copied into the file's CardFileEntry),
 * then the icon CLUT and frames.
 */
/* The header's 64-byte title: Shift-JIS text, then the game's own bytes,
 * which card_read_file_headers copies into the file's CardFileEntry. */
typedef struct {
    u8 text[0x39];       /* 0x00 */
    u8 area;             /* 0x39: the area saved in */
    u16 sp_total;        /* 0x3a: viewer.sp_total */
    u16 items_found;     /* 0x3c: viewer.items_found */
    u8 data_sum;         /* 0x3e: the sum of the data frames' sums */
    u8 unk3F;
} CardTitle;

typedef struct {
    char magic[2];       /* 0x00: "SC" */
    u8 icon;             /* 0x02: 0x12: two icon frames */
    u8 blocks;           /* 0x03 */
    CardTitle title;     /* 0x04 */
    u8 unk44[0x1c];      /* 0x44 */
    u16 clut[16];        /* 0x60: the icon CLUT */
    u8 icons[0x100];     /* 0x80: two 16x16 4-bit icon frames */
    u8 unk180[0x80];     /* 0x180 */
} CardHeader;

void card_clear_sw_events(void);
void card_clear_hw_events(void);
void card_open_events(void);
void card_close_events(void);
int card_wait_sw_event(void);
int card_wait_hw_event(void);
void card_read_directory(int port);
void card_probe(int port, int mode);
int card_check_inserted(int port);
int card_ask_yes_no(void);
int card_choose_port(void);
int card_read_file_headers(void);
int card_choose_file(void);
int card_write_blocks(int fd, int offset, u8 *data, int size);
int card_read_blocks(int fd, int offset, u8 *data, int size);
int card_load_game(u8 *data);
int card_save_game(u8 *data);
long card_format(int port);
int card_screen_run(int arg);
int menu_end_game_run(void);
void pause_screen_run(void);
int menu_game_over_run(void);

/* src/st/menu.c */
int menu_attribute_points_run(int points);

/* src/st/menu.c */
void shop_open(void);
void shop_restore_texture_page(void);

/* src/st/menu.c */
void menu_options_run(void);

/* src/st/menu_equip_open.c */
void menu_equip_save_vram(void);
void menu_equip_restore_vram(void);
void menu_equip_open(void);
void menu_equip_close(void);

#endif
