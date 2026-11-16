/*
 * VS.EXE: memory card event handling (WIP module). ST.EXE links the same
 * routines (unreconstructed there).
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/kernel.h>
#include <psyq/libc.h>
#include <vs/menu.h>
#include <vs/pad.h>
#include <vs/card.h>
#include <vs/sound.h>
#include <vs/funcs.h>

/* The constant pool's slice for this module: the save file name prefix. */
RODATA(0x80011148, 0x18)

/* The card path pieces ("bu", "0:", "0:*", "*") and a two-byte SJIS digit:
 * short literals, so .sdata under -G8. */
SDATA(0x80063244, 0x13)

/* Clears the four software card events. */
ADDRESS(0x8001a124, 0x54)
void func_8001a124(void)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;

    TestEvent(sp->unk0.card.sw_events[0]);
    TestEvent(sp->unk0.card.sw_events[1]);
    TestEvent(sp->unk0.card.sw_events[2]);
    TestEvent(sp->unk0.card.sw_events[3]);
}

/* Clears the four hardware card events. */
ADDRESS(0x8001a178, 0x54)
void func_8001a178(void)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;

    TestEvent(sp->unk0.card.hw_events[0]);
    TestEvent(sp->unk0.card.hw_events[1]);
    TestEvent(sp->unk0.card.hw_events[2]);
    TestEvent(sp->unk0.card.hw_events[3]);
}

/* Opens and enables the eight card events. */
ADDRESS(0x8001a1cc, 0x190)
void func_8001a1cc(void)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;

    EnterCriticalSection();
    sp->unk0.card.sw_events[0] = OpenEvent(SwCARD, EvSpIOE, EvMdNOINTR, NULL);
    sp->unk0.card.sw_events[1] = OpenEvent(SwCARD, EvSpERROR, EvMdNOINTR, NULL);
    sp->unk0.card.sw_events[2] = OpenEvent(SwCARD, EvSpTIMOUT, EvMdNOINTR, NULL);
    sp->unk0.card.sw_events[3] = OpenEvent(SwCARD, EvSpNEW, EvMdNOINTR, NULL);
    sp->unk0.card.hw_events[0] = OpenEvent(HwCARD, EvSpIOE, EvMdNOINTR, NULL);
    sp->unk0.card.hw_events[1] = OpenEvent(HwCARD, EvSpERROR, EvMdNOINTR, NULL);
    sp->unk0.card.hw_events[2] = OpenEvent(HwCARD, EvSpTIMOUT, EvMdNOINTR, NULL);
    sp->unk0.card.hw_events[3] = OpenEvent(HwCARD, EvSpNEW, EvMdNOINTR, NULL);
    EnableEvent(sp->unk0.card.sw_events[0]);
    EnableEvent(sp->unk0.card.sw_events[1]);
    EnableEvent(sp->unk0.card.sw_events[2]);
    EnableEvent(sp->unk0.card.sw_events[3]);
    EnableEvent(sp->unk0.card.hw_events[0]);
    EnableEvent(sp->unk0.card.hw_events[1]);
    EnableEvent(sp->unk0.card.hw_events[2]);
    EnableEvent(sp->unk0.card.hw_events[3]);
    ExitCriticalSection();
}

/* Disables and closes the eight card events. */
ADDRESS(0x8001a35c, 0xf0)
void func_8001a35c(void)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;

    EnterCriticalSection();
    DisableEvent(sp->unk0.card.sw_events[0]);
    DisableEvent(sp->unk0.card.sw_events[1]);
    DisableEvent(sp->unk0.card.sw_events[2]);
    DisableEvent(sp->unk0.card.sw_events[3]);
    DisableEvent(sp->unk0.card.hw_events[0]);
    DisableEvent(sp->unk0.card.hw_events[1]);
    DisableEvent(sp->unk0.card.hw_events[2]);
    DisableEvent(sp->unk0.card.hw_events[3]);
    CloseEvent(sp->unk0.card.sw_events[0]);
    CloseEvent(sp->unk0.card.sw_events[1]);
    CloseEvent(sp->unk0.card.sw_events[2]);
    CloseEvent(sp->unk0.card.sw_events[3]);
    CloseEvent(sp->unk0.card.hw_events[0]);
    CloseEvent(sp->unk0.card.hw_events[1]);
    CloseEvent(sp->unk0.card.hw_events[2]);
    CloseEvent(sp->unk0.card.hw_events[3]);
    ExitCriticalSection();
}

