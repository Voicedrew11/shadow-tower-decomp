/*
 * ST.EXE memory helpers (src/st/memory.c): word/halfword copy and fill, the
 * block heap whose blocks carry a 12-byte header, and the malloc3 wrappers.
 */
#ifndef ST_MEMORY_H
#define ST_MEMORY_H

#include <kf/lib/types.h>

/*
 * Block heap header; the block's data follows it. A run of blocks ends at
 * a header whose state is ST_BLOCK_END.
 */
typedef struct {
    u8 state; /* ST_BLOCK_* */
    u8 unk1;
    u16 unk2;
    s32 size; /* data bytes after this header */
    void **owner; /* pointer to the data, cleared when the block is released */
} StBlock;

/* The header after a block, and the header of a block's data. */
#define NEXT_BLOCK(block) ((StBlock *)((u8 *)(block) + ((block)->size + sizeof(StBlock))))
#define DATA_BLOCK(data) ((StBlock *)(data) - 1)

enum {
    ST_BLOCK_FREE = 0,
    ST_BLOCK_RELEASABLE = 1, /* reclaimed when no free block is large enough */
    ST_BLOCK_USED = 2,
    ST_BLOCK_LOADING = 3, /* waited for before the heap is compacted */
    ST_BLOCK_END = 0xff
};

u32 *func_80014cfc(u32 *dst, u32 *src, int count);
u16 *func_80014d2c(u16 *dst, u16 *src, int count);
u32 *func_80014d5c(u32 *dst, u32 value, int count);
u16 *func_80014d80(u16 *dst, u16 value, int count);
void func_80014da4(StBlock *block);
void func_80014e20(StBlock *block);
StBlock *func_80014e4c(StBlock *heap, u32 size);
void func_80014f90(StBlock *heap);
void func_80015024(StBlock *heap);
void func_80015144(StBlock *heap, int size);
void *func_80015164(StBlock *heap, int size, void **owner);
void func_80015230(void *data);
void func_80015250(void *data, u8 state);
u8 func_80015258(void *data);
void func_80015264(void *data, u8 value);
u8 func_8001526c(void *data);
void func_80015278(void *data, u16 value);
u16 func_80015280(void *data);
void *func_8001528c(u32 size);
void *func_800152d0(u32 size);
void func_800152f8(void *p);

#endif
