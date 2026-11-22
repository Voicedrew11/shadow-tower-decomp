/*
 * The effects: spawning an effect, moving it, the
 * VRAM block move and roll, the per-kind effect update, the effect
 * pool and the effects' model draw. One translation unit: retail lays out
 * its constants (0x800118c4, the model draw's initializers) before its
 * switch tables (0x80011934, effect_spawn's and effect_update's), the
 * order cc1 writes one object's read-only data in.
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
#include <LIBGPU.H>
#include <vs/funcs.h>
#include <INLINE_C.H>
#include <GTEMAC.H>
#include <st/mesh.h>
#include <vs/scratch.h>

/* The module's constants, then its switch tables. */
RODATA(0x800118c4, 0x670)

/*
 * Spawning an effect. VS's form of ST.EXE's
 * effect_spawn (src/st/effect_spawn.c): the same kinds from 0x30 up,
 * owned by a fighter and one of its attacks.
 */
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
StEffect *effect_spawn(int n, u8 attack, u8 flags, u8 kind, VECTOR *pos, SVECTOR *dir, ...)
{
    int *args = (int *)&dir;
    StEffect *e;
    VECTOR a;
    VECTOR b;
    u8 frame;

    e = effect_alloc();
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
                e->unk50.b[0] = (rand() >> 9) - 128;
            else
                e->unk50.b[0] = (rand() >> 11) + 16;
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
                e->unk20.vy = shared_buffer.fighter[e->unk4C].unk4DF18.unk1B6 + 1600;
            else
                e->unk20.vy = shared_buffer.fighter[e->unk4C].unk4DF18.unk1B6;
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
            e->unk50.b[0] = (rand() >> 11) + 16;
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
            effect_spawn(e->unk4C, e->unk4D, e->unk0, 0xd6, &e->unk20, &e->unk40, e->unk38.vx, e);
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
            e->unk50.b[0] = (rand() >> 11) + 16;
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
            effect_spawn(e->unk4C, e->unk4D, e->unk0, 0xe9, &e->unk20, &e->unk40, e->unk38.vx, e);
            effect_spawn(e->unk4C, e->unk4D, e->unk0, 0xea, &e->unk20, &e->unk40, e->unk38.vx, e);
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

/*
 * Moving the current effect and the VRAM block move and roll. VS's copies of ST.EXE's src/st/effect_move.c functions; the
 * VRAM pair is shared (src/lib/vram.inc).
 */
/* The effect attack, then moves the current effect and turns its roll by
 * `spin`; returns the attack's result. */
ADDRESS(0x8003333c, 0x78)
int effect_attack_move(int arg0, u_long height, int spin)
{
    StEffect *e = vs_effects.cur;
    int result = effect_reaches_opponent(arg0, height);

    e->unk20.vx += e->unk40.vx;
    e->unk20.vy += e->unk40.vy;
    e->unk20.vz += e->unk40.vz;
    e->unk30.vz = (e->unk30.vz - spin) & 0xfff;
    return result;
}

/* Moves the current effect back by its velocity. */
ADDRESS(0x800333b4, 0x40)
void effect_step_back(void)
{
    StEffect *e = vs_effects.cur;

    e->unk20.vx -= e->unk40.vx;
    e->unk20.vy -= e->unk40.vy;
    e->unk20.vz -= e->unk40.vz;
}

/* Moves the current effect back by half its velocity. */
ADDRESS(0x800333f4, 0x7c)
void effect_step_back_half(void)
{
    StEffect *e = vs_effects.cur;

    e->unk20.vx -= e->unk40.vx / 2;
    e->unk20.vy -= e->unk40.vy / 2;
    e->unk20.vz -= e->unk40.vz / 2;
}

#include "../lib/vram.inc"

/*
 * The per-kind effect update: one frame of the
 * current effect, by kind. VS's form of ST.EXE's effect_update
 * (src/st/effect_spawn.c): the same kinds from 0x30 up, hitting the
 * owner's opponent instead of making ST's attack requests.
 */
/* Fades the effect in over its last 32 frames. */
#define FADE_IN(e)                                                             \
    if ((e)->unk10 < 32)                                                       \
        (e)->unk12 = (e)->unk10 * 128

/* The owner's attack hits its opponent. */
#define HIT(e) fighter_apply_attack_hit((e)->unk4C, (e)->unk4D)

/* A hit, while the effect has hits left. */
#define HIT_COUNTED(e, reached)                                                \
    if ((reached) && (e)->unk8) {                                              \
        HIT(e);                                                                \
        (e)->unk8--;                                                           \
    }

/* A hit that ends the effect. */
#define HIT_ONCE(e, reached)                                                   \
    if (reached) {                                                             \
        HIT(e);                                                                \
        (e)->unk0 = ST_EFFECT_FREE;                                            \
    }

/* Spawns an effect of kind `kind` for the current effect's owner. */
#define SPAWN(e, kind) effect_spawn((e)->unk4C, (e)->unk4D, (e)->unk0, kind

/* The two effects a kind turns into, at its place. */
#define SPAWN_PAIR(e, first, second)                                           \
    SPAWN(e, first), &(e)->unk20, &(e)->unk40, (e)->unk38.vx, &(e)->unk30, e); \
    SPAWN(e, second), &(e)->unk20, &(e)->unk40, (e)->unk38.vx, &(e)->unk30, e)

