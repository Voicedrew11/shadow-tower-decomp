/*
 * ST.EXE placement records and TIM upload (src/st/tim.c).
 */
#ifndef ST_TIM_H
#define ST_TIM_H

#include <kf/lib/types.h>
#include <sys/types.h>

/* One 16-byte placement record as placements_expand reads it. */
typedef struct {
    u16 id;              /* 0xffff: empty */
    u8 frame_count;
    u8 frame_period;
    u8 kind;
    u8 bx;               /* 0x05: coordinate blocks of 4096 */
    u8 by;
    u8 bz;
    u8 turn;
    u8 unk9;
    s16 x;               /* 0x0a: offsets inside the blocks */
    s16 y;
    s16 z;
} StPlacement;

/* placements_expand fills placement_objects.objects (PlacedObject, st/gfx.h). */

/* A TIM file header: its id word and its flags (8: a CLUT block follows). */
typedef struct {
    u_long id;
    u_long flag;
} TimHeader;

/* A TIM CLUT or pixel block header: its byte count (header included) and
 * its VRAM rectangle, whose size is read unsigned. */
typedef struct {
    u_long bnum;
    u_short x;
    u_short y;
    u_short w;
    u_short h;
} TimBlock;

void placements_expand(StPlacement *src);
int tim_load_images(u_long *buf, u_long size);

#endif
