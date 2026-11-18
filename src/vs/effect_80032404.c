/*
 * VS.EXE: spawning an effect (WIP module). VS's form of ST.EXE's
 * func_80036484 (src/st/effect_spawn.c): the same kinds from 0x30 up,
 * owned by a fighter and one of its attacks.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <psyq/libc.h>
#include <st/effect.h>
#include <vs/data.h>
#include <vs/buffer.h>
#include <vs/effect.h>

JUMPTABLES(0x80011934, 0x300)

/* The same scale on the three axes. */
#define SCALE(e, s) (e)->unk38.vx = (e)->unk38.vy = (e)->unk38.vz = (s)

/* The attack's reach, and the same as its height. */
#define REACH(e, r)                                                            \
    (e)->unkC = (r);                                                           \
    (e)->unk18 = (r) | 0x80000000

/* A frame 0-2 of the model, at random. */
#define RANDOM_FRAME(frame)                                                    \
    frame = rand() * 3 / 32768;                                                \
    if (frame > 2)                                                             \
        frame = 2

/*
 * Spawns an effect of kind `kind` for attack `attack` of fighter `n` at
 * `pos` moving by `dir` (either may be NULL); the arguments after `dir`
 * depend on the kind: mostly a scale, a rotation and a first delay.
 * Returns the effect, freed again for a kind without one, or NULL when
 * the pool is full.
 */