/*
 * Runs the current effect (vs_effects.cur) for one frame.
 *
 * `unused` stands for 32 bytes of locals that retail's frame has ahead of
 * the first block's and that nothing reads or writes (ST.EXE's
 * effect_update has the same 32); their real declarations are not known.
 * The block locals whose addresses are passed on keep their stack slots
 * for the rest of the function (their kinds' pairs sit side by side);
 * only the ring kinds' block gives its slots back.
 */
ADDRESS(0x800335e8, 0x1ce4)
void effect_update(void)
{
    StEffect *e = vs_effects.cur;
    int kind = e->unk1;
    VECTOR unused[2];
    StEffect *child;
    int i;
    int r;
    int x;
    int y;
    int z;
    s16 shake;

    switch (kind) {
    case 0x30:
    case 0x38:
        e->unk38.vx += 96;
        e->unk38.vy += 96;
        REACH(e, e->unk38.vx * 896 / 4096);
        HIT_COUNTED(e, effect_attack_move(e->unkC, e->unk18, 32));
        break;
    case 0x66:
        e->unk2 = (e->unk10 % 3) | 0x80;
        e->unk38.vx += 96;
        e->unk38.vy += 96;
        REACH(e, e->unk38.vx * 896 / 4096);
        HIT_COUNTED(e, effect_attack_move(e->unkC, e->unk18, 32));
        break;
    case 0x7d:
    case 0xa1:
        e->unk2 = (e->unk10 % 3) | 0x84;
        e->unk38.vx += 96;
        e->unk38.vy += 96;
        REACH(e, e->unk38.vx * 896 / 4096);
        HIT_COUNTED(e, effect_attack_move(e->unkC, e->unk18, 32));
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
    case 0x49:
    case 0x4a:
    case 0x4b:
    case 0x4d:
    case 0x4f:
    case 0x51:
    case 0x60:
    case 0x6a:
    case 0x6b:
    case 0x6d:
    case 0x6e:
    case 0x6f:
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
    case 0x9d:
    case 0x9f:
    case 0xa3:
    case 0xa5:
        e->unk38.vx += 96;
        e->unk38.vy += 96;
        REACH(e, e->unk38.vx * 896 / 4096);
        HIT_COUNTED(e, effect_attack_move(e->unkC, e->unk18, 32));
        if (e->unk20.vy > 0)
            e->unk0 = ST_EFFECT_FREE;
        FADE_IN(e);
        e->unk40.vy += 2;
        break;
    case 0xc8:
    case 0xd3:
    case 0xe4:
    case 0xee:
        e->unk30.vz = (e->unk30.vz - 32) & 0xfff;
        if (e->unk10 == 8) {
            if (kind == 0xc8) {
                child = SPAWN(e, 0x35), &e->unk20, &e->unk40, e->unk38.vx, &e->unk30, e);
                SPAWN(e, 0xc9), &e->unk20, &e->unk40, e->unk38.vx, &e->unk30, child);
            } else if (kind == 0xee) {
                child = SPAWN(e, 0x82), &e->unk20, &e->unk40, e->unk38.vx, &e->unk30, e);
                SPAWN(e, 0xe5), &e->unk20, &e->unk40, e->unk38.vx, &e->unk30, child);
            } else if (kind == 0xe4) {
                child = SPAWN(e, 0x82), &e->unk20, &e->unk40, e->unk38.vx, &e->unk30, e);
                SPAWN(e, 0xe5), &e->unk20, &e->unk40, e->unk38.vx, &e->unk30, child);
            } else if (kind == 0xd3) {
                child = SPAWN(e, 0x53), &e->unk20, &e->unk40, e->unk38.vx, &e->unk30, e);
                SPAWN(e, 0xd4), &e->unk20, &e->unk40, e->unk38.vx, &e->unk30, child);
            }
        }
        if (e->unk10 < 16) {
            e->unk38.vx -= 64;
            e->unk38.vy -= 64;
            e->unk38.vz -= 64;
        } else {
            e->unk38.vx += 64;
            e->unk38.vy += 64;
            e->unk38.vz += 64;
        }
        FADE_IN(e);
        break;
    case 0xc9:
    case 0xd4:
    case 0xe5:
    case 0xef:
        effect_attack_move(e->unkC, e->unk18, 128);
        FADE_IN(e);
        if (e->unk1C->unk0 == ST_EFFECT_FREE)
            e->unk0 = ST_EFFECT_FREE;
        break;
    case 0x35:
    case 0x53:
    case 0x82:
    case 0x8d:
        e->unk38.vz += 224;
        HIT_COUNTED(e, effect_attack_move(e->unkC, e->unk18, 32));
        if (e->unk20.vx < -30000 || e->unk20.vx > 30000 || e->unk20.vz > 30000
            || e->unk20.vz < -30000)
            e->unk0 = ST_EFFECT_FREE;
        FADE_IN(e);
        break;
    case 0x3a:
        HIT_COUNTED(e, effect_attack_move(e->unkC, e->unk18, 128));
        if (e->unk10 < 32) {
            e->unk7 = 13;
            e->unk12 = e->unk10 * 128;
        }
        e->unk40.vy += 2;
        break;
    case 0x3e:
    case 0x48:
    case 0x6c:
    case 0x71:
    case 0x7a:
    case 0x93:
        if (effect_attack_move(e->unkC, e->unk18, 128) || e->unk20.vy >= 0) {
            for (i = 0; i < 4; i++) {
                SVECTOR dir;
                VECTOR pos;

                dir.vx = 8 - rand() / 2024;
                dir.vy = -48 - rand() / 1024;
                dir.vz = 8 - rand() / 2024;
                pos.vx = e->unk20.vx - dir.vx;
                pos.vy = e->unk20.vy;
                pos.vz = e->unk20.vz - dir.vz;
                if (kind == 0x3e)
                    SPAWN(e, 0x3f), &pos, &dir, 1536, i & 1);
                else if (kind == 0x6c)
                    SPAWN(e, 0x6d), &pos, &dir, 1536, i & 1);
                else if (kind == 0x48)
                    SPAWN(e, 0x49), &pos, &dir, 1536, i & 1);
                else if (kind == 0x93)
                    SPAWN(e, 0x94), &pos, &dir, 1536, i & 1);
                else if (kind == 0x71)
                    SPAWN(e, 0x72), &pos, &dir, 1536, i & 1);
                else if (kind == 0x7a)
                    SPAWN(e, 0x7b), &pos, &dir, 1536, i & 1);
            }
            e->unk0 = ST_EFFECT_FREE;
        }
        if (e->unk10 < 32) {
            e->unk7 = 13;
            e->unk12 = e->unk10 * 128;
        }
        e->unk40.vy += 8;
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
        e->unk30.vz = (e->unk30.vz - 128) & 0xfff;
        {
            VsFighter *f = &shared_buffer.fighter[e->unk4C];
            SVECTOR v;
            MATRIX m;
            SVECTOR rot;
            VECTOR vec;

            if (f->unk4DF18.unk14A[e->unk4D].unk2 == -32768) {
                if (f->unk4DF10 == 2) {
                    anim_get_vertex(e->unk4C, f->unk4DEE8, f->unk4DEEA,
                                  f->unk4DF18.unk14A[e->unk4D].unk6, &v);
                } else {
                    anim_set_node_vertex(e->unk4C, f->unk4DF18.unk14A[e->unk4D].unk4,
                                  f->unk4DF18.unk14A[e->unk4D].unk6, &vec);
                    v.vx = vec.vx;
                    v.vy = vec.vy;
                    v.vz = vec.vz;
                }
                rot.vx = 0;
                rot.vy = f->unk4DF02;
                rot.vz = 0;
                RotMatrix(&rot, &m);
                ApplyMatrixSV(&m, &v, &v);
                e->unk20.vx = f->unk4DEF0.vx + v.vx;
                e->unk20.vy = f->unk4DEF0.vy + v.vy;
                e->unk20.vz = f->unk4DEF0.vz + v.vz;
            }
        }
        if (e->unk10 == 33) {
            if (kind == 0xca) {
                SPAWN_PAIR(e, 0xcb, 0x3d);
            } else if (kind == 0xeb) {
                SPAWN_PAIR(e, 0xec, 0x8a);
            } else if (kind == 0xcd) {
                SPAWN_PAIR(e, 0xce, 0x46);
            } else if (kind == 0xe1) {
                SPAWN_PAIR(e, 0xe2, 0x81);
            } else if (kind == 0xd0) {
                SPAWN_PAIR(e, 0xd1, 0x52);
            } else if (kind == 0xe6) {
                SPAWN_PAIR(e, 0xe7, 0x86);
            } else if (kind == 0xd8) {
                SPAWN_PAIR(e, 0xd9, 0x5a);
            } else if (kind == 0xdb) {
                SPAWN_PAIR(e, 0xdc, 0x64);
            } else if (kind == 0xde) {
                SPAWN_PAIR(e, 0xdf, 0x7e);
            }
        }
        if (e->unk10 < 24) {
            e->unk38.vx -= 64;
            e->unk38.vy -= 64;
            e->unk38.vz -= 64;
        } else {
            e->unk38.vx += 64;
            e->unk38.vy += 64;
            e->unk38.vz += 64;
        }
        FADE_IN(e);
        break;
    case 0x3d:
    case 0x46:
    case 0x52:
    case 0x5a:
    case 0x64:
    case 0x7e:
    case 0x81:
    case 0x86:
    case 0x8a: {
        SVECTOR dir;
        VECTOR pos;
        int len;

        SCALE(e, rcos(e->unk10 << 11) / 4);
        e->unk50.h += 32;
        if (e->unk50.h >= -128)
            e->unk50.h = -128;
        if (e->unk1 == 0x7e || e->unk1 == 0x81)
            len = (current_fighter->unk4DEF0.vy + current_fighter->unk4DF18.unk1B2 - e->unk1C->unk20.vy)
                * rcos(e->unk50.h) / rsin(e->unk50.h);
        else
            len = (current_fighter->unk4DEF0.vy - e->unk1C->unk20.vy) * rcos(e->unk50.h)
                / rsin(e->unk50.h);
        e->unk20.vx = e->unk1C->unk20.vx - len * rcos(e->unk1C->unk30.vy + 1024) / 4096;
        e->unk20.vz = e->unk1C->unk20.vz + len * rsin(e->unk1C->unk30.vy + 1024) / 4096;
        e->unk20.vy = 0;
        FADE_IN(e);
        HIT_COUNTED(e, effect_reaches_opponent(1024, 0x80000400));
        pos.vx = e->unk20.vx;
        pos.vy = e->unk20.vy;
        pos.vz = e->unk20.vz;
        dir.vx = 0;
        dir.vy = 0;
        dir.vz = 0;
        if (kind == 0x3d)
            SPAWN(e, 0xcc), &pos, &dir, 512, 256, 6, 3);
        else if (kind == 0x8a)
            SPAWN(e, 0xed), &pos, &dir, 512, 256, 6, 3);
        else if (kind == 0x46)
            SPAWN(e, 0xcf), &pos, &dir, 512, 256, 6, 3);
        else if (kind == 0x81)
            SPAWN(e, 0xe3), &pos, &dir, 512, 256, 6, 3);
        else if (kind == 0x52)
            SPAWN(e, 0xd2), &pos, &dir, 512, 256, 6, 3);
        else if (kind == 0x86)
            SPAWN(e, 0xe8), &pos, &dir, 512, 256, 6, 3);
        else if (kind == 0x5a)
            SPAWN(e, 0xda), &pos, &dir, 512, 256, 6, 3);
        else if (kind == 0x64)
            SPAWN(e, 0xdd), &pos, &dir, 512, 256, 6, 3);
        else if (kind == 0x7e)
            SPAWN(e, 0xe0), &pos, &dir, 512, 256, 6, 3);
        break;
    }
    case 0xcb:
    case 0xce:
    case 0xd1:
    case 0xd9:
    case 0xdc:
    case 0xdf:
    case 0xe2:
    case 0xe7:
    case 0xec:
        e->unk20.vx = e->unk1C->unk20.vx;
        e->unk20.vy = e->unk1C->unk20.vy;
        e->unk20.vz = e->unk1C->unk20.vz;
        e->unk50.h += 32;
        if (e->unk50.h >= -128)
            e->unk50.h = -128;
        {
            int dy;
            int len;

            if (e->unk1 == 0xdf || e->unk1 == 0xe2) {
                len = (current_fighter->unk4DEF0.vy + current_fighter->unk4DF18.unk1B2
                       - e->unk1C->unk20.vy)
                      * rcos(e->unk50.h) / rsin(e->unk50.h);
                dy = current_fighter->unk4DEF0.vy + current_fighter->unk4DF18.unk1B2
                     - e->unk1C->unk20.vy;
            } else {
                len = (current_fighter->unk4DEF0.vy - e->unk1C->unk20.vy) * rcos(e->unk50.h)
                      / rsin(e->unk50.h);
                dy = current_fighter->unk4DEF0.vy - e->unk1C->unk20.vy;
            }
            e->unk38.vz = SquareRoot0(len * len + dy * dy);
        }
        e->unk30.vx = -e->unk50.h;
        if (e->unk10 < 8)
            e->unk12 = e->unk10 * 512;
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
        e->unk20.vx += e->unk40.vx;
        e->unk20.vy += e->unk40.vy;
        e->unk20.vz += e->unk40.vz;
        e->unk38.vx += e->unk50.h;
        e->unk38.vz = e->unk38.vx;
        if (e->unk5 < 3)
            e->unk38.vy = e->unk38.vx * 2;
        else
            e->unk38.vy -= e->unk50.h;
        e->unk5++;
        break;
    case 0x47:
    case 0x9b:
        e->unk38.vx += 128;
        e->unk38.vy += 128;
        REACH(e, e->unk38.vx * 896 / 4096);
        effect_attack_move(e->unkC, e->unk18, e->unk50.b[0]);
        FADE_IN(e);
        e->unk40.vy += 2;
        if (e->unk8 == 0) {
            HIT(e);
            e->unk8 = 32;
        } else {
            e->unk8--;
        }
        shake = (rand() - 16384) >> ((64 - e->unk10) / 8 + 9);
        if (shake < -2 || shake > 2)
            vs_gfx_state.unk2CA78.vy += shake * 10;
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
        e->unk38.vx += 96;
        e->unk38.vy += 96;
        e->unk38.vz += 96;
        REACH(e, e->unk38.vx / 4);
        HIT_ONCE(e, effect_attack_move(e->unkC, e->unk18, 256));
        if (e->unk10 % 5 == 0) {
            SVECTOR dir;
            VECTOR pos;

            dir.vx = e->unk40.vx / 4 - (s16)(rand() / 1024 - 16);
            dir.vy = e->unk40.vy / 4 - rand() / 1024;
            dir.vz = e->unk40.vz / 4 - (s16)(rand() / 1024 - 16);
            pos.vx = e->unk20.vx - dir.vx;
            pos.vy = e->unk20.vy;
            pos.vz = e->unk20.vz - dir.vz;
            SPAWN(e, 0xd7), &pos, &dir, 512);
        }
        FADE_IN(e);
        if (e->unk20.vy > 0)
            e->unk0 = ST_EFFECT_FREE;
        break;
    case 0xd6:
        e->unk20.vx = e->unk1C->unk20.vx;
        e->unk20.vy = e->unk1C->unk20.vy;
        e->unk20.vz = e->unk1C->unk20.vz;
        e->unk38.vx = e->unk1C->unk38.vx;
        e->unk38.vy = e->unk1C->unk38.vy;
        e->unk38.vz = e->unk1C->unk38.vz;
        e->unk2 = (e->unk10 % 3) | 0x80;
        FADE_IN(e);
        if (e->unk1C->unk0 == ST_EFFECT_FREE)
            e->unk0 = ST_EFFECT_FREE;
        break;
    case 0xd7:
        e->unk38.vx += 96;
        e->unk38.vy += 96;
        effect_attack_move(e->unkC, e->unk18, e->unk50.b[0]);
        e->unk12 = e->unk10 * 256;
        e->unk40.vy += 2;
        break;
    case 0x56:
    case 0x5f:
    case 0x70:
        e->unk38.vx += 96;
        e->unk38.vy += 96;
        REACH(e, e->unk38.vx * 896 / 4096);
        HIT_COUNTED(e, effect_attack_move(e->unkC, e->unk18, e->unk50.b[0]));
        if (e->unk10 % 5 == 0) {
            SVECTOR dir;
            VECTOR pos;

            dir.vx = e->unk40.vx / 4 - (s16)(rand() / 1024 - 16);
            dir.vy = e->unk40.vy / 4 - (s16)(rand() / 1024 + 48);
            dir.vz = e->unk40.vz / 4 - (s16)(rand() / 1024 - 16);
            pos.vx = e->unk20.vx - dir.vx;
            pos.vy = e->unk20.vy;
            pos.vz = e->unk20.vz - dir.vz;
            SPAWN(e, 0xd5), &pos, &dir, 4096);
        }
        FADE_IN(e);
        e->unk40.vy += 2;
        break;
    case 0xd5:
        e->unk38.vx += 96;
        e->unk38.vy += 96;
        effect_attack_move(e->unkC, e->unk18, e->unk50.b[0]);
        e->unk12 = e->unk10 * 256;
        e->unk40.vy -= 16;
        break;
    case 0x55:
    case 0x59:
    case 0x97:
        e->unk38.vy = rcos((32 - e->unk10) << 6);
        e->unk38.vx += 192;
        e->unk38.vz += 192;
        REACH(e, e->unk38.vx * 1536 / 4096);
        e->unk30.vy += 128;
        HIT_COUNTED(e, effect_attack_move(e->unkC, e->unk18, 0));
        FADE_IN(e);
        break;
    case 0x5d:
    case 0x65:
    case 0x75:
    case 0x77:
    case 0x78:
    case 0x79:
    case 0x80:
    case 0xa6:
        HIT_ONCE(e, effect_attack_move(e->unkC, e->unk18, -256));
        FADE_IN(e);
        if (e->unk20.vy > 0)
            e->unk0 = ST_EFFECT_FREE;
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
        e->unk2 = (e->unk10 % 3) | 0x80;
        HIT_ONCE(e, effect_attack_move(e->unkC, e->unk18, -256));
        FADE_IN(e);
        break;
    case 0xe9:
        if (e->unk1C->unk0 == ST_EFFECT_FREE) {
            e->unk0 = ST_EFFECT_FREE;
            break;
        }
        r = e->unk1C->unk38.vx * rcos(e->unk1C->unk30.vz) / 4096;
        x = r * rcos(e->unk1C->unk30.vy) / 4096;
        z = r * rsin(e->unk1C->unk30.vy) / 4096;
        y = rsin(e->unk1C->unk30.vz);
        e->unk20.vx = e->unk1C->unk20.vx + x;
        e->unk20.vy = e->unk1C->unk20.vy + e->unk1C->unk38.vx * rsin(e->unk1C->unk30.vz) / 4096;
        e->unk20.vz = e->unk1C->unk20.vz + z;
        FADE_IN(e);
        break;
    case 0xea:
        if (e->unk1C->unk0 == ST_EFFECT_FREE) {
            e->unk0 = ST_EFFECT_FREE;
            break;
        }
        r = e->unk1C->unk38.vx * rcos(e->unk1C->unk30.vz + 2048) / 4096;
        x = r * rcos(e->unk1C->unk30.vy) / 4096;
        z = r * rsin(e->unk1C->unk30.vy) / 4096;
        y = rsin(e->unk1C->unk30.vz + 2048);
        e->unk20.vx = e->unk1C->unk20.vx + x;
        e->unk20.vy = e->unk1C->unk20.vy
                      + e->unk1C->unk38.vx * rsin(e->unk1C->unk30.vz + 2048) / 4096;
        e->unk20.vz = e->unk1C->unk20.vz + z;
        FADE_IN(e);
        break;
    case 0x40:
    case 0x5e:
    case 0x69:
    case 0x76:
    case 0x88:
    case 0x89:
    case 0x8b:
    case 0xa2:
    case 0xa4:
    case 0xa8:
        if (kind != 0x5e)
            e->unk30.vy += rsin(64 - e->unk10);
        e->unk38.vx += 128;
        e->unk38.vy += 128;
        e->unk38.vz += 128;
        e->unkC = e->unk38.vx / 2;
        e->unk18 = e->unk38.vx / 2;
        HIT_COUNTED(e, effect_reaches_opponent(e->unkC, e->unk18 | 0x80000000));
        break;
    default:
        break;
    }
}

