/*
 * VS.EXE: a fighter's attack: the effects each attack kind spawns from
 * the fighter's facing, or the direct hit (WIP module). VS's form of
 * ST.EXE's func_8004cee8 (src/st/vis_attack.c).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/effect.h>
#include <vs/data.h>
#include <vs/buffer.h>
#include <vs/effect.h>

JUMPTABLES(0x80011588, 0x31c)

void func_8002f174(SVECTOR *offset, VECTOR *out, VECTOR *pos, short dy, short height);
void func_8002f264(SVECTOR *off, VECTOR *out, VECTOR *base);
/* src/vs/fighter_8002e114.c's hit, called without a prototype (retail
 * passes the fighter number unconverted). */
void func_8002e314();

/* The attack's origin: its offset turned by the fighter's facing. */
#define ORIGIN(o)                                                              \
    s = rsin(f->unk4DF02 - 0x800);                                             \
    c = rcos(f->unk4DF02 - 0x800);                                             \
    (o).vx = f->unk4DEF0.vx + (a->unk2 * c + a->unk6 * s) / 4096;              \
    (o).vy = f->unk4DEF0.vy + a->unk4;                                         \
    (o).vz = f->unk4DEF0.vz + (a->unk6 * c - a->unk2 * s) / 4096

/* The attack's index among the fighter's attacks. */
#define INDEX() ((u8)(a - data->unk14A))