/* Waits for a software card event: 0 done, 1 error, 2 timeout, 3 new card. */
ADDRESS(0x8001a44c, 0xb4)
int func_8001a44c(void)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    long ioe, error, timeout, new;

    for (;;) {
        ioe = TestEvent(sp->unk0.card.sw_events[0]);
        error = TestEvent(sp->unk0.card.sw_events[1]);
        timeout = TestEvent(sp->unk0.card.sw_events[2]);
        new = TestEvent(sp->unk0.card.sw_events[3]);
        if (ioe == 1)
            return 0;
        if (error == 1)
            return 1;
        if (timeout == 1)
            return 2;
        if (new == 1)
            return 3;
    }
}

/* The same wait on the hardware card events. */
ADDRESS(0x8001a500, 0xb4)
int func_8001a500(void)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    long ioe, error, timeout, new;

    for (;;) {
        ioe = TestEvent(sp->unk0.card.hw_events[0]);
        error = TestEvent(sp->unk0.card.hw_events[1]);
        timeout = TestEvent(sp->unk0.card.hw_events[2]);
        new = TestEvent(sp->unk0.card.hw_events[3]);
        if (ioe == 1)
            return 0;
        if (error == 1)
            return 1;
        if (timeout == 1)
            return 2;
        if (new == 1)
            return 3;
    }
}

/* Checks that the card file of player `port` holds that player's save. */
ADDRESS(0x8001a5b4, 0x128)
void func_8001a5b4(int port)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    char path[32];
    long buf[32];
    int fd;

    strcpy(path, "bu");
    strcat(path, itoa(port));
    strcat(path, "0:");
    strcat(path, DAT_801a0a30[port]);
    fd = open(path, O_RDONLY);
    if (fd == -1) {
        sp->unk1B8->status = 4;
        return;
    }
    lseek(fd, 512, SEEK_SET);
    read(fd, buf, 128);
    close(fd);
    if (DAT_801a0a80[port].unk0 != buf[0])
        sp->unk1B8->status = 4;
    else
        sp->unk1B8->status = 0;
}

/* Scans card `port`: free space when unformatted space is asked for, then
 * the game's save files and the first unused save number. */
ADDRESS(0x8001a6dc, 0x3a8)
void func_8001a6dc(int port)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    struct DIRENTRY dir[15];
    char path[32];
    long used[8];
    int i, n, total;

    sp->unk1B8 = &sp->unk0.card.ports[port];
    if (sp->unk1CC & 0xff00) {
        strcpy(path, "bu");
        strcat(path, itoa(port));
        strcat(path, "0:*");
        sp->unk1B8->count = 0;
        if (firstfile(path, &dir[sp->unk1B8->count])) {
            do {
                sp->unk1B8->count++;
            } while (nextfile(&dir[sp->unk1B8->count]));
        }
        total = 0;
        for (i = 0; i < sp->unk1B8->count; i++)
            total += dir[i].size;
        if (total > 0x1a000)
            sp->unk1B8->unkC8 = 0;
        else
            sp->unk1B8->unkC8 = 1;
    }
    strcpy(path, "bu");
    strcat(path, itoa(port));
    strcat(path, "0:");
    strcat(path, "BASLUS-00863FS_ST");
    strcat(path, "*");
    sp->unk1B8->count = 0;
    if (firstfile(path, &dir[sp->unk1B8->count])) {
        do {
            sp->unk1B8->count++;
        } while (nextfile(&dir[sp->unk1B8->count]));
    }
    if (sp->unk1B8->count == 0) {
        sp->unk1C8 = 1;
        if (sp->unk1CC & 0xff00) {
            if (sp->unk1B8->unkC8 == 0)
                sp->unk1B8->status = 5;
            else
                sp->unk1B8->status = 0;
        } else {
            sp->unk1B8->status = 4;
        }
        return;
    }
    for (i = 0; i < 8; i++)
        used[i] = 0;
    for (i = 0; i < sp->unk1B8->count; i++) {
        strcpy(sp->unk1B8->files[i].name, dir[i].name);
        n = (((u_char *)dir[i].name)[19] - '0') * 10 + (((u_char *)dir[i].name)[20] - '0');
        used[n] = -1;
    }
    for (i = 1; i < 8; i++) {
        if (used[i] != -1)
            break;
    }
    sp->unk1C8 = i;
    sp->unk1B8->status = 0;
}

