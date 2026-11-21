/*
 * VS.EXE: the graphics start-up and the round camera (WIP module split
 * from src/vs/main.c around its non-exact functions).
 * $gp-relative accesses to the round state, so -G8.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBETC.H>
#include <INLINE_C.H>
#include <st/anim.h>
#include <st/mesh.h>
#include <vs/data.h>
#include <vs/buffer.h>
#include <vs/menu.h>
#include <vs/sound.h>
#include <vs/main.h>
#include <vs/pad.h>
#include <vs/funcs.h>

#define PACKET_WORDS (0x20000 / 4)

/* The camera's height over the fighters' midpoint and its distance from
 * them, by the fighters' size. */
#define CAMERA_NEAR 1400
#define CAMERA_NEAR_TALL 1700
#define CAMERA_FAR 8000

/* Tentative definitions: ASPSX needs the size to address these $gp-relative.
 * An earlier object allocates them (docs/executable-linking.md). */
u32 DAT_80063314;
s32 DAT_80063324;
s32 DAT_80063354;
u32 DAT_80063364;

/* The graphics start-up, VS's version of ST.EXE's func_80016fb8
 * (src/st/gfx.c): the 320x240 double buffer, the GTE offset, screen, back
 * and far colours, and the two packet buffers. */
ADDRESS(0x80014480, 0x1b4)
void func_80014480(void)
{
    ResetGraph(0);
    SetGraphDebug(0);
    InitGeom();
    gte_SetGeomOffset(160, 120);
    gte_SetGeomScreen(300);
    SetDefDrawEnv(&DAT_801624d0.draw[0], 0, 0, 320, 240);
    SetDefDispEnv(&DAT_801624d0.disp[0], 0, 240, 320, 240);
    SetDefDrawEnv(&DAT_801624d0.draw[1], 0, 240, 320, 240);
    SetDefDispEnv(&DAT_801624d0.disp[1], 0, 0, 320, 240);
    DAT_801624d0.draw[0].dtd = 1;
    DAT_801624d0.draw[1].dtd = 1;
    DAT_801624d0.draw[0].isbg = 1;
    DAT_801624d0.draw[1].isbg = 1;
    setRGB0(&DAT_801624d0.draw[0], 0, 0, 0);
    setRGB0(&DAT_801624d0.draw[1], 0, 0, 0);
    PutDispEnv(&DAT_801624d0.disp[0]);
    PutDrawEnv(&DAT_801624d0.draw[0]);
    gte_SetBackColor(80, 80, 80);
    gte_SetFarColor(5, 5, 5);
    DAT_801624d0.frame = 255;
    DAT_801624d0.unkE0 = DAT_8011c528;
    DAT_801624d0.packets[0].start = DAT_801624d0.unkE0;
    DAT_801624d0.packets[0].end = DAT_801624d0.packets[0].start + PACKET_WORDS;
    DAT_801624d0.packets[1].start = DAT_801624d0.packets[0].end;
    DAT_801624d0.packets[1].end = DAT_801624d0.packets[1].start + PACKET_WORDS;
    SetDispMask(1);
}

/*
 * Moves the camera: after a round it circles the winner (fighter
 * DAT_80063324 & 1), framed by its height; otherwise it looks at the
 * fighters' midpoint from the side, from a distance that follows their
 * separation, sweeping in during the round start. The camera eases toward
 * its target at DAT_80063354 units per frame, keeps within +-32767 on x
 * and z, and faces the target.
 */
