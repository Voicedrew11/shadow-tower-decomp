/*
 * ST.EXE: archive members loaded into heap blocks (WIP module). Split from
 * src/st/bank.c only because func_800237a8 there is not exact (4 bytes
 * shorter than retail).
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/memory.h>
#include <st/cd.h>
#include <st/data.h>
#include <st/gfx.h>
#include <st/bank.h>
#include <st/funcs.h>

/*
 * Loads member `index + 4` of `archive` into a heap block owned by
 * world_state.unk8308[slot + 1].
 *
 * Not exact (size-exact): retail allocates index + 4 to $s2 and slot to $s1;
 * this source swaps the two.
 */
ADDRESS(0x800239ec, 0xd0)
void func_800239ec(archive, index, slot, tag)
int archive;
int index;
u16 slot;
u8 tag;
{
    u32 size;
    void *data;

    index += 4;
    size = func_80015ec8(archive, index);
    if (size == 0)
        return;
    data = func_80015164(DAT_8009c888, size, &world_state.unk8308[slot + 1]);
    if (data == NULL)
        return;
    func_80015250(data, ST_BLOCK_LOADING);
    func_80015278(data, slot - 16);
    func_80015264(data, tag);
    func_80015d54(archive, index, data, func_80023288);
}

/* Loads members `index` and `index + 1` of `archive` into one heap block (the larger size). */
ADDRESS(0x80023abc, 0x108)
void func_80023abc(archive, index, slot, tag)
int archive;
int index;
u16 slot;
u8 tag;
{
    u32 second;
    u32 size;
    void *data;

    second = func_80015ec8(archive, index + 1);
    size = func_80015ec8(archive, index);
    if (size < second)
        size = second;
    data = func_80015164(DAT_8009c888, size, &world_state.unk834C[slot]);
    if (data == NULL)
        return;
    func_80015250(data, ST_BLOCK_LOADING);
    func_80015278(data, slot);
    func_80015264(data, tag);
    func_80015d54(archive, index + 1, data, func_80023604);
    func_80015d54(archive, index, data, func_80023470);
}
