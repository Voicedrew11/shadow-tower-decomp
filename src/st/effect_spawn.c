/*
 * ST.EXE: spawning an effect (WIP module split from src/st/effect.c and
 * src/st/effect_move.c by address). VS.EXE's form is func_80032404
 * (src/vs/effect_80032404.c).
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <psyq/libc.h>
#include <st/data.h>
#include <st/vis.h>
#include <st/funcs.h>
#include <st/player.h>
#include <st/effect.h>

JUMPTABLES(0x800114cc, 0x3d8)

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

/* The spell's sound at the effect: 28, or 36 with flag 0x20. */
#define EFFECT_SOUND(e)                                                        \
    func_80058254((((e)->unk0 & 0x20) >> 2) + 28, &(e)->unk20, 128, 28000, 29000, 0)

/*
 * Spawns an effect of kind `kind` at `pos` moving by `dir` (either may be
 * NULL); `type` is kept in the effect, flag 0x20 picks the second model
 * set, and the arguments after `dir` depend on the kind: mostly a scale,
 * a rotation and a model. Returns the effect, freed again for a kind
 * without one, or NULL when the pool is full.
 *
 * Not exact (1633 of 1635 instructions; the rest is identical): kinds
 * 0x9a/0xf0-0xf5 at +0x1084. Retail loads the entry into $v0, copies it
 * to $v1 and stores $v0 (`lw v0,4(s3); move v1,v0; sw v0,76(s2)`); this
 * keeps one register. Ruled out: the variable set before the store, a
 * chained assignment, args[1] read twice, a variable shared with another
 * kind (the copy then sinks and the store takes $v1), a member store, and
 * a do/while (0) around the store, the copy or the three entry stores.
 */
