/*
 * ST.EXE: a table A entry's attack: the effects each attack kind spawns
 * toward the viewer, or the direct hit (WIP module).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <psyq/libc.h>
#include <st/data.h>
#include <st/effect.h>
#include <st/funcs.h>
#include <st/player.h>
#include <st/vis.h>

JUMPTABLES(0x800122b4, 0x31c)

/* Entry e attacks with effect kind `kind` from pos. */
ADDRESS(0x8004cee8, 0x874)
void func_8004cee8(Vis212 *e, int kind, VECTOR *pos)
{
    VECTOR p;
    SVECTOR dir;
    SVECTOR rot;
    u16 dmg[9];
    long from[4];
    Vis192 *r;
    StEffect *fx;
    int i;
    int speed;
    int scale;
    u16 effect;

    r = &vis_table_a.unk3500[e->unk02];
    memset(dmg, 0, 18);
    memset(from, 0, 16);
    p.vx = pos->vx;
    p.vy = pos->vy;
    p.vz = pos->vz;
    switch (kind) {
    case 0x30:
    case 0x38:
    case 0x66:
    case 0x7d:
    case 0xa1:
        for (i = 0, speed = 256; i < 3; i++, speed += 64) {
            func_8004cc70(e, (VECTOR *)&viewer.x, speed, &p, &dir, &rot, -1, 16, 1);
            rot.vx = 0;
            rot.vz = 0;
            fx = func_80036484(r->unk08, 0x81, kind, &p, &dir, 256, &rot, e->unk0B);
            if (fx)
                fx->unk8 = 3;
        }
        break;
    case 0x31:
    case 0x32:
    case 0x6a:
        for (i = 0; i < 4; i++) {
            func_8004ca90(&dir, &p, pos, (int)e, -e->unk30, -48);
            fx = func_80036484(r->unk08, 0x81, kind, &p, &dir, 0x600, e->unk0B);
            if (fx)
                fx->unk8 = 3;
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
        for (i = 0; i < 4; i++) {
            func_8004cc70(e, (VECTOR *)&viewer.x, 32, &p, &dir, &rot, -1, 16, 1);
            func_8004cba0(&dir, &p, pos);
            fx = func_80036484(r->unk08, 0x81, kind, &p, &dir, 0x400, e->unk0B);
            if (fx)
                fx->unk8 = 3;
        }
        break;
    case 0x3a:
        for (i = 0, speed = 512; i < 3; i++, speed += 128) {
            func_8004cc70(e, (VECTOR *)&viewer.x, speed, &p, &dir, &rot, -1, 16, 1);
            func_8004cba0(&dir, &p, pos);
            fx = func_80036484(r->unk08, 0x81, kind, &p, &dir, 0x1000, &rot, e->unk0B);
            if (fx)
                fx->unk8 = 0;
        }
        break;
    case 0x3b:
    case 0x50:
    case 0x54:
        effect = 24;
        goto hit;
    case 0x3e:
    case 0x48:
    case 0x5e:
    case 0x6c:
    case 0x71:
    case 0x7a:
    case 0x89:
    case 0x93:
        for (i = 0; i < 1; i++) {
            func_8004cc70(e, (VECTOR *)&viewer.x, 256, &p, &dir, &rot, -1, 16, 1);
            func_8004cba0(&dir, &p, pos);
            fx = func_80036484(r->unk08, 0x81, kind, &p, &dir, 0x200, &rot, e->unk0B);
            if (fx)
                fx->unk8 = 3;
        }
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
        func_8004cc70(e, (VECTOR *)&viewer.x, 384, &p, &dir, &rot, -1, 16, 1);
        scale = 0x1000;
        goto aimed;
    case 0x41:
        func_8004ca90(&dir, &p, pos, (int)e, 0, 48);
        rot.vx = 0;
        rot.vy = 0;
        rot.vz = 0;
        fx = func_80036484(r->unk08, 0x81, kind, &p, &dir, 0x600, &rot, e->unk0B);
        goto slow;
    case 0x44:
    case 0x4e:
        effect = 128;
        goto hit;
    case 0x47:
    case 0x6b:
    case 0x9b:
    case 0xa3:
        for (i = 0; i < 4; i++) {
            func_8004ca90(&dir, &p, pos, (int)e, 0, -48);
            fx = func_80036484(r->unk08, 0x81, kind, &p, &dir, 0x600, e->unk0B);
            if (fx)
                fx->unk8 = 3;
        }
        break;
    case 0x4c:
        effect = 256;
        goto hit;
    case 0x55:
    case 0x59:
    case 0x97:
        func_8004cc70(e, (VECTOR *)&viewer.x, 256, &p, &dir, &rot, -1, 16, 1);
        fx = func_80036484(r->unk08, 0x81, kind, &p, &dir, 0x200, e->unk0B);
        if (fx)
            fx->unk40.vy = 0;
        break;
    case 0x56:
    case 0x5f:
    case 0x70:
        for (i = 0; i < 4; i++) {
            func_8004cc70(e, (VECTOR *)&viewer.x, 512, &p, &dir, &rot, -1, 16, 1);
            func_8004cba0(&dir, &p, pos);
            fx = func_80036484(r->unk08, 0x81, kind, &p, &dir, 0x400, e->unk0B);
            if (fx)
                fx->unk8 = 0;
        }
        break;
    case 0x57:
    case 0x61:
    case 0x68:
    case 0x74:
        func_8004cc70(e, (VECTOR *)&viewer.x, 384, &p, &dir, &rot, -1, 16, 1);
        fx = func_80036484(r->unk08, 0x81, kind, &p, &dir, 0x400, e->unk0B, 16);
        goto fast;
    case 0x58:
    case 0x5c:
    case 0x62:
    case 0x63:
    case 0x67:
    case 0x8c:
    case 0x95:
    case 0x9c:
    case 0x9e:
        func_8004cc70(e, (VECTOR *)&viewer.x, 512, &p, &dir, &rot, -1, 16, 1);
        fx = func_80036484(r->unk08, 0x81, kind, &p, &dir, 0x200, e->unk0B, 16);
        goto fast;
    case 0x7c:
    case 0x83:
    case 0x84:
    case 0x8e:
    case 0x91:
    case 0x96:
    case 0xa0:
    case 0xa7:
    case 0xa9:
        func_8004cc70(e, (VECTOR *)&viewer.x, 384, &p, &dir, &rot, -1, 16, 1);
        scale = 0x200;
    aimed:
        fx = func_80036484(r->unk08, 0x81, kind, &p, &dir, scale, &rot, e->unk0B);
    fast:
        if (fx)
            fx->unk8 = 0;
        break;
    case 0x9a:
    case 0xf0:
    case 0xf1:
    case 0xf2:
    case 0xf3:
    case 0xf4:
    case 0xf5:
        func_80036484(r->unk08, 0x81, kind, &p, &dir, e);
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
        func_8004cc70(e, (VECTOR *)&viewer.x, 768, &p, &dir, &rot, -1, 16, 1);
        fx = func_80036484(r->unk08, 0x81, kind, &p, &dir, 0x800, &rot, e->unk0B, e);
    slow:
        if (fx)
            fx->unk8 = 3;
        break;
    case 0xf6:
        effect = 96;
    hit:
        player_hit(dmg, 0x1000, effect, from);
        break;
    }
}
