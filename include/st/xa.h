/*
 * ST.EXE XA voice playback for table A entries and still images
 * (src/st/xa.c). Field names stay address-derived.
 */
#ifndef ST_XA_H
#define ST_XA_H

#include <kf/lib/types.h>
#include <sys/types.h>
#include <st/vis.h>

/*
 * The XA sector counts per stream file (DAT_801cba08.unk1618) and channel:
 * 40 rows of 6 words (load data, owner unknown; WIP extent).
 */
extern u32 DAT_800804ac[40][6];

/* LIBC memcpy as the game declared it: retail calls it even for 4 bytes, so
 * this prototype is not GCC's builtin one. */
void *memcpy(void *dst, void *src, int size);

void func_80054000(u8 intr, u8 *result);
void func_8005408c(u8 file, u8 channel);
int func_800541f4(void);
void func_80054278(void);
void func_800542d0(int id, int full);
void func_80054368(Vis212 *e, int fixed, int arg2);
void func_80054858(int id);

#endif
