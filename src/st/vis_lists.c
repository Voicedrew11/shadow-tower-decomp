/*
 * ST.EXE visible-set queries and list builders (WIP module): membership
 * tests against the visible id list, and three passes that collect the
 * entries of three object tables whose id is visible.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <st/vis.h>

/* 0x801ba138: 64 entries of 212 bytes, then the collected index list. */
typedef struct {
    u8 unk00[3];
    u8 unk03;
    u8 unk04;
    u8 unk05;
    u8 unk06[0x20];
    u16 unk26;
    u8 unk28[0xac];
} Vis212;

typedef struct {
    Vis212 entries[64];
    u8 unk3500[0x2c00];
    u8 count;            /* 0x6100 */
    u8 unk6101[3];
    u16 list[12];        /* 0x6104 */
} Dat801ba138;

/* 0x801c0310: 240 entries of 88 bytes at +0x1f20, then the index list. */
typedef struct {
    u8 unk00[4];
    u8 unk04;
    u8 unk05;
    u8 unk06;
    u8 unk07;
    u16 unk08;
    u16 unk0a;
    u8 unk0c[0x4c];
} Vis88;

typedef struct {
    u8 unk0000[0x1f20];
    Vis88 entries[240];
    u8 unk71a0[0x14];
    u16 list[96];        /* 0x71b4 */
    u8 count;            /* 0x7274 */
} Dat801c0310;

/* 0x801c76f8: the index list, then 64 entries of 48 bytes at +0x3710. */
typedef struct {
    u16 unk00;
    u16 unk02;
    u8 unk04[0x28];
    u8 unk2c;
    u8 unk2d[3];
} Vis48;

typedef struct {
    u8 unk0000[6];
    u8 count;            /* 0x0006 */
    u8 list[64];         /* 0x0007 */
    u8 unk0047[0x36c9];
    Vis48 entries[64];   /* 0x3710 */
} Dat801c76f8;

extern Dat801ba138 DAT_801ba138;
extern Dat801c0310 DAT_801c0310;
extern Dat801c76f8 DAT_801c76f8;

ADDRESS(0x8001c800, 0x8c)
int func_8001c800(int a, int b, int c)
{
    Dat8017bed0 *rec;
    int i;

    i = DAT_801b5948.n_visible;
    while (i--) {
        rec = &DAT_8017bed0[DAT_801b5948.visible[i]];
        if (rec->unk08 == a && rec->unk09 == b && rec->unk0a == c) {
            return 1;
        }
    }
    return 0;
}

ADDRESS(0x8001c88c, 0x54)
int func_8001c88c(u16 id)
{
    int i;

    i = DAT_801b5948.n_visible;
    while (i--) {
        if (id == DAT_801b5948.visible[i]) {
            return 1;
        }
    }
    return 0;
}

ADDRESS(0x8001c8e0, 0xe8)
void func_8001c8e0(void)
{
    Vis212 *e;
    int i;

    DAT_801ba138.count = 0;
    e = DAT_801ba138.entries;
    i = 64;
    while (i--) {
        if (e->unk03 != 2 && (e->unk05 == 1 || e->unk05 == 5)
            && (e->unk03 == 1 || func_8001c88c(e->unk26))) {
            if (DAT_801ba138.count >= 12) {
                return;
            }
            DAT_801ba138.list[DAT_801ba138.count] = 63 - i;
            DAT_801ba138.count++;
        } else {
            e->unk04 = 0;
        }
        e++;
    }
}

ADDRESS(0x8001c9c8, 0xf0)
void func_8001c9c8(void)
{
    Vis88 *e;
    int i;

    e = DAT_801c0310.entries;
    DAT_801c0310.count = 0;
    i = 240;
    while (i--) {
        if (e->unk0a < 280) {
            e->unk04 &= 0x7f;
            if (func_8001c88c(e->unk08) || e->unk05 == 236) {
                if (DAT_801c0310.count >= 96) {
                    return;
                }
                e->unk04 |= 0x80;
                DAT_801c0310.list[DAT_801c0310.count] = 239 - i;
                DAT_801c0310.count++;
            }
        } else {
            e->unk07 = 0;
        }
        e++;
    }
}

ADDRESS(0x8001cab8, 0xd0)
void func_8001cab8(void)
{
    Vis48 *e;
    int i;

    e = DAT_801c76f8.entries;
    DAT_801c76f8.count = 0;
    i = 64;
    while (i--) {
        if (e->unk00 != 0xffff && e->unk2c == DAT_801b5920.unk09 && func_8001c88c(e->unk02)) {
            DAT_801c76f8.list[DAT_801c76f8.count] = 63 - i;
            DAT_801c76f8.count++;
        }
        e++;
    }
}
