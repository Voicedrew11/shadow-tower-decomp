/*
 * ST.EXE world object passes (src/st/objects.c): each frame they walk the
 * visible entries of the three object tables (st/vis.h) and two free-standing
 * object arrays, flag the model slots they need, and draw the models through
 * model_draw.
 */
#ifndef ST_OBJECTS_H
#define ST_OBJECTS_H

#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>

/* func_800255f0 also draws the effects (effects.slots, st/effect.h). */

/* src/st/objects.c */
void func_800241d0(void);
void func_800246e4(void);
void func_800251c8(void);
void func_800255f0(void);

/* src/st/slots.c */
void func_80023bc4(int archive, int index, int slot);
void func_80023c6c(int archive, int first, int slot, int count);
void func_80023ed8(int archive, int first, int slot, int count);
void func_80024004(int archive, int first, int slot, int count);

#endif
