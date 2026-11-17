/*
 * VS.EXE: the versus start-up module (WIP): the round loop, the menu
 * resources load, the end-of-match screens and main. func_800140d8,
 * func_80014480/func_80014634 and func_80015020 before it are separate
 * units (src/vs/fighter_800140d8.c, src/vs/camera_80014480.c,
 * src/vs/frame_80015020.c) because 0x800140d8, 0x80014634 and 0x80015020
 * are not exact. $gp-relative accesses to the round state, so -G8.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>

/* The first side's picked fighter and rounds won: one-word tentative
 * definitions ahead of vs/data.h, whose shared declarations leave the
 * extent open (func_80021984 and func_80029f28 index both sides from
 * them), so that ASPSX addresses them $gp-relative. */
s32 DAT_80063330[1];
s32 DAT_80063370[1];

#include <psyq/kernel.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBETC.H>
#include <LIBCD.H>
#include <LIBPAD.H>
#include <LIBSND.H>
#include <LIBMCRD.H>
#include <st/data.h>
#include <vs/data.h>
#include <vs/buffer.h>
#include <vs/main.h>
#include <vs/menu.h>
#include <vs/pad.h>
#include <vs/sound.h>
#include <vs/funcs.h>

/* Top of the heap: main RAM below the 8 KB stack. */
#define HEAP_TOP ((u8 *)0x801fe000)

/* The heap starts at the first 2 KB boundary past the load image. */
#define HEAP_ALIGN 2048

/* Frames each screen fades over. */
#define FADE_FRAMES 20

/* DAT_80063324: the round loop's screens (func_80015020 draws them). */
#define SCREEN_FADE_IN 16
#define SCREEN_SAVE_ASK 17
#define SCREEN_RETRY_ASK 18
#define SCREEN_FADE_OUT 19
#define SCREEN_SAVE 20

/* DAT_80063324: the round's phase. */
#define PHASE_START 0
#define PHASE_FIGHT 1
#define PHASE_PAUSE 2
#define PHASE_TIME_UP 5
#define PHASE_DRAW 4
#define PHASE_KO_SECOND 6
#define PHASE_KO_FIRST 7
#define PHASE_FIRST_WINS 8
#define PHASE_SECOND_WINS 9

/* Frames a round's result shows. */
#define RESULT_FRAMES 240

int func_8002ee90();

/* The module's initialized small data. */
DATA(0x80063220, 0x4, ".sdata")
s32 DAT_80063220 = 0;
DATA(0x80063224, 0x4, ".sdata")
s32 DAT_80063224 = 0;
DATA(0x80063228, 0x4, ".sdata")
s32 DAT_80063228 = 1;

/* The module's RECT initializer. */
SDATA(0x8006322c, 0x8)

/* The frame statistics' format strings and main's file name. */
RODATA(0x80011050, 0x50)

/* Tentative definitions: ASPSX needs the size to address these $gp-relative.
 * An earlier object allocates them (docs/executable-linking.md). */
s32 DAT_80063310;
u32 DAT_80063314;
s32 DAT_8006331c;
s32 DAT_80063320;
s32 DAT_80063324;
s32 DAT_80063338;
s32 DAT_80063354;
s16 DAT_8006335a;
u32 DAT_80063364;
s32 DAT_8006336c;

/*
 * Plays one round: loads the fighters, places the camera, starts the
 * round's music, then runs the frame loop: the pause menu (resume or
 * quit), the round clock, knock-outs and the result, until the result has
 * shown for RESULT_FRAMES frames. Button 1 with 8 held toggles the frame
 * statistics, button 1 held for a second calls func_800138a0.
 */
