/*
 * VS.EXE: a data-only module (WIP) owning the end of the game .sdata. The
 * bss group starts at 0x80063300 in retail (the .sbss start NONE3 clears
 * from and DAT_80011f34 holds), but the last literal of .sdata
 * (vs.cd_archive's ";1") ends at 0x800632f3 and PSYLINK starts the group
 * at the next eight-byte boundary, 0x800632f8; an .sbss object's own
 * section does not move it further (a static in the first-linked .sbss
 * unit lands eight-aligned). So at least five more bytes of initialized
 * small data follow the literal: zero in the image and referenced by
 * nothing. Two zero words are the smallest whole-word reading; their
 * owner, sizes and types are unknown. -G8 places them in .sdata, and the
 * unit links after every code unit (link = "last").
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>

DATA(0x800632f4, 0x4, ".sdata")
s32 DAT_800632f4 = 0;
DATA(0x800632f8, 0x4, ".sdata")
s32 DAT_800632f8 = 0;
