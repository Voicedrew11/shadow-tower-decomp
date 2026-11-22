/*
 * ST.EXE XA voice playback for table A entries and still images
 * (src/st/xa.c). Field names stay address-derived.
 */
#ifndef ST_XA_H
#define ST_XA_H

#include <kf/lib/types.h>
#include <sys/types.h>
#include <st/vis.h>

/*
 * The XA sector counts per stream file (game_progress.unk1618) and channel:
 * 48 rows of 6 words, the file count of the save block's per-file arrays
 * (load data, owner unknown).
 */
extern u32 xa_stream_lengths[48][6];

void xa_ready_callback(u8 intr, u8 *result);
/* The channel travels as a word; playback uses its low byte. */
void xa_start_stream(u8 file, int channel);
int xa_stream_finished(void);
void xa_stop_stream(void);
void talk_load_image(int id, int full);
void creature_talk(Vis212 *e, int fixed, int arg2);
void talk_show_message(int id);

#endif