/*
 * Clearing and running the effect pool. VS's copies of
 * ST.EXE's src/st/effect_run.c functions.
 */
ADDRESS(0x800352cc, 0x30)
void effects_clear(void)
{
    StEffect *e = vs_effects.slots;
    u16 i;

    for (i = 0; i < ST_EFFECT_SLOTS; i++) {
        e->unk0 = ST_EFFECT_FREE;
        e++;
    }
}

/* Runs every live effect for one frame through the interpreter, freeing
 * those whose time is up. */
ADDRESS(0x800352fc, 0xd4)
void effects_run(void)
{
    StEffect *e = vs_effects.slots;

    for (vs_effects.index = 0; vs_effects.index < ST_EFFECT_SLOTS; vs_effects.index++) {
        if (e->unk0 != ST_EFFECT_FREE) {
            vs_effects.cur = e;
            vs_effects.def = &vs_effects.defs[e->unk1];
            if (--e->unk10 == -1)
                e->unk0 = ST_EFFECT_FREE;
            else
                effect_update();
        }
        e++;
    }
}

/*
 * Drawing one effect model part and the effects' models.
 */
/* Projects the current part's vertices: RTPT in threes, the first group
 * ahead of the loop so each pass stores one group and starts the next, then
 * RTPS for the rest. */
