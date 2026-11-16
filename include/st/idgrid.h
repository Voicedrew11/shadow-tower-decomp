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
    u8 unk0;             /* 0x00: 104 or 98 */
    u8 unk1;
    s16 unk2;            /* 0x02: -1 when unset */
    u8 unk4;             /* 0x04: nonzero while a range is held */
    u8 unk5[3];
    s32 unk8;            /* 0x08: range start, -1 when unset */
    s32 unkC;            /* 0x0c: range end */
    s32 unk10;           /* 0x10 */
} IdGridState;

extern IdGridState DAT_801e0d38;

/* Raw controller bits (active low), owner unknown; func_80068440 polls them,
 * so they change under the code. */
extern volatile s16 DAT_801cd132;

s16 func_8006616c(void);
void func_800661c8(void);
void func_8006620c(u16 archive, u16 member);
void func_80066240(void);
void func_800666b0(void);
void func_80067580(void);
void func_800675f4(void);
void func_80067624(void);
void func_80067bec(u8 grid);
void func_80068258(void);
void func_80068440(void);

#endif
