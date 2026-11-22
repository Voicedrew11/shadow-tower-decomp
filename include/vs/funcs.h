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
void cd_wait_frame(void);                 /* ST cd_wait_frame */
void gfx_begin_frame(void);                 /* ST gfx_begin_frame */
void gfx_end_frame(void);                 /* ST gfx_end_frame */
StCdRequest *cd_request_queue(int kind, CdlLOC *loc, u32 size, u8 *dest, void (*done)()); /* ST cd_request_queue */
int cd_archive_member_locate(int archive, int member, CdlLOC *loc); /* ST cd_archive_member_locate */
void gfx_flip(void);                 /* ST gfx_flip */
void gfx_draw_ot(void);                 /* ST gfx_draw_ot */

/* src/vs/model_8002345c.c */
void anim_set_free(struct StAnimSet *set);
void anim_sets_free_all(void);
void *mem_alloc_main_ram(u32 size);           /* ST mem_alloc_checked */
void anim_set_alloc(struct StAnimSet **out);

/* src/vs/model_8002345c.c */
int anim_set_pose(struct StAnimSet **setp, int id, int clip, u_long frame); /* ST anim_prepare_any + anim_set_pose */

/* src/vs/anim_80027f30.c */
int anim_pose(struct StAnimSlot **slotp, int id, int clip, int frame); /* ST anim_prepare + anim_pose */

/* src/vs/mirror_80028474.c */
void model_draw_mirrored(void);

/* src/vs/anim_80027c64.c */
void vertices_sub(SVECTOR *dst, SVECTOR *from, SVECTOR *to, int count); /* ST vertices_sub */

/* The second renderer set src/vs/mesh_80027ba4.c runs (not reconstructed). */
void mesh_draw_ft3_nccs(int count, MeshFaceFT3 *face, u_long *ot);
void mesh_draw_ft4_nccs(int count, MeshFaceFT4 *face, u_long *ot);
void mesh_draw_gt3_nccs(int count, MeshFaceGT3 *face, u_long *ot);
void mesh_draw_gt4_nccs(int count, MeshFaceGT4 *face, u_long *ot);

/* src/vs/spark_8002d738.c */
void func_8002d738(int n);

/* src/vs/cd_8002e5e4.c */
void loading_screen_step(void);                 /* ST loading_screen_step */
void *mem_alloc_checked(u32 size);            /* ST mem_alloc_checked */
u16 *copy_halfwords(u16 *dst, u16 *src, int count); /* ST copy_halfwords */
void *mem_alloc(u32 size);            /* ST mem_alloc */
void mem_free(void *p);              /* ST mem_free */
void cd_request_ready_callback(u_char status, u_char *result); /* ST cd_request_ready_callback */
void cd_request_sync_callback(u_char status, u_char *result); /* ST cd_request_sync_callback */
void cd_request_retire(StCdRequest *request); /* ST cd_request_retire */
u32 cd_loc_to_sector(CdlLOC *pos);           /* ST cd_loc_to_sector */
void cd_loc_add_sectors(CdlLOC *pos, int offset, CdlLOC *out); /* ST cd_loc_add_sectors */
int cd_checksum_mismatch(u32 *data, int sectors); /* ST cd_checksum_mismatch */
void cd_read_sectors(CdlLOC *loc, u_long *buf, int sectors); /* ST cd_read_sectors */
void cd_read_file_extent(u_long *buf, StFileExtent *file, u32 size); /* ST cd_read_file_extent */

/* src/vs/cd_8002efcc.c */
void cd_stop_drive(void);                 /* ST cd_stop_drive */
void cd_archive_open(u16 archive, char *name); /* ST cd_archive_open */

/* src/vs/snd_80012a70.c */
void tim_load_images_at(u_long *data, DVECTOR *pos, DVECTOR *clut_pos);

/* src/vs/model_8002345c.c */
void *mem_alloc_main_ram(u32 size);            /* malloc, NULL outside main RAM */

/* src/vs/stub_8001ca10.c */
int cd_queue_busy(void);

/* src/vs/cd_8001cb34.c */
void clear_vram(void);

/* src/vs/cd_8002efcc.c */
void cd_init(void);

/* src/vs/mesh_800258f0.c */
void mesh_draw_part_nccs(u_short part);

/* VS main module callees (src/vs/main.c). */
u32 *copy_words(u32 *dst, u32 *src, int count);       /* src/vs/cd_8002e5e4.c */
void sound_bank_open(int bank);                            /* src/vs/main.c */
void tim_load_images_shifted(u_long *data, int page, int clut_y);  /* src/vs/main.c */
void fighter_offset_textures(int id);                              /* src/vs/main.c */
void pad_clear_input(void);                                /* src/vs/main.c */
int card_check_saves_changed(void);                                 /* src/vs/card_8001c530.c */
void anim_set_register(u16 id, StAnimFile *file);            /* src/vs/model_8002345c.c */
void anim_register(u16 id, StAnimFile *file);            /* src/vs/anim_80027f30.c */
void model_fixup_face_indices(ModelFile *file);                     /* src/vs/model_8002345c.c */
int load_file(u_long *buf, char *name, u32 size);    /* src/vs/cd_8002e5e4.c */
void rcnt2_handler(void);                                /* src/vs/main.c */
void vsync_handler(void);                                /* src/vs/main.c */
void sound_init(void);                                /* src/vs/main.c */
void cd_open_vs_archives(void);                                /* src/vs/main.c */
void load_menu_sound_bank(void);                                /* src/vs/card_8001c8b8.c */
void gfx_setup_320x240(void);                                /* src/vs/gfx_80016ca8.c */
void gfx_setup_640x240(void);                                /* src/vs/gfx_80016ca8.c */
int menu_vs_setup_run(void);                                 /* not reconstructed */
int match_award_requested_item(int won, int again);                   /* not reconstructed */
void gfx_init(void);                                /* src/vs/main.c */
void round_draw_frame(void);                                /* src/vs/main.c */
void round_play(void);                                /* src/vs/main.c */
void match_load_resources(void);                                /* src/vs/main.c */
void match_end_screen_run(void);                                /* src/vs/main.c */
void camera_update(void);                                /* src/vs/main.c */
void fighters_load_round(void);                                /* src/vs/main.c */
void xa_cue_bgm(int channel);                         /* src/vs/main.c */
void fighter_set_semi_trans(int id, int mode);                    /* src/vs/main.c */
void debug_pan_display(void);                                /* src/vs/main.c */
void fighters_play_anim_sounds(void);                                /* src/vs/main.c */
void sparks_clear_all(void);                                /* src/vs/texanim_8002d710.c */
int func_8002dcd8(void);                                 /* src/vs/fighter_8002dcd8.c */
int func_8002e510(int id);                               /* src/vs/fighter_8002e114.c */
void func_800352cc(void);                                /* src/vs/effect_80032404.c */
void func_800352fc(void);                                /* src/vs/effect_80032404.c */
void func_80031478(int id);                              /* not reconstructed */
int func_8003333c(int arg0, u_long height, int spin);    /* src/vs/effect_80032404.c */
void func_8002e314(u8 id, u8 attack);                    /* src/vs/fighter_8002e114.c */
void anim_set_node_vertex(int id, int node, int index, VECTOR *out); /* src/vs/model_8002345c.c */
int anim_get_vertex(int id, int part, int frame, int index, SVECTOR *out); /* src/vs/anim_80028d94.c */

#endif