ADDRESS(0x80036484, 0x198c)
void *func_80036484(u8 type, u8 flags, u8 kind, VECTOR *pos, SVECTOR *dir, ...)
{
    int *args = (int *)&dir;
    u8 second = (flags & 0x20) >> 5;
    StEffect *e;
    Vis212 *entry;
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
        e->unk5 = 0;
        e->unk4 = type;
        e->unk30.vx = e->unk30.vy = e->unk30.vz = 0;
        e->unk14 = 0;
        e->unk3 = 0;
        e->unk8 = 0;
        e->unk12 = 0;
        e->unk7 = 0;
        e->unk10 = -1;
        e->unkE = 0;
        e->unk1C = NULL;
        switch (e->unk1) {
        case 0x00:
        case 0x0a:
            EFFECT_SOUND(e);
            e->unk10 = 50;
            e->unk50.ub[0] = 0;
            e->unk50.ub[1] = 0;
            e->unk6 = 0;
            e->unkC = 64;
            e->unk18 = 0x80000040;
            func_80036484(10, e->unk0, 0xab, &e->unk20, &e->unk40, args[1], e);
            func_80036484(10, e->unk0, 0xaa, &e->unk20, &e->unk40, args[1], e);
            break;
        case 0xab:
            effect_preset_5(e, (second << 3) | 4, args[1], 13);
            e->unk10 = 50;
            e->unk1C = (StEffect *)args[2];
            break;
        case 0xaa:
            effect_preset_5(e, (second << 3) | 3, args[1], 15);
            e->unk10 = 50;
            e->unk50.ub[0] = 0;
            e->unk1C = (StEffect *)args[2];
            break;
        case 0xac:
        case 0xad:
        case 0xae:
            effect_preset_1(e, args[4] + (second << 3), args[1], args[6]);
            e->unk50.h = args[2];
            e->unk10 = args[3];
            e->unk52.h = args[5];
            e->unk30 = *(SVECTOR *)args[7];
            break;
        case 0x07:
            EFFECT_SOUND(e);
            e->unk30 = *(SVECTOR *)args[2];
            e->unk10 = 50;
            e->unk50.ub[0] = 0;
            e->unk50.ub[1] = 0;
            e->unk6 = 0;
            e->unkC = 64;
            e->unk18 = 0x80000040;
            e->unk30.vy += 2048;
            func_80036484(10, e->unk0, 0xaf, &e->unk20, &e->unk40, args[1], e);
            func_80036484(10, e->unk0, 0xb0, &e->unk20, &e->unk40, args[1], e);
            break;
        case 0xaf:
            effect_preset_1(e, second << 3, args[1], 15);
            e->unk10 = 50;
            e->unk50.ub[0] = 0;
            e->unk1C = (StEffect *)args[2];
            e->unk30 = e->unk1C->unk30;
            break;
        case 0xb0:
            effect_preset_5(e, (second << 3) | 1, args[1], 15);
            e->unk10 = 50;
            e->unk50.ub[0] = 0;
            e->unk50.ub[1] = 0;
            e->unk1C = (StEffect *)args[2];
            break;
        case 0xb1:
            effect_preset_5(e, args[4] + (second << 3), args[1], args[5]);
            e->unk50.h = args[2];
            e->unk10 = args[3];
            break;
        case 0xb2:
            effect_preset_5(e, args[4] + (second << 3), args[1], args[5]);
            e->unk50.h = args[2];
            e->unk10 = args[3];
            break;
        case 0xb3:
            effect_preset_1(e, args[4] + (second << 3), args[1], args[5]);
            e->unk50.h = args[2];
            e->unk10 = args[3];
            break;
        case 0x03:
            e->unk50.ub[0] = 16;
            e->unkC = e->unk38.vx / 16;
            e->unk18 = (e->unk38.vy / 16) | 0x80000000;
            goto bolt;
        case 0x09:
            e->unk50.ub[0] = 8;
            e->unkC = e->unk38.vx / 16;
            e->unk18 = (e->unk38.vy / 16) | 0x80000000;
            e->unk52.h = rand() / 128;
            goto bolt;
        case 0x0c:
            e->unk7 = 15;
            e->unk50.ub[0] = 0;
            e->unkC = e->unk38.vx * 384 / 4096;
            e->unk18 = (e->unk38.vy * 384 / 4096) | 0x80000000;
            goto bolt;
        case 0x06:
            e->unk50.ub[0] = 0;
            e->unkC = e->unk38.vx / 16;
            e->unk18 = (e->unk38.vy / 64) | 0x80000000;
            goto bolt;
        case 0x0f:
            e->unk50.ub[0] = 0;
            e->unkC = e->unk38.vx / 16;
            e->unk18 = (e->unk38.vy / 256) | 0x80000000;
            goto bolt;
        case 0x01:
        case 0x04:
        case 0x0d:
        case 0x10:
            e->unk50.ub[0] = 0;
            e->unkC = e->unk38.vx / 16;
            e->unk18 = (e->unk38.vy / 16) | 0x80000000;
        bolt:
            EFFECT_SOUND(e);
            e->unk10 = 50;
            e->unk2 = 128;
            e->unkA = second << 3;
            e->unk7 = 13;
            e->unk9 = 51;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            e->unk30 = *(SVECTOR *)args[2];
            func_80036484(10, e->unk0, 0x2b, &e->unk20, &e->unk40, args[1], e);
            break;
        case 0x2b:
            effect_preset_5(e, (second << 3) | 1, args[1], 15);
            e->unk10 = 50;
            e->unk50.ub[0] = 0;
            e->unk1C = (StEffect *)args[2];
            break;
        case 0x2c:
        case 0x2d:
        case 0x2e:
        case 0x2f:
            e->unk30 = *(SVECTOR *)args[1];
            e->unk10 = 50;
            e->unk2 = 128;
            e->unk9 = 51;
            e->unk12 = 4096;
            SCALE(e, 4096);
            e->unkC = 32;
            e->unk18 = 0x80000020;
            e->unk7 = 0;
            e->unk50.ub[0] = 8;
            e->unkA = e->unk1 - 94;
            e->unk52.h = args[2];
            break;
        case 0x30:
        case 0x38:
            e->unk10 = 96;
            e->unk2 = 128;
            e->unkA = (u8)args[3] + 16;
            e->unk7 = 15;
            e->unk9 = 51;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            e->unk30 = *(SVECTOR *)args[2];
            REACH(e, e->unk38.vx * 896 / 4096);
            break;
        case 0x66:
            e->unk10 = 96;
            e->unk2 = 128;
            e->unkA = (u8)args[3] + 16;
            e->unk7 = 13;
            e->unk9 = 51;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            e->unk30 = *(SVECTOR *)args[2];
            REACH(e, e->unk38.vx * 896 / 4096);
            break;
        case 0x7d:
        case 0xa1:
            e->unk10 = 96;
            e->unk2 = 132;
            e->unkA = (u8)args[3] + 16;
            e->unk7 = 13;
            e->unk9 = 51;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            e->unk30 = *(SVECTOR *)args[2];
            REACH(e, e->unk38.vx * 896 / 4096);
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
            e->unkA = (u8)args[2] + 16;
            if (e->unk1 == 0x34 || e->unk1 == 0x8f || e->unk1 == 0x85 || e->unk1 == 0x4b
                || e->unk1 == 0x73 || e->unk1 == 0x99)
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
            break;
        case 0xc8:
        case 0xd3:
        case 0xe4:
        case 0xee:
            e->unk6 = 1;
            e->unk10 = 32;
            e->unk2 = 130;
            e->unkA = (u8)args[3] + 16;
            e->unk7 = 13;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            e->unk30 = *(SVECTOR *)args[2];
            REACH(e, e->unk38.vx / 8);
            ST_EFFECT_ENTRY(e) = (Vis212 *)args[4];
            break;
        case 0xc9:
        case 0xd4:
        case 0xe5:
        case 0xef:
            e->unk6 = 1;
            e->unk10 = 96;
            e->unk2 = 128;
            e->unkA = (u8)args[3] + 16;
            e->unk7 = 13;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            e->unk30 = *(SVECTOR *)args[2];
            REACH(e, e->unk38.vx / 8);
            e->unk50.h = -896;
            e->unk1C = (StEffect *)args[4];
            break;
        case 0x35:
        case 0x53:
        case 0x82:
        case 0x8d:
            e->unk6 = 1;
            e->unk10 = 96;
            e->unk2 = 129;
            e->unkA = (u8)args[3] + 16;
            e->unk7 = 13;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            e->unk30 = *(SVECTOR *)args[2];
            REACH(e, e->unk38.vx / 8);
            e->unk50.h = -896;
            e->unk1C = (StEffect *)args[4];
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
            e->unkA = (u8)args[3] + 16;
            e->unk7 = 13;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            e->unk30 = *(SVECTOR *)args[2];
            REACH(e, e->unk38.vx / 8);
            ST_EFFECT_ENTRY(e) = (Vis212 *)args[4];
            e->unk50.h = ST_EFFECT_ENTRY(e)->unkA8->arg.u[6];
            e->unk52.h = ST_EFFECT_ENTRY(e)->unkA8->arg.u[9];
            break;
        case 0x3d:
        case 0x46:
        case 0x52:
        case 0x5a:
        case 0x64:
        case 0x86:
            e->unk6 = 5;
            e->unk10 = 32;
            e->unk2 = 128;
            e->unkA = (u8)args[3] + 16;
            e->unk7 = 13;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            e->unk30 = *(SVECTOR *)args[2];
            e->unk50.h = -896;
            REACH(e, e->unk38.vx);
            e->unk1C = (StEffect *)args[4];
            ST_EFFECT_ENTRY(e) = ST_EFFECT_ENTRY(e->unk1C);
            e->unk20.vy = ST_EFFECT_ENTRY(e)->unk74.vy;
            e->unk30.vy = 0;
            e->unk40.vx = 0;
            e->unk40.vy = 0;
            e->unk40.vz = 0;
            break;
        case 0x7e:
        case 0x81:
        case 0x8a:
            e->unk6 = 5;
            e->unk10 = 32;
            e->unk2 = 128;
            e->unkA = (u8)args[3] + 16;
            e->unk7 = 13;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            e->unk30 = *(SVECTOR *)args[2];
            e->unk50.h = -896;
            REACH(e, e->unk38.vx);
            e->unk1C = (StEffect *)args[4];
            ST_EFFECT_ENTRY(e) = ST_EFFECT_ENTRY(e->unk1C);
            e->unk20.vy = viewer.position.vy + 1600;
            e->unk30.vy = 0;
            e->unk40.vx = 0;
            e->unk40.vy = 0;
            e->unk40.vz = 0;
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
            e->unkA = (u8)args[3] + 16;
            e->unk7 = 13;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            e->unk30 = *(SVECTOR *)args[2];
            REACH(e, e->unk38.vx / 8);
            e->unk50.h = -896;
            e->unk1C = (StEffect *)args[4];
            ST_EFFECT_ENTRY(e) = ST_EFFECT_ENTRY(e->unk1C);
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
            effect_preset_5(e, args[4] + 16, args[1], args[5] | 4);
            e->unk2 = 131;
            e->unk50.h = args[2];
            e->unk10 = args[3];
            break;
        case 0x3a:
            e->unk6 = 1;
            e->unk10 = 32;
            e->unk2 = 128;
            e->unkA = (u8)args[3] + 16;
            e->unk7 = 0;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            e->unk30 = *(SVECTOR *)args[2];
            REACH(e, e->unk38.vx / 8);
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
            if (e->unk1 == 0xa4)
                e->unk2 = 133;
            e->unkA = (u8)args[3] + 16;
            e->unk7 = 13;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            e->unk30 = *(SVECTOR *)args[2];
            REACH(e, e->unk38.vx / 2);
            break;
        case 0x9a:
        case 0xf0:
        case 0xf1:
        case 0xf2:
        case 0xf3:
        case 0xf4:
        case 0xf5:
            e->unk10 = 16;
            e->unk6 = 0;
            ST_EFFECT_ENTRY(e) = (Vis212 *)args[1];
            entry = ST_EFFECT_ENTRY(e);
            entry->unk24 = 4096;
            entry->unk1C.bytes[2] = 13;
            entry->unk36 = 4095;
            break;
        case 0xd5:
            e->unk6 = 5;
            e->unk10 = 16;
            RANDOM_FRAME(frame);
            e->unk2 = (frame + 3) | 0x80;
            e->unkA = (u8)args[2] + 16;
            e->unk7 = 15;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            e->unk50.b[0] = (rand() >> 11) + 16;
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
            e->unkA = (u8)args[2] + 16;
            e->unk7 = 15;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            REACH(e, e->unk38.vx * 1536 / 4096);
            func_80036484(10, e->unk0, 0xd6, &e->unk20, &e->unk40, e->unk38.vx, e->unkA - 16, e);
            break;
        case 0xd6:
            e->unk6 = 5;
            e->unk10 = 64;
            e->unk2 = 128;
            e->unkA = (u8)args[2] + 16;
            e->unk7 = 15;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            e->unk1C = (StEffect *)args[3];
            break;
        case 0xd7:
            e->unk6 = 5;
            e->unk10 = 16;
            RANDOM_FRAME(frame);
            e->unk2 = frame | 0x80;
            e->unkA = (u8)args[2] + 16;
            e->unk7 = 15;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            e->unk50.b[0] = (rand() >> 11) + 16;
            break;
        case 0x55:
            e->unk6 = 1;
            e->unk10 = 64;
            e->unk2 = 134;
            e->unkA = (u8)args[2] + 16;
            e->unk7 = 15;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            REACH(e, e->unk38.vx * 1536 / 4096);
            break;
        case 0x59:
            e->unk6 = 1;
            e->unk10 = 64;
            e->unk2 = 131;
            e->unkA = (u8)args[2] + 16;
            e->unk7 = 15;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            REACH(e, e->unk38.vx * 1536 / 4096);
            break;
        case 0x97:
            e->unk6 = 1;
            e->unk10 = 64;
            e->unk2 = 132;
            e->unkA = (u8)args[2] + 16;
            e->unk7 = 15;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            REACH(e, e->unk38.vx * 1536 / 4096);
            break;
        case 0x65:
        case 0x75:
        case 0x77:
        case 0x80:
        case 0xa6:
            e->unk6 = 5;
            e->unk10 = 64;
            e->unk2 = 128;
            e->unkA = (u8)args[3] + 16;
            e->unk7 = 13;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            e->unk30 = *(SVECTOR *)args[2];
            e->unk30.vx = 0;
            e->unk30.vy = 0;
            e->unk30.vz = 0;
            REACH(e, e->unk38.vx / 8);
            break;
        case 0x5d:
            e->unk6 = 1;
            e->unk10 = 64;
            e->unk2 = 132;
            e->unkA = (u8)args[3] + 16;
            e->unk7 = 128;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            e->unk30 = *(SVECTOR *)args[2];
            REACH(e, e->unk38.vx / 8);
            break;
        case 0x69:
            e->unk6 = 1;
            e->unk10 = 64;
            e->unk2 = 132;
            e->unkA = (u8)args[3] + 16;
            e->unk7 = 13;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            e->unk30 = *(SVECTOR *)args[2];
            REACH(e, e->unk38.vx / 2);
            break;
        case 0x76:
            e->unk6 = 1;
            e->unk10 = 64;
            e->unk2 = 129;
            e->unkA = (u8)args[3] + 16;
            e->unk7 = 13;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            e->unk30 = *(SVECTOR *)args[2];
            e->unkC = e->unk38.vx / 2;
            e->unk18 = e->unk38.vy / 2;
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
            e->unkA = (u8)args[3] + 16;
            if (e->unk1 == 0x40)
                e->unk7 = 14;
            else
                e->unk7 = 13;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            e->unk30 = *(SVECTOR *)args[2];
            e->unkC = e->unk38.vx / 2;
            e->unk18 = e->unk38.vy / 2;
            break;
        case 0x78:
        case 0x79:
            e->unk6 = 1;
            e->unk10 = 64;
            e->unk2 = 128;
            e->unkA = (u8)args[3] + 16;
            e->unk7 = 0;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            e->unk30 = *(SVECTOR *)args[2];
            REACH(e, e->unk38.vx / 8);
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
            e->unkA = (u8)args[3] + 16;
            e->unk7 = 13;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            e->unk30 = *(SVECTOR *)args[2];
            REACH(e, e->unk38.vx);
            a.vx = e->unk20.vx + e->unk38.vx * rcos(e->unk30.vz) / 4096;
            a.vy = e->unk20.vy + e->unk38.vx * rsin(e->unk30.vz) / 4096;
            b.vx = e->unk20.vx + e->unk38.vx * rcos(e->unk30.vz + 2048) / 4096;
            b.vy = e->unk20.vy + e->unk38.vx * rsin(e->unk30.vz + 2048) / 4096;
            func_80036484(10, e->unk0, 0xe9, &a, &e->unk40, 1024, e->unkA - 16, e);
            func_80036484(10, e->unk0, 0xea, &b, &e->unk40, 1024, e->unkA - 16, e);
            break;
        case 0xe9:
        case 0xea:
            e->unk6 = 5;
            e->unk10 = 64;
            e->unk2 = 131;
            e->unkA = (u8)args[2] + 16;
            e->unk7 = 13;
            e->unk12 = 4096;
            SCALE(e, args[1]);
            e->unk1C = (StEffect *)args[3];
            break;
        default:
            e->unk0 = ST_EFFECT_FREE;
            break;
        }
    }
    return e;
}
