/*
 * ST.EXE: table A entries (vis_table_a.entries) taking damage, the per-frame
 * pass over them, and loading them from an area file (WIP module; the damage
 * helpers 0x8004da2c..0x8004de24 lie just before it and may share its file).
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <psyq/libc.h>
#include <st/data.h>
#include <st/memory.h>
#include <st/menu.h>
#include <st/anim.h>
#include <st/player.h>
#include <st/funcs.h>
#include <st/vis.h>

/* Takes `power` damage (times three for a critical hit), doubled by luck or
 * kind 2; on death starts the record's 'P' script, on a heavy hit its '@'
 * script. Returns the damage dealt, or -1 for an entry in state 2. */
ADDRESS(0x8004def8, 0x3e0)
int func_8004def8(e, kind, power, levels, crit)
    Vis212 *e;
    int kind;
    short power;
    u16 *levels;
    u8 crit;
{
    Vis192 *t;
    VisScript **p;
    VisScript *script = NULL;
    int dmg;
    int found;
    u8 index;
    int luck;
    int i;

    e->unk64 = 1;
    if (e->unk1C.bytes[3] == 2)
        return -1;
    dmg = func_8004de24(e, 0, levels);
    if (crit)
        dmg *= 3;
    dmg = dmg * power / 5120;
    if (dmg == 0)
        return 0;
    t = &vis_table_a.unk3500[e->unk02];
    if (viewer.unk2C & 0x200000)
        dmg = dmg * 3 / 2;
    luck = viewer.unk1C6;
    if (rand() * 100 / 32768 < luck) {
        func_800582f0(21, 128);
        dmg <<= 1;
    }
    i = 24;
    if (kind == 2)
        dmg <<= 1;
    found = 0;
    e->unk1C.bytes[2] &= 0xca;
    index = 0;
    if (dmg >= e->unk2C) {
        if (e->unk1C.bytes[3] != 3) {
            e->unk05 = 5;
            func_8004da2c(e);
            func_8004db0c(e);
            DAT_801cba08.unk14A8[5 + (t->unk10 >> 5)] |= 1 << (t->unk10 & 0x1f);
            if (e < &vis_table_a.entries[48]) {
                DAT_801cba08.unk16E4[area_state.area]++;
                viewer.unk30++;
            }
            e->unk2C = 0;
            e->unk1C.bytes[2] |= 0x15;
            p = t->unk60;
            while (i--) {
                script = *p++;
                if (script == NULL)
                    break;
                if (script->unk00 == 'P') {
                    found = 1;
                    break;
                }
                index++;
            }
            goto done;
        }
    } else if (e->unk1C.bytes[3] != 3) {
        e->unk2C -= dmg;
    }
    e->unk1C.bytes[2] |= 0x20;
    if (dmg >= e->unk2C * 15 / 100) {
        p = t->unk60;
        while (i--) {
            script = *p++;
            if (script == NULL)
                break;
            if (script->unk00 == '@') {
                found = 1;
                break;
            }
            index++;
        }
    }
done:
    if (found) {
        e->unk18 = index;
        e->unkA8 = script;
        e->unk0F = script->unk00;
        e->unk1A = 0;
        e->unk2A = 0;
        e->unkB0 = script->unk0C;
    }
    e->unk34 = 0xfff;
    if (viewer.unk218 != 4) {
        if (viewer.unk1C9)
            player_add_hp(dmg * viewer.unk1C9 / 100);
        if (viewer.unk1CA)
            player_add_mp(dmg * viewer.unk1CA / 100);
    }
    return (short)dmg;
}

/* Gives every entry of the first 32 left in the previous area (0xfd) that
 * area's id. */
ADDRESS(0x8004e2d8, 0x4c)
void func_8004e2d8(void)
{
    Vis212 *e = vis_table_a.entries;
    int i = 32;

    while (i--) {
        if (e->unk07 == 0xfd)
            e->unk07 = vis_table_a.unk6103;
        e++;
    }
}

/* The per-frame table A pass: on an area change frees the animation files
 * and parks the entries of other areas, then measures each entry against
 * the viewer, runs it and moves it. */