ADDRESS(0x80032404, 0xf38)
void *func_80032404(int n, u8 attack, u8 flags, u8 kind, VECTOR *pos, SVECTOR *dir, ...)
{
    int *args = (int *)&dir;
    StEffect *e;
    VECTOR a;
    VECTOR b;
    u8 frame;

    e = func_800322f0();
    if (e != NULL) {
        e->unk0 = flags;
        e->unk1 = kind;
        if (pos != NULL)
            e->unk20 = *pos;
        if (dir != NULL)
            e->unk40 = *dir;
        else
            e->unk40.vx = e->unk40.vy = e->unk40.vz = 0;
        SCALE(e, 4096);
        e->unk6 = 1;
        e->unk9 = 255;
        e->unk4 = e->unk5 = 0;
        e->unk30.vx = e->unk30.vy = e->unk30.vz = 0;
        e->unk14 = 0;
        e->unk3 = 0;
        e->unk8 = 0;
        e->unk12 = 0;
        e->unk7 = 0;
        e->unk10 = -1;
        e->unkE = 0;
        e->unk1C = NULL;
        e->unk4C = n;
        e->unk4D = attack;
        switch (e->unk1) {
        case 0x30:
        case 0x38:
            e->unk10 = 96;
            e->unk2 = 128;
            e->unk7 = 7;
            e->unk9 = 51;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            e->unk30 = *(SVECTOR *)args[2];
            REACH(e, e->unk38.vx * 896 / 4096);
            e->unk8 = args[3];
            break;
        case 0x66:
            e->unk10 = 96;
            e->unk2 = 128;
            e->unk7 = 5;
            e->unk9 = 51;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            e->unk30 = *(SVECTOR *)args[2];
            REACH(e, e->unk38.vx * 896 / 4096);
            e->unk8 = args[3];
            break;
        case 0x7d:
        case 0xa1:
            e->unk10 = 96;
            e->unk2 = 132;
            e->unk7 = 5;
            e->unk9 = 51;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            e->unk30 = *(SVECTOR *)args[2];
            REACH(e, e->unk38.vx * 896 / 4096);
            e->unk8 = args[3];
            break;
        case 0x31:
        case 0x32:
        case 0x33:
        case 0x34:
        case 0x36:
        case 0x37:
        case 0x39:
        case 0x3c:
        case 0x3f:
        case 0x42:
        case 0x43:
        case 0x45:
        case 0x47:
        case 0x49:
        case 0x4a:
        case 0x4b:
        case 0x4d:
        case 0x4f:
        case 0x51:
        case 0x56:
        case 0x5f:
        case 0x60:
        case 0x6a:
        case 0x6b:
        case 0x6d:
        case 0x6e:
        case 0x6f:
        case 0x70:
        case 0x72:
        case 0x73:
        case 0x7b:
        case 0x7f:
        case 0x85:
        case 0x87:
        case 0x8f:
        case 0x90:
        case 0x92:
        case 0x94:
        case 0x98:
        case 0x99:
        case 0x9b:
        case 0x9d:
        case 0x9f:
        case 0xa3:
        case 0xa5:
            e->unk6 = 5;
            if (e->unk1 == 0x87)
                e->unk10 = 128;
            else
                e->unk10 = 64;
            RANDOM_FRAME(frame);
            if (e->unk1 == 0x60)
                frame += 4;
            if (e->unk1 == 0x99)
                frame += 3;
            if (e->unk1 == 0x79)
                frame += 2;
            if (e->unk1 == 0x85)
                frame += 4;
            if (e->unk1 == 0xa3)
                frame += 2;
            e->unk2 = frame | 0x80;
            if (e->unk1 == 0x34 || e->unk1 == 0x8f || e->unk1 == 0x85 || e->unk1 == 0x4b
                || e->unk1 == 0x73)
                e->unk7 = 14;
            else
                e->unk7 = 15;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            REACH(e, e->unk38.vx / 8);
            if (e->unk1 == 0x56 || e->unk1 == 0x5f || e->unk1 == 0x79 || e->unk1 == 0x70)
                e->unk50.b = (rand() >> 9) - 128;
            else
                e->unk50.b = (rand() >> 11) + 16;
            e->unk8 = args[2];
            break;
        case 0xc8:
        case 0xd3:
        case 0xe4:
        case 0xee:
            e->unk6 = 1;
            e->unk10 = 32;
            e->unk2 = 130;
            e->unk7 = 13;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            e->unk30 = *(SVECTOR *)args[2];
            REACH(e, e->unk38.vx / 8);
            e->unk8 = 1;
            break;
        case 0xc9:
        case 0xd4:
        case 0xe5:
        case 0xef:
            e->unk6 = 1;
            e->unk10 = 96;
            e->unk2 = 128;
            e->unk7 = 13;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            e->unk30 = *(SVECTOR *)args[2];
            REACH(e, e->unk38.vx / 8);
            e->unk50.h = -896;
            e->unk1C = (StEffect *)args[3];
            e->unk8 = 1;
            break;
        case 0x35:
        case 0x53:
        case 0x82:
        case 0x8d:
            e->unk6 = 1;
            e->unk10 = 96;
            e->unk2 = 129;
            e->unk7 = 13;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            e->unk30 = *(SVECTOR *)args[2];
            REACH(e, e->unk38.vx / 8);
            e->unk50.h = -896;
            e->unk1C = (StEffect *)args[3];
            e->unk8 = 1;
            break;
        case 0xca:
        case 0xcd:
        case 0xd0:
        case 0xd8:
        case 0xdb:
        case 0xde:
        case 0xe1:
        case 0xe6:
        case 0xeb:
            e->unk6 = 1;
            e->unk10 = 36;
            e->unk2 = 130;
            e->unk7 = 13;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            e->unk30 = *(SVECTOR *)args[2];
            REACH(e, e->unk38.vx / 8);
            e->unk8 = 1;
            break;
        case 0x3d:
        case 0x46:
        case 0x52:
        case 0x5a:
        case 0x64:
        case 0x7e:
        case 0x81:
        case 0x86:
        case 0x8a:
            e->unk6 = 5;
            e->unk10 = 32;
            e->unk2 = 128;
            e->unk7 = 13;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            e->unk30 = *(SVECTOR *)args[2];
            e->unk50.h = -896;
            REACH(e, e->unk38.vx);
            e->unk1C = (StEffect *)args[3];
            if (e->unk1 == 0x7e || e->unk1 == 0x81)
                e->unk20.vy = DAT_800633b0.fighter[e->unk4C].unk4DF18.unk1B6 + 1600;
            else
                e->unk20.vy = DAT_800633b0.fighter[e->unk4C].unk4DF18.unk1B6;
            e->unk30.vy = 0;
            e->unk40.vx = 0;
            e->unk40.vy = 0;
            e->unk40.vz = 0;
            e->unk8 = 1;
            break;
        case 0xcb:
        case 0xce:
        case 0xd1:
        case 0xd9:
        case 0xdc:
        case 0xdf:
        case 0xe2:
        case 0xe7:
        case 0xec:
            e->unk6 = 1;
            e->unk10 = 32;
            e->unk2 = 129;
            e->unk7 = 13;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            e->unk30 = *(SVECTOR *)args[2];
            REACH(e, e->unk38.vx / 8);
            e->unk50.h = -896;
            e->unk1C = (StEffect *)args[3];
            e->unk8 = 1;
            break;
        case 0xcc:
        case 0xcf:
        case 0xd2:
        case 0xda:
        case 0xdd:
        case 0xe0:
        case 0xe3:
        case 0xe8:
        case 0xed:
            e->unk6 = 5;
            e->unk7 = args[4] | 4;
            e->unk9 = 51;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            e->unk2 = 131;
            e->unk50.h = args[2];
            e->unk10 = args[3];
            e->unk8 = 1;
            break;
        case 0x3a:
            e->unk6 = 1;
            e->unk10 = 32;
            e->unk2 = 128;
            e->unk7 = 0;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            e->unk30 = *(SVECTOR *)args[2];
            REACH(e, e->unk38.vx / 8);
            e->unk8 = 1;
            break;
        case 0x3e:
        case 0x41:
        case 0x48:
        case 0x5e:
        case 0x6c:
        case 0x71:
        case 0x7a:
        case 0x89:
        case 0x93:
        case 0xa4:
            e->unk6 = 1;
            e->unk10 = 64;
            e->unk2 = 131;
            if (e->unk1 == 0x5e)
                e->unk2 = 128;
            if (e->unk1 == 0xa4)
                e->unk2 = 133;
            e->unk7 = 13;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            e->unk30 = *(SVECTOR *)args[2];
            e->unk8 = 1;
            REACH(e, e->unk38.vx / 2);
            break;
        case 0xd5:
            e->unk6 = 5;
            e->unk10 = 16;
            RANDOM_FRAME(frame);
            e->unk2 = (frame + 3) | 0x80;
            e->unk7 = 15;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            e->unk50.b = (rand() >> 11) + 16;
            e->unk8 = 1;
            break;
        case 0x57:
        case 0x58:
        case 0x5c:
        case 0x61:
        case 0x62:
        case 0x63:
        case 0x67:
        case 0x68:
        case 0x74:
        case 0x8c:
        case 0x95:
        case 0x9c:
        case 0x9e:
            e->unk6 = 1;
            e->unk10 = 64;
            e->unk2 = 131;
            e->unk7 = 15;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            REACH(e, e->unk38.vx * 1536 / 4096);
            func_80032404(e->unk4C, e->unk4D, e->unk0, 0xd6, &e->unk20, &e->unk40, e->unk38.vx, e);
            e->unk8 = 1;
            break;
        case 0xd6:
            e->unk6 = 5;
            e->unk10 = 64;
            e->unk2 = 128;
            e->unk7 = 15;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            e->unk1C = (StEffect *)args[2];
            e->unk8 = 1;
            break;
        case 0xd7:
            e->unk6 = 5;
            e->unk10 = 16;
            RANDOM_FRAME(frame);
            e->unk2 = frame | 0x80;
            e->unk7 = 15;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            e->unk50.b = (rand() >> 11) + 16;
            e->unk8 = 1;
            break;
        case 0x55:
            e->unk6 = 1;
            e->unk10 = 64;
            e->unk2 = 134;
            e->unk7 = 15;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            REACH(e, e->unk38.vx * 1536 / 4096);
            e->unk8 = 1;
            break;
        case 0x59:
            e->unk6 = 1;
            e->unk10 = 64;
            e->unk2 = 131;
            e->unk7 = 15;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            REACH(e, e->unk38.vx * 1536 / 4096);
            e->unk8 = 1;
            break;
        case 0x97:
            e->unk6 = 1;
            e->unk10 = 64;
            e->unk2 = 132;
            e->unk7 = 15;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            REACH(e, e->unk38.vx * 1536 / 4096);
            e->unk8 = 1;
            break;
        case 0x65:
        case 0x75:
        case 0x77:
        case 0x80:
        case 0xa6:
            e->unk6 = 5;
            e->unk10 = 64;
            e->unk2 = 128;
            e->unk7 = 13;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            e->unk30 = *(SVECTOR *)args[2];
            e->unk30.vx = 0;
            e->unk30.vy = 0;
            e->unk30.vz = 0;
            REACH(e, e->unk38.vx / 8);
            e->unk8 = 1;
            break;
        case 0x5d:
            e->unk6 = 1;
            e->unk10 = 64;
            e->unk2 = 132;
            e->unk7 = 0;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            e->unk30 = *(SVECTOR *)args[2];
            REACH(e, e->unk38.vx / 8);
            e->unk8 = 1;
            break;
        case 0x69:
            e->unk6 = 1;
            e->unk10 = 64;
            e->unk2 = 132;
            e->unk7 = 13;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            e->unk30 = *(SVECTOR *)args[2];
            e->unk8 = 1;
            REACH(e, e->unk38.vx / 2);
            break;
        case 0x76:
            e->unk6 = 1;
            e->unk10 = 64;
            e->unk2 = 129;
            e->unk7 = 13;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            e->unk30 = *(SVECTOR *)args[2];
            e->unk8 = 1;
            REACH(e, e->unk38.vx / 2);
            break;
        case 0x40:
        case 0x88:
        case 0x8b:
        case 0xa2:
        case 0xa8:
            e->unk6 = 1;
            e->unk10 = 64;
            e->unk2 = 128;
            if (e->unk1 == 0xa8)
                e->unk2 = 132;
            if (e->unk1 == 0x40)
                e->unk2 = 132;
            if (e->unk1 == 0x40)
                e->unk7 = 14;
            else
                e->unk7 = 13;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            e->unk30 = *(SVECTOR *)args[2];
            e->unk8 = 1;
            e->unkC = e->unk38.vx / 2;
            e->unk18 = e->unk38.vy / 2;
            break;
        case 0x78:
        case 0x79:
            e->unk6 = 1;
            e->unk10 = 64;
            e->unk2 = 128;
            e->unk7 = 0;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            e->unk30 = *(SVECTOR *)args[2];
            REACH(e, e->unk38.vx / 8);
            e->unk8 = 1;
            break;
        case 0x7c:
        case 0x83:
        case 0x84:
        case 0x8e:
        case 0x91:
        case 0x96:
        case 0xa0:
        case 0xa7:
        case 0xa9:
            e->unk6 = 1;
            e->unk10 = 64;
            e->unk2 = 128;
            e->unk7 = 13;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            e->unk30 = *(SVECTOR *)args[2];
            REACH(e, e->unk38.vx);
            a.vx = e->unk20.vx + e->unk38.vx * rcos(e->unk30.vz) / 4096;
            a.vy = e->unk20.vy + e->unk38.vx * rsin(e->unk30.vz) / 4096;
            b.vx = e->unk20.vx + e->unk38.vx * rcos(e->unk30.vz + 2048) / 4096;
            b.vy = e->unk20.vy + e->unk38.vx * rsin(e->unk30.vz + 2048) / 4096;
            func_80032404(e->unk4C, e->unk4D, e->unk0, 0xe9, &e->unk20, &e->unk40, e->unk38.vx, e);
            func_80032404(e->unk4C, e->unk4D, e->unk0, 0xea, &e->unk20, &e->unk40, e->unk38.vx, e);
            e->unk8 = 1;
            break;
        case 0xe9:
        case 0xea:
            e->unk6 = 5;
            e->unk10 = 64;
            e->unk2 = 131;
            e->unk7 = 13;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            e->unk1C = (StEffect *)args[2];
            e->unk8 = 1;
            break;
        default:
            e->unk0 = ST_EFFECT_FREE;
            break;
        }
    }
    return e;
}
