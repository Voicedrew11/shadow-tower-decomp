/*
 * ST.EXE lighting environments (src/st/light.c): the table at 0x8018b6d0
 * that holds the per-area light selection and the 0x6c-byte environments,
 * the 0x30-byte defaults it is filled from, and the selection and blend
 * functions. Address-derived names stay until behaviour is proven.
 */
#ifndef ST_LIGHT_H
#define ST_LIGHT_H

#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/gfx.h>

/* One 0x30-byte default environment (lighting_load_defaults expands it to an
 * StLight); lighting_blend blends the parts its flags select. */
typedef struct {
    StLightMatrix unk0;
    StLightMatrix color;
    CVECTOR back;
    s16 unk2C;
    s16 unk2E;
} StLightInit;

/*
 * The lighting table: state bytes, one light-select halfword per area
 * (high nibble: row flags 0x10..0x80, low nibble: column flags 1..4) and
 * the environments. 0..11 are the row/column grid lighting_grid_index indexes,
 * 12..18 the special selections of lighting_select, 43 and 44 the current
 * and blend-start environments, 45..47 func_80017d28's blend targets.
 */
typedef struct {
    u8 unk0;
    u8 unk1;
    u8 unk2;             /* 0x02: set when the light matrices need rebuilding */
    u8 unk3;
    u8 unk4;
    u8 unk5;             /* 0x05: column flags last applied */
    u8 unk6;             /* 0x06: row flags last applied */
    u8 unk7;             /* 0x07: area last seen */
    u8 unk8;
    u8 unk9;
    u8 unkA;
    u8 unkB;
    s16 area_mode[48];   /* 0x0c */
    StLight light[64];   /* 0x6c */
} StLighting;

extern StLighting lighting;

/* The 64 default environments in the load image (0x8007f78c). */
extern StLightInit DAT_8007f78c[64];

/* Not reconstructed yet (0x80049aac..0x8004a888). */
void func_80049aac(StLight *light, StLightMatrix *out, int index);
int func_8004a830(int from, int to, short t);
void func_8004a888(StLightMatrix *out, StLightMatrix *to, StLightMatrix *from, short t);

/* src/st/light.c */
void lighting_load_defaults(void);
void lighting_build_matrices(void);
void lighting_blend(int flags, StLightInit *init, short t);
int lighting_grid_index(int flags);
void lighting_select(int rows, int columns, int blend);
void lighting_start(void);
void lighting_clear_requests(void);
void lighting_update(void);

#endif
