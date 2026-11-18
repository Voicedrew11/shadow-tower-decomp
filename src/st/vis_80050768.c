/*
 * ST.EXE: the per-frame pass over table B (vis_table_b.entries: doors,
 * traps, warps and the other fixed things of an area) (WIP module, -G8: the
 * two shake amounts are $gp-relative statics).
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <psyq/libc.h>
#include <st/data.h>
#include <st/cd.h>
#include <st/area.h>
#include <st/menu.h>
#include <st/light.h>
#include <st/player.h>
#include <st/effect.h>
#include <st/funcs.h>
#include <st/vis.h>

/* func_80050768's switch table (a door's swing side). */
JUMPTABLES(0x80012720, 0x18)

/* The shake (x, z) a kind 224 entry gives the view while its entries rise. */
DATA(0x800881e0, 0x8, ".sbss")
static SVECTOR DAT_800881e0;

/* The halfwords and bytes other kinds keep from 0x50. */
#define H50(e) (*(s16 *)&(e)->unk50)
#define H52(e) (((s16 *)&(e)->unk50)[1])
#define B50(e) (*(u8 *)&(e)->unk50)
#define B51(e) (((s8 *)&(e)->unk50)[1])
#define B52(e) (((s8 *)&(e)->unk50)[2])

/* The three bytes from 0x48 and from 0x50, 0xffffff when unset. */
#define W48(e) (*(u_long *)&(e)->unk48 & 0xffffff)
#define W50(e) (*(u_long *)&(e)->unk50 & 0xffffff)

/* Starts the entry's sound n (its record's sounds start at 208). */
#define SOUND(e, n) func_8004ed54(e, (e)->unk0a * 3 + (n))

/* The moving sound as the entry leaves its rest position. */
#define START(e, step)                                                         \
    if ((e)->unk10 == 0 && (e)->unk10 + (step) * 4 > 0)                        \
        SOUND(e, 208)

/* Swaps the first bytes of the entry's two record ranges. */
#define SWAP_RECORDS(e)                                                        \
    n = (e)->unk4A.door.unk4E;                                                 \
    while (n--) {                                                              \
        t = vis_records[(e)->unk4A.door.unk4C + n].part;                      \
        vis_records[(e)->unk4A.door.unk4C + n].part =                         \
            vis_records[(e)->unk4A.door.unk4A + n].part;                      \
        vis_records[(e)->unk4A.door.unk4A + n].part = t;                      \
    }

/*
 * Updates every table B entry by its kind.
 *
 * Not exact (2104 of retail's 2123 instructions; the blocks and their order
 * agree): the frame is 144 bytes against 160. Retail keeps the warp's area
 * byte in a stack slot (104(sp)) with copies in s2 and s8, and gives the
 * loop's hoisted constants (255, 1, 3, &vis_table_b) no register, so each
 * use rematerialises them in t2/t3; here 255 takes s8 and the area a
 * register. Retail also recomputes step * 4 in the door arms (the speed is a
 * 16-bit variable, which is in) where cse still reaches the first copy here,
 * and copies the kind to a0 before the switch. Not tried yet: x/y/z/area as
 * function-level bytes, and the arms' statement order against sched.
 */