ADDRESS(0x80014634, 0x9ec)
void func_80014634(void)
{
    SVECTOR look;
    SVECTOR eye;
    VsFighter *f;
    int height;
    int dist;
    int near;
    int far;
    int x;
    int y;
    int z;

    if (DAT_80063324 & 8) {
        f = &DAT_800633b0.fighter[DAT_80063324 & 1];
        height = f->unk4DF18.unk1B6 + f->unk4DF18.unk1B2;
        if (height > 3000) {
            look.vx = f->unk4DEF0.vx;
            look.vz = f->unk4DEF0.vz;
            look.vy = -1800;
            eye.vx = f->unk4DEF0.vx + rsin(f->unk4DF02 + 1536) * 3 / 2;
            eye.vy = -800;
            eye.vz = f->unk4DEF0.vz + rcos(f->unk4DF02 + 1536) * 3 / 2;
        } else if (height > 2000) {
            look.vx = f->unk4DEF0.vx;
            look.vz = f->unk4DEF0.vz;
            look.vy = -1500;
            eye.vx = f->unk4DEF0.vx + rsin(f->unk4DF02 + 1536);
            eye.vy = -500;
            eye.vz = f->unk4DEF0.vz + rcos(f->unk4DF02 + 1536);
        } else if (height < 1000) {
            look.vx = f->unk4DEF0.vx;
            look.vz = f->unk4DEF0.vz;
            look.vy = -1000;
            eye.vx = f->unk4DEF0.vx + rsin(f->unk4DF02 + 1536);
            eye.vy = -1000;
            eye.vz = f->unk4DEF0.vz + rcos(f->unk4DF02 + 1536);
        } else {
            look.vx = f->unk4DEF0.vx;
            look.vz = f->unk4DEF0.vz;
            look.vy = -1200;
            eye.vx = f->unk4DEF0.vx + rsin(f->unk4DF02 + 1536) * 3 / 4;
            eye.vy = -500;
            eye.vz = f->unk4DEF0.vz + rcos(f->unk4DF02 + 1536) * 3 / 4;
        }
        if (DAT_801603c0.unk3C < DAT_80063314 + 5) {
            DAT_801624d0.unk2CA78.vx = f->unk4DEF0.vx + rsin(f->unk4DF02 + 3072) * 2;
            DAT_801624d0.unk2CA78.vy = -7000;
            DAT_801624d0.unk2CA78.vz = f->unk4DEF0.vz + rcos(f->unk4DF02 + 3072) * 2;
        }
    } else {
        int delta_x;
        int delta_z;

        look.vx = (DAT_800633b0.fighter[0].unk4DEF0.vx + DAT_800633b0.fighter[1].unk4DEF0.vx) / 2;
        delta_x = DAT_800633b0.fighter[1].unk4DEF0.vx - look.vx;
        look.vz = (DAT_800633b0.fighter[0].unk4DEF0.vz + DAT_800633b0.fighter[1].unk4DEF0.vz) / 2;
        delta_z = DAT_800633b0.fighter[1].unk4DEF0.vz - look.vz;
        look.vy = (DAT_800633b0.fighter[0].unk4DEF0.vy + DAT_800633b0.fighter[1].unk4DEF0.vy) / 2;
        dist = SquareRoot0(delta_z * delta_z + delta_x * delta_x) + 1;
        near = CAMERA_NEAR;
        far = CAMERA_FAR;
        if (DAT_800633b0.fighter[0].unk4DF18.unk1B6 > 2500
            || DAT_800633b0.fighter[1].unk4DF18.unk1B6 > 2500)
            near = CAMERA_NEAR_TALL;
        if (dist < near) {
            eye.vy = -near * 2;
            eye.vx = look.vx + (DAT_800633b0.fighter[1].unk4DEF0.vz - look.vz) * near * 3 / dist;
            eye.vz = look.vz + (look.vx - DAT_800633b0.fighter[1].unk4DEF0.vx) * near * 3 / dist;
        } else if (dist > far) {
            eye.vy = -far * 2;
            eye.vx = look.vx + (DAT_800633b0.fighter[1].unk4DEF0.vz - look.vz) * far * 3 / dist;
            eye.vz = look.vz + (look.vx - DAT_800633b0.fighter[1].unk4DEF0.vx) * far * 3 / dist;
        } else {
            eye.vx = look.vx + (DAT_800633b0.fighter[1].unk4DEF0.vz - look.vz) * 3;
            eye.vy = -dist * 2;
            eye.vz = look.vz + (look.vx - DAT_800633b0.fighter[1].unk4DEF0.vx) * 3;
        }
        if (DAT_80063324 == 0) {
            if ((s32)DAT_80063364 < 34) {
                look.vx = 0;
                look.vz = DAT_80063364 * 300 - 10000;
            } else {
                look.vx = 0;
                look.vz = 0;
            }
        }
    }
    eye.vx -= DAT_801624d0.unk2CA78.vx;
    eye.vy -= DAT_801624d0.unk2CA78.vy;
    eye.vz -= DAT_801624d0.unk2CA78.vz;
    dist = SquareRoot0(eye.vx * eye.vx + eye.vy * eye.vy + eye.vz * eye.vz) + 1;
    if (dist < 200) {
        DAT_80063354 = 0;
    } else if (dist < 1000) {
        if (DAT_80063354 > 100)
            DAT_80063354 -= 20;
        else if (DAT_80063354 < 100)
            DAT_80063354 += 20;
    } else if (dist < 3000) {
        if (DAT_80063354 > 200)
            DAT_80063354 -= 20;
        else if (DAT_80063354 < 200)
            DAT_80063354 += 20;
    } else {
        if (DAT_80063354 > 500)
            DAT_80063354 -= 20;
        else if (DAT_80063354 < 500)
            DAT_80063354 += 20;
    }
    x = eye.vx * DAT_80063354 / dist;
    y = eye.vy * DAT_80063354 / dist;
    z = eye.vz * DAT_80063354 / dist;
    DAT_801624d0.unk2CA78.vx += x;
    DAT_801624d0.unk2CA78.vy += y;
    DAT_801624d0.unk2CA78.vz += z;
    if (DAT_801624d0.unk2CA78.vx > 32767) {
        DAT_801624d0.unk2CA78.vy += 32767 - DAT_801624d0.unk2CA78.vx;
        DAT_801624d0.unk2CA78.vx = 32767;
    }
    if (DAT_801624d0.unk2CA78.vx < -32767) {
        DAT_801624d0.unk2CA78.vy += 32767 + DAT_801624d0.unk2CA78.vx;
        DAT_801624d0.unk2CA78.vx = -32767;
    }
    if (DAT_801624d0.unk2CA78.vz > 32767) {
        DAT_801624d0.unk2CA78.vy += 32767 - DAT_801624d0.unk2CA78.vz;
        DAT_801624d0.unk2CA78.vz = 32767;
    }
    if (DAT_801624d0.unk2CA78.vz < -32767) {
        DAT_801624d0.unk2CA78.vy += 32767 + DAT_801624d0.unk2CA78.vz;
        DAT_801624d0.unk2CA78.vz = -32767;
    }
    if (look.vz == DAT_801624d0.unk2CA78.vz)
        DAT_801624d0.unk2CA88.vy = 2048;
    else
        DAT_801624d0.unk2CA88.vy = ratan2(look.vx - DAT_801624d0.unk2CA78.vx,
                                          look.vz - DAT_801624d0.unk2CA78.vz);
    look.vx -= DAT_801624d0.unk2CA78.vx;
    look.vz -= DAT_801624d0.unk2CA78.vz;
    if (DAT_80063324 & 8)
        DAT_801624d0.unk2CA88.vx = ratan2(DAT_801624d0.unk2CA78.vy - look.vy,
                                          SquareRoot0(look.vx * look.vx + look.vz * look.vz));
    else
        DAT_801624d0.unk2CA88.vx = ratan2((DAT_801624d0.unk2CA78.vy - look.vy) * 2 / 3,
                                          SquareRoot0(look.vx * look.vx + look.vz * look.vz));
    RotMatrixZYX(&DAT_801624d0.unk2CA88, &DAT_801624d0.unk2C998);
    TransposeMatrix(&DAT_801624d0.unk2C998, &DAT_801624d0.unk2C998);
    DAT_801624d0.unk2C998.t[0] = 0;
    DAT_801624d0.unk2C998.t[1] = 0;
    DAT_801624d0.unk2C998.t[2] = 0;
}
