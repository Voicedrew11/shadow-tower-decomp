/*
 * ST.EXE: warping the viewer to a destination, and table C
 * (vis_table_c.entries: items lying in the areas): clearing, the collected
 * id bits, removing and loading entries, and the nearest one a request can
 * pick up (WIP module; the pick-up animation func_8005335c and the per-frame
 * pass func_80053aa4 follow).
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/data.h>
#include <st/cd.h>
#include <st/area.h>
#include <st/menu.h>
#include <st/light.h>
#include <st/player.h>
#include <st/effect.h>
#include <st/funcs.h>
#include <st/vis.h>

/* Fades out, loads `to`'s area and places the viewer there, then fades in;
 * `keep` skips re-collecting the visible sources and the light reset. */
ADDRESS(0x80052894, 0x390)
void func_80052894(to, keep)
    StWarp *to;
    u8 keep;
{
    Vis88 *e = vis_table_b.entries;
    StEffect *f;
    int i;
    u8 x;
    u8 y;
    u8 z;
    u8 area;
    int j;

    for (i = 240; i != 0; i--) {
        if (e->unk05 == 237)
            *(u8 *)&e->unk50 = 1;
        e++;
    }
    x = to->x;
    y = to->y;
    z = to->z;
    func_80057c54();
    func_800500ac(2, 0, 0x1000, 0x100);
    area = to->area;
    do {
        func_8001531c();
        func_800456f8();
    } while (area_state.unk0 != 0);
    viewer.unk278.vy = to->yaw;
    viewer.unk278.vx = 0;
    if (area != area_state.unk4) {
        func_80045044(area, area, area, 0xff, 0xff, 127, 127, 127, 0, 0, 0);
    } else {
        func_80045044(0xff, 0xff, area, 0xff, 0xff, 127, 127, 127, 0, 0, 0);
        viewer.unk21C = 1;
        func_8004e2d8();
        vis_table_a.unk6158[0] = vis_table_a.unk6158[1] = vis_table_a.unk6158[2] = vis_table_a.unk6158[3] = 0xff;
        vis_table_a.unk6103 = 0xff;
    }
    viewer.unk218 = 0;
    viewer.unk28C = 0;
    viewer.unk28A = 0;
    viewer.unk288 = 0;
    *(SVECTOR *)&viewer.unk268 = viewer.unk278;
    viewer.position.vx = (x << 12) + to->dx;
    viewer.position.vz = (z << 12) + to->dz;
    viewer.position.vy = (y << 12) + to->dy;
    func_8003ead0();
    f = effects.slots;
    vis_table_b.unk71A8 = 0;
    vis_table_b.unk71A6 = 0;
    vis_table_b.unk71A4 = 0;
    vis_lists.unk47D2 = 0;
    vis_lists.unk47D0 = 0;
    vis_lists.unk47CE = 0;
    vis_table_a.unk6164 = 0;
    vis_table_a.unk6162 = 0;
    vis_table_a.unk6160 = 0;
    viewer.unk1D0 = 0;
    viewer.unk1CE = 0;
    viewer.unk1CC = 0;
    j = 128;
    while (j--) {
        f->unk0 = ST_EFFECT_FREE;
        f++;
    }
    do {
        func_8001531c();
        func_800456f8();
    } while (area_state.unk0 != 0);
    if (!keep) {
        vis_lists.n_sources = 0;
        vis_collect_sources((VECTOR *)&viewer.position.vx, NULL, NULL);
        if (vis_lists.n_sources)
            viewer.unk174 = &vis_records[vis_lists.sources[0]];
        func_8004e324();
    }
    func_800500ac(2, 0x1000, 0x1000, 0);
    func_80015ad4();
    func_80057c54();
    func_800500ac(2, 0x1000, 0, -0x100);
    if (!keep) {
        func_80045044(0xff, 0xff, 0xff, area, area, 127, 127, 127, 0, 0, 0);
        lighting_clear_requests();
    }
    func_80057a3c();
    func_80042c10();
}

/* Warps to viewer.unk22C. */
ADDRESS(0x80052c24, 0x38)
void func_80052c24(void)
{
    func_80052894((StWarp *)&viewer.unk22C, 0);
    viewer.unk2B = 0;
}

/* Warps to viewer.unk220 keeping the current sources. */
ADDRESS(0x80052c5c, 0x48)
void func_80052c5c(void)
{
    viewer.unk218 = 5;
    func_8003d030();
    func_80052894((StWarp *)viewer.unk220, 1);
    DAT_801cd0e4 = 0;
}

/* Frees every table C entry and clears the group counts and id bits. */
ADDRESS(0x80052ca4, 0x6c)
void func_80052ca4(void)
{
    Vis48 *c = vis_table_c.entries;
    int i;

    vis_table_c.unk0000[5] = 0;
    vis_table_c.unk0000[4] = 0;
    vis_table_c.unk0000[3] = 0;
    vis_table_c.unk0000[2] = 0;
    i = 64;
    while (i--) {
        c->unk00 = 0xffff;
        c->unk22 = 0;
        c++;
    }
    /* Ten words each; the tenth of unk0070 is the first of unk0094. */
    i = 10;
    while (i--) {
        vis_table_c.unk0048[i] = 0;
        vis_table_c.unk0070[i] = 0;
    }
}

