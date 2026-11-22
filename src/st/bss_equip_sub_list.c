/*
 * ST.EXE: a data-only module (WIP) defining game .bss:
 * the equipment screen's slot sub-list.
 * Every user declares it extern.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/data.h>
#include <st/equip_menu.h>

DATA(0x801e0d08, 0x18, ".bss")
EquipSubList equip_magic_list;