/* Reads card `port`'s state through the BIOS card events, three tries. */
ADDRESS(0x8001aa84, 0x178)
void func_8001aa84(int port, int new_ok)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    int tries;
    int result;

    func_8001a1cc();
    tries = 2;
    do {
        func_8001a124();
        _card_info(port << 4);
        result = func_8001a44c();
        if (result == 0) {
            sp->unk0.card.ports[port].status = 0;
        } else if (result == 1) {
            if (tries)
                continue;
            sp->unk0.card.ports[port].status = 2;
            break;
        } else if (result == 2) {
            if (tries)
                continue;
            sp->unk0.card.ports[port].status = result;
            break;
        } else if (result == 3) {
            sp->unk0.card.ports[port].status = 1;
            if (!new_ok)
                break;
        }
        func_8001a178();
        _card_clear(port << 4);
        if (func_8001a500()) {
            sp->unk0.card.ports[port].status = 2;
            continue;
        }
        func_8001a124();
        _card_load(port << 4);
        result = func_8001a44c();
        if (result == 1 || result == 2) {
            if (tries)
                continue;
            sp->unk0.card.ports[port].status = 2;
            break;
        }
        if (result == 0)
            sp->unk0.card.ports[port].status = 0;
        else if (result == 3)
            sp->unk0.card.ports[port].status = result;
        break;
    } while (tries--);
    func_8001a35c();
}

/* A yes/no prompt: returns the choice (0 or 1), 0 on cancel. */
ADDRESS(0x8001abfc, 0x11c)
s16 func_8001abfc(int first)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    u32 buttons;

    sp->unk1D4 = 0;
    sp->unk1C6 = 0;
    for (;;) {
        if (first) {
            DAT_801603c0[0].unk8 = 0;
            DAT_801603c0[0].unkC = 0;
            DAT_801603c0[0].unk0 = 0;
        } else {
            DAT_801603c0[1].unk8 = 0;
            DAT_801603c0[1].unkC = 0;
            DAT_801603c0[1].unk0 = 0;
        }
        buttons = DAT_801603c0[0].unk8 | DAT_801603c0[1].unk8;
        if (buttons & 0x4000) {
            func_8001c978();
            break;
        }
        if (buttons & 0x2000) {
            func_8001c8f8();
            sp->unk1C6 = 0;
            break;
        }
        if (buttons & 0x80) {
            func_8001c8b8();
            sp->unk1C6 = 1;
        }
        if ((DAT_801603c0[0].unk8 | DAT_801603c0[1].unk8) & 0x20) {
            func_8001c8b8();
            sp->unk1C6 = 0;
        }
        DAT_801603c0[0].unk8 = 0;
        DAT_801603c0[1].unk8 = 0;
        func_800204b0();
    }
    return sp->unk1C6;
}

