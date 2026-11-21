/*
 * ST.EXE: a data-only module (WIP) defining game .data: the ring sub-list
 * message offsets.
 * No code unit but st.main contributes .data, and retail keeps these after
 * st.main's run, so a module linked after the code defines them; every user
 * declares them extern.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/data.h>
#include <st/equip_menu.h>

DATA(0x8008092c, 0x12, ".data")
u8 DAT_8008092c[18] = {0, 1, 255, 2, 3, 255, 4, 5, 255,
                       6, 7, 255, 8, 9, 255, 10, 11, 255};
