#ifndef ST_VIS_H
#define ST_VIS_H

/*
 * ST.EXE visible-set lists (band 0x8001c100..0x8001cb88, WIP). All objects
 * here are unresolved .bss globals; only the members these functions touch
 * are named, as unkXXX until their meaning is proven.
 */
#include <kf/lib/types.h>
#include <sys/types.h>

/* 0x801b5920: a state block; func_8001c100 reads a halfword at +0 and a
 * byte at +0x1d, func_8001cab8 a byte at +9. */
typedef struct {
    s16 unk00;
    u8 unk02[7];
    u8 unk09;
    u8 unk0a[0x13];
    u8 unk1d;
    u8 unk1e[0xa];
} Dat801b5920;

/* 0x801b5948: the source id list func_80046270 fills and the visible id
 * list func_8001c100 builds from it; func_8001c800/func_8001c88c search the
 * visible list. */
typedef struct {
    u8 unk0000[0x44ca];
    u16 n_sources;       /* 0x44ca */
    u16 n_visible;       /* 0x44cc */
    u16 sources[0x80];   /* 0x44ce */
    u16 visible[0x80];   /* 0x45ce */
} Dat801b5948;

/* 0x8017bed0: 12-byte records indexed by visible id. */
typedef struct {
    u8 unk00[8];
    u8 unk08;
    u8 unk09;
    u8 unk0a;
    u8 unk0b;
} Dat8017bed0;

/* 0x8017d6d0: 64-byte records, a 512-bit mask per source id. */
typedef struct {
    u_long bits[16];
} Dat8017d6d0;

extern Dat801b5920 DAT_801b5920;
extern Dat801b5948 DAT_801b5948;
extern Dat8017bed0 DAT_8017bed0[];
extern Dat8017d6d0 DAT_8017d6d0[];

#endif
