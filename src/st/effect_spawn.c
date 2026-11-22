/*
 * ST.EXE: the effect spawner, mover and per-kind update as one
 * translation unit (WIP module, -G8: the update's three small initialisers
 * are .sdata), 0x80036484..0x8003bc10. effect_spawn's switch tables start at
 * 0x800114cc, 4 mod 8 relative to the world renderer's read-only data, so
 * they open a new object; effect_update's switch tables at 0x800118a4
 * are 8-aligned relative to that start: one object's read-only data.
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
#include <LIBGPU.H>
#include <st/anim.h>

RODATA(0x800114cc, 0x7b0)
/* The script's flame frames, smoke strip VRAM source and spark frames. */
SDATA(0x80088018, 0x14)

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
    sound_play_at_range((((e)->unk0 & 0x20) >> 2) + 28, &(e)->unk20, 128, 28000, 29000, 0)

/*
 * Spawns an effect of kind `kind` at `pos` moving by `dir` (either may be
 * NULL); `type` is kept in the effect, flag 0x20 picks the second model
 * set, and the arguments after `dir` depend on the kind: mostly a scale,
 * a rotation and a model. Returns the effect, freed again for a kind
 * without one, or NULL when the pool is full.
 */
ADDRESS(0x80036484, 0x198c)
StEffect *effect_spawn(u8 type, u8 flags, u8 kind, VECTOR *pos, SVECTOR *dir, ...)
{
    int *args = (int *)&dir;
    u8 second = (flags & 0x20) >> 5;
    StEffect *e;
    Creature *entry;
    Creature *owner;
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
            effect_spawn(10, e->unk0, 0xab, &e->unk20, &e->unk40, args[1], e);
            effect_spawn(10, e->unk0, 0xaa, &e->unk20, &e->unk40, args[1], e);
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
            effect_spawn(10, e->unk0, 0xaf, &e->unk20, &e->unk40, args[1], e);
            effect_spawn(10, e->unk0, 0xb0, &e->unk20, &e->unk40, args[1], e);
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
            effect_spawn(10, e->unk0, 0x2b, &e->unk20, &e->unk40, args[1], e);
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
            ST_EFFECT_ENTRY(e) = (Creature *)args[4];
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
            ST_EFFECT_ENTRY(e) = (Creature *)args[4];
            e->unk50.h = ST_EFFECT_ENTRY(e)->script->arg.u[6];
            e->unk52.h = ST_EFFECT_ENTRY(e)->script->arg.u[9];
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
            e->unk20.vy = ST_EFFECT_ENTRY(e)->position.vy;
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
            e->unk10 = 16;
            e->unk6 = 0;
            owner = (Creature *)args[1];
            entry = owner;
        mark_owner:
            ST_EFFECT_ENTRY(e) = owner;
            entry->unk24 = 4096;
            entry->status.bytes[2] = 13;
            entry->fade = 4095;
            break;
        case 0xf3:
        case 0xf4:
        case 0xf5:
            /*
             * The second family (the interpreter keeps the two apart as
             * well) repeats the set-up and joins the first. Retail's one
             * merged copy still holds the entry in two registers, which
             * takes two variables that meet at a label.
             */
            e->unk10 = 16;
            e->unk6 = 0;
            owner = (Creature *)args[1];
            entry = owner;
            goto mark_owner;
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
            effect_spawn(10, e->unk0, 0xd6, &e->unk20, &e->unk40, e->unk38.vx, e->unkA - 16, e);
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
            effect_spawn(10, e->unk0, 0xe9, &a, &e->unk40, 1024, e->unkA - 16, e);
            effect_spawn(10, e->unk0, 0xea, &b, &e->unk40, 1024, e->unkA - 16, e);
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

/*
 * Adds `accel` (scaled by vector_scale) to the current effect's velocity,
 * limits its length to `limit` and moves the effect by it; then, unless
 * `height` is -1, requests an attack (effect_attack). Returns -1 when the
 * attack was made, else 0.
 */
ADDRESS(0x80037e10, 0x1e8)
int effect_accelerate(int scale, int limit, int attack, int height, SVECTOR *accel)
{
    StEffect *e = effects.cur;
    int length;

    vector_scale(scale, accel);
    e->unk40.vx += accel->vx;
    e->unk40.vy += accel->vy;
    e->unk40.vz += accel->vz;
    length = vector_length_xyz(e->unk40.vx, e->unk40.vy, e->unk40.vz);
    if (limit < length) {
        e->unk40.vx = e->unk40.vx * limit / length;
        e->unk40.vy = e->unk40.vy * limit / length;
        e->unk40.vz = e->unk40.vz * limit / length;
    }
    e->unk20.vx += e->unk40.vx;
    e->unk20.vy += e->unk40.vy;
    e->unk20.vz += e->unk40.vz;
    if (height != -1 && effect_attack(attack, height, 0))
        return -1;
    return 0;
}

/*
 * Turns the current effect toward its target (the entry creature_find_in_cone
 * finds for an effect with flag 0x40, else the viewer when player_distance_in_arc
 * allows) by at most `turn`, and accelerates it along its heading; with
 * `scale_near` instead of `scale` while the target is within `near`.
 */
ADDRESS(0x80037ff8, 0x1b0)
int effect_home(int limit, int scale, int turn, int attack, int height, int near,
                int scale_near, int range)
{
    StEffect *e = effects.cur;
    VECTOR target;
    SVECTOR dir;
    SVECTOR v;
    int dist;
    Creature *t;

    if (e->unk0 & 0x40) {
        t = creature_find_in_cone(&e->unk20, e->unk30.vy, e->unk30.vx, 24000, range, range, &dist, 0);
        if (t == NULL)
            goto move;
        target.vx = t->position.vx;
        target.vy = t->position.vy - (t->height >> 1);
        target.vz = t->position.vz;
    aim:
        vector_to_angles(target.vx - e->unk20.vx, target.vy - e->unk20.vy,
                      target.vz - e->unk20.vz, &dir);
        e->unk30.vx = angle_step_toward(e->unk30.vx, dir.vx, turn);
        e->unk30.vy = angle_step_toward(e->unk30.vy, dir.vy, turn);
    } else {
        dist = player_distance_in_arc(&e->unk20, e->unk30.vy, 24000, 4096);
        if (dist == -1)
            goto move;
        target.vx = viewer.position.vx;
        target.vy = viewer.position.vy;
        target.vz = viewer.position.vz;
        goto aim;
    }
move:
    vector_from_angles(&e->unk30, &v);
    if (dist >= 0 && near >= dist)
        scale = scale_near;
    return effect_accelerate(scale, limit, attack, height, &v);
}

/*
 * Fires a kind-128 projectile request (player_distance_in_reach) from the current
 * effect along its velocity once its countdown has run out, then moves
 * it and turns its roll by `spin`. Returns 128 when the request was
 * taken, else 0.
 */
ADDRESS(0x800381a8, 0x158)
int effect_fire(int arg0, int height, int spin)
{
    StEffect *e = effects.cur;
    CollisionState *req;
    int fired = 0;

    if (e->unk8 == 0) {
        req = &vis_lists;
        req->position = e->unk20;
        req->radius = arg0;
        req->height = height;
        req->velocity = e->unk40;
        req->speed = SquareRoot0(e->unk40.vx * e->unk40.vx + e->unk40.vz * e->unk40.vz);
        req->heading = ratan2(e->unk40.vz, e->unk40.vx);
        vis_lists.tests = 128;
        fired = (player_distance_in_reach(req) != -1) << 7;
        e->unk20.vx += e->unk40.vx;
        e->unk20.vy += e->unk40.vy;
        e->unk20.vz += e->unk40.vz;
        e->unk30.vz = (e->unk30.vz - spin) & 0xfff;
    } else {
        e->unk8--;
    }
    return fired;
}

/* effect_attack, then moves the current effect and turns its roll by
 * `spin`; returns the attack's result. */
ADDRESS(0x80038300, 0x7c)
int effect_attack_move(int arg0, u_long height, int spin)
{
    StEffect *e = effects.cur;
    int result = effect_attack(arg0, height, 0);

    e->unk20.vx += e->unk40.vx;
    e->unk20.vy += e->unk40.vy;
    e->unk20.vz += e->unk40.vz;
    e->unk30.vz = (e->unk30.vz - spin) & 0xfff;
    return result;
}

/* Moves the current effect back by its velocity (no direct callers). */
ADDRESS(0x8003837c, 0x40)
void effect_step_back(void)
{
    StEffect *e = effects.cur;

    e->unk20.vx -= e->unk40.vx;
    e->unk20.vy -= e->unk40.vy;
    e->unk20.vz -= e->unk40.vz;
}

/* Moves the current effect back by half its velocity (no direct
 * callers). */
ADDRESS(0x800383bc, 0x7c)
void effect_step_back_half(void)
{
    StEffect *e = effects.cur;

    e->unk20.vx -= e->unk40.vx / 2;
    e->unk20.vy -= e->unk40.vy / 2;
    e->unk20.vz -= e->unk40.vz / 2;
}

#include "../lib/vram.inc"

/* Fades the effect in over its last 32 frames. */
#define FADE_IN(e)                                                             \
    if ((e)->unk10 < 32)                                                       \
        (e)->unk12 = (e)->unk10 * 128

/* The table A entry the last attack request hit. */
#define HIT_ENTRY() (&vis_table_a.entries[vis_lists.creature_in_reach])

/* Emits for a hit `r`, unless it hit an entry that is dead or in state 1. */
#define EMIT_LIVE(r)                                                           \
    if ((r) & 0x10) {                                                          \
        Creature *t = HIT_ENTRY();                                               \
                                                                               \
        if (t->hp != 0 && t->status.bytes[3] != 1)                           \
            effect_emit_behind(r);                                             \
    } else {                                                                   \
        effect_emit_behind(r);                                                 \
    }

/* A hit ends the effect, emitting when it hit the viewer. */
#define HIT_ENDS(e, r)                                                         \
    if (r) {                                                                   \
        if ((r) & 0x80)                                                        \
            effect_emit_behind(r);                                             \
        (e)->unk0 = ST_EFFECT_FREE;                                            \
    }

/* The spell sound: 29, or 37 with flag 0x20. */
#define SPELL_SOUND(e) effect_play_sound(e, (((e)->unk0 & 0x20) >> 2) + 29)

/* Spawns an effect of kind `kind` with the current effect's flags. */
#define SPAWN(e, kind) effect_spawn(10, (e)->unk0, kind

/* The two effects a kind turns into, at its place. */
#define SPAWN_PAIR(e, first, second)                                           \
    SPAWN(e, first), &(e)->unk20, &(e)->unk40, (e)->unk38.vx, &(e)->unk30,     \
          (e)->unkA - 16, e);                                                  \
    SPAWN(e, second), &(e)->unk20, &(e)->unk40, (e)->unk38.vx, &(e)->unk30,    \
          (e)->unkA - 16, e)

