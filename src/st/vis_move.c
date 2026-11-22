/*
 * ST.EXE: empty hooks and table B entry interpolation helpers (WIP module,
 * 0x80053d38..0x80054000). No ST.EXE code or data references these
 * functions; their callers are outside the image (overlay code is the likely
 * user). Band M's file before 0x80053d38 may continue here.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/data.h>
#include <st/vis.h>
#include <st/light.h>
#include <st/funcs.h>

ADDRESS(0x80053d38, 0x8)
void func_80053d38(void)
{
}

ADDRESS(0x80053d40, 0x8)
void func_80053d40(void)
{
}

ADDRESS(0x80053d48, 0x8)
void func_80053d48(void)
{
}

ADDRESS(0x80053d50, 0x8)
void func_80053d50(void)
{
}

/* World position of the point (x, y + dy, z + dz) turned by (-pitch, yaw, 0),
 * relative to the viewer (1600 units below its eye height). */
ADDRESS(0x80053d58, 0xb4)
void player_point_to_world(int x, int y, int z, int pitch, int yaw, int dy, int dz, VECTOR *out)
{
    SVECTOR v;
    SVECTOR rot;

    v.vx = x;
    v.vy = y + dy;
    v.vz = z + dz;
    rot.vx = -pitch;
    rot.vy = yaw;
    rot.vz = 0;
    func_8004a078(&rot, &v, out);
    out->vx += viewer.position.vx;
    out->vy += viewer.position.vy + dy - 1600;
    out->vz += viewer.position.vz;
}

/* Moves the entry's position and rotation a fraction t / 4096 of the way
 * from one pose to another; a null pose keeps that part. */
ADDRESS(0x80053e0c, 0xe4)
void object_lerp_pose(Vis88 *e, VECTOR *from, VECTOR *to, SVECTOR *rfrom, SVECTOR *rto, int t)
{
    if (from) {
        e->unk24.vx = func_8004a830(from->vx, to->vx, t);
        e->unk24.vy = func_8004a830(from->vy, to->vy, t);
        e->unk24.vz = func_8004a830(from->vz, to->vz, t);
    }
    if (rfrom) {
        e->unk34 = func_8004a848(rfrom->vx, rto->vx, t);
        e->unk38 = func_8004a848(rfrom->vz, rto->vz, t);
        e->unk36 = func_8004a848(rfrom->vy, rto->vy, t);
    }
}

ADDRESS(0x80053ef0, 0x88)
void object_lerp_position(Vis88 *e, VECTOR *from, VECTOR *to, int t)
{
    if (from) {
        e->unk24.vx = func_8004a830(from->vx, to->vx, t);
        e->unk24.vy = func_8004a830(from->vy, to->vy, t);
        e->unk24.vz = func_8004a830(from->vz, to->vz, t);
    }
}

ADDRESS(0x80053f78, 0x88)
void object_lerp_rotation(Vis88 *e, SVECTOR *rfrom, SVECTOR *rto, int t)
{
    if (rfrom) {
        e->unk34 = func_8004a848(rfrom->vx, rto->vx, t);
        e->unk38 = func_8004a848(rfrom->vz, rto->vz, t);
        e->unk36 = func_8004a848(rfrom->vy, rto->vy, t);
    }
}
