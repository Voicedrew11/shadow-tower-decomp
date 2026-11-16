/*
 * ST.EXE: per-frame refresh of the heap blocks behind the world_state.unk834C
 * slots (WIP module). A flag byte per entry (SCRATCHPAD->unkF0, filled by
 * the object passes) says whether the entry is wanted this frame: wanted
 * slots are loaded or kept, unwanted ones become releasable.
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
#include <st/scratch.h>
#include <st/vis.h>
#include <st/bank.h>
#include <st/funcs.h>

ADDRESS(0x80023bc4, 0xa8)
void func_80023bc4(int archive, int index, int slot)
{
    void *data;

    data = func_80015164(DAT_8009c888, func_80015ec8(archive, index), &world_state.unk834C[slot]);
    if (data != NULL) {
        func_80015250(data, ST_BLOCK_LOADING);
        func_80015278(data, slot);
        func_80015d54(archive, index, data, func_80023788);
    }
}

/*
 * Not exact: retail forms the sound-slot address as (i << 3) + &DAT_801dd1e8[8]
 * with the shift first (its copy fills the beqz delay slot); this build
 * loads the constant base first.
 */
ADDRESS(0x80023c6c, 0x26c)
void func_80023c6c(int archive, int first, int slot, int count)
{
    ScratchPad *sp = SCRATCHPAD;
    u8 *flags = sp->unkF0;
    int end = count + first;
    int i;
    u16 id;
    void *a;
    void *b;
    s16 *vab;

    for (i = first; i < end; i++, slot++) {
        id = slot;
        a = world_state.unk834C[id];
        b = world_state.unk834C[id - 16];
        if (*flags++) {
            if (a == NULL) {
                func_800237a8(archive, i * 5, id, sp->unkC8[slot & 0xf]);
            } else if (func_80015258(a) == ST_BLOCK_RELEASABLE) {
                if (func_8001526c(a) != sp->unkC8[slot & 0xf]) {
                    func_80015230(a);
                    func_800237a8(archive, i * 5, id, sp->unkC8[slot & 0xf]);
                } else {
                    func_80015250(a, ST_BLOCK_USED);
                }
            }
            if (b == NULL) {
                func_800239ec(archive, i * 5, (u16)slot, sp->unkC8[slot & 0xf]);
            } else if (func_80015258(b) == ST_BLOCK_RELEASABLE) {
                if (func_8001526c(b) != sp->unkC8[slot & 0xf]) {
                    func_80015230(b);
                    func_800239ec(archive, i * 5, (u16)slot, sp->unkC8[slot & 0xf]);
                } else {
                    func_80015250(b, ST_BLOCK_USED);
                }
            }
        } else {
            if (a != NULL && func_80015258(a) != ST_BLOCK_LOADING)
                func_80015250(a, ST_BLOCK_RELEASABLE);
            if (b != NULL && func_80015258(b) != ST_BLOCK_LOADING)
                func_80015250(b, ST_BLOCK_RELEASABLE);
            vab = (&DAT_801dd1e8[8])[i].block;
            if (vab != NULL && vab[2] == 1)
                vab[2] = 2;
        }
    }
}

ADDRESS(0x80023ed8, 0x12c)
void func_80023ed8(int archive, int first, int slot, int count)
{
    u8 *flags = SCRATCHPAD->unkF0;
    int end = count + first;
    int i;
    void *p;

    for (i = first; i < end; i++, slot++) {
        p = world_state.unk834C[slot];
        if (*flags++) {
            if (p == NULL)
                func_80023abc(archive, i * 2, (u16)slot, vis_table_c.unk0000[slot - 440]);
            else if (func_80015258(p) == ST_BLOCK_RELEASABLE)
                func_80015250(p, ST_BLOCK_USED);
        } else if (p != NULL && func_80015258(p) != ST_BLOCK_LOADING) {
            func_80015250(p, ST_BLOCK_RELEASABLE);
        }
    }
}

/*
 * Not exact: retail keeps i << 3 as the induction value and rematerializes
 * &DAT_801dd1e8[24] at each use; this build folds the base into the
 * induction pointer.
 */
ADDRESS(0x80024004, 0x1cc)
void func_80024004(int archive, int first, int slot, int count)
{
    u8 *flags = SCRATCHPAD->unkF0;
    int end = count + first;
    int i;
    void *p;
    s16 *vab;

    for (i = first; i < end; i++, slot++) {
        p = world_state.unk834C[slot];
        if (*flags++) {
            if (p == NULL)
                func_80023bc4(archive, i * 3, slot);
            else if (func_80015258(p) == ST_BLOCK_RELEASABLE)
                func_80015250(p, ST_BLOCK_USED);
            if (func_80015ec8(archive, i * 3 + 1)) {
                vab = (&DAT_801dd1e8[24])[i].block;
                if (vab == NULL)
                    func_80058a0c(archive, i * 3 + 1, i + 24);
                else if (vab[2] == 2)
                    vab[2] = 1;
            }
        } else {
            if (p != NULL && func_80015258(p) != ST_BLOCK_LOADING)
                func_80015250(p, ST_BLOCK_RELEASABLE);
            vab = (&DAT_801dd1e8[24])[i].block;
            if (vab != NULL && vab[2] == 1)
                vab[2] = 2;
        }
    }
}