ADDRESS(0x80015e50, 0x638)
void func_80015e50(void)
{
    int choice;
    int ko;
    int side;

    DAT_80063354 = 100;
    DAT_80063324 = PHASE_START;
    DAT_80063364 = 0;
    if (DAT_80063330[0] != -1 && DAT_80063334 != -1)
        DAT_80063320 = 1;
    else
        DAT_80063320 = 0;
    DAT_80063310 = 0;
    func_800140d8();
    func_800352cc();
    func_8002d710();
    DAT_801624d0.unk2CA78.vy = -14000;
    DAT_801624d0.unk2CA78.vx = 0;
    DAT_801624d0.unk2CA78.vz = -15000;
    DAT_801624d0.unk2CA88.vx = 0;
    DAT_801624d0.unk2CA88.vy = 0;
    DAT_801624d0.unk2CA88.vz = 0;
    if (DAT_80063338 == 0) {
        func_80016d94();
        DrawSync(0);
    }
    func_800129e8(DAT_80063338);
    for (;;) {
        func_80014634();
        if (DAT_80063324 == PHASE_PAUSE) {
            choice = 0;
            DAT_8006336c = 0;
            for (;;) {
                if ((DAT_801603c0.pads[0].unk8 | DAT_801603c0.pads[1].unk8) & 0x10)
                    choice = 0;
                if ((DAT_801603c0.pads[0].unk8 | DAT_801603c0.pads[1].unk8) & 0x40)
                    choice = 1;
                if ((DAT_801603c0.pads[0].unk8 | DAT_801603c0.pads[1].unk8) & 9) {
                    DAT_80063364 += DAT_801603c0.unk3C;
                    DAT_80063324 = PHASE_FIGHT;
                    DAT_80063314 = DAT_801603c0.unk3C;
                    while (!CdRead2(CdlModeSpeed | CdlModeRT | CdlModeSF))
                        ;
                    break;
                }
                if ((DAT_801603c0.pads[0].unk8 | DAT_801603c0.pads[1].unk8) & 0x4000) {
                    if (choice == 0) {
                        DAT_80063364 += DAT_801603c0.unk3C;
                        DAT_80063324 = PHASE_FIGHT;
                        DAT_80063314 = DAT_801603c0.unk3C;
                        while (!CdRead2(CdlModeSpeed | CdlModeRT | CdlModeSF))
                            ;
                        break;
                    }
                    DAT_80063338 = DAT_80063228;
                    DAT_80063370[0] = 0;
                    DAT_80063374 = 0;
                    return;
                }
                if (choice) {
                    if (++DAT_8006336c > 10)
                        DAT_8006336c = 10;
                } else {
                    if (--DAT_8006336c < 0)
                        DAT_8006336c = 0;
                }
                DAT_801603c0.pads[0].unk8 = 0;
                DAT_801603c0.pads[1].unk8 = 0;
                func_80015020();
            }
        } else if (DAT_80063324 == PHASE_FIGHT
                   && ((DAT_801603c0.pads[0].unk8 | DAT_801603c0.pads[1].unk8) & 9)) {
            while (!CdControl(CdlPause, 0, 0))
                ;
            DAT_8006331c = 12;
            DAT_80063324 = PHASE_PAUSE;
            DAT_80063364 -= DAT_801603c0.unk3C;
            DAT_80063314 = DAT_801603c0.unk3C;
        }
        if ((DAT_801603c0.pads[0].unk8 & 1) && (DAT_801603c0.pads[0].unk0 & 8))
            DAT_801603c0.unk28 = 1 - DAT_801603c0.unk28;
        if (DAT_801603c0.pads[0].unk0 & 1) {
            if (++DAT_801603c0.unk2C >= 60)
                func_800138a0();
        } else {
            DAT_801603c0.unk2C = 0;
        }
        if (DAT_80063324 == PHASE_FIGHT) {
            if (DAT_80063364 <= DAT_801603c0.unk3C) {
                DAT_80063324 = PHASE_TIME_UP;
                DAT_80063314 = DAT_801603c0.unk3C;
            } else {
                if (DAT_800633b0.fighter[0].unk4DF08 == 0) {
                    if (DAT_800633b0.fighter[1].unk4DF08 == 0) {
                        DAT_80063324 = PHASE_DRAW;
                        DAT_80063364 -= DAT_801603c0.unk3C;
                        DAT_80063314 = DAT_801603c0.unk3C;
                    } else {
                        ko = 0;
                        side = 1;
                        DAT_80063324 = PHASE_SECOND_WINS;
                        DAT_80063364 -= DAT_801603c0.unk3C;
                        DAT_80063314 = DAT_801603c0.unk3C;
                        func_8001336c(ko, side);
                    }
                } else if (DAT_800633b0.fighter[1].unk4DF08 == 0) {
                    ko = 1;
                    side = 1;
                    DAT_80063324 = PHASE_FIRST_WINS;
                    DAT_80063364 -= DAT_801603c0.unk3C;
                    DAT_80063314 = DAT_801603c0.unk3C;
                    func_8001336c(ko, side);
                }
                if (func_8002e510(0)) {
                    DAT_80063324 = PHASE_KO_FIRST;
                    DAT_80063364 -= DAT_801603c0.unk3C;
                    DAT_80063314 = DAT_801603c0.unk3C;
                }
                if (func_8002e510(1)) {
                    DAT_80063324 = PHASE_KO_SECOND;
                    DAT_80063364 -= DAT_801603c0.unk3C;
                    DAT_80063314 = DAT_801603c0.unk3C;
                }
            }
        } else if (DAT_80063324 == PHASE_DRAW || DAT_80063324 == PHASE_FIRST_WINS
                   || DAT_80063324 == PHASE_SECOND_WINS) {
            func_8001381c();
            if (DAT_801603c0.unk3C - DAT_80063314 > RESULT_FRAMES) {
                if (DAT_80063324 == PHASE_FIRST_WINS)
                    DAT_80063370[0]++;
                else if (DAT_80063324 == PHASE_SECOND_WINS)
                    DAT_80063374++;
                DAT_80063338++;
                return;
            }
        }
        func_800352fc();
        if (DAT_80063324 != PHASE_START && DAT_80063324 != PHASE_PAUSE) {
            func_80031478(DAT_80063364 & 1);
            func_80031478(1 - (DAT_80063364 & 1));
            func_8002dcd8();
        }
        DAT_801603c0.pads[0].unk8 = 0;
        DAT_801603c0.pads[1].unk8 = 0;
        func_80015020();
        func_80013c5c();
        DAT_801624d0.unk831C[1] = 0;
        DAT_801624d0.unk8324 = 0;
        if (DAT_801603c0.unk40 + 59 < DAT_801603c0.unk3C) {
            DAT_801624d0.unk2C971 = DAT_801624d0.unk2C970 * 60
                                    / (DAT_801603c0.unk3C - DAT_801603c0.unk40);
            DAT_801624d0.unk2C970 = 0;
            DAT_801603c0.unk40 = DAT_801603c0.unk3C;
        }
        if (DAT_801603c0.unk28 == 1) {
            FntPrint("frm      =%d\n", DAT_801624d0.unk2C971);
            FntPrint("frm/draw =%d\n", DAT_80063220);
            FntPrint("calc_poly=%d\n", DAT_801624d0.unk831C[1]);
            FntPrint("draw_poly=%d\n", DAT_801624d0.unk8324);
        }
    }
}