/* Unassigns every Vis44 slot. */
ADDRESS(0x80052d10, 0x30)
void func_80052d10(void)
{
    int i = 128;

    while (i--)
        vis_table_c.unk0094[i] = 0xff;
    vis_table_c.unk0114 = 0;
}

/* 0 when id has been collected (ids below 266 keep a bit), else 1. */
ADDRESS(0x80052d40, 0x58)
int func_80052d40(int id)
{
    if (id < 266) {
        if (vis_table_c.unk0070[id / 32] & (1 << (id % 32)))
            return 0;
    }
    return 1;
}

/* Counts the pick-up of item *id and marks it collected. */
ADDRESS(0x80052d98, 0xb8)
void func_80052d98(u16 *id)
{
    DAT_801c7590.counts[*id].count++;
    if (*id == 266)
        viewer.unk27++;
    if (*id < 266) {
        viewer.unk28++;
        vis_table_c.unk0070[*id >> 5] |= 1 << (*id & 0x1f);
    }
}

/* Removes an entry, closing up the rest of its group (32..47 or 48..63). */
ADDRESS(0x80052e50, 0xe0)
void func_80052e50(Vis48 *c)
{
    int i = ((u8 *)c - (u8 *)vis_table_c.entries) / sizeof(Vis48);
    int n;

    if (i < 32) {
        vis_table_c.unk0000[1]--;
    } else {
        if (i < 48) {
            n = 47 - i;
            vis_table_c.unk0000[2]--;
        } else {
            n = 63 - i;
            vis_table_c.unk0000[3]--;
        }
        while (n--) {
            c[0] = c[1];
            c++;
        }
    }
    c->unk22 = 0;
}

/* Loads the first 32 entries from an area file, then drops the collected
 * ones of the 16 from entry 32.
 * Not exact: this build hoists &area_state into s7 out of the first loop;
 * retail reads area_state.area with lui+lbu inside it (frame 48, not 56). */
ADDRESS(0x80052f30, 0x244)
void func_80052f30(VisCFileEntry *src)
{
    Vis48 *c = vis_table_c.entries;
    Vis44 *rec;
    int i;

    vis_table_c.unk0000[0] = 0;
    for (i = 0; i < 32; i++, c++, src++) {
        c->unk00 = src->unk00;
        c->unk02 = src->unk16;
        c->unk08.vx = (src->unk04 << 12) + src->unk08;
        c->unk08.vy = (src->unk05 << 12) + src->unk0A;
        c->unk08.vz = (src->unk06 << 12) + src->unk0C;
        c->unk18 = src->unk0E & 0xfff;
        c->unk1A = src->unk10 & 0xfff;
        c->unk1C = src->unk12 & 0xfff;
        c->unk20 = src->unk02;
        c->unk26 = 0;
        if (c->unk00 == 0xffff) {
            c->unk22 = 0;
            continue;
        }
        c->unk2c = area_state.area;
        rec = &vis_table_c.unk0118[c->unk00];
        c->unk2D = rec->unk5[1];
        vis_table_c.unk0048[c->unk00 >> 5] |= 1 << (c->unk00 & 0x1f);
        vis_table_c.unk0000[1]++;
        vis_table_c.unk0000[0]++;
        if (vis_table_c.unk0094[rec->unk4] == 0xff)
            vis_table_c.unk0094[rec->unk4] = vis_table_c.unk0114++;
        if (i >= 28 || func_80052d40(c->unk00))
            c->unk22 = 2;
        else
            c->unk22 = 0;
    }
    c = &vis_table_c.entries[32];
    for (i = 0; i < 16; i++) {
        if (func_80052d40(c->unk00) == 0)
            func_80052e50(c);
        else
            c++;
    }
}

/* The nearest lying item in view of the viewer within the request's reach
 * (its record's radius plus req->unk1C); -1 when none. */
ADDRESS(0x80053174, 0x1e8)
int func_80053174(VisLists *req, int unused)
{
    Vis48 *c;
    Vis44 *rec;
    int best = -1;
    int nearest = 0x7fffffff;
    int i;
    int dx;
    int dz;
    int dist;

    c = vis_table_c.entries;
    for (i = 0; i < 64; i++, c++) {
        if (c->unk22 != 1 && c->unk22 != 2)
            continue;
        if (c->unk2c != area_state.area)
            continue;
        rec = &vis_table_c.unk0118[c->unk00];
        if (rec->unk0[1] == 0)
            continue;
        if (c->unk08.vy < viewer.position.vy - 0x400 || c->unk08.vy > viewer.position.vy + 0xa40)
            continue;
        if (!func_8004a148(viewer.unk278.vy,
                           func_8004a1b4(c->unk08.vx - viewer.position.vx, c->unk08.vz - viewer.position.vz), 300))
            continue;
        dx = c->unk08.vx - req->unk4.vx;
        if (dx < -0x2800 || dx > 0x2800)
            continue;
        dz = c->unk08.vz - req->unk4.vz;
        if (dz < -0x2800 || dz > 0x2800)
            continue;
        dist = SquareRoot0(dx * dx + dz * dz);
        if (dist < rec->unk0[1] * 256 + req->unk1C && dist < nearest) {
            nearest = dist;
            best = i;
        }
    }
    return best;
}