ADDRESS(0x80050768, 0x212c)
void func_80050768(void)
{
    u16 dmg[9];
    u16 dmg2[9];
    Vis88 *e = vis_table_b.entries;
    Vis24 *rec;
    int i;
    int dx;
    int dy;
    int dz;
    int angle;
    int turn;
    u16 n;
    u8 t;

    for (i = 240; i != 0; i--, e++) {
        if (e->unk0a == 0xffff)
            continue;
        vis_table_b.unk71B0 = e;
        vis_table_b.unk71AC = rec = &vis_table_b.unk0000[e->unk0a];
        dx = e->unk24.vx - viewer.position.vx;
        dy = e->unk24.vy - viewer.position.vy;
        dz = e->unk24.vz - viewer.position.vz;
        if (dx + 0x8000 > 0x10000u || dy > 0x8000 || dy < -0x8000 || dz > 0x8000 || dz < -0x8000) {
            e->unk18 = 0x40000000;
            e->unk1C = 0x8000;
        } else {
            e->unk18 = dx * dx + dy * dy + dz * dz;
            e->unk1C = SquareRoot0(dx * dx + dz * dz);
        }
        angle = (ratan2(dz, dx) - 1024) & 0xfff;
        e->unk20 = angle;
        turn = ((u16)e->unk36 - angle) & 0xfff;
        e->unk22 = turn;
        if (turn > 2048)
            e->unk22 = 4096 - turn;
        if (e->unk05 == 255)
            continue;
        switch (e->unk05) {
        case 0:
        case 1: {
            int step = e->unk48;
            short speed = step * 4;

            switch (e->unk0C) {
            case 0:
                if (e->unk05 == 1) {
                    if (world_state.unk834C[e->unk0a + 256] == NULL)
                        e->unk00 = 0;
                    else
                        e->unk00 = 2;
                }
                if (e->unk49 >= 249 && e->unk49 <= 250 && e->unk1C <= 8192)
                    e->unk0C = 1;
                break;
            case 1:
                if (e->unk10 == 0 && e->unk10 + step * 4 > 0) {
                    SWAP_RECORDS(e);
                    if (e->unk05 == 1)
                        e->unk00 = 1;
                    SOUND(e, 208);
                }
                if (speed + e->unk10 >= 4095) {
                    e->unk10 = 4095;
                    e->unk0C = 2;
                } else {
                    e->unk10 = (e->unk10 + speed) & 0xfff;
                }
                if (e->unk10 < 3072 && speed + e->unk10 >= 3072)
                    SOUND(e, 210);
                if (speed != 0 && e->unk10 <= 2048 && e->unk18 <= 2048)
                    func_8003ead0();
                if (e->unk1C <= 2048 && e->unk10 <= 2048) {
                    int push = 0;

                    switch (rec->unk12) {
                    case 0:
                        if ((s16)e->unk22 > -1024 && (s16)e->unk22 < 1024)
                            push = 1;
                        break;
                    case 1:
                        if (e->unk22 > 0 && e->unk22 < 2048)
                            push = 1;
                        break;
                    case 2:
                        if (e->unk22 > 1024 && e->unk22 < 3072)
                            push = 1;
                        break;
                    case 3:
                        if (e->unk22 > 2048 && e->unk22 < 4096)
                            push = 1;
                        break;
                    case 5:
                        push = 1;
                        break;
                    case 4:
                        break;
                    }
                    if (speed != 0 && push) {
                        short away = e->unk20;

                        func_8003ead0();
                        away -= 1024;
                        vis_table_b.unk71A4 += rcos(away) / 32;
                        vis_table_b.unk71A8 += rsin(away) / 32;
                    }
                }
                break;
            case 2:
                if (e->unk49 == 252 || e->unk49 == 250) {
                    e->unk0C = 255;
                    break;
                }
                if (e->unk1C < 8192)
                    break;
                if (!(e->unk49 & 0x80)) {
                    e->unk02 = e->unk49 == 0;
                    e->unk10 = 0;
                }
                e->unk0C = 3;
                SOUND(e, 208);
                break;
            case 3:
                if (e->unk49 & 0x80) {
                    if (step * 4 < e->unk10) {
                        e->unk10 = (e->unk10 - speed) & 0xfff;
                        break;
                    }
                    SWAP_RECORDS(e);
                } else {
                    if (e->unk10 + step * 4 < 4095) {
                        e->unk10 = (e->unk10 + speed) & 0xfff;
                        break;
                    }
                    SWAP_RECORDS(e);
                    if (!(e->unk49 & 0x80))
                        e->unk02 = e->unk49;
                }
                e->unk10 = 0;
                e->unk0C = 0;
                SOUND(e, 209);
                break;
            case 7:
                if (e->unk05 == 1)
                    e->unk00 = 2;
                break;
            case 8:
                e->unk00 = 1;
                START(e, step);
                if (speed + e->unk10 < 4095) {
                    e->unk10 = (e->unk10 + speed) & 0xfff;
                } else {
                    e->unk10 = 4095;
                    e->unk00 = 0;
                    e->unk0C = 255;
                }
                break;
            case 11:
                break;
            case 12:
                START(e, step);
                e->unk10 += speed;
                if (speed + e->unk10 >= 4095) {
                    e->unk10 = 4095;
                    e->unk00 = 0;
                    e->unk0C = 255;
                }
                break;
            case 255:
                e->unk10 = 4095;
                break;
            }
            break;
        }
        case 50:
        case 51: {
            int step = e->unk48;
            short speed = step * 4;

            switch (e->unk0C) {
            case 0:
                if (e->unk05 == 51)
                    func_8004ee48(e->unk4A.snd.unk4C, 0);
                if (e->unk49 >= 249 && e->unk49 <= 250 && e->unk1C <= 8192)
                    e->unk0C = 1;
                break;
            case 1:
                if (e->unk10 == 0 && e->unk10 + step * 4 > 0) {
                    SOUND(e, 208);
                    if (e->unk05 == 51)
                        func_8004ee48(e->unk4A.snd.unk4C, 1);
                }
                if (e->unk10 < 3072 && speed + e->unk10 >= 3072)
                    SOUND(e, 210);
                if (speed + e->unk10 < 4095) {
                    e->unk10 = (e->unk10 + speed) & 0xfff;
                } else {
                    e->unk10 = 4095;
                    e->unk0C = 2;
                }
                break;
            case 2:
                if (e->unk49 == 252 || e->unk49 == 250) {
                    e->unk0C = 255;
                    break;
                }
                if (e->unk1C < 8192)
                    break;
                if (!(e->unk49 & 0x80)) {
                    e->unk02 = e->unk49 == 0;
                    e->unk10 = 0;
                }
                e->unk0C = 3;
                SOUND(e, 208);
                break;
            case 3:
                if (e->unk49 & 0x80) {
                    if (speed < e->unk10) {
                        e->unk10 = (e->unk10 - e->unk48) & 0xfff;
                        break;
                    }
                } else {
                    if (speed + e->unk10 < 4095) {
                        e->unk10 = (e->unk10 + speed) & 0xfff;
                        break;
                    }
                    e->unk02 = e->unk49;
                }
                e->unk10 = 0;
                e->unk0C = 0;
                break;
            case 4:
                START(e, step);
                e->unk10 += speed;
                if (e->unk10 >= 4095)
                    e->unk10 &= 0xfff;
                break;
            case 5:
                if (world_state.tint_level < 54 || H50(e) >= 600) {
                    e->unk0C = 6;
                    H50(e) = 0;
                    SOUND(e, 208);
                } else {
                    H50(e)++;
                }
                break;
            case 6:
                e->unk10 += speed;
                if (e->unk10 + step * 4 >= 4095) {
                    e->unk10 = 0;
                    e->unk0C = 5;
                }
                break;
            case 8:
                START(e, step);
                if (speed + e->unk10 < 4095) {
                    e->unk10 = (e->unk10 + speed) & 0xfff;
                } else {
                    e->unk10 = 4095;
                    e->unk0C = 255;
                }
                break;
            case 9:
                if ((rand() >> 12) == 0) {
                    if (H50(e) % 20 == 0) {
                        e->unk0C = 10;
                        e->unk00 = 0;
                        H50(e) = 0;
                        SOUND(e, 208);
                    }
                } else {
                    H50(e)++;
                }
                break;
            case 10:
                e->unk10 += speed;
                if (e->unk10 + step * 4 >= 4096) {
                    e->unk10 = 0;
                    e->unk00 = 0;
                    e->unk0C = 9;
                }
                break;
            case 11:
                break;
            case 12:
                START(e, step);
                e->unk10 += speed;
                if (speed + e->unk10 >= 4095) {
                    e->unk10 = 4095;
                    e->unk00 = 0;
                    e->unk0C = 255;
                }
                break;
            case 255:
                e->unk10 = 4095;
                break;
            }
            break;
        }
        case 52: {
            int step = e->unk48;
            short speed = step * 4;
            Vis88 *o;
            int j;
            int group;

            switch (e->unk0C) {
            case 0:
                break;
            case 1:
                START(e, step);
                if (speed + e->unk10 < 4095) {
                    e->unk10 = (e->unk10 + speed) & 0xfff;
                    break;
                }
                if (e->unk49 != 255 && e->unk49 != 252) {
                    e->unk02 = 1;
                    e->unk10 = 0;
                }
                e->unk0C = 255;
                if (e->unk4A.snd.unk4B == 255)
                    break;
                group = e->unk4A.snd.unk4B;
                for (j = 0, o = vis_table_b.entries; j < 240; j++, o++) {
                    if ((o->unk05 == 0 || o->unk05 == 50) && o->unk4A.door.unk4F == group) {
                        if (o->unk0C == 0) {
                            o->unk49 = 252;
                            o->unk0C = 1;
                            o->unk02 = 0;
                            o->unk10 = 0;
                        } else if (o->unk0C == 255) {
                            o->unk0C = 3;
                            o->unk02 = 1;
                            o->unk10 = 0;
                            SOUND(o, 208);
                        }
                    }
                }
                break;
            case 3:
                START(e, step);
                if (speed + e->unk10 < 4095) {
                    e->unk10 = (e->unk10 + speed) & 0xfff;
                    break;
                }
                e->unk02 = 0;
                e->unk10 = 0;
                e->unk0C = 0;
                if (e->unk4A.snd.unk4B == 255)
                    break;
                group = e->unk4A.snd.unk4B;
                for (j = 0, o = vis_table_b.entries; j < 240; j++, o++) {
                    if (o->unk05 == 0 && o->unk4A.door.unk4F == group) {
                        if (o->unk0C == 0) {
                            o->unk49 = 252;
                            o->unk0C = 1;
                            o->unk02 = 0;
                            o->unk10 = 0;
                        } else if (o->unk0C == 255) {
                            o->unk49 = 0;
                            o->unk0C = 3;
                            o->unk02 = 1;
                            o->unk10 = 0;
                            SOUND(o, 208);
                        }
                    }
                }
                break;
            }
            break;
        }
        case 2: {
            u_long a = rand();
            u_long b;
            u_long c;

            a >>= 8;
            b = rand();
            b >>= 6;
            c = rand();
            c >>= 8;
            if (B50(e))
                B50(e)--;
            if (e->unk0C == 0) {
                if (e->unk1C < 1024 && B50(e) == 0) {
                    memset(dmg, 0, 18);
                    dmg[3] = 100;
                    player_hit(dmg, 4096, 0, (long *)&e->unk24);
                    B50(e) = 20;
                }
                if (e->unk3C.vy > 4096) {
                    e->unk3C.vx = e->unk3C.vx - 512 - a;
                    e->unk3C.vz = e->unk3C.vz - 512 - c;
                } else {
                    e->unk3C.vx = a + (e->unk3C.vx + 512);
                    e->unk3C.vz = c + (e->unk3C.vz + 512);
                }
                e->unk3C.vy = b + (e->unk3C.vy + 1256);
                if (e->unk3C.vy > 8192)
                    e->unk0C = 1;
            } else if (e->unk0C == 1) {
                e->unk3C.vx = 0;
                e->unk3C.vy = 0;
                e->unk3C.vz = 0;
                e->unk0C = 2;
            } else if (rand() < 2048 && B50(e) == 0) {
                e->unk0C = 0;
            }
            break;
        }
        case 3:
            func_80050468(e, (u16)(240 - i));
            break;
        case 4:
        case 8:
            if (e->unk48)
                e->unk48--;
            if (e->unk1C < e->unk3C.vx / 4 && e->unk48 == 0) {
                if (e->unk05 == 4) {
                    memset(dmg, 0, 18);
                    dmg[3] = 180;
                    player_hit(dmg, 4096, 0, (long *)&e->unk24);
                    e->unk48 = 20;
                } else if (e->unk05 == 8) {
                    memset(dmg2, 0, 18);
                    dmg2[6] = 135;
                    player_hit(dmg2, 4096, 5120, (long *)&e->unk24);
                    e->unk48 = 20;
                }
            }
            if (e->unk0C >= 5) {
                e->unk24 = vis_table_b.entries[H50(e)].unk24;
                e->unk24.vy -= 1280;
                e->unk3C.vx = e->unk3C.vy = e->unk3C.vz = 0;
                H52(e) = rcos(e->unk36 - 1024) * 384 / 4096;
                e->unk54 = 0;
                e->unk56 = rsin(e->unk36 - 1024) * 384 / 4096;
                e->unk24.vx += H52(e);
                e->unk24.vz += e->unk56;
                e->unk0C = 0;
            } else if (e->unk0C >= 3) {
                H52(e) = e->unk0C * (rcos(e->unk36 - 1024) << 7) / 4096;
                e->unk54 = e->unk0C << 5;
                e->unk56 = e->unk0C * (rsin(e->unk36 - 1024) << 7) / 4096;
                e->unk24.vx += H52(e) - rand() / 256;
                e->unk24.vy += e->unk54 + rand() / 2048;
                e->unk24.vz += e->unk56 - rand() / 256;
                e->unk3C.vx += 1024;
                e->unk3C.vy += 1024;
                e->unk3C.vz += 1024;
                e->unk0C++;
            } else {
                H52(e) = e->unk0C * (rcos(e->unk36 - 1024) << 7) / 4096;
                e->unk54 = e->unk0C << 6;
                e->unk56 = e->unk0C * (rsin(e->unk36 - 1024) << 7) / 4096;
                e->unk24.vx += H52(e) + rand() / 256;
                e->unk24.vy += e->unk54 + rand() / 2048;
                e->unk24.vz += e->unk56 + rand() / 256;
                e->unk3C.vx += 1024;
                e->unk3C.vy += 1024;
                e->unk3C.vz += 1024;
                e->unk0C++;
            }
            break;
        case 5:
            func_80050564(e, (u16)(240 - i));
            break;
        case 6:
            if (e->unk48)
                e->unk48--;
            if (e->unk1C < e->unk3C.vx / 4 && e->unk48 == 0) {
                memset(dmg, 0, 18);
                dmg[3] = 100;
                player_hit(dmg, 4096, 0, (long *)&e->unk24);
                e->unk48 = 20;
            }
            if (e->unk0C >= 5) {
                e->unk24 = vis_table_b.entries[H50(e)].unk24;
                e->unk24.vx -= rcos(e->unk36) * 1280 / 4096;
                e->unk24.vy -= 2000;
                e->unk24.vz -= rsin(e->unk36) * 1280 / 4096;
                e->unk3C.vx = e->unk3C.vy = e->unk3C.vz = 0;
                H52(e) = rcos(e->unk36) * 384 / 4096;
                e->unk54 = 128;
                e->unk56 = rsin(e->unk36) * 384 / 4096;
                e->unk24.vx += H52(e);
                e->unk24.vz += e->unk56;
                e->unk0C = 0;
            } else if (e->unk0C >= 3) {
                H52(e) = e->unk0C * (rcos(e->unk36) << 7) / 4096;
                e->unk54 = e->unk0C << 6;
                e->unk56 = e->unk0C * (rsin(e->unk36) << 7) / 4096;
                e->unk24.vx += H52(e) - rand() / 256;
                e->unk24.vy += e->unk54 + rand() / 2048;
                e->unk24.vz += e->unk56 - rand() / 256;
                e->unk3C.vx += 1024;
                e->unk3C.vy += 1024;
                e->unk3C.vz += 1024;
                e->unk0C++;
            } else {
                H52(e) = e->unk0C * (rcos(e->unk36) << 7) / 4096;
                e->unk54 = e->unk0C << 7;
                e->unk56 = e->unk0C * (rsin(e->unk36) << 7) / 4096;
                e->unk24.vx += H52(e) + rand() / 256;
                e->unk24.vy += e->unk54 + rand() / 2048;
                e->unk24.vz += e->unk56 + rand() / 256;
                e->unk3C.vx += 1024;
                e->unk3C.vy += 1024;
                e->unk3C.vz += 1024;
                e->unk0C++;
            }
            break;
        case 7:
            func_8005066c(e, (u16)(240 - i));
            break;
        case 224: {
            Vis212 *a;
            int j;
            int rising;

            if (func_8004fc94(e->unk24.vx >> 12, e->unk24.vz >> 12, 1, 1, e->unk24.vy) == 0
                && area_state.unk0 == 0)
                break;
            if ((DAT_801cd180.unk2 & DAT_801cd180.unk22[10]) && vis_table_a.unk6169 == 0
                && W50(e) != 0xffffff && W48(e) != 0xffffff) {
                vis_table_a.unk6169 = 1;
                func_8003ead0();
                func_80057a3c();
            }
            if (vis_table_a.unk6169 == 1) {
                a = &vis_table_a.entries[56];
                rising = 0;
                func_80015ad4();
                viewer.unk21E = 1;
                for (j = 7; j != -1; j--, a++) {
                    a->unk2A = 0;
                    if (a->unk00 != 255) {
                        func_80058220(a->unk0B * 10 + 48, &a->unk74, 127, (rand() * 5 >> 15) - 2);
                        a->unkB0 = 128;
                        DAT_800881e0.vx = rcos((s16)a->unk56 - 1024) / 256;
                        DAT_800881e0.vz = rsin((s16)a->unk56 - 1024) / 256;
                        rising = 1;
                    }
                }
                if (rising)
                    vis_table_a.unk6169 = 2;
            }
            if (vis_table_a.unk6169 == 2) {
                func_8003ead0();
                a = &vis_table_a.entries[56];
                for (j = 7; j != -1; j--, a++) {
                    vis_table_b.unk71A4 = DAT_800881e0.vx;
                    vis_table_b.unk71A8 = DAT_800881e0.vz;
                    if (a->unk00 != 255 && (s16)a->unk2A + a->unkB0 > 3584) {
                        a->unkB0 = 0;
                        a->unk2A = 4095;
                        vis_table_a.unk6169 = 3;
                        func_80057a3c();
                    }
                }
                break;
            }
            if (vis_table_a.unk6169 == 3) {
                DAT_800881e0.vx = 0;
                DAT_800881e0.vz = 0;
            } else if (W50(e) != 0xffffff) {
                break;
            }
            vis_table_a.unk6169 = 4;
            func_80045044(e->unk4A.snd.unk4D, e->unk4A.snd.unk4D, e->unk4A.snd.unk4D, e->unk4A.snd.unk4D,
                          e->unk4A.snd.unk4D, (s8)B50(e), B52(e), B51(e), (s8)e->unk4A.snd.unk4B,
                          (s8)e->unk4A.door.unk4F, (s8)e->unk4A.snd.unk4C);
            if (W48(e) != 0xffffff) {
                vis_table_b.unk71A4 = -DAT_800881e0.vx * 2;
                vis_table_b.unk71A8 = -DAT_800881e0.vz * 2;
                viewer.position.vx = e->unk24.vx - 2048;
                viewer.position.vy = e->unk24.vy - 1600;
                viewer.unk2B8.vx = 0;
                viewer.unk2B8.vz = 0;
                viewer.position.vz = e->unk24.vz - 2048;
            }
            if (e->unk4A.door.unk4E != 255)
                viewer.unk278.vy += e->unk4A.door.unk4E << 10;
            break;
        }
        case 225:
            break;
        case 237:
        case 239: {
            StWarp *here;
            Vis88 *o;
            int j;
            u8 x;
            u8 y;
            u8 z;
            u8 area;

            if (func_8004fd28(e->unk24.vx, e->unk24.vz, 3072, 3072, e->unk24.vy) == 0)
                goto away;
            turn = (viewer.unk26A - e->unk20) & 0xfff;
            if (turn > 2048)
                turn |= 0xf000;
            if (!((DAT_801cd180.unk2 & DAT_801cd180.unk22[10]) && (s16)turn > -400 && (s16)turn < 400)
                && e->unk05 != 239)
                goto away;
            if (B50(e))
                break;
            here = (StWarp *)viewer.unk220;
            here->x = viewer.position.vx >> 12;
            here->area = area_state.unk4;
            here->y = viewer.position.vy >> 12;
            here->z = viewer.position.vz >> 12;
            here->dx = viewer.position.vx - (here->x << 12);
            here->dy = viewer.position.vy - (here->y << 12);
            here->dz = viewer.position.vz - (here->z << 12);
            here->yaw = e->unk36 + 2048;
            for (j = 240, o = vis_table_b.entries; j != 0; j--, o++) {
                if (o->unk05 == e->unk05)
                    B50(o) = 1;
            }
            x = e->unk49 + 64;
            z = e->unk4A.snd.unk4B + 64;
            y = e->unk4A.snd.unk4A + 64;
            func_80057c54();
            func_800500ac(2, 0, 0x1000, 0x100);
            area = e->unk48;
            do {
                func_8001531c();
                func_800456f8();
            } while (area_state.unk0 != 0);
            viewer.unk278.vx = 0;
            viewer.unk278.vy = e->unk4A.door.unk4F << 10;
            if (area != area_state.unk4) {
                func_80045044(area, area, area, 0xff, 0xff, 127, 127, 127, 0, 0, 0);
            } else {
                func_80045044(0xff, 0xff, area, 0xff, 0xff, 127, 127, 127, 0, 0, 0);
                viewer.unk21C = 1;
                func_8004e2d8();
                vis_table_a.unk6158[0] = vis_table_a.unk6158[1] = vis_table_a.unk6158[2] =
                    vis_table_a.unk6158[3] = 0xff;
                vis_table_a.unk6103 = 0xff;
            }
            viewer.unk218 = 0;
            viewer.unk28C = 0;
            viewer.unk28A = 0;
            viewer.unk288 = 0;
            *(SVECTOR *)&viewer.unk268 = viewer.unk278;
            viewer.position.vx = (x << 12) + ((s8)e->unk4A.snd.unk4C << 7);
            viewer.position.vz = (z << 12) + ((s8)e->unk4A.door.unk4E << 7);
            viewer.unk2F0 = 0;
            viewer.unk2EC = 0;
            viewer.position.vy = (y << 12) + ((s8)e->unk4A.snd.unk4D << 7) - 1600;
            func_8003ead0();
            vis_table_b.unk71A8 = 0;
            vis_table_b.unk71A6 = 0;
            vis_table_b.unk71A4 = 0;
            vis_lists.unk47D2 = 0;
            vis_lists.unk47D0 = 0;
            vis_lists.unk47CE = 0;
            vis_table_a.unk6164 = 0;
            vis_table_a.unk6162 = 0;
            vis_table_a.unk6160 = 0;
            viewer.unk1D0 = 0;
            viewer.unk1CE = 0;
            viewer.unk1CC = 0;
            do {
                func_8001531c();
                func_800456f8();
            } while (area_state.unk0 != 0);
            vis_lists.n_sources = 0;
            vis_collect_sources((VECTOR *)&viewer.position.vx, NULL, NULL);
            if (vis_lists.n_sources)
                viewer.unk174 = &vis_records[vis_lists.sources[0]];
            func_8004e324();
            func_800500ac(2, 0x1000, 0x1000, 0);
            func_80015ad4();
            func_80057c54();
            func_800500ac(2, 0x1000, 0, -0x100);
            if (area < 44)
                func_80045044(0xff, 0xff, 0xff, area, area, 127, 127, 127, 0, 0, 0);
            lighting_clear_requests();
            break;
        away:
            B50(e) = 0;
            break;
        }
        default:
            area_state.unkC->unk24();
            break;
        }
    }
    vis_table_b.unk71B0 = NULL;
    vis_table_b.unk71A4 /= 2;
    vis_table_b.unk71A6 /= 2;
    vis_table_b.unk71A8 /= 2;
    if (vis_table_b.unk71A4 > -4 && vis_table_b.unk71A4 < 4)
        vis_table_b.unk71A4 = 0;
    if (vis_table_b.unk71A6 > -4 && vis_table_b.unk71A6 < 4)
        vis_table_b.unk71A6 = 0;
    if (vis_table_b.unk71A8 > -4 && vis_table_b.unk71A8 < 4)
        vis_table_b.unk71A8 = 0;
}