#define PROJECT(sp, out, in, n)                                                \
    {                                                                          \
        (sp)->z_ot += (sp)->header->n_vertices;                                \
        out = (MeshVertex *)(sp)->vertices;                                    \
        in = (SVECTOR *)(sp)->local_vertices;                                  \
        n = (sp)->header->n_vertices / 3;                                      \
        if (n--) {                                                             \
            gte_ldv3c(in);                                                     \
            gte_rtpt();                                                        \
            in += 3;                                                           \
            for (;;) {                                                         \
                gte_stsxy3(&out[0].sxy, &out[1].sxy, &out[2].sxy);             \
                gte_stsz3(&out[0].z, &out[1].z, &out[2].z);                    \
                out += 3;                                                      \
                if (!n--)                                                      \
                    break;                                                     \
                gte_ldv3c(in);                                                 \
                in += 3;                                                       \
                gte_rtpt();                                                    \
            }                                                                  \
        }                                                                      \
        n = (sp)->header->n_vertices % 3;                                      \
        while (n--) {                                                          \
            gte_ldv0(in);                                                      \
            gte_rtps();                                                        \
            in++;                                                              \
            gte_stsxy(&out->sxy);                                              \
            gte_stsz(&out->z);                                                 \
            out++;                                                             \
        }                                                                      \
    }

