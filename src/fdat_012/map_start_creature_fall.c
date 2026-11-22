/*
 * FDAT_012.BIN: the entry's drop (WIP module; a unit of its own until its neighbours in
 * the run are exact).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <psyq/libc.h>
#include <st/vis.h>
#include <st/data.h>
#include <st/anim.h>
#include <st/hud.h>
#include <st/menu.h>
#include <st/light.h>
#include <st/xa.h>
#include <st/fdat.h>

/* Not exact (size-exact; the first block's registers and, from them, its
 * order): retail keeps the 1 of the two bytes[] stores and then the record
 * index in $a0 and the unk70 mask in $a1; here they are $a1 and $a0, and the
 * three sit at the top instead of after the collision load. No load has to
 * be held in place: sched2 lifts every load as far as its register allows.
 * The block is the plain statements in this order,
 *     e->unk70 &= ~0x2000000; e->unk0F = 0x80; e->unk08 = 9; e->unkB0 = 0x40;
 *     e->unk0E = 8; e->unk50 &= ~0x80; e->unk1C.bytes[0] = 1;
 *     e->unk1C.bytes[3] = 1; r = &vis_table_a.types[e->unk02];
 *     e->unk2A = 0; e->unk16 = 0;
 * and it comes out as retail's, instruction for instruction, once two of its
 * pseudos are allocated by global-alloc instead of local-alloc: the mask
 * constant (it then takes $a1, because the record index already holds $a0
 * locally) and the 1, in a variable that is set more than once in the
 * function (its `li` is then not "birthing" for sched1, floats to the top of
 * the block, conflicts with $v0 and $v1 and takes $a0). A pseudo is left to
 * global-alloc when it is referenced in a second basic block or dies twice.
 * Still missing: what in the source references both again without leaving
 * an instruction. This source keeps the reused `record`, which is such a
 * variable but pins the two stores above the constants.
 * Ruled out: a dependence that holds the loads (a volatile pointer gives the
 * source order with the 1 in $v0), every spelling and type of a block-local
 * 1 (literal, variable, chain, initialiser: always $v0, its life is two
 * instructions), bit-field clears (same RTL as &=), the block in identical
 * if/else arms (r is then set twice and its sum moves up), 990 orders of
 * the plain statements, 39,600 with one store repeated, and batch K's
 * 141,120 orders of the split forms.
 * Narrowed since (sched1 trace): the 1 and the index are two pseudos. With
 * one variable for both, the index load is a priority-1 load that every
 * ready store beats, so the two bytes[] stores come out above the four
 * constants (retail has them below). That variable plus a mask referenced
 * in a second block does give retail's $a0/$a1. A local 1 with two sets
 * and one death would need no second block at all (four refs rank it above
 * the mask), but nothing natural sets it twice; inline helpers taking the 1
 * as a parameter compile like the literal. */
/*
 * Starts the entry's fall (animation 9) and, once its record's stream
 * counter is 4, adds table C item 0x10a at its place to the drop slots
 * 48..63 (the oldest drops out when all 16 are used).
 */
ADDRESS(0x801eda38, 0x1b4)
void map_start_creature_fall(Creature *e)
{
    Vis48 *c;
    CreatureType *r;
    u16 info;
    u32 range;
    u32 collision;
    int distance;
    int record;
    int i;

    range = e->flags;
    e->command = 0x80;
    e->anim = 9;
    e->anim_step = 0x40;
    e->move_mode = 8;
    collision = e->collision_tests;
    record = 1;
    e->status.bytes[0] = record;
    e->status.bytes[3] = record;
    record = e->type;
    r = &vis_table_a.types[record];
    e->anim_frame = 0;
    e->vertical_speed = 0;
    e->flags = range & ~0x2000000;
    e->collision_tests = collision & ~0x80;
    if (game_progress.unk1448[r->talk_id] != 4)
        return;
    if (vis_table_c.unk0000[3] == 16) {
        c = &vis_table_c.entries[48];
        i = 15;
        while (i--) {
            c[0] = c[1];
            c++;
        }
    } else {
        c = &vis_table_c.entries[48 + vis_table_c.unk0000[3]];
        vis_table_c.unk0000[3]++;
    }
    c->unk00 = 0x10a;
    c->unk02 = e->block;
    *(u32 *)c->unk04 = 0;
    c->unk08 = e->position;
    *(SVECTOR *)&c->unk18 = *(SVECTOR *)&e->rot_x;
    info = vis_table_c.unk0118[266].unk1E;
    c->unk22 = 2;
    c->unk23 = 2;
    c->unk2c = 1;
    c->unk24 = 0;
    c->unk20 = info;
    distance = e->player_dist_xz;
    c->unk26 = 0;
    c->unk25 = 0;
    c->unk28 = distance;
}
