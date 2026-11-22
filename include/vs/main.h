/*
 * VS.EXE: globals of the start-up, round loop and main module
 * (fighters_load_round..main, src/vs/main.c).
 */
#ifndef VS_MAIN_H
#define VS_MAIN_H

#include <kf/lib/types.h>
#include <sys/types.h>

/*
 * Words at the end of the game .rdata, before LIBCD's (as ST.EXE's
 * DAT_80013980..; st/data.h): 0x80011f40 and 0x80011f48 bound the load
 * image's end, and main stores the heap base at 0x80011f4c. Their owner is
 * unknown; main addresses each with its own lui pair, as separate scalars.
 */
extern u8 *DAT_80011f40;
extern u8 *DAT_80011f48;
extern u8 *DAT_80011f4c;

/* A file load buffer (bss; WIP extent: the item name file match_load_resources
 * reads, a word then the 0x5400-byte table). */
extern u_long DAT_80117708[];

/* Three 5000-byte file buffers (bss, up to DAT_8015ffc0). */
extern u8 DAT_8015c528[5000];
extern u8 DAT_8015d8b0[5000];

/*
 * The round state (.sbss, allocated by an earlier object; this module
 * reads most of it $gp-relative through tentative definitions, the words
 * below absolutely).
 */
extern s32 DAT_80063334;  /* the second player's requested item id, -1 for none */
extern s32 DAT_80063374;  /* the second fighter's rounds won */
extern u8 DAT_80063380;
extern u8 DAT_80063381;

#endif