ADDRESS(0x8004e324, 0x5c8)
void func_8004e324(void)
{
    VisRecord *r = viewer.unk174;
    Vis212 *e;
    Vis192 *t;
    StAnimFile *file;
    StAnimFile *anim;
    int n;
    int dx;
    int dy;
    int dz;
    int d;
    u16 h;

    if (r != NULL && r->unk07 != 0xff) {
        u8 prev = vis_table_a.unk6168;

        vis_table_a.unk6103 = prev;
        vis_table_a.unk6168 = r->unk07;
        if (prev != vis_table_a.unk6168) {
            anim_slots_free_idle();
            anim_sets_free_idle();
            anim_slots_free_all();
            anim_sets_free_all();
            n = 16;
            while (n--) {
                anim_get_file(n + 128, &file);
                if (file)
                    func_80015230(file);
            }
            func_8004e2d8();
            n = 16;
            while (n--) {
                anim_get_file(n + 112, &file);
                if (file)
                    func_80015230(file);
            }
            e = vis_table_a.entries;
            for (vis_table_a.unk6144 = 0; vis_table_a.unk6144 < 64; vis_table_a.unk6144++, e++) {
                t = &vis_table_a.unk3500[e->unk02];
                if (e->unk05 == 1) {
                    if (vis_table_a.unk6168 == 0xff)
                        break;
                    if (vis_table_a.unk6168 != e->unk07) {
                        if (e->unk07 == 0xff && e->unk40 < 0x7000)
                            break;
                        vis_table_a.unk612C -= t->unk12;
                        vis_table_a.unk6128 -= e->unk0D;
                        e->unk05 = 0;
                        func_8004ac38(e);
                    }
                }
            }
        }
    }
    vis_table_a.unk614C = 0;
    for (e = vis_table_a.entries, vis_table_a.unk6144 = 0; vis_table_a.unk6144 < 64;
         e++, vis_table_a.unk6144++) {
        if (e->unk00 != 0xff) {
            func_8004c03c(e);
            dx = e->unk74.vx - viewer.position.vx;
            dy = e->unk74.vy - viewer.position.vy;
            dz = e->unk74.vz - viewer.position.vz;
            if (dx < -0x8000 || dx > 0x8000 || dy > 0x8000 || dy < -0x8000 || dz > 0x8000
                || dz < -0x8000) {
                e->unk3C = 0x40000000;
                e->unk40 = 0x8000;
            } else {
                e->unk3C = dx * dx + dy * dy + dz * dz;
                e->unk40 = SquareRoot0(dx * dx + dz * dz);
            }
            h = (ratan2(dz, dx) - 0x400) & 0xfff;
            e->unk56 = h;
            d = ((u16)e->unk8E - h) & 0xfff;
            e->unk58 = d;
            if (d > 0x800)
                e->unk58 = 0x1000 - d;
            e->unk9C.vz = 0;
            e->unk9C.vy = 0;
            e->unk9C.vx = 0;
            if ((vis_table_a.unk6150 & 3) == (e->unk07 & 3) || viewer.unk21C)
                func_8004b2dc();
            if (e->unk05 == 1) {
                if ((vis_table_a.unk6150 & 3) == (vis_table_a.unk614C & 3))
                    area_state.hooks->unk40();
                area_state.hooks->unk44();
                if (e->unk36 != -1) {
                    anim_get_file(e->unk0B + 128, &anim);
                    if (anim) {
                        e->unk36 -= 128;
                        if (e->unk36 < 0) {
                            e->unk36 = -1;
                            e->unk1C.bytes[2] &= 0xf2;
                        }
                    }
                }
                e->unk39 = 0;
                if (*(s32 *)&e->unk9C != 0 || e->unk9C.vz != 0 || e->unk16 != 0) {
                    func_8004c1b4(e);
                    if (!(viewer.unk2C & 0x40000)) {
                        e->unk74.vx += e->unk9C.vx;
                        e->unk74.vy += e->unk9C.vy;
                        e->unk74.vz += e->unk9C.vz;
                    }
                }
                vis_table_a.unk614C++;
            } else if (e->unk05 == 5) {
                area_state.hooks->unk44();
            }
        }
        e->unk23 = 0;
    }
    vis_table_a.unk6158[3] = vis_table_a.unk6158[2];
    vis_table_a.unk6158[2] = vis_table_a.unk6158[1];
    vis_table_a.unk6158[1] = vis_table_a.unk6158[0];
    vis_table_a.unk6158[0] = vis_table_a.unk6168;
    vis_table_a.unk6150++;
    vis_table_a.unk6160 >>= 1;
    vis_table_a.unk6162 >>= 1;
    vis_table_a.unk6164 >>= 1;
    if (abs(vis_table_a.unk6160) < 4)
        vis_table_a.unk6160 = 0;
    if (abs(vis_table_a.unk6162) < 4)
        vis_table_a.unk6162 = 0;
    if (abs(vis_table_a.unk6164) < 4)
        vis_table_a.unk6164 = 0;
    func_8004c03c(NULL);
    if (DAT_801cd0e4)
        func_80052c5c();
}