/* Loads the menu screens' resources: the item names (archive 8 member 2)
 * with the picture behind them, the font pages (archive 5 member 10) and
 * the four files func_8002345c sets up, then clears the round state. */
ADDRESS(0x80016488, 0x16c)
void func_80016488(void)
{
    RECT pos;
    RECT clut;
    RECT unused = {0, 0, 1023, 511};
    u_long *p;

    pos.x = 640;
    pos.y = 0;
    clut.x = 640;
    clut.y = 480;
    p = DAT_80117708;
    func_8002ee90(8, 2, p);
    func_8002e630((u32 *)DAT_80196230[1].offsets, (u32 *)(p + 1), 0x1500);
    p = (u_long *)((u8 *)p + (p[0] + 4));
    func_80012c2c(p + 1, (DVECTOR *)&pos, (DVECTOR *)&clut);
    DrawSync(0);
    func_8002ee90(5, 10, DAT_80117708);
    func_80012b50(DAT_80117708, 0, 0);
    DrawSync(0);
    func_8002ee90(5, 0, DAT_80117708);
    func_8002345c((ModelFile *)DAT_80117708);
    func_8002ee90(5, 1, (u_long *)DAT_8015c528);
    func_8002345c((ModelFile *)DAT_8015c528);
    func_8002ee90(5, 2, (u_long *)DAT_8015d8b0);
    func_8002345c((ModelFile *)DAT_8015d8b0);
    func_8002ee90(5, 3, (u_long *)DAT_8015ec38);
    func_8002345c((ModelFile *)DAT_8015ec38);
    DAT_80063370[0] = 0;
    DAT_80063374 = 0;
    DAT_80063338 = 0;
}

