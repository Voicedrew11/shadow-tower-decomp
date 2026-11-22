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
ADDRESS(0x8002d738, 0x260)
void sparks_start_burst(int n)
{
    VsFighter *f = &shared_buffer.fighter[n];
    VsFighter *o = &shared_buffer.fighter[1 - n];
    VsSpark *rec = sparks;
    SVECTOR off;
    int count = 0;
    int i = 2000;

    off.vy = ratan2(f->position.vx - o->position.vx, f->position.vz - o->position.vz);
    off.vx = f->data.radius * rsin(off.vy - 2048) / 16384;
    off.vz = f->data.radius * rcos(off.vy - 2048) / 16384;
    while (i--) {
        if (rec->life == 0) {
            int vx;
            int vz;

            rec->life = rand() & 0x7f;
            rec->position.vx = f->position.vx + off.vx;
            rec->position.vy = f->position.vy - f->data.height / 2;
            rec->position.vz = f->position.vz + off.vz;
            rec->velocity.vx = (rand() & 0x7f) - 63;
            rec->velocity.vy = (rand() & 0x7f) - 63;
            vz = (rand() & 0x7f) - 63;
            vx = rec->velocity.vx;
            rec->velocity.vz = vz;
            if (vx >= 48 && (vz >= 48 || vz < -47))
                rec->life = 0;
            else if (vx < -47) {
                if ((short)vz >= 48)
                    rec->life = 0;
                else if ((short)vz < -47)
                    rec->life = 0;
            }
            if (++count == 200)
                break;
        }
        rec++;
    }
}
