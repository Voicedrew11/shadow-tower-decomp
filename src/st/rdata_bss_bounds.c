/*
 * ST.EXE: a data-only module (WIP) holding seven read-only words between the
 * last code unit's switch tables and LIBCD's .rdata: the start of the bss
 * group (.sbss), its end three times, then two bounds of the FDAT overlay
 * area that starts there: past the overlay header's id and handler table,
 * and the end of the largest overlay (twice). main rounds the heap up from
 * DAT_80013980 past DAT_80013988 and stores the base into DAT_8001398c;
 * st.area_load takes the overlay's handler table from DAT_80013980 (st/data.h
 * declares those three writable, so this file does not include it).
 *
 * No Psy-Q kit object holds such a table (NONE3 from 4.5, 2MBYTE, 8MBYTE,
 * NOHEAP, LIBSN's SNMAIN, Release 2.5 MALLOC). The source form is inferred
 * from the values, not recovered: PSYLINK defines the group bounds _bss_org
 * and _bss_orgend, and const pointers initialized with them are what places
 * link-time addresses in a read-only section. The overlay bounds most likely
 * came from an overlay group that retail's link had and this build's ST.EXE
 * link does not (the overlays link on their own); they are spelled as
 * _bss_orgend plus the constants below. The object links after every code
 * unit (link = "last" in units.toml).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <kf/lib/offsetof.h>
#include <st/bounds.h>
#include <st/fdat.h>

/* The overlay's id word and 32-entry handler table: 0x84 bytes. */
#define FDAT_HANDLERS_END offsetof(FdatHeader, unk84)

/*
 * The FDAT overlay area: the extent of FDAT_272.BIN with its .bss, the
 * largest of the 36 overlays (its native link ends at __bss_orgend
 * 0x801ef940 from org 0x801eb560; the next largest, FDAT_302.BIN, ends at
 * 0x801ee484).
 */
#define FDAT_AREA_SIZE 0x43e0

DATA(0x80013974, 0x4, ".rodata")
u8 *const DAT_80013974 = _bss_org;
DATA(0x80013978, 0x4, ".rodata")
u8 *const DAT_80013978 = _bss_orgend;
DATA(0x8001397c, 0x4, ".rodata")
u8 *const DAT_8001397c = _bss_orgend;
DATA(0x80013980, 0x4, ".rodata")
u8 *const DAT_80013980 = _bss_orgend;
DATA(0x80013984, 0x4, ".rodata")
u8 *const DAT_80013984 = _bss_orgend + FDAT_HANDLERS_END;
DATA(0x80013988, 0x4, ".rodata")
u8 *const DAT_80013988 = _bss_orgend + FDAT_AREA_SIZE;
DATA(0x8001398c, 0x4, ".rodata")
u8 *const DAT_8001398c = _bss_orgend + FDAT_AREA_SIZE;