/*
 * The end of a match: fades in, then asks whether to save (when a card is
 * ready; the winner side passed) or to retry, and repeats until the
 * player picks retry or the save is done. Left/right (0x10/0x40) move the
 * choice, 0x4000 confirms; the choice also slides DAT_8006336c (0..10).
 */
ADDRESS(0x800165f4, 0x248)
void func_800165f4(void)
{
    int asked = 0;
    int choice;
    int bits;
    int i;

    StartCARD();
    _bu_init();
    func_8001381c();
    DAT_80063324 = SCREEN_FADE_IN;
    DAT_80063338--;
    DAT_80063314 = DAT_801603c0.unk3C;
    for (i = FADE_FRAMES - 1; i != -1; i--) {
        func_80015020();
        DrawSync(0);
    }
    for (;;) {
        if (func_8001c530() == 0) {
            DAT_80063324 = SCREEN_SAVE_ASK;
        } else {
            choice = 0;
            DAT_80063324 = SCREEN_RETRY_ASK;
            DAT_8006336c = 0;
        }
        DAT_80063314 = DAT_801603c0.unk3C;
        for (i = FADE_FRAMES - 1; i != -1; i--) {
            func_80015020();
            DrawSync(0);
        }
        if (DAT_80063324 == SCREEN_SAVE_ASK) {
            if (func_8001c5e0(DAT_80063370[0] < DAT_80063374, asked) == 0) {
                DrawSync(0);
                return;
            }
            asked = 1;
            choice = 0;
            DAT_80063324 = SCREEN_SAVE;
            DAT_8006336c = 0;
            DAT_80063314 = DAT_801603c0.unk3C;
            for (i = FADE_FRAMES - 1; i != -1; i--) {
                func_80015020();
                DrawSync(0);
            }
        }
        DAT_801603c0.pads[0].unk8 = 0;
        DAT_801603c0.pads[1].unk8 = 0;
        for (;;) {
            bits = DAT_801603c0.pads[0].unk8 | DAT_801603c0.pads[1].unk8;
            if (bits & 0x10)
                choice = 0;
            if (bits & 0x40)
                choice = 1;
            if (bits & 0x4000) {
                if (choice) {
                    DrawSync(0);
                    return;
                }
                break;
            }
            if (choice) {
                if (++DAT_8006336c > 10)
                    DAT_8006336c = 10;
            } else {
                if (--DAT_8006336c < 0)
                    DAT_8006336c = 0;
            }
            DAT_801603c0.pads[0].unk8 = 0;
            DAT_801603c0.pads[1].unk8 = 0;
            func_80015020();
            DrawSync(0);
        }
        DAT_80063324 = SCREEN_FADE_OUT;
        DAT_80063314 = DAT_801603c0.unk3C;
        for (i = FADE_FRAMES - 1; i != -1; i--) {
            func_80015020();
            DrawSync(0);
        }
    }
}

/*
 * VS.EXE's main: the heap, controllers, CD and memory card, the two root
 * counter events, the graphics, the loading picture and the menu sound
 * bank; then the character select / match loop until func_80016f10
 * declines, and the shutdown back to TITLE.EXE (next program 1).
 * Not exact (size-exact): retail schedules the second addiu of
 * &DAT_800633b0 before the loading picture's position stores (first
 * divergence at +0x16c).
 */