ADDRESS(0x800353d0, 0x798)
void effect_draw_model_part(u8 *model, VECTOR *pos, SVECTOR *rot, SVECTOR *scale, MATRIX *view,
                   u8 part, short light, u8 flags, int z)
{
    VsScratchPad *sp = VS_SCRATCHPAD;
    SVECTOR rel;
    VECTOR s;
    MATRIX work0;
    MATRIX work1;
    VsLights lights = {
        {{3000, -3700, -1500}, {-2000, -3700, -3000}, {0, 1000, 0}}, 0,
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, 0,
        {200, 200, 200, 0}, {-1, -1}};
    MATRIX unit = {{{4096, 0, 0}, {0, 4096, 0}, {0, 0, 4096}}, {0, 0, 0}};
    long flag;
    MeshVertex *out;
    SVECTOR *in;
    int n;

    sp->vertices = vs_gfx_state.unkA7EC;
    sp->unk48 = vs_gfx_state.unkA7EC;
    sp->z_ot = vs_gfx_state.unk831C[0];
    sp->face_count = vs_gfx_state.unk831C[1];
    sp->prim_count = vs_gfx_state.unk8324;
    sp->unk94 = vs_gfx_state.unk8328;
    sp->prim = vs_gfx_state.packet->cur;
    sp->prim_end = vs_gfx_state.packet->end;
    gte_SetRotMatrix(&vs_gfx_state.unk2C998);
    SetTransMatrix(&vs_gfx_state.unk2C998);
    if (view) {
        rel.vx = pos->vx - vs_gfx_state.unk2CA78.vx;
        rel.vy = pos->vy - vs_gfx_state.unk2CA78.vy;
        rel.vz = pos->vz - vs_gfx_state.unk2CA78.vz;
        gte_RotTrans(&rel, (VECTOR *)sp->matrix.t, &flag);
    } else {
        sp->matrix.t[0] = pos->vx;
        sp->matrix.t[1] = pos->vy;
        sp->matrix.t[2] = pos->vz;
    }
    RotMatrixYXZ(rot, &sp->matrix);
    gte_SetColorMatrix(&lights.lc);
    gte_SetLightMatrix(&lights.l);
    if (scale) {
        s.vx = scale->vx;
        s.vy = scale->vy;
        s.vz = scale->vz;
        ScaleMatrix(&sp->matrix, &s);
    }
    if (view) {
        gte_MulMatrix0(view, &sp->matrix, &sp->matrix);
    }
    gte_SetRotMatrix(&sp->matrix);
    SetTransMatrix(&sp->matrix);
    sp->model = model;
    sp->header = (MeshHeader *)(sp->model + 12) + part;
    sp->local_vertices = sp->model + 12 + sp->header->vertices;
    if (view) {
        PROJECT(sp, out, in, n);
    } else {
        PROJECT(sp, out, in, n);
    }
    sp->rgb.r = light / 32;
    sp->rgb.g = light / 32;
    sp->rgb.b = light / 32;
    sp->rgb.cd = 0;
    mesh_draw_part_nccs(part);
    vs_gfx_state.packet->cur = sp->prim;
    vs_gfx_state.packet->end = sp->prim_end;
    vs_gfx_state.unk831C[0] = sp->z_ot;
    vs_gfx_state.unk831C[1] = sp->face_count;
    vs_gfx_state.unk8324 = sp->prim_count;
    vs_gfx_state.unk8328 = sp->unk94;
}

