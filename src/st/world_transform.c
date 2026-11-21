/*
 * ST.EXE world part transform (WIP module): world_transform_parts gives
 * each visible record its camera-space translation for world_draw. It is
 * its own unit only because it is not exact and one instruction longer
 * than retail.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <INLINE_C.H>
#include <st/data.h>
#include <st/mesh.h>
#include <st/scratch.h>
#include <st/vis.h>


/*
 * Transforms each visible record's position into camera space for its
 * part's translation. The viewer-relative offset is halved until it fits
 * an SVECTOR, rotated with RTV0TR, and the result scaled back up.
 */
ADDRESS(0x8001cb88, 0x270)
void world_transform_parts(VECTOR *trans)
{
    ScratchPad *sp = SCRATCHPAD;
    VisRecord *rec;
    int i;

    i = vis_lists.n_visible;
    while (i--) {
        rec = &vis_records[vis_lists.visible[i]];
        sp->x = rec->unk08 << 12;
        sp->y = (rec->unk09 << 12) + ((rec->unk0b - 128) << 7);
        sp->z = rec->unk0a << 12;
        sp->x = sp->x - viewer.position.vx;
        sp->y = sp->y - (viewer.position.vy + viewer.unk2CE + viewer.unk2D2);
        sp->z = sp->z - viewer.position.vz;
        sp->part_shift = 0;
        while (sp->x > 32767 || sp->x < -32768 || sp->y > 32767 || sp->y < -32768
               || sp->z > 32767 || sp->z < -32768) {
            sp->x >>= 1;
            sp->y >>= 1;
            sp->z >>= 1;
            sp->part_shift++;
        }
        sp->part_offset.vx = sp->x;
        sp->part_offset.vy = sp->y;
        sp->part_offset.vz = sp->z;
        gte_ldv0(&sp->part_offset);
        gte_rtv0tr();
        gte_stlvnl(&sp->part_pos);
        gte_stflg(&sp->flag);
        trans[i].pad = sp->part_shift;
        while (--sp->part_shift != -1) {
            sp->part_pos.vx <<= 1;
            sp->part_pos.vy <<= 1;
            sp->part_pos.vz <<= 1;
        }
        trans[i].vx = sp->part_pos.vx;
        trans[i].vy = sp->part_pos.vy;
        trans[i].vz = sp->part_pos.vz;
    }
}
