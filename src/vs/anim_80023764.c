/*
 * VS.EXE: registering an animated model file by id, the 12 skeletal
 * animation slots (initialise, retire, free), a malloc wrapper and the
 * animation node lists: initialisation, key lookup, insertion, removal,
 * the node command interpreter and the block runner (WIP module). The
 * list functions are ST.EXE's (src/lib/anim_list*.inc, shared with
 * src/st/anim.c); the rest are VS's copies of ST.EXE's src/st/anim.c and
 * anim_set.c functions. VS keeps anim_list_find_id elsewhere
 * (src/vs/anim_80025670.c).
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <MALLOC.H>
#include <st/memory.h>
#include <st/gfx.h>
#include <st/mesh.h>
#include <st/scratch.h>
#include <st/anim.h>
#include <vs/data.h>
#include <vs/funcs.h>

/* func_8002420c's strings and anim_node_command's switch table. */
RODATA(0x800112a8, 0x28)
JUMPTABLES(0x800113b8, 0x24)

/* The file's mesh model. */
#define ANIM_MODEL(file) ((u8 *)(file) + (file)->model)

/* VS's first copy of anim_register; the second is func_80027fec
 * (src/vs/anim_80027f30.c). */
ADDRESS(0x80023764, 0x3c)
void func_80023764(u16 id, StAnimFile *file)
{
    DAT_801624d0.unk834C[id] = file;
    SCRATCHPAD->model = ANIM_MODEL(file);
}

ADDRESS(0x800237a0, 0x44)
void func_800237a0(void)
{
    StAnimSet *set = DAT_801624d0.anim_sets;
    u16 i;

    i = 12;
    do {
        set->unk0 = 0;
        set->unk28 = NULL;
        set->unk2C = NULL;
        set->unk30 = NULL;
        set->unk34 = NULL;
        set++;
    } while (--i);
}

/* Puts every used slot in state 1. */
ADDRESS(0x800237e4, 0x3c)
void func_800237e4(void)
{
    StAnimSet *set = DAT_801624d0.anim_sets;
    u16 i;

    i = 12;
    do {
        if (set->unk0)
            set->unk0 = 1;
        set++;
    } while (--i);
}

/* Frees a slot: clears its state and its owner's pointer and frees its four
 * buffers (ST.EXE's anim_set_free frees one node buffer). */
ADDRESS(0x80023820, 0xa0)
void func_80023820(StAnimSet *set)
{
    set->unk0 = 0;
    *set->unk38 = NULL;
    if (set->unk28 != NULL) {
        free(set->unk28);
        set->unk28 = NULL;
    }
    if (set->unk2C != NULL) {
        free(set->unk2C);
        set->unk2C = NULL;
    }
    if (set->unk30 != NULL) {
        free(set->unk30);
        set->unk30 = NULL;
    }
    if (set->unk34 != NULL) {
        free(set->unk34);
        set->unk34 = NULL;
    }
}

/* Frees every slot in state 1. */
ADDRESS(0x800238c0, 0x6c)
void func_800238c0(void)
{
    StAnimSet *set = DAT_801624d0.anim_sets;
    u16 i;

    i = 12;
    do {
        if (set->unk0 == 1)
            func_80023820(set);
        set++;
    } while (--i);
}

/* Frees every slot in use. */
ADDRESS(0x8002392c, 0x60)
void func_8002392c(void)
{
    StAnimSet *set = DAT_801624d0.anim_sets;
    s16 i;

    i = 12;
    do {
        if (set->unk0)
            func_80023820(set);
        set++;
    } while (--i);
}

/* The C library's malloc, or NULL when the result is not in main RAM: a
 * second copy of func_8002e690 (src/vs/cd_8002e5e4.c). */
ADDRESS(0x8002398c, 0x44)
void *func_8002398c(u32 size)
{
    u8 *p = malloc(size);

    if ((u32)(p - (u8 *)0x80000000) > 0x1fffff)
        return NULL;
    return p;
}

#include "../lib/anim_list_init.inc"

#include "../lib/anim_list.inc"

/* Finds a free slot, or NULL (ST.EXE's anim_set_alloc frees the state-1
 * slots and searches again; VS gives up). */
ADDRESS(0x800241b4, 0x58)
void func_800241b4(StAnimSet **out)
{
    u16 i;

    *out = DAT_801624d0.anim_sets;
    i = 12;
    do {
        if (!(*out)->unk0) {
            (*out)->unk4 = 255;
            return;
        }
        (*out)++;
    } while (--i);
    *out = NULL;
}

/* Debug print of a matrix (unused). */
ADDRESS(0x8002420c, 0x90)
void func_8002420c(MATRIX *m)
{
    FntPrint("==== Matrixprint ====\n");
    FntPrint("%5d %5d %5d\n", m->m[0][0], m->m[0][1], m->m[0][2]);
    FntPrint("%5d %5d %5d\n", m->m[1][0], m->m[1][1], m->m[1][2]);
    FntPrint("%5d %5d %5d\n", m->m[2][0], m->m[2][1], m->m[2][2]);
    FntPrint("%5d %5d %5d\n", m->t[0], m->t[1], m->t[2]);
}
