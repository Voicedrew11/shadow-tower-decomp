/*
 * ST.EXE equipment screen (src/st/equip_menu.c, WIP): its state block and
 * functions. Field names stay address-derived until behaviour is proven.
 */
#ifndef ST_EQUIP_MENU_H
#define ST_EQUIP_MENU_H

#include <kf/lib/types.h>
#include <sys/types.h>

/* The ring sub-list: a bit word per id from 196, the entries listed and
 * the entries, 255 when unused. */
typedef struct {
    u32 *bits;
    s32 count;
    u8 entries[0x10];
} EquipSubList;

/*
 * The state block at 0x801e0ce8: the id shown and the one whose image is
 * loaded, the list index, a frame count that holds the input, the
 * viewer.unk38 slot being changed, and the slot sub-list (its bit table,
 * entry count and entries). WIP extent: the slot sub-list follows as its
 * own global DAT_801e0d08, whose bit table is the 144-byte block the option
 * screens load (src/st/menu_vram.c addresses it apart from this block).
 */
typedef struct {
    s32 unk0;            /* 0x00: id shown, -1 for none */
    s32 unk4;            /* 0x04: id whose image is loaded */
    s32 unk8;            /* 0x08: index in the list, -1 for none */
    s32 unkC;            /* 0x0c: frames the paging input stays cleared */
    u16 *unk10;          /* 0x10: the viewer.unk38 slot being changed */
    u8 unk14[0xc];
} Data801e0ce8;

/* 0x801e0d08: the slot sub-list. menu_equip_open allocates its 144-byte bit
 * table (src/st/menu_vram.c) and menu_equip_close frees it. */
extern EquipSubList DAT_801e0d08;

/* The equipment ids menu_equip_open records (viewer.unk38[3], [4], [16],
 * [17], [6], [12]) for menu_equip_close to compare. */
extern u16 DAT_801e0d28[6];

extern Data801e0ce8 DAT_801e0ce8;

/* 0x8008092c: message offsets from 916 by sub-list entry (load data,
 * owner unknown). */
extern u8 DAT_8008092c[18];

void func_80063bdc(void);
void func_80063cb0(void);
int func_800642e8(int *sel);
void func_80064ba8(void);
void func_80064ee4(void);
void func_80064ff0(void);
void func_80065020(void);
void func_800650dc(void);
void func_800651f0(void);
void func_80065270(void);
void func_80065770(void);
void func_80065de0(void);
void func_80065f40(void);
void func_80066004(void);

#endif
