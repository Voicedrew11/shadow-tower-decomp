/*
 * ST.EXE: block heap functions (wait, compact, initialise, allocate, header
 * accessors) and the malloc3 wrappers (WIP module). Split from
 * src/st/memory.c only because func_80014e4c there is not exact.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <MALLOC.H>
#include <st/memory.h>
#include <st/cdreq.h>
#include <st/funcs.h>

ADDRESS(0x80014f90, 0x94)
void func_80014f90(StBlock *heap)
{
    StBlock *p;

    for (p = heap; p->state != ST_BLOCK_END; p = NEXT_BLOCK(p)) {
        while (p->state == ST_BLOCK_LOADING)
            func_80015ad4();
    }
}

/* Moves the used blocks down over the free ones and updates their owners. */
ADDRESS(0x80015024, 0x120)
void func_80015024(StBlock *heap)
{
    StBlock *dst = heap;
    StBlock *p = heap;
    StBlock *next;
    u32 *from;
    u32 *to;
    u32 words;

    func_80014f90(heap);
    if (heap->state == ST_BLOCK_END)
        return;
    do {
        next = (StBlock *)((u8 *)p + p->size + sizeof(StBlock));
        if (p->state == ST_BLOCK_RELEASABLE)
            func_80014e20(p);
        if (p->state == ST_BLOCK_USED) {
            if (p != dst) {
                from = (u32 *)p;
                to = (u32 *)dst;
                words = (p->size + sizeof(StBlock)) >> 2;
                do {
                    *to++ = *from++;
                } while (--words);
                func_800341a0();
                func_8003509c();
                func_80034224();
                func_80035120();
                *dst->owner = dst + 1;
                dst = (StBlock *)to;
            } else {
                dst = next;
            }
        }
        p = next;
    } while (p->state != ST_BLOCK_END);
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
void func_80015144(StBlock *heap, int size)
{
    heap->state = ST_BLOCK_FREE;
    heap->size = size - 2 * sizeof(StBlock);
    size -= sizeof(StBlock);
    heap = (StBlock *)((u8 *)heap + size);
    heap->state = ST_BLOCK_END;
}

/* Allocates `size` bytes, compacting the heap if needed; NULL when full. */
ADDRESS(0x80015164, 0xcc)
void *func_80015164(StBlock *heap, int size, void **owner)
{
    StBlock *block;
    StBlock *rest;
    void *data;
    s32 avail;
    int left;

    block = func_80014e4c(heap, size);
    if (block == NULL) {
        func_80015024(heap);
        block = func_80014e4c(heap, size);
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
void func_80015230(void *data)
{
    func_80014e20(DATA_BLOCK(data));
}

ADDRESS(0x80015250, 0x8)
void func_80015250(void *data, u8 state)
{
    DATA_BLOCK(data)->state = state;
}

ADDRESS(0x80015258, 0xc)
u8 func_80015258(void *data)
{
    return DATA_BLOCK(data)->state;
}

ADDRESS(0x80015264, 0x8)
void func_80015264(void *data, u8 value)
{
    DATA_BLOCK(data)->unk1 = value;
}

ADDRESS(0x8001526c, 0xc)
u8 func_8001526c(void *data)
{
    return DATA_BLOCK(data)->unk1;
}

ADDRESS(0x80015278, 0x8)
void func_80015278(void *data, u16 value)
{
    DATA_BLOCK(data)->unk2 = value;
}

ADDRESS(0x80015280, 0xc)
u16 func_80015280(void *data)
{
    return DATA_BLOCK(data)->unk2;
}

/* malloc3, or NULL when the result is not in main RAM. */
ADDRESS(0x8001528c, 0x44)
void *func_8001528c(u32 size)
{
    u8 *p = malloc3(size);

    if ((u32)(p - (u8 *)0x80000000) > 0x1fffff)
        return NULL;
    return p;
}

ADDRESS(0x800152d0, 0x28)
void *func_800152d0(u32 size)
{
    return func_8001528c((size + 7) & ~7);
}

/*
 * free3 of a non-NULL pointer. The body equals LIBSN's _OP_DELE
 * (__builtin_delete) instruction for instruction, but it calls free3 and
 * ST.EXE links no LIBSN member, so it is game code.
 */
ADDRESS(0x800152f8, 0x24)
void func_800152f8(void *p)
{
    if (p != NULL)
        free3(p);
}
