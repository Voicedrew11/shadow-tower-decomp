/*
 * VS.EXE's controller state: two 20-byte records at 0x801603c0 (bss), one
 * per player. Only the words the code proves are spelled.
 */
#ifndef VS_PAD_H
#define VS_PAD_H

#include <kf/lib/types.h>
#include <sys/types.h>

typedef struct {
    u32 unk0;            /* 0x00 */
    u32 unk4;
    u32 unk8;            /* 0x08: button bits the screens test and clear */
    u32 unkC;            /* 0x0c: more button bits, tested likewise */
    u32 unk10;
} VsPad;

typedef char VsPadSize[sizeof(VsPad) == 0x14 ? 1 : -1];

extern VsPad DAT_801603c0[2];

#endif
