/*
 * ST.EXE: archive members loaded into heap blocks through the CD request
 * ring, their completion callbacks, and the sound bank slots (WIP module).
 * The module ends at func_800237a8, which is not exact, so that the loaders
 * after it keep their retail offsets in their own unit (src/st/bankload.c).
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBSND.H>
#include <st/memory.h>
#include <st/cd.h>
#include <st/data.h>
#include <st/gfx.h>
#include <st/bank.h>
#include <st/funcs.h>

RODATA(0x80011030, 0xa0)

ADDRESS(0x80023208, 0x80)
u16 func_80023208(u16 id)
{
    if (id < 300)
        return id + 256;
    if (area_state.unk0 == 1 && area_state.unk1E == 1 && area_state.unk10 != 0xff)
        return area_state.unk10 * 32 + 256 + id;
    return area_state.unk4 * 32 + 256 + id;
}

ADDRESS(0x80023288, 0x100)
void func_80023288(void *data)
{
    DVECTOR a[4] = {{0, 0}, {64, 0}, {128, 0}, {192, 0}};
    DVECTOR b[4] = {{0, 0}, {0, 3}, {0, 6}, {0, 9}};
    u8 n = func_8001526c(data);

    func_800177a0((Model *)((u8 *)data + ((u32 *)data)[2]), &a[n], &b[n], 0);
    func_80015250(data, ST_BLOCK_USED);
}

ADDRESS(0x80023388, 0xe8)
void func_80023388(void *data)
{
    DVECTOR a[4] = {{0, 0}, {64, 0}, {128, 0}, {192, 0}};
    DVECTOR b[4] = {{0, 0}, {0, 3}, {0, 6}, {0, 9}};
    u8 n = func_8001526c(data);

    func_8003ec5c(data, &a[n], &b[n], 0);
}

ADDRESS(0x80023470, 0x194)
void func_80023470(void *data)
{
    DVECTOR a[16] = {
        {0, 0},   {0, 32},   {0, 64},   {0, 96},   {0, 128},  {0, 160},  {0, 192},  {0, 224},
        {64, 0},  {64, 32},  {64, 64},  {64, 96},  {64, 128}, {64, 160}, {64, 192}, {64, 224},
    };
    DVECTOR b[16] = {
        {0, 0}, {0, 1}, {0, 2},  {0, 3},  {0, 4},  {0, 5},  {0, 6},  {0, 7},
        {0, 8}, {0, 9}, {0, 10}, {0, 11}, {0, 12}, {0, 13}, {0, 14}, {0, 15},
    };
    u8 n = func_8001526c(data);

    func_800177a0((Model *)((u8 *)data + ((u32 *)data)[2]), &a[n], &b[n], 0);
    func_80015250(data, ST_BLOCK_USED);
}

ADDRESS(0x80023604, 0x184)
void func_80023604(void *data)
{
    DVECTOR a[16] = {
        {0, 0},   {0, 32},   {0, 64},   {0, 96},   {0, 128},  {0, 160},  {0, 192},  {0, 224},
        {64, 0},  {64, 32},  {64, 64},  {64, 96},  {64, 128}, {64, 160}, {64, 192}, {64, 224},
    };
    DVECTOR b[16] = {
        {0, 0}, {0, 1}, {0, 2},  {0, 3},  {0, 4},  {0, 5},  {0, 6},  {0, 7},
        {0, 8}, {0, 9}, {0, 10}, {0, 11}, {0, 12}, {0, 13}, {0, 14}, {0, 15},
    };
    u8 n = func_8001526c(data);

    func_8003ec5c(data, &a[n], &b[n], 0);
}

ADDRESS(0x80023788, 0x20)
void func_80023788(void *data)
{
    func_80015250(data, ST_BLOCK_USED);
}

/*
 * Loads members `index` and `index + 1` of `archive` into one heap block
 * (the larger size), then, when member `index + 2` exists, frees the other
 * slots of the bank's column and loads it as a sound bank.
 *
 * A skipped bank advances from self; visited slots advance from the cursor.
 * Keep these as distinct paths so no second VAB-field cursor spans the loop.
 */
ADDRESS(0x800237a8, 0x244)
void func_800237a8(archive, index, slot, tag)
int archive;
int index;
u16 slot;
u8 tag;
{
    StVabSlot *self;
    StVabSlot *p;
    u32 second;
    u32 size;
    void *data;
    int bank;
    int i;

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
    if (second != 0)
        func_80015d54(archive, index + 1, data, func_80023388);
    func_80015d54(archive, index, data, func_80023288);
    if (func_80015ec8(archive, index + 2) == 0)
        return;
    bank = index / 5;
    self = &DAT_801dd1c0.slots[8 + bank];
    p = &DAT_801dd1c0.slots[8 + bank % 4];
    for (i = 4; i--;) {
        if (p == self) {
            p = self + 4;
            continue;
        }
        if (p->block != NULL) {
            while (p->block[2] == ST_BLOCK_LOADING)
                func_8001531c();
            SsVabClose(p->vab);
            p->vab = -1;
            p->block = NULL;
        }
        p += 4;
    }
    if (self->block == NULL)
        func_80058a0c(archive, index + 2, bank + 8);
    else if (self->block[2] == 1)
        self->block[2] = 2;
}
