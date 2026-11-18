/*
 * The last nine of the 21 functions every FDAT overlay starts with (WIP),
 * as a unit of its own for the 14 overlays whose map handlers are still
 * separate units (bodies in fdat_common_b.inc; the 22 fused overlay
 * sources include that directly). The others bind by name.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <psyq/libc.h>
#include <st/cd.h>
#include <st/gfx.h>
#include <st/anim.h>
#include <st/sound.h>
#include <st/data.h>
#include <st/hud.h>
#include <st/xa.h>
#include <st/vis.h>
#include <st/fdat.h>

#include "fdat_common_b.inc"