/* Turns the script offsets of the loaded records into pointers. */
ADDRESS(0x8004e8ec, 0x74)
void func_8004e8ec(void)
{
    Vis192 *t = vis_table_a.unk3500;
    VisScript **p;
    int i;
    int j;

    for (i = 0; i < 16; i++, t++) {
        if (t->unk00 == 0xff)
            return;
        p = t->unk60;
        for (j = 0; j < 24; j++, p++) {
            if ((s32)*p == -1)
                *p = NULL;
            else
                *p = (VisScript *)((u8 *)&vis_table_a.unk3500[16] + (s32)*p);
        }
    }
}

/* Loads the 64 table A entries from an area file. */
ADDRESS(0x8004e960, 0x3f4)
void func_8004e960(VisAFileEntry *src)
{
    u8 cnt[4];
    u8 map[16];
    u32 ids[3];
    Vis212 *e;
    Vis192 *t;
    Vis44 *c;
    u16 i;
    u8 k;
    int j;

    vis_table_a.unk6164 = 0;
    vis_table_a.unk6162 = 0;
    vis_table_a.unk6160 = 0;
    fill_words((u32 *)cnt, 0, 1);
    fill_words((u32 *)map, -1, 4);
    e = vis_table_a.entries;
    i = 64;
    while (i--) {
        e->unk00 = src->unk00;
        e->unk05 = 0;
        if (e->unk00 == 0xff)
            goto next;
        e->unk02 = src->unk01;
        t = &vis_table_a.unk3500[e->unk02];
        k = src->unk01;
        if (map[k] == 0xff) {
            map[k] = e->unk0C = cnt[k >> 2];
            k >>= 2;
            cnt[k] = (cnt[k] + t->unk04) & 3;
        } else {
            e->unk0C = map[k];
        }
        e->unk28 = src->unk02;
        e->unk06 = vis_records[e->unk28].unk01;
        e->unk07 = vis_records[e->unk28].unk07;
        e->unk09 = t->unk04;
        e->unk0B = e->unk02;
        e->unk03 = 0;
        e->unk10 = ids[0] = src->unk04[0];
        e->unk12 = ids[1] = src->unk04[1];
        e->unk14 = ids[2] = src->unk04[2];
        e->unk65 = src->unk0B;
        e->unk66 = src->unk0C;
        e->unk67 = src->unk0D;
        e->unk68 = src->unk10 & 0xfff;
        e->unk6A = src->unk12;
        e->unk6C = src->unk14;
        e->unk6E = src->unk16;
        func_8004acec(e);
        e->unk08 = 0;
        e->unk2A = 0;
        e->unk1B = 0;
        e->unkAC = 0;
        e->unk0D = src->unk0A;
        e->unk0E = t->unk05;
        e->unk0F = 2;
        e->unk16 = 0;
        e->unk1A = 0xff;
        e->unkA8 = NULL;
        e->unk22 = -1;
        e->unk21 = t->unk03;
        e->unk0A = t->unk10;
        e->unk2E = t->unk16;
        e->unk30 = t->unk18;
        e->unk50 = 178;
        if (63 - i >= 56)
            e->unk1C.bytes[3] = 1;
        else
            e->unk1C.bytes[3] = 0;
        e->unk1C.bytes[0] = 0;
        e->unk64 = 0;
        func_8004ac38(e);
        e->unk84 = e->unk74.vy;
        for (j = 0; j < 3; j++) {
            if (ids[j] != 0xffff) {
                c = &vis_table_c.unk0118[ids[j]];
                if (vis_table_c.unk0094[c->unk4] == 0xff)
                    vis_table_c.unk0094[c->unk4] = vis_table_c.unk0114++;
            }
        }
        if (e->unk70 & 0x10) {
            if (t->unk00 == 2) {
                if (e->unk6C == -1)
                    e->unk6C = t->unk32;
                if (e->unk6E == -1)
                    e->unk6E = t->unk08;
            } else {
                t->unk00 = 1;
                e->unk6E = 0;
            }
        }
    next:
        src++;
        e++;
    }
    vis_table_a.unk6130 = 0;
    vis_table_a.unk612C = 0;
    vis_table_a.unk6128 = 0;
}
