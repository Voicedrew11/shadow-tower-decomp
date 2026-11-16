/*
 * VS.EXE: clip key lookup, ST.EXE's (src/lib/anim_find_key.inc, shared
 * with src/st/anim.c), VS's second copy of anim_register and its
 * vertex-animation slot release (WIP module).
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/memory.h>
#include <st/gfx.h>
#include <st/mesh.h>
#include <st/scratch.h>
#include <st/anim.h>
#include <vs/data.h>
#include <vs/funcs.h>
#include <MALLOC.H>

/* The file's mesh model. */
#define ANIM_MODEL(file) ((u8 *)(file) + (file)->model)

#include "../lib/anim_find_key.inc"

ADDRESS(0x80027fec, 0x3c)
void func_80027fec(u16 id, StAnimFile *file)
{
    DAT_801624d0.unk834C[id] = file;
    SCRATCHPAD->model = ANIM_MODEL(file);
}

/* Frees a slot: clears its state and its owner's pointer and frees its
 * delta buffer (ST.EXE's anim_slot_free checks the slot first and uses
 * func_800152f8). */
ADDRESS(0x80028028, 0x4c)
void func_80028028(StAnimSlot *slot)
{
    slot->unk0 = 0;
    *slot->unk14 = NULL;
    if (slot->unk10 != NULL) {
        free(slot->unk10);
        slot->unk10 = NULL;
    }
}

/* Frees every slot in state 1. */
ADDRESS(0x80028074, 0x6c)
void func_80028074(void)
{
    StAnimSlot *slot = DAT_801624d0.anim_slots;
    u16 i;

    i = 12;
    do {
        if (slot->unk0 == 1)
            func_80028028(slot);
        slot++;
    } while (--i);
}

/* Puts every used slot in state 1. */
ADDRESS(0x800280e0, 0x3c)
void func_800280e0(void)
{
    StAnimSlot *slot = DAT_801624d0.anim_slots;
    u16 i;

    i = 12;
    do {
        if (slot->unk0)
            slot->unk0 = 1;
        slot++;
    } while (--i);
}