/* A piece falling off the effect: its speed and place. */
#define DEBRIS(e, dir, pos, up)                                                \
    (dir).vx = (e)->unk40.vx / 4 - (s16)(rand() / 1024 - 16);                  \
    (dir).vy = (e)->unk40.vy / 4 - (up);                                       \
    (dir).vz = (e)->unk40.vz / 4 - (s16)(rand() / 1024 - 16);                  \
    (pos).vx = (e)->unk20.vx - (dir).vx;                                       \
    (pos).vy = (e)->unk20.vy;                                                  \
    (pos).vz = (e)->unk20.vz - (dir).vz

/* A splash piece: its speed and place. */
#define SPLASH(e, dir, pos)                                                    \
    (dir).vx = 8 - rand() / 2024;                                              \
    (dir).vy = -48 - rand() / 1024;                                            \
    (dir).vz = 8 - rand() / 2024;                                              \
    (pos).vx = (e)->unk20.vx - (dir).vx;                                       \
    (pos).vy = (e)->unk20.vy;                                                  \
    (pos).vz = (e)->unk20.vz - (dir).vz

/* A spell bolt's own movement. */
#define BOLT_MOVE(e)                                                           \
    (e)->unk20.vx += (e)->unk40.vx;                                            \
    (e)->unk20.vy += (e)->unk40.vy;                                            \
    (e)->unk20.vz += (e)->unk40.vz;                                            \
    (e)->unk38.vx += (e)->unk50.h;                                             \
    (e)->unk38.vz = (e)->unk38.vx;                                             \
    if ((e)->unk5 < 3)                                                         \
        (e)->unk38.vy = (e)->unk38.vx * 2;                                     \
    else                                                                       \
        (e)->unk38.vy -= (e)->unk50.h;                                         \
    (e)->unk5++

