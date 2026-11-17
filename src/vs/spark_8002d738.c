/*
 * VS.EXE: starting a burst of hit-spark particles (WIP module).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <psyq/libc.h>
#include <vs/data.h>
#include <vs/buffer.h>

/*
 * Starts up to 200 free particles at fighter n's hit radius on the side
 * facing the opponent, each with a random life and velocity; a particle
 * whose x and z speeds are both 48 or more is dropped again.
 */
/* Not exact: retail tests the second speed pair from registers without
 * re-reading the record; 0x10 bytes long. */
ADDRESS(0x8002d738, 0x260)
void func_8002d738(int n)
{
    VsFighter *f = &DAT_800633b0.fighter[n];
    VsFighter *o = &DAT_800633b0.fighter[1 - n];
    VsRecord20 *rec = DAT_801c4a58;
    SVECTOR off;
    int count = 0;
    int i = 2000;

    off.vy = ratan2(f->unk4DEF0.vx - o->unk4DEF0.vx, f->unk4DEF0.vz - o->unk4DEF0.vz);
    off.vx = f->unk4DF18.unk1B4 * rsin(off.vy - 2048) / 16384;
    off.vz = f->unk4DF18.unk1B4 * rcos(off.vy - 2048) / 16384;
    while (i--) {
        if (rec->unk10 == 0) {
            rec->unk10 = rand() & 0x7f;
            rec->unk0.vx = f->unk4DEF0.vx + off.vx;
            rec->unk0.vy = f->unk4DEF0.vy - f->unk4DF18.unk1B6 / 2;
            rec->unk0.vz = f->unk4DEF0.vz + off.vz;
            rec->unk8.vx = (rand() & 0x7f) - 63;
            rec->unk8.vy = (rand() & 0x7f) - 63;
            rec->unk8.vz = (rand() & 0x7f) - 63;
            if (rec->unk8.vx >= 48 && (rec->unk8.vz >= 48 || rec->unk8.vz < -47))
                rec->unk10 = 0;
            if (rec->unk8.vx < -47 && (rec->unk8.vz >= 48 || rec->unk8.vz < -47))
                rec->unk10 = 0;
            if (++count == 200)
                break;
        }
        rec++;
    }
}
