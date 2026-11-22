/*
 * PSYLINK's bss group bounds as ST.EXE's heap words (src/st/rdata_bss_bounds.c)
 * spell them: the linker defines them, no object does. _bss_org is the first
 * byte of the group (.sbss), _bss_orgend the byte after it (after the SDK's
 * COMMON). The FDAT overlay area follows it.
 */
#ifndef ST_BOUNDS_H
#define ST_BOUNDS_H

#include <kf/lib/types.h>

extern u8 _bss_org[];
extern u8 _bss_orgend[];

#endif