/* Draws every live effect with a model (VS's copy of ST.EXE's
 * world_draw_effects_placements, src/st/objects2.c): kind 0 through the camera, kind 4
 * without a view. */
ADDRESS(0x80035b68, 0x204)
void effects_draw(void)
{
    VsScratchPad *sp = VS_SCRATCHPAD;
    StEffect *o = vs_effects.slots;
    MATRIX unit = {{{ONE, 0, 0}, {0, ONE, 0}, {0, 0, ONE}}};
    int i;

    for (i = 127; i != -1; o++, i--) {
        if (o->unk0 == 0xff)
            continue;
        if ((o->unk6 & 3) == 0)
            continue;
        sp->unk13C = o->unk20;
        sp->clut_base = o->unkE;
        switch (o->unk6 & 0xc) {
        case 0:
            sp->near_z = 11;
            sp->far_z = 4103;
            sp->ot = vs_gfx_state.ot_cur - 7;
            effect_draw_model_part((u8 *)shared_buffer.fighter[o->unk4C].unk4B000, &o->unk20, &o->unk30,
                          &o->unk38, &vs_gfx_state.unk2C998, o->unk2 & 0x7f, o->unk12, o->unk7, 0);
            break;
        case 4:
            sp->near_z = 11;
            sp->far_z = 4103;
            sp->ot = vs_gfx_state.ot_cur - 7;
            effect_draw_model_part((u8 *)shared_buffer.fighter[o->unk4C].unk4B000, &o->unk20, &o->unk30,
                          &o->unk38, &unit, o->unk2 & 0x7f, o->unk12, o->unk7, 0);
            break;
        case 8:
        case 12:
            break;
        }
    }
}