/* Picks a card slot: returns nonzero when one was chosen. */
ADDRESS(0x8001ad18, 0x160)
int func_8001ad18(int first)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    u32 buttons;

    sp->unk1D4 = 0;
    for (;;) {
        if (first) {
            DAT_801603c0[0].unk8 = 0;
            DAT_801603c0[0].unkC = 0;
            DAT_801603c0[0].unk0 = 0;
        } else {
            DAT_801603c0[1].unk8 = 0;
            DAT_801603c0[1].unkC = 0;
            DAT_801603c0[1].unk0 = 0;
        }
        buttons = DAT_801603c0[0].unk8 | DAT_801603c0[1].unk8;
        if (buttons & 0x4000) {
            func_8001c978();
            sp->unk1CC <<= 1;
            break;
        }
        if (buttons & 0x2000) {
            func_8001c8f8();
            sp->unk1CC = 0;
            break;
        }
        if ((DAT_801603c0[0].unkC | DAT_801603c0[1].unkC) & 0x10) {
            func_8001c8b8();
            sp->unk1CC = 2;
        }
        if ((DAT_801603c0[0].unkC | DAT_801603c0[1].unkC) & 0x40) {
            func_8001c8b8();
            sp->unk1CC = 512;
        }
        DAT_801603c0[0].unk8 = 0;
        DAT_801603c0[1].unk8 = 0;
        DAT_801603c0[0].unkC = 0;
        DAT_801603c0[1].unkC = 0;
        func_800204b0();
        sp->unk1D4 = (sp->unk1D4 + 1) % 32;
    }
    return sp->unk1CC != 0;
}

/* Reads the three header values of each listed save file; -1 on a read
 * error. */
ADDRESS(0x8001ae78, 0x154)
int func_8001ae78(void)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    char path[32];
    VsCardHeader header;
    int i, fd;

    i = sp->unk1B8->count;
    while (--i != -1) {
        strcpy(path, "bu");
        strcat(path, itoa(sp->unk1BC));
        strcat(path, "0:");
        strcat(path, sp->unk1B8->files[i].name);
        fd = open(path, O_RDONLY);
        lseek(fd, 0, SEEK_SET);
        if (read(fd, &header, 512) == -1)
            return -1;
        sp->unk1B8->files[i].unk15 = header.unk3D;
        sp->unk1B8->files[i].unk16 = header.unk3E;
        sp->unk1B8->files[i].unk18 = header.unk40;
        close(fd);
    }
    return 1;
}

/* The save file list: returns 1 with the chosen file in unk1C2, 0 on
 * cancel. */