/*
 * Runs the current effect (effects.cur) for one frame.
 *
 * Three sets of locals are here only because retail's frame has their
 * bytes and nothing reads or writes them; their real declarations are not
 * known: `unused` (32 bytes ahead of the first block's locals, as in
 * VS.EXE's effect_update), `spare` in the ring kinds' block (8 bytes: the
 * debris pair of kinds 0x57... only fits behind it) and `spare_dir` /
 * `spare_pos` (24 bytes behind the last pair; put in the last block, their
 * place among the later kinds is a guess).
 */
ADDRESS(0x800385b0, 0x3660)
void effect_update(void)
{
    StEffect *e = effects.cur;
    int r = 0;
    int kind = e->unk1;
    int step = e->unk5;
    int i = 0;
    VECTOR unused[2];
    int c;
    int x;
    int y;
    int z;
    int n;

    switch (kind) {
    case 0x00:
    case 0x0a: {
        VECTOR pos;
        SVECTOR rot;
        SVECTOR dir;

        r = effect_attack_move(e->unkC, e->unk18, 0);
        if (r == 0)
            break;
        EMIT_LIVE(r);
        SPELL_SOUND(e);
        pos.vx = e->unk20.vx;
        pos.vy = e->unk20.vy;
        pos.vz = e->unk20.vz;
        dir.vx = e->unk40.vx >> 1;
        dir.vy = e->unk40.vy >> 1;
        dir.vz = e->unk40.vz >> 1;
        if (r & 1) {
            dir.vy = 32;
            rot.vx = 0;
            rot.vy = 0;
            rot.vz = 0;
        } else if (r & 2) {
            r = ratan2(vis_lists.wall_normal_z, vis_lists.wall_normal_x) + 1024;
            c = rcos(ratan2(dir.vz, dir.vx) - r) * 300 / 4096;
            dir.vx = c * rcos(r) / 4096;
            dir.vz = c * rsin(r) / 4096;
            rot.vx = 1024;
            rot.vy = r;
            rot.vz = 0;
        } else if (r & 4) {
            dir.vy = 0;
            rot.vx = 2048;
            rot.vy = 0;
            rot.vz = 0;
        } else {
            rot.vx = 0;
            rot.vy = 0;
            rot.vz = 0;
        }
        SPAWN(e, 0xac), &pos, &dir, 1024, 1024, 6, 0, 0, 13, &rot);
        SPAWN(e, 0xad), &pos, &dir, 1024, 1024, 6, 1, 0, 15, &rot);
        SPAWN(e, 0xae), &pos, &dir, 0, 2048, 14, 2, 0, 15, &rot);
        SPAWN(e, 0xae), &pos, &dir, 2024, 2048, 12, 2, 0, 15, &rot);
        SPAWN(e, 0xae), &pos, &dir, 4096, 2048, 10, 2, 0, 15, &rot);
        e->unk0 = ST_EFFECT_FREE;
        break;
    }
    case 0xab:
        if (e->unk1C->unk0 != ST_EFFECT_FREE)
            e->unk20 = e->unk1C->unk20;
        else
            e->unk0 = ST_EFFECT_FREE;
        if (e->unk10 < 8)
            e->unk12 = e->unk10 * 512;
        break;
    case 0xaa:
        if (e->unk1C->unk0 != ST_EFFECT_FREE) {
            u8 frames[6] = { 0, 1, 2, 3, 2, 1 };

            e->unk50.b[0]++;
            if (e->unk50.b[0] >= 6)
                e->unk50.b[0] = 0;
            e->unk2 = frames[e->unk50.b[0]] | 0x80;
            e->unk20 = e->unk1C->unk20;
        } else {
            e->unk0 = ST_EFFECT_FREE;
        }
        if (e->unk10 < 8)
            e->unk12 = e->unk10 * 512;
        break;
    case 0xac:
        e->unk40.vy += e->unk52.h;
        e->unk20.vx += e->unk40.vx;
        e->unk20.vz += e->unk40.vz;
        e->unk20.vy += e->unk40.vy;
        if (e->unk5 < 6)
            e->unk38.vx += e->unk50.h;
        else
            e->unk38.vx -= e->unk50.h;
        e->unk38.vz = e->unk38.vx;
        e->unk38.vy = e->unk38.vx;
        e->unk5++;
        if (e->unk10 < 8)
            e->unk12 = e->unk10 * 512;
        break;
    case 0xad:
        e->unk40.vy += e->unk52.h;
        e->unk20.vx += e->unk40.vx;
        e->unk20.vz += e->unk40.vz;
        e->unk38.vx += e->unk50.h;
        e->unk38.vz = e->unk38.vx;
        e->unk20.vy += e->unk40.vy;
        if (e->unk5 < 3)
            e->unk38.vy = e->unk38.vx * 2;
        else
            e->unk38.vy -= e->unk50.h;
        e->unk5++;
        if (e->unk10 < 8)
            e->unk12 = e->unk10 * 512;
        break;
    case 0xae:
        e->unk40.vy += e->unk52.h;
        e->unk20.vx += e->unk40.vx;
        e->unk20.vz += e->unk40.vz;
        e->unk38.vx += e->unk50.h - e->unk5 * 128;
        e->unk38.vz = e->unk38.vx;
        e->unk5++;
        e->unk20.vy += e->unk40.vy;
        if (e->unk10 < 8)
            e->unk12 = e->unk10 * 512;
        break;
    case 0x07: {
        RECT strip = { 768, 352, 16, 64 };
        DVECTOR to;

        to.vx = 784;
        to.vy = 352;
        vram_roll(((e->unk0 & 0x20) >> 5) != 0, &e->unk50.ub[0], &strip, &to);
        e->unk50.ub[0] += 4;
        if (step == 0) {
            SVECTOR dir;
            VECTOR pos;
            SVECTOR rot;
            s16 depth;

            e->unk50.b[1]++;
            if (e->unk50.b[1] < 10)
                e->unk40.vy -= 56;
            r = effect_attack_move(e->unkC, e->unk18, 0);
            if (r != 0) {
                EMIT_LIVE(r);
                SPELL_SOUND(e);
                pos.vx = e->unk20.vx;
                pos.vy = e->unk20.vy;
                pos.vz = e->unk20.vz;
                dir.vx = e->unk40.vx >> 1;
                dir.vy = e->unk40.vy >> 1;
                dir.vz = e->unk40.vz >> 1;
                if (r & 0x11) {
                    e->unk5 = 1;
                    dir.vy = 0;
                    rot.vx = 0;
                    rot.vy = 0;
                    rot.vz = 0;
                    SPAWN(e, 0xb3), &pos, &dir, 512, 256, 8, 4, 15);
                    e->unk52.ub[0] = 0;
                    e->unk52.ub[1] = 0;
                } else {
                    e->unk5 = 2;
                    e->unk40.vx = 0;
                    e->unk40.vy = 8;
                    e->unk40.vz = 0;
                }
            }
            depth = 2048 - vis_lists.floor_dist;
            if (depth > 0) {
                pos.vx = e->unk20.vx;
                pos.vy = e->unk20.vy + vis_lists.floor_dist;
                pos.vz = e->unk20.vz;
                dir.vx = e->unk40.vx >> 1;
                dir.vy = e->unk40.vy >> 1;
                dir.vz = e->unk40.vz >> 1;
                SPAWN(e, 0xb1), &pos, &dir, depth >> 2, depth >> 3, 6, 2, 15);
            }
            break;
        }
        if (step == 1) {
            SVECTOR dir;
            VECTOR pos;
            SVECTOR v;
            MATRIX m;
            SVECTOR rot;

            e->unk52.b[0]++;
            e->unk52.ub[1]++;
            if (e->unk52.b[0] >= 14) {
                e->unk0 = ST_EFFECT_FREE;
            } else if (e->unk52.b[0] < 8) {
                dir.vx = e->unk40.vx >> 1;
                dir.vy = 0;
                dir.vz = e->unk40.vz >> 1;
                rot.vx = -e->unk30.vx;
                rot.vy = -e->unk30.vy;
                rot.vz = -e->unk30.vz;
                RotMatrixZYX(&rot, &m);
                v.vx = rcos(e->unk52.b[0] * 512 + 2048) / 16 + 256;
                v.vy = 0;
                v.vz = rsin(e->unk52.b[0] * 512 + 2048) / 16;
                ApplyMatrixSV(&m, &v, &v);
                pos.vx = e->unk20.vx + v.vx;
                pos.vy = e->unk20.vy;
                pos.vz = e->unk20.vz + v.vz;
                SPAWN(e, 0xb2), &pos, &dir, 256, 128, 6, 3, 15);
                v.vx = rcos(e->unk52.b[0] * 512) / 16 - 256;
                v.vy = 0;
                v.vz = rsin(e->unk52.b[0] * 512) / 16;
                ApplyMatrixSV(&m, &v, &v);
                pos.vx = e->unk20.vx + v.vx;
                pos.vy = e->unk20.vy;
                pos.vz = e->unk20.vz + v.vz;
                SPAWN(e, 0xb2), &pos, &dir, 256, 128, 6, 3, 15);
            }
            break;
        }
        e->unk30.vx += 64;
        if (e->unk5++ >= 14)
            e->unk0 = ST_EFFECT_FREE;
        break;
    }
    case 0xaf:
        if (e->unk1C->unk0 != ST_EFFECT_FREE && e->unk1C->unk5 != 1) {
            if (e->unk1C->unk5 == 0) {
                e->unk20 = e->unk1C->unk20;
            } else {
                e->unk50.b[0]++;
                e->unk20.vy += e->unk50.b[0] * 16;
            }
            e->unk30 = e->unk1C->unk30;
        } else {
            e->unk0 = ST_EFFECT_FREE;
        }
        if (e->unk10 < 8)
            e->unk12 = e->unk10 * 512;
        break;
    case 0xb0:
        if (e->unk1C->unk0 != ST_EFFECT_FREE && e->unk1C->unk5 != 1) {
            if (e->unk1C->unk5 == 0) {
                e->unk20 = e->unk1C->unk20;
            } else {
                e->unk50.b[0]++;
                e->unk20.vy += e->unk50.b[0] * 4;
            }
            e->unk50.b[1]++;
            if (e->unk50.b[1] >= 4)
                e->unk50.b[1] = 0;
            e->unk2 = e->unk50.ub[1] | 0x80;
        } else {
            e->unk0 = ST_EFFECT_FREE;
        }
        if (e->unk10 < 8)
            e->unk12 = e->unk10 * 512;
        break;
    case 0xb1:
        BOLT_MOVE(e);
        if (e->unk10 < 8)
            e->unk12 = e->unk10 * 512;
        break;
    case 0xb2:
        BOLT_MOVE(e);
        if (e->unk10 < 8)
            e->unk12 = e->unk10 * 512;
        break;
    case 0xb3:
        BOLT_MOVE(e);
        if (e->unk10 < 8)
            e->unk12 = e->unk10 * 512;
        break;
    case 0x03:
        if ((e->unk10 >> 3) & 1)
            e->unk38.vx -= 64;
        else
            e->unk38.vx += 64;
    case 0x06:
        r = 0;
        goto bolt;
    case 0x09:
        e->unk30.vy += e->unk52.h;
        e->unk40.vx += 32 - rand() / 512;
        e->unk40.vy += 16 - rand() / 1024;
        e->unk40.vz += 32 - rand() / 512;
        goto bolt;
    case 0x04:
        r = 128;
        i = 1;
        goto bolt;
    case 0x0c:
        r = 384;
        goto bolt;
    case 0x0d:
        r = 384;
        i = 1;
        goto bolt;
    case 0x0f:
        e->unk30.vx = 0;
        e->unk30.vy += 384;
    bolt:
        r = effect_attack_move(e->unkC, e->unk18, r);
        if (r != 0) {
            if (r & 0x10) {
                Creature *t = HIT_ENTRY();

                if (t->hp != 0 && t->status.bytes[3] != 1) {
                    effect_emit_behind(r);
                    e->unk5 = 1;
                    e->unk54 = t;
                }
            } else {
                effect_emit_behind(r);
            }
            if (!i)
                e->unk0 = ST_EFFECT_FREE;
            SPELL_SOUND(e);
            e->unk8 = 7;
        } else {
            e->unk40.vy += e->unk50.b[0];
        }
        FADE_IN(e);
        break;
    case 0x01:
    case 0x10:
        e->unk30.vx = 0;
        e->unk30.vy += 128;
        if (e->unk5 != 0) {
            if (kind == 1)
                e->unk30.vz += 128;
            e->unk38.vx += 128;
            e->unk38.vy += 128;
            e->unk38.vz += 128;
            REACH(e, e->unk38.vx / 16);
            if (e->unk38.vx >= 6144) {
                int hit;

                e->unk0 = ST_EFFECT_FREE;
                hit = effect_attack(e->unkC, e->unk18, 0);
                if (hit & 0x10)
                    effect_emit_behind(hit);
            }
        } else {
            r = effect_attack_move(e->unkC, e->unk18, 0);
            if (r != 0) {
                SPELL_SOUND(e);
                if (r & 0x10) {
                    Creature *t = HIT_ENTRY();

                    if (t->hp != 0 && t->status.bytes[3] != 1) {
                        effect_emit_behind(r);
                        if (e->unk10 >= 32)
                            e->unk10 = 32;
                        e->unk5 = 1;
                        e->unk54 = t;
                        if (kind == 1)
                            break;
                        if (kind == 16)
                            break;
                        e->unk0 = ST_EFFECT_FREE;
                    } else {
                        e->unk0 = ST_EFFECT_FREE;
                    }
                } else {
                    effect_emit_behind(r);
                    e->unk0 = ST_EFFECT_FREE;
                }
            }
        }
        FADE_IN(e);
        break;
    case 0x2b:
        if (e->unk1C->unk0 != ST_EFFECT_FREE) {
            u8 frames[4] = { 0, 1, 2, 1 };

            e->unk50.b[0]++;
            if (e->unk50.b[0] >= 4)
                e->unk50.b[0] = 0;
            e->unk2 = frames[e->unk50.b[0]] | 0x80;
            e->unk20 = e->unk1C->unk20;
        } else {
            e->unk0 = ST_EFFECT_FREE;
        }
        if (e->unk10 < 16)
            e->unk12 = e->unk10 * 256;
        break;
    case 0x2c:
    case 0x2d:
    case 0x2e:
    case 0x2f:
        r = effect_attack_move(e->unkC, e->unk18, 0);
        if (r != 0) {
            EMIT_LIVE(r);
            e->unk0 = ST_EFFECT_FREE;
        } else {
            e->unk40.vy += e->unk50.b[0];
            vector_to_angles_ratan2(e->unk40.vx, e->unk40.vy, e->unk40.vz, &e->unk30);
        }
        break;
    case 0x30:
    case 0x38:
        e->unk38.vx += 96;
        e->unk38.vy += 96;
        REACH(e, e->unk38.vx * 896 / 4096);
        r = effect_attack_move(e->unkC, e->unk18, 32);
        if (r != 0) {
            if (r & 0x80) {
                effect_emit_behind(r);
                e->unk8 = 3;
            } else if (r & 0xf) {
                e->unk0 = ST_EFFECT_FREE;
            }
        }
        break;
    case 0x66:
        e->unk2 = (e->unk10 % 3) | 0x80;
        e->unk38.vx += 96;
        e->unk38.vy += 96;
        REACH(e, e->unk38.vx * 896 / 4096);
        r = effect_attack_move(e->unkC, e->unk18, 32);
        if (r != 0) {
            if (r & 0x80) {
                effect_emit_behind(r);
                e->unk8 = 3;
            } else if (r & 0xf) {
                e->unk0 = ST_EFFECT_FREE;
            }
        }
        break;
    case 0x7d:
    case 0xa1:
        e->unk2 = (e->unk10 % 3) | 0x84;
        e->unk38.vx += 96;
        e->unk38.vy += 96;
        REACH(e, e->unk38.vx * 896 / 4096);
        r = effect_attack_move(e->unkC, e->unk18, 32);
        if (r != 0) {
            if (r & 0x80) {
                effect_emit_behind(r);
                e->unk8 = 3;
            } else if (r & 0xf) {
                e->unk0 = ST_EFFECT_FREE;
            }
        }
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
        r = effect_fire(e->unkC, e->unk18, e->unk50.b[0]);
        if (r != 0 && (r & 0x80)) {
            effect_emit_behind(r);
            e->unk8 = 3;
        }
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
                SPAWN_PAIR(e, 0x35, 0xc9);
            } else if (kind == 0xee) {
                SPAWN_PAIR(e, 0x8d, 0xef);
            } else if (kind == 0xe4) {
                SPAWN_PAIR(e, 0x82, 0xe5);
            } else if (kind == 0xd3) {
                SPAWN_PAIR(e, 0x53, 0xd4);
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
        if (effect_attack_move(e->unkC, e->unk18, 128))
            e->unk0 = ST_EFFECT_FREE;
        FADE_IN(e);
        break;
    case 0x35:
    case 0x53:
    case 0x82:
    case 0x8d:
        e->unk38.vz += 224;
        r = effect_attack_move(e->unkC, e->unk18, 32);
        HIT_ENDS(e, r);
        FADE_IN(e);
        break;
    case 0x3a:
        r = effect_attack_move(e->unkC, e->unk18, 128);
        HIT_ENDS(e, r);
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
        r = effect_attack_move(e->unkC, e->unk18, 128);
        if (r != 0) {
            if (r & 0x80)
                effect_emit_behind(r);
            for (i = 0; i < 4; i++) {
                SVECTOR dir;
                VECTOR pos;

                SPLASH(e, dir, pos);
                if (kind == 0x3e)
                    SPAWN(e, 0x3f), &pos, &dir, 1536, e->unkA - 16);
                else if (kind == 0x6c)
                    SPAWN(e, 0x6d), &pos, &dir, 1536, e->unkA - 16);
                else if (kind == 0x48)
                    SPAWN(e, 0x49), &pos, &dir, 1536, e->unkA - 16);
                else if (kind == 0x93)
                    SPAWN(e, 0x94), &pos, &dir, 1536, e->unkA - 16);
                else if (kind == 0x71)
                    SPAWN(e, 0x72), &pos, &dir, 1536, e->unkA - 16);
                else if (kind == 0x7a)
                    SPAWN(e, 0x7b), &pos, &dir, 1536, e->unkA - 16);
            }
            e->unk0 = ST_EFFECT_FREE;
        }
        if (e->unk10 < 32) {
            e->unk7 = 13;
            e->unk12 = e->unk10 * 128;
        }
        e->unk40.vy += 4;
        break;
    case 0x41:
        r = effect_attack_move(e->unkC, e->unk18, 0);
        if (r != 0) {
            if (r & 0x80)
                effect_emit_behind(r);
            for (r = 0; r < 2; r++) {
                SVECTOR dir;
                VECTOR pos;

                SPLASH(e, dir, pos);
                SPAWN(e, 0x42), &pos, &dir, 1536, e->unkA - 16);
            }
            e->unk0 = ST_EFFECT_FREE;
        }
        if (e->unk10 % 5 == 0)
            for (r = 0; r < 1; r++) {
                SVECTOR dir;
                VECTOR pos;

                SPLASH(e, dir, pos);
                SPAWN(e, 0x42), &pos, &dir, 1536, e->unkA - 16);
            }
        if (e->unk10 < 32) {
            e->unk7 = 13;
            e->unk12 = e->unk10 * 128;
        }
        e->unk40.vy += 16;
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
            Creature *owner = ST_EFFECT_ENTRY(e);
            SVECTOR v;
            MATRIX m;
            SVECTOR rot;
            VECTOR vec;
            SVECTOR spare;

            if (e->unk50.h != -1) {
                if (kind == 0xca || kind == 0xe1 || kind == 0xd0 || kind == 0xe6 || kind == 0xdb
                    || kind == 0xde) {
                    anim_get_vertex(owner->model_slot + 128, owner->anim, (s16)owner->anim_frame, e->unk50.h,
                                    &v);
                } else if (kind == 0xcd || kind == 0xeb || kind == 0xd8) {
                    anim_set_node_vertex((StAnimSet *)owner->anim_slot, owner->model_slot + 128,
                                         e->unk52.h, e->unk50.h, &vec);
                    v.vx = vec.vx;
                    v.vy = vec.vy;
                    v.vz = vec.vz;
                }
                rot.vx = 0;
                rot.vy = -owner->facing;
                rot.vz = 0;
                RotMatrix(&rot, &m);
                ApplyMatrixSV(&m, &v, &v);
                e->unk20.vx = owner->position.vx + v.vx;
                e->unk20.vy = owner->position.vy + v.vy;
                e->unk20.vz = owner->position.vz + v.vz;
            } else {
                e->unk20.vx = owner->position.vx;
                e->unk20.vy = owner->position.vy;
                e->unk20.vz = owner->position.vz;
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
    case 0x86: {
        SVECTOR dir;
        VECTOR pos;
        Creature *entry;
        int len;
        int model;

        SCALE(e, rcos(e->unk10 << 11) / 4);
        entry = ST_EFFECT_ENTRY(e);
        e->unk50.h += 32;
        if (e->unk50.h >= -128)
            e->unk50.h = -128;
        len = (entry->position.vy - e->unk1C->unk20.vy) * rcos(e->unk50.h) / rsin(e->unk50.h);
        e->unk20.vx = e->unk1C->unk20.vx + len * rcos(e->unk1C->unk30.vy + 1024) / 4096;
        e->unk20.vz = e->unk1C->unk20.vz + len * rsin(e->unk1C->unk30.vy + 1024) / 4096;
        FADE_IN(e);
        r = effect_attack(1024, 0x80000400, 0);
        if (r != 0 && (r & 0x80))
            effect_emit_behind(r);
        model = e->unkA - 16;
        pos.vx = e->unk20.vx;
        pos.vy = e->unk20.vy;
        pos.vz = e->unk20.vz;
        dir.vx = 0;
        dir.vy = 0;
        dir.vz = 0;
        if (kind == 0x3d)
            SPAWN(e, 0xcc), &pos, &dir, 512, 256, 6, model, 3);
        else if (kind == 0x8a)
            SPAWN(e, 0xed), &pos, &dir, 512, 256, 6, model, 3);
        else if (kind == 0x46)
            SPAWN(e, 0xcf), &pos, &dir, 512, 256, 6, model, 3);
        else if (kind == 0x81)
            SPAWN(e, 0xe3), &pos, &dir, 512, 256, 6, model, 3);
        else if (kind == 0x52)
            SPAWN(e, 0xd2), &pos, &dir, 512, 256, 6, model, 3);
        else if (kind == 0x86)
            SPAWN(e, 0xe8), &pos, &dir, 512, 256, 6, model, 3);
        else if (kind == 0x5a)
            SPAWN(e, 0xda), &pos, &dir, 512, 256, 6, model, 3);
        else if (kind == 0x64)
            SPAWN(e, 0xdd), &pos, &dir, 512, 256, 6, model, 3);
        else if (kind == 0x7e)
            SPAWN(e, 0xe0), &pos, &dir, 512, 256, 6, model, 3);
        break;
    }
    case 0x7e:
    case 0x81:
    case 0x8a: {
        SVECTOR dir;
        VECTOR pos;
        int cosine;
        int sine;
        int top;
        int len;
        int model;

        SCALE(e, rcos(e->unk10 << 11) / 4);
        e->unk50.h += 32;
        if (e->unk50.h >= -128)
            e->unk50.h = -128;
        cosine = rcos(e->unk50.h);
        sine = rsin(e->unk50.h);
        top = e->unk1C->unk20.vy - 1600;
        len = (viewer.position.vy - top) * cosine / sine;
        e->unk20.vx = e->unk1C->unk20.vx + len * rcos(e->unk1C->unk30.vy + 1024) / 4096;
        e->unk20.vz = e->unk1C->unk20.vz + len * rsin(e->unk1C->unk30.vy + 1024) / 4096;
        FADE_IN(e);
        r = effect_attack(1024, 0x80000400, 0);
        if (r != 0 && (r & 0x80))
            effect_emit_behind(r);
        model = e->unkA - 16;
        pos.vx = e->unk20.vx;
        pos.vy = e->unk20.vy;
        pos.vz = e->unk20.vz;
        dir.vx = 0;
        dir.vy = 0;
        dir.vz = 0;
        if (kind == 0x3d)
            SPAWN(e, 0xcc), &pos, &dir, 512, 256, 6, model, 3);
        else if (kind == 0x8a)
            SPAWN(e, 0xed), &pos, &dir, 512, 256, 6, model, 3);
        else if (kind == 0x46)
            SPAWN(e, 0xcf), &pos, &dir, 512, 256, 6, model, 3);
        else if (kind == 0x81)
            SPAWN(e, 0xe3), &pos, &dir, 512, 256, 6, model, 3);
        else if (kind == 0x52)
            SPAWN(e, 0xd2), &pos, &dir, 512, 256, 6, model, 3);
        else if (kind == 0x86)
            SPAWN(e, 0xe8), &pos, &dir, 512, 256, 6, model, 3);
        else if (kind == 0x5a)
            SPAWN(e, 0xda), &pos, &dir, 512, 256, 6, model, 3);
        else if (kind == 0x64)
            SPAWN(e, 0xdd), &pos, &dir, 512, 256, 6, model, 3);
        else if (kind == 0x7e)
            SPAWN(e, 0xe0), &pos, &dir, 512, 256, 6, model, 3);
        break;
    }
    case 0xcb:
    case 0xce:
    case 0xd1:
    case 0xd9:
    case 0xdc:
    case 0xe7: {
        Creature *entry;
        int cosine;
        int sine;
        int dy;
        int len;

        e->unk20.vx = e->unk1C->unk20.vx;
        e->unk20.vy = e->unk1C->unk20.vy;
        e->unk20.vz = e->unk1C->unk20.vz;
        entry = ST_EFFECT_ENTRY(e);
        e->unk50.h += 32;
        if (e->unk50.h >= -128)
            e->unk50.h = -128;
        cosine = rcos(e->unk50.h);
        sine = rsin(e->unk50.h);
        dy = entry->position.vy - e->unk1C->unk20.vy;
        len = dy * cosine / sine;
        e->unk38.vz = SquareRoot0(len * len + dy * dy);
        e->unk30.vx = -e->unk50.h;
        if (e->unk10 < 8)
            e->unk12 = e->unk10 * 512;
        break;
    }
    case 0xdf:
    case 0xe2:
    case 0xec: {
        int cosine;
        int sine;
        int top;
        int dy;
        int len;

        e->unk20.vx = e->unk1C->unk20.vx;
        e->unk20.vy = e->unk1C->unk20.vy;
        e->unk20.vz = e->unk1C->unk20.vz;
        e->unk50.h += 32;
        if (e->unk50.h >= -128)
            e->unk50.h = -128;
        cosine = rcos(e->unk50.h);
        sine = rsin(e->unk50.h);
        top = e->unk1C->unk20.vy - 1600;
        dy = viewer.position.vy - top;
        len = dy * cosine / sine;
        e->unk38.vz = SquareRoot0(len * len + dy * dy);
        e->unk30.vx = -e->unk50.h;
        if (e->unk10 < 8)
            e->unk12 = e->unk10 * 512;
        break;
    }
    case 0xcc:
    case 0xcf:
    case 0xd2:
    case 0xda:
    case 0xdd:
    case 0xe0:
    case 0xe3:
    case 0xe8:
    case 0xed:
        BOLT_MOVE(e);
        break;
    case 0x47:
    case 0x9b: {
        s16 shake;

        e->unk38.vx += 128;
        e->unk38.vy += 128;
        REACH(e, e->unk38.vx * 896 / 4096);
        effect_fire(e->unkC, e->unk18, e->unk50.b[0]);
        FADE_IN(e);
        e->unk40.vy += 2;
        if (e->unk8 == 0 && e->unk10 >= 48) {
            effect_emit_behind(128);
            e->unk8 = 4;
        } else {
            e->unk8--;
        }
        shake = (rand() - 16384) >> ((64 - e->unk10) / 8 + 9);
        if (shake < -2 || shake > 2)
            viewer.shake_roll += shake;
        if (viewer.shake_roll > 2048 && viewer.shake_roll < 4032)
            viewer.shake_roll = 4032;
        if (viewer.shake_roll > 64 && viewer.shake_roll < 2048)
            viewer.shake_roll = 64;
        break;
    }
    case 0xf0:
        n = 2;
        goto drain;
    case 0xf1:
        n = 4;
        goto drain;
    case 0x9a:
    case 0xf2:
        n = 8;
    drain:
        if (e->unk10 < 16) {
            Creature *owner = ST_EFFECT_ENTRY(e);

            owner->fade = e->unk10 * 256;
            owner->status.bytes[2] |= 0xd;
        } else if (e->unk10 <= 0) {
            Creature *owner = ST_EFFECT_ENTRY(e);

            owner->fade = 0;
            if (owner->status.bytes[2] & 0x10)
                owner->status.bytes[2] &= 0xf7;
            else
                owner->status.bytes[2] &= 0xf2;
        }
        if (e->unk8 != 0) {
            e->unk8--;
        } else {
            s16 left = viewer.mp;

            left -= n;
            if (left < 0)
                left = 0;
            viewer.mp = left;
        }
        break;
    case 0xf3:
        n = 2;
        goto wound;
    case 0xf4:
        n = 4;
        goto wound;
    case 0xf5:
        n = 8;
    wound:
        if (e->unk10 < 16) {
            Creature *owner = ST_EFFECT_ENTRY(e);

            owner->fade = e->unk10 * 256;
            owner->status.bytes[2] |= 0xd;
        }
        if (e->unk10 <= 0) {
            Creature *owner = ST_EFFECT_ENTRY(e);

            owner->fade = 0;
            if (owner->status.bytes[2] & 0x10)
                owner->status.bytes[2] &= 0xf7;
            else
                owner->status.bytes[2] &= 0xf2;
        }
        if (e->unk8 != 0) {
            e->unk8--;
        } else {
            s16 left = viewer.hp;

            left -= n;
            if (left <= 0) {
                left = 0;
                player_kill(0);
            }
            viewer.hp = left;
        }
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
        r = effect_attack_move(e->unkC, e->unk18, 256);
        HIT_ENDS(e, r);
        if (e->unk10 % 5 == 0)
            for (r = 0; r < 1; r++) {
                SVECTOR dir;
                VECTOR pos;

                DEBRIS(e, dir, pos, rand() / 1024);
                SPAWN(e, 0xd7), &pos, &dir, 512, e->unkA - 16);
            }
        FADE_IN(e);
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
        effect_fire(e->unkC, e->unk18, e->unk50.b[0]);
        e->unk12 = e->unk10 * 256;
        e->unk40.vy += 2;
        break;
    case 0x56:
    case 0x5f:
    case 0x70:
        e->unk38.vx += 96;
        e->unk38.vy += 96;
        REACH(e, e->unk38.vx * 896 / 4096);
        r = effect_attack_move(e->unkC, e->unk18, e->unk50.b[0]);
        if (r != 0) {
            if (r & 0x80) {
                effect_emit_behind(r);
                e->unk8 = 3;
            }
            e->unk0 = ST_EFFECT_FREE;
        }
        if (e->unk10 % 5 == 0)
            for (r = 0; r < 1; r++) {
                SVECTOR dir;
                VECTOR pos;

                DEBRIS(e, dir, pos, (s16)(rand() / 1024 + 48));
                SPAWN(e, 0xd5), &pos, &dir, 4096, e->unkA - 16);
            }
        FADE_IN(e);
        e->unk40.vy += 2;
        break;
    case 0xd5:
        e->unk38.vx += 96;
        e->unk38.vy += 96;
        effect_fire(e->unkC, e->unk18, e->unk50.b[0]);
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
        r = effect_fire(e->unkC, e->unk18, 0);
        if (r != 0) {
            if (r & 0x80) {
                effect_emit_behind(r);
                e->unk8 = 3;
            }
            e->unk0 = ST_EFFECT_FREE;
        }
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
        r = effect_attack_move(e->unkC, e->unk18, -256);
        HIT_ENDS(e, r);
        FADE_IN(e);
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
        r = effect_attack_move(e->unkC, e->unk18, -256);
        HIT_ENDS(e, r);
        FADE_IN(e);
        break;
    case 0xe9: {
        int len;

        if (e->unk1C->unk0 == ST_EFFECT_FREE) {
            e->unk0 = ST_EFFECT_FREE;
            break;
        }
        len = e->unk1C->unk38.vx * rcos(e->unk1C->unk30.vz) / 4096;
        x = len * rcos(e->unk1C->unk30.vy) / 4096;
        z = len * rsin(e->unk1C->unk30.vy) / 4096;
        y = rsin(e->unk1C->unk30.vz);
        e->unk20.vx = e->unk1C->unk20.vx + x;
        e->unk20.vy = e->unk1C->unk20.vy + e->unk1C->unk38.vx * rsin(e->unk1C->unk30.vz) / 4096;
        e->unk20.vz = e->unk1C->unk20.vz + z;
        FADE_IN(e);
        break;
    }
    case 0xea: {
        int len;

        if (e->unk1C->unk0 == ST_EFFECT_FREE) {
            e->unk0 = ST_EFFECT_FREE;
            break;
        }
        len = e->unk1C->unk38.vx * rcos(e->unk1C->unk30.vz + 2048) / 4096;
        x = len * rcos(e->unk1C->unk30.vy) / 4096;
        z = len * rsin(e->unk1C->unk30.vy) / 4096;
        y = rsin(e->unk1C->unk30.vz + 2048);
        e->unk20.vx = e->unk1C->unk20.vx + x;
        e->unk20.vy = e->unk1C->unk20.vy
                      + e->unk1C->unk38.vx * rsin(e->unk1C->unk30.vz + 2048) / 4096;
        e->unk20.vz = e->unk1C->unk20.vz + z;
        FADE_IN(e);
        break;
    }
    case 0x40:
    case 0x5e:
    case 0x69:
    case 0x76:
    case 0x88:
    case 0x89:
    case 0x8b:
    case 0xa2:
    case 0xa4:
    case 0xa8: {
        int height;
        SVECTOR spare_dir;
        VECTOR spare_pos;

        if (kind != 0x5e)
            e->unk30.vy += rsin(64 - e->unk10);
        e->unk38.vx += 128;
        e->unk38.vy += 128;
        e->unk38.vz += 128;
        e->unkC = e->unk38.vx / 2;
        height = e->unk38.vy;
        height /= 2;
        e->unk18 = height;
        r = effect_attack(e->unkC, height | 0x80000000, height / 2);
        if (r != 0 && (r & 0x80)) {
            effect_emit_behind(r);
            e->unk8 = 3;
        }
        break;
    }
    default:
        break;
    }
}
