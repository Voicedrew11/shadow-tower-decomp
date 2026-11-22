/*
 * ST.EXE id grid screen (src/st/idgrid.c, WIP): its state block and
 * functions. Field names stay address-derived until behaviour is proven.
 */
#ifndef ST_IDGRID_H
#define ST_IDGRID_H

#include <kf/lib/types.h>
#include <sys/types.h>

/*
 * The state block at 0x801e0d38, just before menu_messages: the grid the
 * cursor is in ('h' 104 or 'b' 98), a pending selection range in grid
 * entries and its anchor. WIP extent.
 */
typedef struct {
    u8 grid;             /* 0x00: 104 or 98 */
    u8 unk1;
    s16 shown_id;        /* 0x02: -1 when unset */
    u8 range_held;       /* 0x04: nonzero while a range is held */
    u8 unk5[3];
    s32 range_start;     /* 0x08: range start, -1 when unset */
    s32 range_end;       /* 0x0c: range end */
    s32 range_anchor;    /* 0x10 */
} ArrangeListState;

extern ArrangeListState arrange_list_state;

/* Raw controller bits (active low), owner unknown; debug_pan_display polls them,
 * so they change under the code. */

s16 cd_queue_busy(void);
void menu_arrange_list_upload_image(void);
void menu_arrange_list_queue_image(u16 archive, u16 member);
void menu_arrange_list_open(void);
void menu_arrange_list_loop(void);
void menu_arrange_list_close(void);
void menu_arrange_list_run(void);
void menu_arrange_list_draw_details(void);
void menu_arrange_list_draw_list(u8 grid);
void menu_arrange_list_draw(void);
void debug_pan_display(void);

#endif
