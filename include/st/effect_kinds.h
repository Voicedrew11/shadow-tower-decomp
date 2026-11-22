/*
 * ST.EXE effect kinds the player casts (StEffect.kind, st/effect.h): an
 * effect kind is also its effects.defs index. A magic slot
 * (EQUIP_SLOT_RIGHT_MAGIC, EQUIP_SLOT_LEFT_MAGIC) holds the kind it casts
 * (player_load_magic, player_cast_magic, player_fire_magic_shot), and the
 * equip screen names it with STAT.T message 916 + magic_name_messages[kind]
 * (menu_equip_draw_slots, menu_equip_draw_magic_list): those names are used
 * here. Kinds 2, 5, 8, 11, 14 and 17 have no name (magic_name_messages 255).
 * The creatures' attack kinds (48..) come from their scripts and stay
 * numeric.
 */
#ifndef ST_EFFECT_KINDS_H
#define ST_EFFECT_KINDS_H

#define EFFECT_KIND_EXPLOSIVE 0x00
#define EFFECT_KIND_BLAZE 0x01
#define EFFECT_KIND_FROZEN_HORNET 0x03
#define EFFECT_KIND_ICE_SABRE 0x04
#define EFFECT_KIND_QUEEN_EYES 0x06
#define EFFECT_KIND_DUAL_POISON 0x07
#define EFFECT_KIND_BUBBLE_SHOWER 0x09
#define EFFECT_KIND_CORROSIVE_ELEMENTS 0x0a
#define EFFECT_KIND_JUDGEMENT 0x0c
#define EFFECT_KIND_CLEANSING_RAY 0x0d
#define EFFECT_KIND_SLAYER 0x0f
#define EFFECT_KIND_UNHOLY_SPIRITS 0x10

/* A bow's shot: the held bow's model kind (31..34) + 13
 * (player_update_hand_action). */
#define EFFECT_KIND_BOW_SHOT_0 0x2c
#define EFFECT_KIND_BOW_SHOT_1 0x2d
#define EFFECT_KIND_BOW_SHOT_2 0x2e
#define EFFECT_KIND_BOW_SHOT_3 0x2f

#endif