ADDRESS(0x8001afcc, 0x348)
int func_8001afcc(int first)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;

    sp->unk1BE = sp->unk1B8->count;
    sp->unk1C0 = 0;
    sp->unk0.menu.half[1] = 0;
    sp->unk1C2 = 0;
    sp->unk1C4 = 0;
    sp->unk1D4 = 0;
    for (;;) {
        if (first) {
            DAT_801603c0[0].unk8 = 0;
            DAT_801603c0[0].unkC = 0;
            DAT_801603c0[0].unk0 = 0;
        } else {
            DAT_801603c0[1].unk8 = 0;
            DAT_801603c0[1].unkC = 0;
            DAT_801603c0[1].unk0 = 0;
        }
        if ((sp->unk1CC & 0xff00) || ((DAT_801603c0[0].unk8 | DAT_801603c0[1].unk8) & 0x4000)) {
            func_8001c978();
            DAT_801603c0[0].unk8 = 0;
            DAT_801603c0[1].unk8 = 0;
            sp->unk0.menu.half[1] = 1;
            sp->unk1D4 = 32;
            sp->unk1C6 = 1;
            for (;;) {
                u32 buttons;

                if (first) {
                    DAT_801603c0[0].unk8 = 0;
                    DAT_801603c0[0].unkC = 0;
                    DAT_801603c0[0].unk0 = 0;
                } else {
                    DAT_801603c0[1].unk8 = 0;
                    DAT_801603c0[1].unkC = 0;
                    DAT_801603c0[1].unk0 = 0;
                }
                buttons = DAT_801603c0[0].unk8 | DAT_801603c0[1].unk8;
                if (buttons & 0x4000) {
                    func_8001c978();
                    break;
                }
                if (buttons & 0x2000) {
                    func_8001c8f8();
                    sp->unk1C6 = 0;
                    break;
                }
                if (buttons & 0x80) {
                    func_8001c8b8();
                    sp->unk1C6 = 1;
                }
                if ((DAT_801603c0[0].unk8 | DAT_801603c0[1].unk8) & 0x20) {
                    func_8001c8b8();
                    sp->unk1C6 = 0;
                }
                DAT_801603c0[0].unk8 = 0;
                DAT_801603c0[1].unk8 = 0;
                func_800204b0();
            }
            DAT_801603c0[0].unk8 = 0;
            DAT_801603c0[1].unk8 = 0;
            if (sp->unk1C6)
                goto done;
            if (sp->unk1CC & 0xff00) {
                sp->unk1C2 = -1;
                goto done;
            }
            sp->unk0.menu.half[1] = 0;
            continue;
        }
        if ((DAT_801603c0[0].unk8 | DAT_801603c0[1].unk8) & 0x2000) {
            func_8001c8f8();
            sp->unk1C2 = -1;
            goto done;
        }
        if ((DAT_801603c0[0].unkC | DAT_801603c0[1].unkC) & 0x10) {
            func_8001c8b8();
            if (sp->unk1C2 == 0) {
                if (sp->unk1C4 > 0)
                    sp->unk1C4--;
            } else {
                sp->unk1C2--;
            }
        }
        if ((DAT_801603c0[0].unkC | DAT_801603c0[1].unkC) & 0x40) {
            func_8001c8b8();
            if (sp->unk1C2 == 2) {
                if (sp->unk1C4 < sp->unk1BE - 3)
                    sp->unk1C4++;
            } else if (sp->unk1C2 + sp->unk1C4 < sp->unk1BE - 1) {
                sp->unk1C2++;
            }
        }
        DAT_801603c0[0].unk8 = 0;
        DAT_801603c0[1].unk8 = 0;
        DAT_801603c0[0].unkC = 0;
        DAT_801603c0[1].unkC = 0;
        func_800204b0();
        sp->unk1D4 = (sp->unk1D4 + 1) % 32;
        }
done:
    if (sp->unk1C2 == -1)
        return 0;
    sp->unk1C2 += sp->unk1C4;
    return 1;
}

/* Writes `size` bytes of `data` at `offset` in 128-byte blocks of 127 data
 * bytes and an inverted sum byte; returns the sum of those bytes, or -1. */
ADDRESS(0x8001b314, 0xe4)
int func_8001b314(int fd, int offset, u8 *buf, int size)
{
    u8 *data = buf;
    u8 block[128];
    int sum = 0;
    int n, i;

    for (n = 0; size > 0; n++) {
        for (block[127] = i = 0; i < 127; i++) {
            block[i] = *data;
            block[127] += *data;
            data++;
        }
        block[127] = ~block[127];
        lseek(fd, offset + n * 128, SEEK_SET);
        if (write(fd, block, 128) != 128)
            return -1;
        size -= 127;
        sum += block[127];
    }
    return sum & 0xff;
}

/* Reads blocks written by func_8001b314, rereading a bad block up to three
 * times; returns the sum of the check bytes, or -1. */
ADDRESS(0x8001b3f8, 0x10c)
int func_8001b3f8(int fd, int offset, u8 *buf, int size)
{
    u8 *data = buf;
    u8 block[128];
    int retries = 0;
    int sum = 0;
    int n = 0;
    int check;
    int i;

    while (size > 0) {
        lseek(fd, offset + n * 128, SEEK_SET);
        if (read(fd, block, 128) != 128)
            return -1;
        for (check = i = 0; i < 127; i++) {
            check += block[i];
            *data++ = block[i];
        }
        check = ~check;
        if ((u8)check != block[127]) {
            data -= 127;
            if (retries >= 3)
                return -1;
            retries++;
        } else {
            size -= 127;
            n++;
            sum += check;
        }
    }
    return sum & 0xff;
}

