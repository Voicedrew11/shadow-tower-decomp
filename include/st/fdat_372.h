#ifndef ST_FDAT_372_H
#define ST_FDAT_372_H

/*
 * FDAT_372.BIN's overlay words that a later unit reads. Kept out of
 * st/fdat.h because FDAT_132.BIN has a static of the same name (a
 * file-scope extern would change that overlay's .bss layout).
 */
extern int fdat_801eda3c; /* src/fdat_372/map_801ed4a0.c */

#endif
