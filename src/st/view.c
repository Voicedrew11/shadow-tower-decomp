/*
 * ST.EXE: view-cone proximity test (WIP module).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/data.h>
#include <st/gfx.h>
#include <st/scratch.h>


/*
 * Returns nonzero when the scratchpad position lies inside the wedge in
 * front of the viewer's position (opening with its pitch, facing its yaw)
 * and closer than 12 units (of 2048).
 */
ADDRESS(0x80016554, 0x324)
int view_point_in_near_cone(void)
{
    ScratchPad *sp = SCRATCHPAD;
    int pitch = viewer.pitch;
    int yaw = viewer.yaw + 1024;
    int half;
    int cos0, sin0, cos1, sin1, cos2, sin2, cosp;
    int back, side, front;
    short x0, z0, x1, z1, x2, z2, cx, cz;
    int a1, b1, c1, a2, b2, c2;
    int px, pz, py;
    int e1, e2;

    if (pitch > 2048)
        half = 4096 - pitch;
    else
        half = pitch;
    half = half * 584 / 1024 + 512;
    cos0 = rcos(yaw);
    sin0 = rsin(yaw);
    cos1 = rcos(yaw - half);
    sin1 = rsin(yaw - half);
    cos2 = rcos(yaw + half);
    sin2 = rsin(yaw + half);
    cosp = rcos(pitch);
    back = cosp * -11 >> 12;
    side = cosp * 16 >> 12;
    front = cosp * -8 >> 12;
    x0 = back * cos0 >> 12;
    z0 = back * sin0 >> 12;
    x1 = (side * cos1 + back * cos0) >> 12;
    z1 = (side * sin1 + back * sin0) >> 12;
    x2 = (side * cos2 + back * cos0) >> 12;
    z2 = (side * sin2 + back * sin0) >> 12;
    cx = front * cos0 >> 12;
    cz = front * sin0 >> 12;
    a1 = z1 - z0;
    b1 = x1 - x0;
    c1 = -a1 * x0 + b1 * z0;
    a2 = z2 - z0;
    b2 = x2 - x0;
    c2 = -a2 * x0 + b2 * z0;
    px = (sp->point_x - viewer.position.vx) / 2048 + cx;
    pz = (sp->point_z - viewer.position.vz) / 2048 + cz;
    e1 = a1 * px - b1 * pz + c1;
    e2 = a2 * px - b2 * pz + c2;
    if (e1 > 0 || e2 < 0)
        return 0;
    rsin(pitch);
    py = (sp->point_y - viewer.position.vy) / 2048;
    return (px - cx) * (px - cx) + (pz - cz) * (pz - cz) + py * py < 145;
}
