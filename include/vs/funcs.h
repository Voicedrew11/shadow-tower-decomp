/*
 * VS.EXE functions called across reconstructed units. Most are VS's own
 * copies of ST.EXE functions; each comment names the ST twin.
 */
#ifndef VS_FUNCS_H
#define VS_FUNCS_H

#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBCD.H>
#include <LIBGTE.H>
#include <st/cd.h>
#include <st/mesh.h>
#include <st/anim.h>

/* src/vs/cd_8001cb34.c */
void func_8001cb34(void);                 /* ST func_8001531c */
void func_8001d1b8(void);                 /* ST func_800174c4 */
void func_8001d374(void);                 /* ST func_80017680 */
StCdRequest *func_8001d030(int kind, CdlLOC *loc, u32 size, u8 *dest, void (*done)()); /* ST func_80015bcc */
int func_8001d134(int archive, int member, CdlLOC *loc); /* ST func_80015cd0 */
void func_8001d298(void);                 /* ST func_800175a4 */
void func_8001d328(void);                 /* ST func_80017634 */

/* src/vs/anim_80023764.c */
void func_80023820(struct StAnimSet *set);
void func_8002392c(void);
void *func_8002398c(u32 size);           /* ST func_8001528c */
void func_800241b4(struct StAnimSet **out);

/* src/vs/anim_8002429c.c */
int func_800246d8(struct StAnimSet **setp, int id, int clip, u_long frame); /* ST anim_prepare_any + anim_set_pose */

/* src/vs/anim_80027f30.c */
int func_800281d4(struct StAnimSlot **slotp, int id, int clip, int frame); /* ST anim_prepare + anim_pose */

/* src/vs/mirror_80028474.c */
void func_80028474(void);

/* src/vs/anim_80027c64.c */
void func_80027c64(SVECTOR *dst, SVECTOR *from, SVECTOR *to, int count); /* ST vertices_sub */

/* The second renderer set src/vs/mesh_80027ba4.c runs (not reconstructed). */
void func_80027198(int count, MeshFaceFT3 *face, u_long *ot);
void func_80027400(int count, MeshFaceFT4 *face, u_long *ot);
void func_80027698(int count, MeshFaceGT3 *face, u_long *ot);
void func_800278f8(int count, MeshFaceGT4 *face, u_long *ot);

/* src/vs/spark_8002d738.c */
void func_8002d738(int n);

/* src/vs/cd_8002e5e4.c */
void func_8002e5e4(void);                 /* ST func_800214cc */
void *func_8002e690(u32 size);            /* ST func_8001528c */
u16 *func_8002e660(u16 *dst, u16 *src, int count); /* ST func_80014d2c */
void *func_8002e6d4(u32 size);            /* ST func_800152d0 */
void func_8002e6fc(void *p);              /* ST func_800152f8 */
void func_8002e720(u_char status, u_char *result); /* ST func_800154e8 */
void func_8002e868(u_char status, u_char *result); /* ST func_8001561c */
void func_8002ea94(StCdRequest *request); /* ST func_80015848 */
u32 func_8002eb9c(CdlLOC *pos);           /* ST func_80015950 */
void func_8002ece0(CdlLOC *pos, int offset, CdlLOC *out); /* ST func_80015a94 */
int func_8002ed20(u32 *data, int sectors); /* ST func_80015b78 */
void func_8002ed74(CdlLOC *loc, u_long *buf, int sectors); /* ST func_80015f0c */
void func_8002ee54(u_long *buf, StFileExtent *file, u32 size); /* ST func_80015ff4 */

/* src/vs/cd_8002efcc.c */
void func_8002efcc(void);                 /* ST func_8001624c */
void func_8002f014(u16 archive, char *name); /* ST func_80016294 */

/* src/vs/snd_80012a70.c */
void func_80012c2c(u_long *data, DVECTOR *pos, DVECTOR *clut_pos);

/* src/vs/anim_80023764.c */
void *func_8002398c(u32 size);            /* malloc, NULL outside main RAM */

/* src/vs/stub_8001ca10.c */
int func_8001ca18(void);

/* src/vs/cd_8001cb34.c */
void func_8001d39c(void);

/* src/vs/cd_8002efcc.c */
void func_8002f0f4(void);

/* src/vs/mesh_800258f0.c */
void func_80027ba4(u_short part);

/* VS main module callees (src/vs/main.c). */
u32 *func_8002e630(u32 *dst, u32 *src, int count);       /* src/vs/cd_8002e5e4.c */
void func_80012a70(int bank);                            /* src/vs/snd_800129bc.c */
void func_80012b50(u_long *data, int page, int clut_y);  /* src/vs/snd_800129bc.c */
void func_80012d10(int id);                              /* src/vs/snd_800129bc.c */
void func_8001381c(void);                                /* src/vs/snd_800129bc.c */
int func_8001c530(void);                                 /* src/vs/card_8001c530.c */
void func_80023764(u16 id, StAnimFile *file);            /* src/vs/anim_80023764.c */
void func_80027fec(u16 id, StAnimFile *file);            /* src/vs/anim_80027f30.c */
void func_8002345c(ModelFile *file);                     /* src/vs/model_8002345c.c */
int func_8002ef3c(u_long *buf, char *name, u32 size);    /* src/vs/cd_8002e5e4.c */
void func_80013848(void);                                /* src/vs/snd_800129bc.c */
void func_80013868(void);                                /* src/vs/snd_800129bc.c */
void func_80013aec(void);                                /* src/vs/snd_800129bc.c */
void func_80014070(void);                                /* src/vs/snd_800129bc.c */
void func_8001c9b8(void);                                /* src/vs/card_8001c8b8.c */
void func_80016d94(void);                                /* src/vs/gfx_80016ca8.c */
void func_80016ca8(void);                                /* src/vs/gfx_80016ca8.c */
int func_80016f10(void);                                 /* not reconstructed */
int func_8001c5e0(int won, int again);                   /* not reconstructed */
void func_80014480(void);                                /* src/vs/main.c */
void func_80015020(void);                                /* src/vs/main.c */
void func_80015e50(void);                                /* src/vs/main.c */
void func_80016488(void);                                /* src/vs/main.c */
void func_800165f4(void);                                /* src/vs/main.c */
void func_80014634(void);                                /* src/vs/main.c */
void func_800140d8(void);                                /* src/vs/fighter_800140d8.c */
void func_800129e8(int channel);                         /* src/vs/snd_800129bc.c */
void func_8001336c(int id, int mode);                    /* src/vs/snd_800129bc.c */
void func_800138a0(void);                                /* src/vs/snd_800129bc.c */
void func_80013c5c(void);                                /* src/vs/snd_800129bc.c */
void func_8002d710(void);                                /* src/vs/texanim_8002d710.c */
int func_8002dcd8(void);                                 /* src/vs/fighter_8002dcd8.c */
int func_8002e510(int id);                               /* src/vs/fighter_8002e114.c */
void func_800352cc(void);                                /* src/vs/effect_run_800352cc.c */
void func_800352fc(void);                                /* src/vs/effect_run_800352cc.c */
void func_80031478(int id);                              /* not reconstructed */
int func_8003333c(int arg0, u_long height, int spin);    /* src/vs/effect_move_8003333c.c */
void func_8002e314(u8 id, u8 attack);                    /* src/vs/fighter_8002e114.c */
void func_800256bc(int id, int node, int index, VECTOR *out); /* src/vs/anim_80025670.c */
int func_80028d94(int id, int part, int frame, int index, SVECTOR *out); /* src/vs/anim_80028d94.c */

#endif
