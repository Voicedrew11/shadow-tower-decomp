#ifndef ST_FDAT_382_H
#define ST_FDAT_382_H

/*
 * FDAT_382.BIN's overlay words that another unit reads. Kept out of
 * st/fdat.h because FDAT_142.BIN has statics of the same names (a
 * file-scope extern would change that overlay's .bss layout).
 */
extern int fdat_801edafc; /* src/fdat_382/map_801eda74.c */
extern int fdat_801edb00; /* src/fdat_382/map_801ed9b4.c */

#endif
