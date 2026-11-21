/*
 * PSYLINK's bss group bounds as VS.EXE's heap words (src/vs/rdata_80011f34.c)
 * spell them: the linker defines them, no object does. _bss_org is the first
 * byte of the group (.sbss), _bss_orgend the byte after it (after the SDK's
 * COMMON; the section symbol __bss_orgend stops before that COMMON).
 */
#ifndef VS_BOUNDS_H
#define VS_BOUNDS_H

#include <kf/lib/types.h>

extern u8 _bss_org[];
extern u8 _bss_orgend[];

#endif
