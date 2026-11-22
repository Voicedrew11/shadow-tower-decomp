/*
 * ST.EXE: block heap functions (wait, compact, initialise, allocate, header
 * accessors) and the malloc3 wrappers (WIP module). Split from
 * src/st/memory.c only because heap_find_free there is not exact.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <MALLOC.H>
#include <st/memory.h>
#include <st/cd.h>
#include <st/funcs.h>
#include <st/anim.h>

ADDRESS(0x80014f90, 0x94)
void heap_wait_loading(StBlock *heap)
{
    StBlock *p;

    for (p = heap; p->state != ST_BLOCK_END; p = NEXT_BLOCK(p)) {
        while (p->state == ST_BLOCK_LOADING)
            cd_wait_idle();
    }
}

/* Moves the used blocks down over the free ones and updates their owners. */
/* Not exact: the final end-header test loads dst->state into v0 and uses
 * beq v0,v1; retail loads v1 and compares v1,v0. */
ADDRESS(0x80015024, 0x120)
void heap_compact(StBlock *heap)
{
    StBlock *dst = heap;
    StBlock *p = heap;
    StBlock *next;
    u32 *from;
    u32 *to;
    u32 words;

    heap_wait_loading(heap);
    while (p->state != ST_BLOCK_END) {
        next = (StBlock *)((u8 *)p + p->size + sizeof(StBlock));
        if (p->state == ST_BLOCK_RELEASABLE)
            heap_block_release(p);
        if (p->state == ST_BLOCK_USED) {
            if (p != dst) {
                from = (u32 *)p;
                to = (u32 *)dst;
                words = (p->size + sizeof(StBlock)) >> 2;
                do {
                    *to++ = *from++;
                } while (--words);
                anim_slots_free_idle();
                anim_sets_free_idle();
                anim_slots_free_all();
                anim_sets_free_all();
                *dst->owner = dst + 1;
                dst = (StBlock *)to;
            } else {
                dst = next;
            }
        }
        p = next;
    }
    if (dst->state != ST_BLOCK_END) {
        dst->state = ST_BLOCK_FREE;
        dst->size = (u8 *)p - (u8 *)dst - sizeof(StBlock);
        dst->unk1 = 0;
        dst->unk2 = 0;
        dst->owner = NULL;
    }
}

/* One free block spanning `size` bytes, closed by an end header. */
ADDRESS(0x80015144, 0x20)
void heap_init(StBlock *heap, int size)
{
    heap->state = ST_BLOCK_FREE;
    heap->size = size - 2 * sizeof(StBlock);
    size -= sizeof(StBlock);
    heap = (StBlock *)((u8 *)heap + size);
    heap->state = ST_BLOCK_END;
}

/* Allocates `size` bytes, compacting the heap if needed; NULL when full. */
ADDRESS(0x80015164, 0xcc)
void *heap_alloc(StBlock *heap, int size, void **owner)
{
    StBlock *block;
    StBlock *rest;
    void *data;
    s32 avail;
    int left;

    block = heap_find_free(heap, size);
    if (block == NULL) {
        heap_compact(heap);
        block = heap_find_free(heap, size);
        if (block == NULL)
            return NULL;
    }
    avail = block->size - sizeof(StBlock);
    left = avail - size;
    if (left >= 0x80c) {
        rest = (StBlock *)((u8 *)block + size + sizeof(StBlock));
        rest->state = ST_BLOCK_FREE;
        rest->size = left;
        rest->unk1 = 0;
        rest->unk2 = 0;
        rest->owner = NULL;
    } else {
        size = block->size;
    }
    block->state = ST_BLOCK_USED;
    block->size = size;
    block->unk1 = 0;
    block->unk2 = 0;
    block->owner = owner;
    data = block + 1;
    if (owner != NULL)
        *owner = data;
    return data;
}

ADDRESS(0x80015230, 0x20)
void heap_free(void *data)
{
    heap_block_release(DATA_BLOCK(data));
}

ADDRESS(0x80015250, 0x8)
void heap_set_state(void *data, u8 state)
{
    DATA_BLOCK(data)->state = state;
}

ADDRESS(0x80015258, 0xc)
u8 heap_get_state(void *data)
{
    return DATA_BLOCK(data)->state;
}

ADDRESS(0x80015264, 0x8)
void heap_set_tag(void *data, u8 value)
{
    DATA_BLOCK(data)->unk1 = value;
}

ADDRESS(0x8001526c, 0xc)
u8 heap_get_tag(void *data)
{
    return DATA_BLOCK(data)->unk1;
}

ADDRESS(0x80015278, 0x8)
void heap_set_slot(void *data, u16 value)
{
    DATA_BLOCK(data)->unk2 = value;
}

ADDRESS(0x80015280, 0xc)
u16 heap_get_slot(void *data)
{
    return DATA_BLOCK(data)->unk2;
}

/* malloc3, or NULL when the result is not in main RAM. */
ADDRESS(0x8001528c, 0x44)
void *mem_alloc_checked(u32 size)
{
    u8 *p = malloc3(size);

    if ((u32)(p - (u8 *)0x80000000) > 0x1fffff)
        return NULL;
    return p;
}

ADDRESS(0x800152d0, 0x28)
void *mem_alloc(u32 size)
{
    return mem_alloc_checked((size + 7) & ~7);
}

/*
 * free3 of a non-NULL pointer. The body equals LIBSN's _OP_DELE
 * (__builtin_delete) instruction for instruction, but it calls free3 and
 * ST.EXE links no LIBSN member, so it is game code.
 */
ADDRESS(0x800152f8, 0x24)
void mem_free(void *p)
{
    if (p != NULL)
        free3(p);
}
