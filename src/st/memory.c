/*
 * ST.EXE: word/halfword copy and fill helpers and the first block heap
 * functions: merge, release and search (WIP module). The module ends at
 * heap_find_free, which is not exact, so that the heap functions after it
 * keep their retail offsets in their own unit (src/st/heap.c).
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <st/memory.h>
#include <st/cd.h>
#include <st/funcs.h>

/* Returns the end of the source. */
ADDRESS(0x80014cfc, 0x30)
u32 *copy_words(u32 *dst, u32 *src, int count)
{
    int left;

    while ((left = count, count = count - 1, left))
        *dst++ = *src++;
    return src;
}

ADDRESS(0x80014d2c, 0x30)
u16 *copy_halfwords(u16 *dst, u16 *src, int count)
{
    int left;

    while ((left = count, count = count - 1, left))
        *dst++ = *src++;
    return src;
}

ADDRESS(0x80014d5c, 0x24)
u32 *fill_words(u32 *dst, u32 value, int count)
{
    int left;

    while ((left = count, count = count - 1, left))
        *dst++ = value;
    return dst;
}

ADDRESS(0x80014d80, 0x24)
u16 *fill_halfwords(u16 *dst, u16 value, int count)
{
    int left;

    while ((left = count, count = count - 1, left))
        *dst++ = value;
    return dst;
}

/* Merges the free blocks that follow a run of free blocks into `block`. */
ADDRESS(0x80014da4, 0x7c)
void heap_merge_free(StBlock *block)
{
    StBlock *p = block;

    while (p->state != ST_BLOCK_END) {
        if (p->state == ST_BLOCK_FREE) {
            p = NEXT_BLOCK(p);
            if (p->state != ST_BLOCK_FREE)
                return;
            block->size += p->size + sizeof(StBlock);
        } else {
            p = NEXT_BLOCK(p);
        }
    }
}

ADDRESS(0x80014e20, 0x2c)
void heap_block_release(StBlock *block)
{
    block->state = ST_BLOCK_FREE;
    if (block->owner != NULL) {
        *block->owner = NULL;
        block->owner = NULL;
    }
    block->unk1 = 0xff;
    block->unk2 = 0xffff;
}

/*
 * First free block of at least `size` bytes; failing that, the first run of
 * releasable blocks spanning that many, released and merged.
 */
ADDRESS(0x80014e4c, 0x144)
StBlock *heap_find_free(StBlock *heap, u32 size)
{
    StBlock *p;
    StBlock *run;

    for (p = heap; p->state != ST_BLOCK_END; p = NEXT_BLOCK(p)) {
        if (p->state == ST_BLOCK_FREE && p->size >= size)
            return p;
    }
    for (p = heap; p->state != ST_BLOCK_END;) {
        if (p->state == ST_BLOCK_RELEASABLE) {
            for (run = p; p->state == ST_BLOCK_RELEASABLE; p = NEXT_BLOCK(p))
                ;
            if ((u8 *)p - (u8 *)run - sizeof(StBlock) >= size) {
                for (p = run; p->state == ST_BLOCK_RELEASABLE; p = NEXT_BLOCK(p))
                    heap_block_release(p);
                heap_merge_free(run);
                return run;
            }
        } else {
            p = NEXT_BLOCK(p);
        }
    }
    return NULL;
}