/* Loads the chosen save file into `save`: 1 on success, 0 on failure. */
ADDRESS(0x8001b504, 0x34c)
int func_8001b504(VsSave *save)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    VsSave *loaded;
    char path[32];
    VsCardHeader *dst;
    VsCardHeader *header;
    int tries = 3;
    int fd, sum, i;
    u16 count;

    if (save == &DAT_801a0a80[0])
        dst = &DAT_8015ffc0[0];
    else
        dst = &DAT_8015ffc0[1];
open:
    strcpy(path, "bu");
    strcat(path, itoa(sp->unk1BC));
    strcat(path, "0:");
    strcat(path, sp->unk1B8->files[sp->unk1C2].name);
    fd = open(path, O_WRONLY);
    if (fd == -1) {
        if (!tries)
            return 0;
        tries--;
        goto open;
    }
    header = func_8002398c(512);
read:
    lseek(fd, 0, SEEK_SET);
    if (read(fd, header, 512) == -1) {
        if (!tries) {
            close(fd);
            free(header);
            return 0;
        }
        tries--;
        goto read;
    }
    loaded = &DAT_800633b0;
    sum = func_8001b3f8(fd, 512, (u8 *)loaded, 7560);
    if (sum == -1) {
        if (!tries) {
            close(fd);
            return 0;
        }
        tries--;
        goto read;
    }
    if (sum != header->unk42) {
        if (!tries) {
            close(fd);
            sp->unk0.menu.half[0] = 349;
            return 0;
        }
        tries--;
        goto read;
    }
    close(fd);
    *dst = *header;
    *save = *loaded;
    count = 0;
    {
        char zero[2] = "\x82\x4f";

        for (i = 0; i < 266; i++) {
            if (save->unk14CC[i / 32] & (1 << (i % 32)))
                count++;
        }
    }
    dst->unk40 = count;
    free(header);
    return 1;
}

/* Writes `save` to its card file, creating it first when asked: 1 on
 * success, 0 on failure. */
ADDRESS(0x8001b850, 0x290)
int func_8001b850(VsSave *save)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    char number[3];
    char path[32];
    VsCardHeader *header;
    int tries = 3;
    int blocks = 2;
    int create;
    int fd, sum;

    if (save == &DAT_801a0a80[0])
        header = &DAT_8015ffc0[0];
    else
        header = &DAT_8015ffc0[1];
    create = 0;
open:
    strcpy(path, "bu");
    strcat(path, itoa(sp->unk1BC));
    strcat(path, "0:");
    if (create) {
        strcat(path, "BASLUS-00863FS_ST");
        number[0] = sp->unk1C8 / 10 + '0';
        number[1] = sp->unk1C8 % 10 + '0';
        number[2] = 0;
        strcat(path, number);
        fd = open(path, (blocks << 16) | O_CREAT);
        if (fd == -1)
            goto retry;
        close(fd);
    } else {
        strcat(path, sp->unk1B8->files[sp->unk1C2].name);
    }
    fd = open(path, O_WRONLY);
    if (fd == -1) {
retry:
        if (tries) {
            tries--;
            goto open;
        }
        return 0;
    }
write:
    lseek(fd, 0, SEEK_SET);
    if (write(fd, header, 512) == -1) {
        if (!tries) {
            close(fd);
            return 0;
        }
        tries--;
        lseek(fd, 0, SEEK_SET);
        goto write;
    }
    sum = func_8001b314(fd, 512, (u8 *)save, 7560);
    if (sum == -1) {
        if (!tries) {
            close(fd);
            return 0;
        }
        tries--;
        lseek(fd, 0, SEEK_SET);
        goto write;
    }
    header->unk42 = sum;
    lseek(fd, 0, SEEK_SET);
    if (write(fd, header, 512) == -1) {
        if (!tries) {
            close(fd);
            return 0;
        }
        tries--;
        lseek(fd, 0, SEEK_SET);
        goto write;
    }
    close(fd);
    return 1;
}

/* Formats the card in `port`. */
ADDRESS(0x8001bae0, 0x68)
void func_8001bae0(int port)
{
    char path[32];

    strcpy(path, "bu");
    strcat(path, itoa(port));
    strcat(path, "0:");
    format(path);
}
