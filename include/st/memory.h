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
    u8 tag;
    u16 slot;
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

u32 *copy_words(u32 *dst, u32 *src, int count);
u16 *copy_halfwords(u16 *dst, u16 *src, int count);
u32 *fill_words(u32 *dst, u32 value, int count);
u16 *fill_halfwords(u16 *dst, u16 value, int count);
void heap_merge_free(StBlock *block);
void heap_block_release(StBlock *block);
StBlock *heap_find_free(StBlock *heap, u32 size);
void heap_wait_loading(StBlock *heap);
void heap_compact(StBlock *heap);
void heap_init(StBlock *heap, int size);
void *heap_alloc(StBlock *heap, int size, void **owner);
void heap_free(void *data);
void heap_set_state(void *data, u8 state);
u8 heap_get_state(void *data);
void heap_set_tag(void *data, u8 value);
u8 heap_get_tag(void *data);
void heap_set_slot(void *data, u16 value);
u16 heap_get_slot(void *data);
void *mem_alloc_checked(u32 size);
void *mem_alloc(u32 size);
void mem_free(void *p);

#endif