ADDRESS(0x8001683c, 0x40c)
int main(void)
{
    u8 *heap;
    u_char result[8];
    DVECTOR pos;
    DVECTOR clut;
    DVECTOR pos2;
    DVECTOR clut2;
    u_long *picture;

    ResetCallback();
    heap = DAT_80011f40 + ((DAT_80011f48 - DAT_80011f40 + HEAP_ALIGN - 1) & ~(HEAP_ALIGN - 1));
    DAT_80011f4c = heap;
    InitHeap((unsigned long *)heap, HEAP_TOP - heap);
    InitPAD((char *)&DAT_801961e0[0], 16, (char *)&DAT_801961e0[1], 16);
    StartPAD();
    while (CdInit() == 0)
        CdSync(0, result);
    CdSetDebug(0);
    InitCARD(1);
    ExitCriticalSection();
    DAT_801603c0.unk44 = OpenEvent(RCntCNT3, EvSpINT, EvMdINTR, func_80013868);
    EnableEvent(DAT_801603c0.unk44);
    SetRCnt(RCntCNT3, 60000, RCntMdINTR);
    StartRCnt(RCntCNT3);
    DAT_801603c0.unk38 = OpenEvent(RCntCNT2, EvSpINT, EvMdINTR, func_80013848);
    EnableEvent(DAT_801603c0.unk38);
    SetRCnt(RCntCNT2, 60000, RCntMdINTR);
    StartRCnt(RCntCNT2);
    DAT_801603c0.unk28 = 0;
    func_8002f0f4();
    func_80014480();
    pos.vx = 0;
    pos.vy = 300;
    memset(&clut, 0, sizeof(clut));
    func_8002ef3c((u_long *)&DAT_800633b0, "\\LOAD02.TIM;1", 0);
    func_80012c2c((u_long *)&DAT_800633b0, &pos, &clut);
    DrawSync(0);
    func_80013aec();
    func_80014070();
    func_8002ee90(5, 4, DAT_80116580);
    func_8002ee90(5, 5, DAT_800ff580);
    func_8001c9b8();
    func_8002ee90(5, 20, (u_long *)DAT_80191d00.unk0118);
    DAT_801624b8[0].slot[0] = -1;
    DAT_801624b8[0].slot[1] = -1;
    DAT_801624b8[0].slot[2] = -1;
    DAT_801624b8[1].slot[0] = -1;
    DAT_801624b8[1].slot[1] = -1;
    DAT_801624b8[1].slot[2] = -1;
    DAT_80063330[0] = -1;
    DAT_80063334 = -1;
    DAT_80063380 = 0;
    DAT_80063381 = 0;
    while (func_80016f10()) {
        func_80016488();
        do {
            func_80015e50();
            DrawSync(0);
            SsVabClose(DAT_801604b0[0].vab);
            SsVabClose(DAT_801604b0[1].vab);
        } while (DAT_80063338 < DAT_80063228 && DAT_80063370[0] < 2 && DAT_80063374 < 2);
        while (CdControl(CdlStop, 0, 0) == 0)
            ;
        if (DAT_80063320 != 0 && DAT_80063370[0] != DAT_80063374) {
            func_800165f4();
            DAT_80063330[0] = -1;
            DAT_80063334 = -1;
        }
    }
    DrawSync(0);
    picture = (u_long *)&DAT_800633b0;
    pos2.vx = 0;
    pos2.vy = 60;
    memset(&clut2, 0, sizeof(clut2));
    func_80016d94();
    func_8002ef3c(picture, "\\LOAD02.TIM;1", 0);
    func_80012c2c(picture, &pos2, &clut2);
    DrawSync(0);
    *NEXT_PROGRAM_BYTE = 1;
    SsSetMVol(0, 0);
    SsVabClose(DAT_8006335a);
    SsEnd();
    *MAILBOX_BYTE_2 = 0;
    PadStop();
    ResetGraph(3);
    EnterCriticalSection();
    CloseEvent(DAT_801603c0.unk44);
    ExitCriticalSection();
    StopCallback();
}