/* Fighter n attacks with attack record a. */
ADDRESS(0x8002f334, 0x12ec)
void func_8002f334(int n, VsAttack *a)
{
    VECTOR p;
    SVECTOR dir;
    VECTOR q;
    SVECTOR step;
    VsFighter *f;
    VsFighterData *data;
    int s;
    int c;
    int i;

    f = &DAT_800633b0.fighter[n];
    data = &f->unk4DF18;
    switch ((s16)a->unk0) {
    case 0x30:
    case 0x38:
    case 0x66:
    case 0x7d:
    case 0xa1:
        ORIGIN(p);
        step.vx = rsin(f->unk4DF02 - 0x800) / 16;
        step.vz = rcos(f->unk4DF02 - 0x800) / 16;
        for (i = 0; i < 3; i++) {
            dir.vx = step.vx * (256 + i * 64) / 256;
            dir.vy = 0;
            dir.vz = step.vz * (256 + i * 64) / 256;
            func_80032404(n, INDEX(), 0x81, a->unk0, &p, &dir, 256, (SVECTOR *)&f->unk4DF00, i == 2);
        }
        break;
    case 0x3a:
        ORIGIN(q);
        step.vx = rsin(f->unk4DF02 - 0x800) / 16;
        step.vz = rcos(f->unk4DF02 - 0x800) / 16;
        for (i = 0; i < 3; i++) {
            dir.vx = step.vx * (512 + i * 128) / 512;
            dir.vy = 0;
            dir.vz = step.vz * (512 + i * 128) / 512;
            func_8002f264(&dir, &p, &q);
            func_80032404(n, INDEX(), 0x81, a->unk0, &p, &dir, 0x1000, (SVECTOR *)&f->unk4DF00);
        }
        break;
    case 0x3e:
    case 0x48:
    case 0x5e:
    case 0x6c:
    case 0x71:
    case 0x7a:
    case 0x89:
    case 0x93:
        ORIGIN(q);
        dir.vx = rsin(f->unk4DF02 - 0x800) / 16;
        dir.vy = 0;
        dir.vz = rcos(f->unk4DF02 - 0x800) / 16;
        func_8002f264(&dir, &p, &q);
        func_80032404(n, INDEX(), 0x81, a->unk0, &p, &dir, 0x200, (SVECTOR *)&f->unk4DF00);
        break;
    case 0x57:
    case 0x61:
    case 0x68:
    case 0x74:
        ORIGIN(p);
        dir.vx = rsin(f->unk4DF02 - 0x800) * 0x180 / 4096;
        if ((s16)a->unk0 == 0x74)
            dir.vy = 200;
        else
            dir.vy = 0;
        dir.vz = rcos(f->unk4DF02 - 0x800) * 0x180 / 4096;
        func_80032404(n, INDEX(), 0x81, a->unk0, &p, &dir, 0x400);
        break;
    case 0x40:
    case 0x5d:
    case 0x65:
    case 0x69:
    case 0x75:
    case 0x76:
    case 0x77:
    case 0x78:
    case 0x79:
    case 0x80:
    case 0x88:
    case 0x8b:
    case 0xa2:
    case 0xa4:
    case 0xa6:
    case 0xa8:
        ORIGIN(p);
        dir.vx = rsin(f->unk4DF02 - 0x800) * 0x180 / 4096;
        if ((s16)a->unk0 == 0xa6)
            dir.vy = 200;
        else
            dir.vy = 0;
        dir.vz = rcos(f->unk4DF02 - 0x800) * 0x180 / 4096;
        func_80032404(n, INDEX(), 0x81, a->unk0, &p, &dir, 0x1000, (SVECTOR *)&f->unk4DF00);
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
        ORIGIN(p);
        dir.vx = rsin(f->unk4DF02 - 0x800) * 0x180 / 4096;
        dir.vy = 0;
        dir.vz = rcos(f->unk4DF02 - 0x800) * 0x180 / 4096;
        func_80032404(n, INDEX(), 0x81, a->unk0, &p, &dir, 0x200, 1);
        break;
    case 0x58:
    case 0x5c:
    case 0x62:
    case 0x63:
    case 0x67:
    case 0x8c:
    case 0x95:
    case 0x9c:
    case 0x9e:
        ORIGIN(p);
        dir.vx = rsin(f->unk4DF02 - 0x800) / 8;
        dir.vy = 0;
        dir.vz = rcos(f->unk4DF02 - 0x800) / 8;
        func_80032404(n, INDEX(), 0x81, a->unk0, &p, &dir, 0x200);
        break;
    case 0x56:
    case 0x5f:
    case 0x70:
        ORIGIN(q);
        dir.vx = rsin(f->unk4DF02 - 0x800) / 8;
        dir.vy = 0;
        dir.vz = rcos(f->unk4DF02 - 0x800) / 8;
        for (i = 0; i < 4; i++) {
            func_8002f264(&dir, &p, &q);
            func_80032404(n, INDEX(), 0x81, a->unk0, &q, &dir, 0x400, 1);
        }
        break;
    case 0x55:
    case 0x59:
    case 0x97:
        ORIGIN(p);
        dir.vx = rsin(f->unk4DF02 - 0x800) / 16;
        dir.vy = 0;
        dir.vz = rcos(f->unk4DF02 - 0x800) / 16;
        func_80032404(n, INDEX(), 0x81, a->unk0, &p, &dir, 0x200);
        break;
    case 0x31:
    case 0x32:
    case 0x6a:
        ORIGIN(q);
        for (i = 0; i < 4; i++) {
            func_8002f174(&dir, &p, &q, -data->unk1B6, -48);
            func_80032404(n, INDEX(), 0x81, a->unk0, &p, &dir, 0x600, i & 1);
        }
        break;
    case 0x47:
    case 0x6b:
    case 0x9b:
    case 0xa3:
        ORIGIN(q);
        for (i = 0; i < 4; i++) {
            func_8002f174(&dir, &p, &q, 0, -48);
            func_80032404(n, INDEX(), 0x81, a->unk0, &p, &dir, 0x600, i & 1);
        }
        break;
    case 0x33:
    case 0x34:
    case 0x36:
    case 0x37:
    case 0x39:
    case 0x3c:
    case 0x43:
    case 0x45:
    case 0x4a:
    case 0x4b:
    case 0x4d:
    case 0x4f:
    case 0x51:
    case 0x60:
    case 0x6e:
    case 0x6f:
    case 0x73:
    case 0x7f:
    case 0x85:
    case 0x87:
    case 0x8f:
    case 0x90:
    case 0x92:
    case 0x98:
    case 0x99:
    case 0x9d:
    case 0x9f:
    case 0xa5:
        ORIGIN(q);
        dir.vx = rsin(f->unk4DF02 - 0x800) / 64;
        dir.vy = 0;
        dir.vz = rcos(f->unk4DF02 - 0x800) / 64;
        for (i = 0; i < 4; i++) {
            func_8002f264(&dir, &p, &q);
            func_80032404(n, INDEX(), 0x81, a->unk0, &p, &dir, 0x400, i & 1);
        }
        break;
    case 0xc8:
    case 0xca:
    case 0xcd:
    case 0xd0:
    case 0xd3:
    case 0xd8:
    case 0xdb:
    case 0xde:
    case 0xe1:
    case 0xe4:
    case 0xe6:
    case 0xeb:
    case 0xee:
        ORIGIN(p);
        dir.vx = rsin(f->unk4DF02 - 0x800) * 0x300 / 4096;
        dir.vy = 0;
        dir.vz = rcos(f->unk4DF02 - 0x800) * 0x300 / 4096;
        func_80032404(n, INDEX(), 0x81, a->unk0, &p, &dir, 0x800, (SVECTOR *)&f->unk4DF00);
        break;
    case 0x3b:
    case 0x44:
    case 0x4c:
    case 0x4e:
    case 0x50:
    case 0x54:
    case 0x5b:
    case 0x9a:
    case 0xf0:
    case 0xf1:
    case 0xf2:
    case 0xf3:
    case 0xf4:
    case 0xf5:
    case 0xf6:
        func_8002e314(n, INDEX());
        break;
    }
}
