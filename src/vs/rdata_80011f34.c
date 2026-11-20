/*
 * VS.EXE: a data-only module (WIP) holding seven read-only words between the
 * last code unit's switch tables and LIBCD's .rdata: the start of the bss
 * group (.sbss), then its end six times. main reads DAT_80011f40 and
 * DAT_80011f48 and stores the heap base into DAT_80011f4c (vs/data.h
 * declares those three writable, so this file does not include it).
 *
 * No Psy-Q kit object holds such a table (NONE3 from 4.5; the Psy-Q 3.0
 * NONE, NONE2, 2MBYTE, 8MBYTE and POWERON objects; the LIBSN members). The
 * source form is inferred from the values, not recovered: PSYLINK defines
 * the group bounds _bss_org and _bss_orgend, and const pointers initialized
 * with them are what places link-time addresses in a read-only section.
 * The object links after every code unit (link = "last" in units.toml).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <vs/bounds.h>

DATA(0x80011f34, 0x4, ".rodata")
u8 *const DAT_80011f34 = _bss_org;
DATA(0x80011f38, 0x4, ".rodata")
u8 *const DAT_80011f38 = _bss_orgend;
DATA(0x80011f3c, 0x4, ".rodata")
u8 *const DAT_80011f3c = _bss_orgend;
DATA(0x80011f40, 0x4, ".rodata")
u8 *const DAT_80011f40 = _bss_orgend;
DATA(0x80011f44, 0x4, ".rodata")
u8 *const DAT_80011f44 = _bss_orgend;
DATA(0x80011f48, 0x4, ".rodata")
u8 *const DAT_80011f48 = _bss_orgend;
DATA(0x80011f4c, 0x4, ".rodata")
u8 *const DAT_80011f4c = _bss_orgend;
