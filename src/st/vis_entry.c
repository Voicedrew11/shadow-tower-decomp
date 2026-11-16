/*
 * ST.EXE: placing a table A entry (WIP module; its callers 0x8004ae38 and
 * 0x8004af24 follow in the next band and may share its file).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <psyq/libc.h>
#include <st/vis.h>
#include <st/funcs.h>

/* Moves the entry to the cell of record unk28, offset by unk6A..unk6E. */
ADDRESS(0x8004ac38, 0x64)
void func_8004ac38(Vis212 *e)
{
    VisRecord *r;
    u16 i = e->unk28;

    e->unk26 = i;
    r = &vis_records[i];
    e->unk74.vx = (r->unk08 << 12) + e->unk6A;
    e->unk74.vy = (r->unk09 << 12) + e->unk6C;
    e->unk74.vz = (r->unk0a << 12) + e->unk6E;
}

/* Clears the rotation and sets the facing to unk68, or a random one when
 * unk68 is 0xffff. */
ADDRESS(0x8004ac9c, 0x50)
void func_8004ac9c(Vis212 *e)
{
    e->unk90 = 0;
    e->unk8C = 0;
    if (e->unk68 == 0xffff)
        e->unk8E = rand() >> 3;
    else
        e->unk8E = e->unk68;
}
