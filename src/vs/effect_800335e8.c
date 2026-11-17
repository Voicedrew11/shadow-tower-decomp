/*
 * VS.EXE: the effect script interpreter (VS's copy of ST.EXE's func_800385b0): runs every live effect's per-kind step.
 *
 * First attempt (one pass): a hand-checked transcription of the Ghidra
 * decompile with raw record offsets; structure fields are not named yet.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <vs/data.h>
#include <vs/funcs.h>
#include <vs/buffer.h>
#include <vs/effect.h>

#define uint unsigned int
#define ushort unsigned short
#define byte unsigned char
#define ulonglong unsigned long long
#define longlong long long
#define undefined unsigned char
#define undefined1 unsigned char
#define undefined2 unsigned short
#define undefined4 unsigned int
#define undefined8 unsigned long long
#define bool int
typedef int code();
typedef u16 T2;
typedef u32 T4;

/* Unaligned 8- and 4-byte copies (lwl/lwr). */
typedef struct {
    u8 b[8];
} C8;
typedef struct {
    u8 b[4];
} C4;

/* func_800335e8's switch table. */
JUMPTABLES(0x80011c34, 0x300)

#define D_80000400 (*(s32 *)0x80000400)
#define D_80063300 (*(s32 *)((u8 *)&DAT_80063300 + 0x0))
#define D_800633b0 (*(s32 *)((u8 *)&DAT_800633b0 + 0x0))
#define D_800b1298 (*(s32 *)((u8 *)&DAT_800633b0 + 0x4dee8))
#define D_800b129a (*(s32 *)((u8 *)&DAT_800633b0 + 0x4deea))
#define D_800b12a0 (*(s32 *)((u8 *)&DAT_800633b0 + 0x4def0))
#define D_800b12a4 (*(s32 *)((u8 *)&DAT_800633b0 + 0x4def4))
#define D_800b12a8 (*(s32 *)((u8 *)&DAT_800633b0 + 0x4def8))
#define D_800b12b2 (*(s32 *)((u8 *)&DAT_800633b0 + 0x4df02))
#define D_800b12c0 (*(s32 *)((u8 *)&DAT_800633b0 + 0x4df10))
#define D_8018ef4c (*(s32 *)((u8 *)&DAT_801624d0 + 0x2ca7c))
#define D_801d1b9c (*(u8 **)((u8 *)&DAT_801ce698 + 0x3504))

/* Not exact (one attempt; 6.1%): first divergence at +0x0, the prologue and the record pointer's register; the per-kind bodies follow Ghidra's joined tails, so the layout differs throughout. */
ADDRESS(0x800335e8, 0x1ce4)
void func_800335e8(void)
{
  short *psVar1;
  char *pcVar2;
  char *pcVar3;
  bool bVar4;
  char *pcVar5;
  char cVar6;
  undefined2 uVar7;
  short sVar8;
  undefined4 uVar9;
  int iVar10;
  short sVar11;
  int iVar12;
  uint uVar13;
  int iVar14;
  int iVar15;
  char cVar16;
  uint uVar17;
  char cVar18;
  undefined4 uVar19;
  short local_90;
  short local_8e;
  undefined2 local_8c;
  int local_88;
  undefined4 local_84;
  int local_80;
  short local_78;
  short local_76;
  short local_74;
  undefined4 local_70;
  undefined4 local_6c;
  undefined4 local_68;
  short local_60;
  short local_5e;
  undefined2 local_5c;
  int local_58;
  undefined4 local_54;
  int local_50;
  undefined2 local_4c;
  short local_48;
  short local_46;
  short local_44;
  short local_40;
  int local_38;
  undefined4 local_34;
  int local_30;
  
  pcVar5 = D_801d1b9c;
  cVar6 = D_801d1b9c[1];
  switch(cVar6) {
  case '0':
  case '8':
    *(short *)(D_801d1b9c + 0x38) = *(short *)(D_801d1b9c + 0x38) + 0x60;
    *(short *)(pcVar5 + 0x3a) = *(short *)(pcVar5 + 0x3a) + 0x60;
    iVar12 = *(short *)(pcVar5 + 0x38) * 0x380;
    if (iVar12 < 0) {
      iVar12 = iVar12 + 0xfff;
    }
    *(short *)(pcVar5 + 0xc) = (short)(iVar12 >> 0xc);
    iVar12 = *(short *)(pcVar5 + 0x38) * 0x380;
    if (iVar12 < 0) {
      iVar12 = iVar12 + 0xfff;
    }
    uVar13 = iVar12 >> 0xc | 0x80000000;
    *(uint *)(pcVar5 + 0x18) = uVar13;
    iVar12 = func_8003333c(*(undefined2 *)(pcVar5 + 0xc),uVar13,0x20);
    goto LAB_80035264;
  case '1':
  case '2':
  case '3':
  case '4':
  case '6':
  case '7':
  case '9':
  case '<':
  case '?':
  case 'B':
  case 'C':
  case 'E':
  case 'I':
  case 'J':
  case 'K':
  case 'M':
  case 'O':
  case 'Q':
  case '`':
  case 'j':
  case 'k':
  case 'm':
  case 'n':
  case 'o':
  case 'r':
  case 's':
  case '{':
  case '\x7f':
  case -0x7b:
  case -0x79:
  case -0x71:
  case -0x70:
  case -0x6e:
  case -0x6c:
  case -0x68:
  case -0x67:
  case -99:
  case -0x61:
  case -0x5d:
  case -0x5b:
    *(short *)(D_801d1b9c + 0x38) = *(short *)(D_801d1b9c + 0x38) + 0x60;
    *(short *)(pcVar5 + 0x3a) = *(short *)(pcVar5 + 0x3a) + 0x60;
    iVar12 = *(short *)(pcVar5 + 0x38) * 0x380;
    if (iVar12 < 0) {
      iVar12 = iVar12 + 0xfff;
    }
    *(short *)(pcVar5 + 0xc) = (short)(iVar12 >> 0xc);
    iVar12 = *(short *)(pcVar5 + 0x38) * 0x380;
    if (iVar12 < 0) {
      iVar12 = iVar12 + 0xfff;
    }
    uVar13 = iVar12 >> 0xc | 0x80000000;
    *(uint *)(pcVar5 + 0x18) = uVar13;
    iVar12 = func_8003333c(*(undefined2 *)(pcVar5 + 0xc),uVar13,0x20);
    if ((iVar12 != 0) && (pcVar5[8] != '\0')) {
      func_8002e314(pcVar5[0x4c],pcVar5[0x4d]);
      pcVar5[8] = pcVar5[8] + -1;
    }
    if (0 < *(int *)(pcVar5 + 0x24)) {
      *pcVar5 = -1;
    }
    goto LAB_80034db8;
  case '5':
  case 'S':
  case -0x7e:
  case -0x73:
    pcVar2 = D_801d1b9c + 0xc;
    pcVar3 = D_801d1b9c + 0x18;
    *(short *)(D_801d1b9c + 0x3c) = *(short *)(D_801d1b9c + 0x3c) + 0xe0;
    iVar12 = func_8003333c(*(undefined2 *)pcVar2,*(undefined4 *)pcVar3,0x20);
    if ((iVar12 != 0) && (pcVar5[8] != '\0')) {
      func_8002e314(pcVar5[0x4c],pcVar5[0x4d]);
      pcVar5[8] = pcVar5[8] + -1;
    }
    if (((60000 < *(int *)(pcVar5 + 0x20) + 30000U) || (30000 < *(int *)(pcVar5 + 0x28))) ||
       (*(int *)(pcVar5 + 0x28) < -30000)) {
      *pcVar5 = -1;
    }
    iVar12 = (int)*(short *)(pcVar5 + 0x10);
    bVar4 = iVar12 < 0x20;
    goto LAB_800351c0;
  case ':':
    iVar12 = func_8003333c(*(undefined2 *)(D_801d1b9c + 0xc),*(undefined4 *)(D_801d1b9c + 0x18),
                          0x80);
    if ((iVar12 != 0) && (pcVar5[8] != '\0')) {
      func_8002e314(pcVar5[0x4c],pcVar5[0x4d]);
      pcVar5[8] = pcVar5[8] + -1;
    }
    if (*(short *)(pcVar5 + 0x10) < 0x20) {
      pcVar5[7] = '\r';
      sVar8 = *(short *)(pcVar5 + 0x10) << 7;
      goto LAB_80034dcc;
    }
    goto LAB_80034dd0;
  case '=':
  case 'F':
  case 'R':
  case 'Z':
  case 'd':
  case '~':
  case -0x7f:
  case -0x7a:
  case -0x76:
    iVar12 = rcos((int)*(short *)(D_801d1b9c + 0x10) << 0xb);
    if (iVar12 < 0) {
      iVar12 = iVar12 + 3;
    }
    sVar8 = *(short *)(pcVar5 + 0x50);
    uVar7 = (undefined2)(iVar12 >> 2);
    *(undefined2 *)(pcVar5 + 0x3c) = uVar7;
    *(undefined2 *)(pcVar5 + 0x3a) = uVar7;
    *(undefined2 *)(pcVar5 + 0x38) = uVar7;
    *(short *)(pcVar5 + 0x50) = sVar8 + 0x20;
    if (-0x81 < (short)(sVar8 + 0x20)) {
      pcVar5[0x50] = -0x80;
      pcVar5[0x51] = -1;
    }
    if ((pcVar5[1] == '~') || (pcVar5[1] == -0x7f)) {
      iVar12 = rcos((int)*(short *)(pcVar5 + 0x50));
      iVar10 = rsin((int)*(short *)(pcVar5 + 0x50));
      iVar15 = *(int *)(*(int *)(pcVar5 + 0x1c) + 0x24);
      iVar14 = *(int *)(D_80063300 + 0x4def4) + (int)*(short *)(D_80063300 + 0x4e0ca);
    }
    else {
      iVar12 = rcos((int)*(short *)(pcVar5 + 0x50));
      iVar10 = rsin((int)*(short *)(pcVar5 + 0x50));
      iVar14 = *(int *)(D_80063300 + 0x4def4);
      iVar15 = *(int *)(*(int *)(pcVar5 + 0x1c) + 0x24);
    }
    iVar12 = (iVar14 - iVar15) * iVar12;
    iVar14 = iVar12 / iVar10;
    if (iVar10 == 0) {
      trap(0x1c00);
    }
    if ((iVar10 == -1) && (iVar12 == -0x80000000)) {
      trap(0x1800);
    }
    iVar12 = rcos(*(short *)(*(int *)(pcVar5 + 0x1c) + 0x32) + 0x400);
    iVar12 = iVar14 * iVar12;
    if (iVar12 < 0) {
      iVar12 = iVar12 + 0xfff;
    }
    *(int *)(pcVar5 + 0x20) = *(int *)(*(int *)(pcVar5 + 0x1c) + 0x20) - (iVar12 >> 0xc);
    iVar12 = rsin(*(short *)(*(int *)(pcVar5 + 0x1c) + 0x32) + 0x400);
    iVar14 = iVar14 * iVar12;
    if (iVar14 < 0) {
      iVar14 = iVar14 + 0xfff;
    }
    iVar12 = *(int *)(*(int *)(pcVar5 + 0x1c) + 0x28);
    pcVar5[0x24] = '\0';
    pcVar5[0x25] = '\0';
    pcVar5[0x26] = '\0';
    pcVar5[0x27] = '\0';
    *(int *)(pcVar5 + 0x28) = iVar12 + (iVar14 >> 0xc);
    if (*(short *)(pcVar5 + 0x10) < 0x20) {
      *(short *)(pcVar5 + 0x12) = (short)((int)*(short *)(pcVar5 + 0x10) << 7);
    }
    iVar12 = func_80032110(0x400,&D_80000400);
    if ((iVar12 != 0) && (pcVar5[8] != '\0')) {
      func_8002e314(pcVar5[0x4c],pcVar5[0x4d]);
      pcVar5[8] = pcVar5[8] + -1;
    }
    local_70 = *(undefined4 *)(pcVar5 + 0x20);
    local_6c = *(undefined4 *)(pcVar5 + 0x24);
    local_68 = *(undefined4 *)(pcVar5 + 0x28);
    local_78 = 0;
    local_76 = 0;
    local_74 = 0;
    if (cVar6 == '=') {
      uVar9 = 0xcc;
    }
    else if (cVar6 == -0x76) {
      uVar9 = 0xed;
    }
    else if (cVar6 == 'F') {
      uVar9 = 0xcf;
    }
    else if (cVar6 == -0x7f) {
      uVar9 = 0xe3;
    }
    else if (cVar6 == 'R') {
      uVar9 = 0xd2;
    }
    else if (cVar6 == -0x7a) {
      uVar9 = 0xe8;
    }
    else if (cVar6 == 'Z') {
      uVar9 = 0xda;
    }
    else if (cVar6 == 'd') {
      uVar9 = 0xdd;
    }
    else {
      uVar9 = 0xe0;
      if (cVar6 != '~') {
        return;
      }
    }
    func_80032404(pcVar5[0x4c],pcVar5[0x4d],*pcVar5,uVar9,&local_70,&local_78,0x200,0x100,6,3);
    break;
  case '>':
  case 'H':
  case 'l':
  case 'q':
  case 'z':
  case -0x6d:
    iVar12 = func_8003333c(*(undefined2 *)(D_801d1b9c + 0xc),*(undefined4 *)(D_801d1b9c + 0x18),
                          0x80);
    uVar13 = 0;
    if ((iVar12 != 0) || (-1 < *(int *)(pcVar5 + 0x24))) {
      do {
        iVar12 = rand();
        local_90 = 8 - (short)(iVar12 / 0x7e8);
        iVar12 = rand();
        if (iVar12 < 0) {
          iVar12 = iVar12 + 0x3ff;
        }
        local_8e = -0x30 - (short)(iVar12 >> 10);
        iVar12 = rand();
        iVar12 = 8 - iVar12 / 0x7e8;
        local_8c = (undefined2)iVar12;
        local_88 = *(int *)(pcVar5 + 0x20) - (int)local_90;
        local_84 = *(undefined4 *)(pcVar5 + 0x24);
        local_80 = *(int *)(pcVar5 + 0x28) - (iVar12 * 0x10000 >> 0x10);
        if (cVar6 == '>') {
          uVar9 = 0x3f;
LAB_80033d60:
          func_80032404(pcVar5[0x4c],pcVar5[0x4d],*pcVar5,uVar9,&local_88,&local_90,0x600,uVar13 & 1)
          ;
        }
        else {
          if (cVar6 == 'l') {
            uVar9 = 0x6d;
            goto LAB_80033d60;
          }
          if (cVar6 == 'H') {
            uVar9 = 0x49;
            goto LAB_80033d60;
          }
          if (cVar6 == -0x6d) {
            uVar9 = 0x94;
            goto LAB_80033d60;
          }
          if (cVar6 == 'q') {
            uVar9 = 0x72;
            goto LAB_80033d60;
          }
          uVar9 = 0x7b;
          if (cVar6 == 'z') goto LAB_80033d60;
        }
        uVar13 = uVar13 + 1;
      } while ((int)uVar13 < 4);
      *pcVar5 = -1;
    }
    if (*(short *)(pcVar5 + 0x10) < 0x20) {
      pcVar5[7] = '\r';
      *(short *)(pcVar5 + 0x12) = *(short *)(pcVar5 + 0x10) << 7;
    }
    *(short *)(pcVar5 + 0x42) = *(short *)(pcVar5 + 0x42) + 8;
    break;
  case '@':
  case '^':
  case 'i':
  case 'v':
  case -0x78:
  case -0x77:
  case -0x75:
  case -0x5e:
  case -0x5c:
  case -0x58:
    if (cVar6 != '^') {
      sVar8 = rsin(0x40 - *(short *)(D_801d1b9c + 0x10));
      *(short *)(pcVar5 + 0x32) = *(short *)(pcVar5 + 0x32) + sVar8;
    }
    *(short *)(pcVar5 + 0x38) = *(short *)(pcVar5 + 0x38) + 0x80;
    *(short *)(pcVar5 + 0x3a) = *(short *)(pcVar5 + 0x3a) + 0x80;
    *(short *)(pcVar5 + 0x3c) = *(short *)(pcVar5 + 0x3c) + 0x80;
    *(short *)(pcVar5 + 0xc) =
         (short)(((int)((uint)*(ushort *)(pcVar5 + 0x38) << 0x10) >> 0x10) -
                 ((int)((uint)*(ushort *)(pcVar5 + 0x38) << 0x10) >> 0x1f) >> 1);
    uVar13 = ((int)((uint)*(ushort *)(pcVar5 + 0x38) << 0x10) >> 0x10) -
             ((int)((uint)*(ushort *)(pcVar5 + 0x38) << 0x10) >> 0x1f) >> 1;
    *(uint *)(pcVar5 + 0x18) = uVar13;
    iVar12 = func_80032110(*(undefined2 *)(pcVar5 + 0xc),uVar13 | 0x80000000);
    goto LAB_80035264;
  case 'G':
  case -0x65:
    *(short *)(D_801d1b9c + 0x38) = *(short *)(D_801d1b9c + 0x38) + 0x80;
    *(short *)(pcVar5 + 0x3a) = *(short *)(pcVar5 + 0x3a) + 0x80;
    iVar12 = *(short *)(pcVar5 + 0x38) * 0x380;
    if (iVar12 < 0) {
      iVar12 = iVar12 + 0xfff;
    }
    *(short *)(pcVar5 + 0xc) = (short)(iVar12 >> 0xc);
    iVar12 = *(short *)(pcVar5 + 0x38) * 0x380;
    if (iVar12 < 0) {
      iVar12 = iVar12 + 0xfff;
    }
    uVar13 = iVar12 >> 0xc | 0x80000000;
    *(uint *)(pcVar5 + 0x18) = uVar13;
    func_8003333c(*(undefined2 *)(pcVar5 + 0xc),uVar13,(int)pcVar5[0x50]);
    if (*(short *)(pcVar5 + 0x10) < 0x20) {
      *(short *)(pcVar5 + 0x12) = (short)((int)*(short *)(pcVar5 + 0x10) << 7);
    }
    *(short *)(pcVar5 + 0x42) = *(short *)(pcVar5 + 0x42) + 2;
    if (pcVar5[8] == '\0') {
      func_8002e314(pcVar5[0x4c],pcVar5[0x4d]);
      cVar6 = ' ';
    }
    else {
      cVar6 = pcVar5[8] + -1;
    }
    pcVar5[8] = cVar6;
    iVar12 = rand();
    iVar10 = -(int)*(short *)(pcVar5 + 0x10) + 0x40;
    if (iVar10 < 0) {
      iVar10 = -(int)*(short *)(pcVar5 + 0x10) + 0x47;
    }
    /* unaligned copy (decompile noise dropped) */
    if (4 < (iVar12 + 2U & 0xffff)) {
      D_8018ef4c = D_8018ef4c + (short)iVar12 * 10;
    }
    break;
  case 'U':
  case 'Y':
  case -0x69:
    uVar7 = rcos((0x20 - *(short *)(D_801d1b9c + 0x10)) * 0x40);
    *(undefined2 *)(pcVar5 + 0x3a) = uVar7;
    *(short *)(pcVar5 + 0x38) = *(short *)(pcVar5 + 0x38) + 0xc0;
    *(short *)(pcVar5 + 0x3c) = *(short *)(pcVar5 + 0x3c) + 0xc0;
    iVar12 = *(short *)(pcVar5 + 0x38) * 0x600;
    if (iVar12 < 0) {
      iVar12 = iVar12 + 0xfff;
    }
    *(short *)(pcVar5 + 0xc) = (short)(iVar12 >> 0xc);
    iVar12 = *(short *)(pcVar5 + 0x38) * 0x600;
    if (iVar12 < 0) {
      iVar12 = iVar12 + 0xfff;
    }
    *(uint *)(pcVar5 + 0x18) = iVar12 >> 0xc | 0x80000000;
    *(short *)(pcVar5 + 0x32) = *(short *)(pcVar5 + 0x32) + 0x80;
    iVar12 = func_8003333c(*(undefined2 *)(pcVar5 + 0xc),*(undefined4 *)(pcVar5 + 0x18),0);
    if ((iVar12 != 0) && (pcVar5[8] != '\0')) {
      func_8002e314(pcVar5[0x4c],pcVar5[0x4d]);
      pcVar5[8] = pcVar5[8] + -1;
    }
    iVar12 = (int)*(short *)(pcVar5 + 0x10);
    bVar4 = iVar12 < 0x20;
    goto LAB_800351c0;
  case 'V':
  case '_':
  case 'p':
    *(short *)(D_801d1b9c + 0x38) = *(short *)(D_801d1b9c + 0x38) + 0x60;
    *(short *)(pcVar5 + 0x3a) = *(short *)(pcVar5 + 0x3a) + 0x60;
    iVar12 = *(short *)(pcVar5 + 0x38) * 0x380;
    if (iVar12 < 0) {
      iVar12 = iVar12 + 0xfff;
    }
    *(short *)(pcVar5 + 0xc) = (short)(iVar12 >> 0xc);
    iVar12 = *(short *)(pcVar5 + 0x38) * 0x380;
    if (iVar12 < 0) {
      iVar12 = iVar12 + 0xfff;
    }
    uVar13 = iVar12 >> 0xc | 0x80000000;
    *(uint *)(pcVar5 + 0x18) = uVar13;
    iVar12 = func_8003333c(*(undefined2 *)(pcVar5 + 0xc),uVar13,(int)pcVar5[0x50]);
    if ((iVar12 != 0) && (pcVar5[8] != '\0')) {
      func_8002e314(pcVar5[0x4c],pcVar5[0x4d]);
      pcVar5[8] = pcVar5[8] + -1;
    }
    if (((int)*(short *)(pcVar5 + 0x10) % 5 & 0xffffU) == 0) {
      iVar12 = rand();
      iVar10 = (int)*(short *)(pcVar5 + 0x40);
      if (iVar10 < 0) {
        iVar10 = iVar10 + 3;
      }
      if (iVar12 < 0) {
        iVar12 = iVar12 + 0x3ff;
      }
      local_48 = (short)(iVar10 >> 2) - ((short)(iVar12 >> 10) + -0x10);
      iVar12 = rand();
      iVar10 = (int)*(short *)(pcVar5 + 0x42);
      if (iVar10 < 0) {
        iVar10 = iVar10 + 3;
      }
      if (iVar12 < 0) {
        iVar12 = iVar12 + 0x3ff;
      }
      local_46 = (short)(iVar10 >> 2) - ((short)(iVar12 >> 10) + 0x30);
      iVar12 = rand();
      iVar10 = (int)*(short *)(pcVar5 + 0x44);
      if (iVar10 < 0) {
        iVar10 = iVar10 + 3;
      }
      if (iVar12 < 0) {
        iVar12 = iVar12 + 0x3ff;
      }
      iVar12 = (iVar10 >> 2) - ((iVar12 >> 10) + -0x10);
      local_44 = (short)iVar12;
      local_38 = *(int *)(pcVar5 + 0x20) - (int)local_48;
      local_34 = *(undefined4 *)(pcVar5 + 0x24);
      local_30 = *(int *)(pcVar5 + 0x28) - (iVar12 * 0x10000 >> 0x10);
      func_80032404(pcVar5[0x4c],pcVar5[0x4d],*pcVar5,0xd5,&local_38,&local_48,0x1000);
    }
LAB_80034db8:
    sVar8 = (short)((int)*(short *)(pcVar5 + 0x10) << 7);
    if (*(short *)(pcVar5 + 0x10) < 0x20) {
LAB_80034dcc:
      *(short *)(pcVar5 + 0x12) = sVar8;
    }
LAB_80034dd0:
    *(short *)(pcVar5 + 0x42) = *(short *)(pcVar5 + 0x42) + 2;
    break;
  case 'W':
  case 'X':
  case '\\':
  case 'a':
  case 'b':
  case 'c':
  case 'g':
  case 'h':
  case 't':
  case -0x74:
  case -0x6b:
  case -100:
  case -0x62:
    psVar1 = (short *)(D_801d1b9c + 0x3c);
    *(short *)(D_801d1b9c + 0x38) = *(short *)(D_801d1b9c + 0x38) + 0x60;
    iVar12 = (int)*(short *)(pcVar5 + 0x38);
    *(short *)(pcVar5 + 0x3c) = *psVar1 + 0x60;
    *(short *)(pcVar5 + 0x3a) = *(short *)(pcVar5 + 0x3a) + 0x60;
    if (iVar12 < 0) {
      iVar12 = iVar12 + 3;
    }
    iVar10 = (int)*(short *)(pcVar5 + 0x38);
    *(short *)(pcVar5 + 0xc) = (short)(iVar12 >> 2);
    if (iVar10 < 0) {
      iVar10 = iVar10 + 3;
    }
    uVar13 = iVar10 >> 2 | 0x80000000;
    *(uint *)(pcVar5 + 0x18) = uVar13;
    iVar12 = func_8003333c(*(undefined2 *)(pcVar5 + 0xc),uVar13,0x100);
    if (iVar12 != 0) {
      func_8002e314(pcVar5[0x4c],pcVar5[0x4d]);
      *pcVar5 = -1;
    }
    if (((int)*(short *)(pcVar5 + 0x10) % 5 & 0xffffU) == 0) {
      iVar12 = rand();
      iVar10 = (int)*(short *)(pcVar5 + 0x40);
      if (iVar10 < 0) {
        iVar10 = iVar10 + 3;
      }
      if (iVar12 < 0) {
        iVar12 = iVar12 + 0x3ff;
      }
      local_60 = (short)(iVar10 >> 2) - ((short)(iVar12 >> 10) + -0x10);
      iVar12 = rand();
      iVar10 = (int)*(short *)(pcVar5 + 0x42);
      if (iVar10 < 0) {
        iVar10 = iVar10 + 3;
      }
      if (iVar12 < 0) {
        iVar12 = iVar12 + 0x3ff;
      }
      local_5e = (short)(iVar10 >> 2) - (short)(iVar12 >> 10);
      iVar12 = rand();
      iVar10 = (int)*(short *)(pcVar5 + 0x44);
      if (iVar10 < 0) {
        iVar10 = iVar10 + 3;
      }
      if (iVar12 < 0) {
        iVar12 = iVar12 + 0x3ff;
      }
      iVar12 = (iVar10 >> 2) - ((iVar12 >> 10) + -0x10);
      local_5c = (undefined2)iVar12;
      local_58 = *(int *)(pcVar5 + 0x20) - (int)local_60;
      local_54 = *(undefined4 *)(pcVar5 + 0x24);
      local_50 = *(int *)(pcVar5 + 0x28) - (iVar12 * 0x10000 >> 0x10);
      func_80032404(pcVar5[0x4c],pcVar5[0x4d],*pcVar5,0xd7,&local_58,&local_60,0x200);
    }
    goto LAB_80034f34;
  case ']':
  case 'e':
  case 'u':
  case 'w':
  case 'x':
  case 'y':
  case -0x80:
  case -0x5a:
    iVar12 = func_8003333c(*(undefined2 *)(D_801d1b9c + 0xc),*(undefined4 *)(D_801d1b9c + 0x18),
                          0xffffff00);
    if (iVar12 != 0) {
      func_8002e314(pcVar5[0x4c],pcVar5[0x4d]);
      *pcVar5 = -1;
    }
LAB_80034f34:
    if (*(short *)(pcVar5 + 0x10) < 0x20) {
      *(short *)(pcVar5 + 0x12) = (short)((int)*(short *)(pcVar5 + 0x10) << 7);
    }
    if (0 < *(int *)(pcVar5 + 0x24)) {
      *pcVar5 = -1;
    }
    break;
  case 'f':
    sVar8 = *(short *)(D_801d1b9c + 0x10);
    *(short *)(D_801d1b9c + 0x38) = *(short *)(D_801d1b9c + 0x38) + 0x60;
    *(short *)(pcVar5 + 0x3a) = *(short *)(pcVar5 + 0x3a) + 0x60;
    iVar12 = *(short *)(pcVar5 + 0x38) * 0x380;
    pcVar5[2] = (char)sVar8 +
                ((char)((ulonglong)((longlong)(int)sVar8 * 0x55555556) >> 0x20) -
                (char)(sVar8 >> 0xf)) * -3 | 0x80;
    if (iVar12 < 0) {
      iVar12 = iVar12 + 0xfff;
    }
    *(short *)(pcVar5 + 0xc) = (short)(iVar12 >> 0xc);
    iVar12 = *(short *)(pcVar5 + 0x38) * 0x380;
    if (iVar12 < 0) {
      iVar12 = iVar12 + 0xfff;
    }
    uVar13 = iVar12 >> 0xc | 0x80000000;
    *(uint *)(pcVar5 + 0x18) = uVar13;
    iVar12 = func_8003333c(*(undefined2 *)(pcVar5 + 0xc),uVar13,0x20);
    goto LAB_80035264;
  case '|':
  case -0x7d:
  case -0x7c:
  case -0x72:
  case -0x6f:
  case -0x6a:
  case -0x60:
  case -0x59:
  case -0x57:
    pcVar2 = D_801d1b9c + 0xc;
    pcVar3 = D_801d1b9c + 0x18;
    D_801d1b9c[2] =
         (char)*(short *)(D_801d1b9c + 0x10) +
         (char)((int)*(short *)(D_801d1b9c + 0x10) / 3) * -3 | 0x80;
    iVar12 = func_8003333c(*(undefined2 *)pcVar2,*(undefined4 *)pcVar3,0xffffff00);
    if (iVar12 != 0) {
      func_8002e314(pcVar5[0x4c],pcVar5[0x4d]);
      *pcVar5 = -1;
    }
    iVar12 = (int)*(short *)(pcVar5 + 0x10);
    bVar4 = iVar12 < 0x20;
    goto LAB_800351c0;
  case '}':
  case -0x5f:
    sVar8 = *(short *)(D_801d1b9c + 0x10);
    *(short *)(D_801d1b9c + 0x38) = *(short *)(D_801d1b9c + 0x38) + 0x60;
    *(short *)(pcVar5 + 0x3a) = *(short *)(pcVar5 + 0x3a) + 0x60;
    iVar12 = *(short *)(pcVar5 + 0x38) * 0x380;
    pcVar5[2] = (char)sVar8 +
                ((char)((ulonglong)((longlong)(int)sVar8 * 0x55555556) >> 0x20) -
                (char)(sVar8 >> 0xf)) * -3 | 0x84;
    if (iVar12 < 0) {
      iVar12 = iVar12 + 0xfff;
    }
    *(short *)(pcVar5 + 0xc) = (short)(iVar12 >> 0xc);
    iVar12 = *(short *)(pcVar5 + 0x38) * 0x380;
    if (iVar12 < 0) {
      iVar12 = iVar12 + 0xfff;
    }
    uVar13 = iVar12 >> 0xc | 0x80000000;
    *(uint *)(pcVar5 + 0x18) = uVar13;
    iVar12 = func_8003333c(*(undefined2 *)(pcVar5 + 0xc),uVar13,0x20);
LAB_80035264:
    if ((iVar12 != 0) && (pcVar5[8] != '\0')) {
      func_8002e314(pcVar5[0x4c],pcVar5[0x4d]);
      pcVar5[8] = pcVar5[8] + -1;
    }
    break;
  case -0x38:
  case -0x2d:
  case -0x1c:
  case -0x12:
    psVar1 = (short *)(D_801d1b9c + 0x10);
    *(ushort *)(D_801d1b9c + 0x34) = *(short *)(D_801d1b9c + 0x34) - 0x20U & 0xfff;
    if (*psVar1 == 8) {
      if (cVar6 == -0x38) {
        uVar9 = func_80032404(pcVar5[0x4c],pcVar5[0x4d],*pcVar5,0x35,pcVar5 + 0x20,pcVar5 + 0x40,
                             (int)*(short *)(pcVar5 + 0x38),pcVar5 + 0x30,pcVar5);
        cVar6 = pcVar5[0x4c];
        cVar16 = pcVar5[0x4d];
        cVar18 = *pcVar5;
        iVar12 = (int)*(short *)(pcVar5 + 0x38);
        uVar19 = 0xc9;
      }
      else if (cVar6 == -0x12) {
        uVar9 = func_80032404(pcVar5[0x4c],pcVar5[0x4d],*pcVar5,0x82,pcVar5 + 0x20,pcVar5 + 0x40,
                             (int)*(short *)(pcVar5 + 0x38),pcVar5 + 0x30,pcVar5);
        cVar6 = pcVar5[0x4c];
        cVar16 = pcVar5[0x4d];
        cVar18 = *pcVar5;
        iVar12 = (int)*(short *)(pcVar5 + 0x38);
        uVar19 = 0xe5;
      }
      else if (cVar6 == -0x1c) {
        uVar9 = func_80032404(pcVar5[0x4c],pcVar5[0x4d],*pcVar5,0x82,pcVar5 + 0x20,pcVar5 + 0x40,
                             (int)*(short *)(pcVar5 + 0x38),pcVar5 + 0x30,pcVar5);
        cVar6 = pcVar5[0x4c];
        cVar16 = pcVar5[0x4d];
        cVar18 = *pcVar5;
        iVar12 = (int)*(short *)(pcVar5 + 0x38);
        uVar19 = 0xe5;
      }
      else {
        if (cVar6 != -0x2d) goto LAB_80033a9c;
        uVar9 = func_80032404(pcVar5[0x4c],pcVar5[0x4d],*pcVar5,0x53,pcVar5 + 0x20,pcVar5 + 0x40,
                             (int)*(short *)(pcVar5 + 0x38),pcVar5 + 0x30,pcVar5);
        cVar6 = pcVar5[0x4c];
        cVar16 = pcVar5[0x4d];
        cVar18 = *pcVar5;
        iVar12 = (int)*(short *)(pcVar5 + 0x38);
        uVar19 = 0xd4;
      }
      func_80032404(cVar6,cVar16,cVar18,uVar19,pcVar5 + 0x20,pcVar5 + 0x40,iVar12,pcVar5 + 0x30,uVar9
                  );
    }
LAB_80033a9c:
    if (*(short *)(pcVar5 + 0x10) < 0x10) {
      *(short *)(pcVar5 + 0x38) = *(short *)(pcVar5 + 0x38) + -0x40;
      *(short *)(pcVar5 + 0x3c) = *(short *)(pcVar5 + 0x3c) + -0x40;
      sVar8 = *(short *)(pcVar5 + 0x3a) + -0x40;
    }
    else {
      *(short *)(pcVar5 + 0x38) = *(short *)(pcVar5 + 0x38) + 0x40;
      *(short *)(pcVar5 + 0x3c) = *(short *)(pcVar5 + 0x3c) + 0x40;
      sVar8 = *(short *)(pcVar5 + 0x3a) + 0x40;
    }
    *(short *)(pcVar5 + 0x3a) = sVar8;
    iVar12 = (int)*(short *)(pcVar5 + 0x10);
    bVar4 = iVar12 < 0x20;
    goto LAB_800351c0;
  case -0x37:
  case -0x2c:
  case -0x1b:
  case -0x11:
    func_8003333c(*(undefined2 *)(D_801d1b9c + 0xc),*(undefined4 *)(D_801d1b9c + 0x18),0x80);
    sVar8 = *(short *)(pcVar5 + 0x10);
    goto LAB_80034b68;
  case -0x36:
  case -0x33:
  case -0x30:
  case -0x28:
  case -0x25:
  case -0x22:
  case -0x1f:
  case -0x1a:
  case -0x15:
    uVar17 = (uint)(byte)D_801d1b9c[0x4c];
    uVar13 = (uint)(byte)D_801d1b9c[0x4d];
    *(ushort *)(D_801d1b9c + 0x34) = *(short *)(D_801d1b9c + 0x34) - 0x80U & 0xfff;
    if (*(short *)(&D_800633b0 + uVar17 * 0x1383a + uVar13 * 8 + 0x13819) == -0x8000) {
      if ((&D_800b12c0)[uVar17 * 0x27074] == 2) {
        func_80028d94(uVar17,(&D_800b1298)[uVar17 * 0x4e0e8],
                     (int)(short)(&D_800b129a)[uVar17 * 0x27074],
                     (int)*(short *)(&D_800633b0 + uVar17 * 0x1383a + uVar13 * 8 + 0x1381a),
                     &local_78);
      }
      else {
        func_800256bc(uVar17,(int)*(short *)((int)&D_800633b0 +
                                           uVar13 * 0x20 + uVar17 * 0x4e0e8 + 0x4e066),
                     (int)*(short *)(&D_800633b0 + uVar17 * 0x1383a + uVar13 * 8 + 0x1381a),
                     &local_48);
        local_78 = local_48;
        local_76 = local_44;
        local_74 = local_40;
      }
      local_4c = 0;
      local_50 = (uint)(ushort)(&D_800b12b2)[uVar17 * 0x27074] << 0x10;
      RotMatrix(&local_50,&local_70);
      ApplyMatrixSV(&local_70,&local_78,&local_78);
      *(int *)(pcVar5 + 0x20) = (&D_800b12a0)[uVar17 * 0x1383a] + (int)local_78;
      *(int *)(pcVar5 + 0x24) = (&D_800b12a4)[uVar17 * 0x1383a] + (int)local_76;
      *(int *)(pcVar5 + 0x28) = (&D_800b12a8)[uVar17 * 0x1383a] + (int)local_74;
    }
    if (*(short *)(pcVar5 + 0x10) == 0x21) {
      if (cVar6 == -0x36) {
        func_80032404(pcVar5[0x4c],pcVar5[0x4d],*pcVar5,0xcb,pcVar5 + 0x20,pcVar5 + 0x40,
                     (int)*(short *)(pcVar5 + 0x38),pcVar5 + 0x30,pcVar5);
        cVar6 = pcVar5[0x4c];
        cVar16 = pcVar5[0x4d];
        cVar18 = *pcVar5;
        iVar12 = (int)*(short *)(pcVar5 + 0x38);
        uVar9 = 0x3d;
      }
      else if (cVar6 == -0x15) {
        func_80032404(pcVar5[0x4c],pcVar5[0x4d],*pcVar5,0xec,pcVar5 + 0x20,pcVar5 + 0x40,
                     (int)*(short *)(pcVar5 + 0x38),pcVar5 + 0x30,pcVar5);
        cVar6 = pcVar5[0x4c];
        cVar16 = pcVar5[0x4d];
        cVar18 = *pcVar5;
        iVar12 = (int)*(short *)(pcVar5 + 0x38);
        uVar9 = 0x8a;
      }
      else if (cVar6 == -0x33) {
        func_80032404(pcVar5[0x4c],pcVar5[0x4d],*pcVar5,0xce,pcVar5 + 0x20,pcVar5 + 0x40,
                     (int)*(short *)(pcVar5 + 0x38),pcVar5 + 0x30,pcVar5);
        cVar6 = pcVar5[0x4c];
        cVar16 = pcVar5[0x4d];
        cVar18 = *pcVar5;
        iVar12 = (int)*(short *)(pcVar5 + 0x38);
        uVar9 = 0x46;
      }
      else if (cVar6 == -0x1f) {
        func_80032404(pcVar5[0x4c],pcVar5[0x4d],*pcVar5,0xe2,pcVar5 + 0x20,pcVar5 + 0x40,
                     (int)*(short *)(pcVar5 + 0x38),pcVar5 + 0x30,pcVar5);
        cVar6 = pcVar5[0x4c];
        cVar16 = pcVar5[0x4d];
        cVar18 = *pcVar5;
        iVar12 = (int)*(short *)(pcVar5 + 0x38);
        uVar9 = 0x81;
      }
      else if (cVar6 == -0x30) {
        func_80032404(pcVar5[0x4c],pcVar5[0x4d],*pcVar5,0xd1,pcVar5 + 0x20,pcVar5 + 0x40,
                     (int)*(short *)(pcVar5 + 0x38),pcVar5 + 0x30,pcVar5);
        cVar6 = pcVar5[0x4c];
        cVar16 = pcVar5[0x4d];
        cVar18 = *pcVar5;
        iVar12 = (int)*(short *)(pcVar5 + 0x38);
        uVar9 = 0x52;
      }
      else if (cVar6 == -0x1a) {
        func_80032404(pcVar5[0x4c],pcVar5[0x4d],*pcVar5,0xe7,pcVar5 + 0x20,pcVar5 + 0x40,
                     (int)*(short *)(pcVar5 + 0x38),pcVar5 + 0x30,pcVar5);
        cVar6 = pcVar5[0x4c];
        cVar16 = pcVar5[0x4d];
        cVar18 = *pcVar5;
        iVar12 = (int)*(short *)(pcVar5 + 0x38);
        uVar9 = 0x86;
      }
      else if (cVar6 == -0x28) {
        func_80032404(pcVar5[0x4c],pcVar5[0x4d],*pcVar5,0xd9,pcVar5 + 0x20,pcVar5 + 0x40,
                     (int)*(short *)(pcVar5 + 0x38),pcVar5 + 0x30,pcVar5);
        cVar6 = pcVar5[0x4c];
        cVar16 = pcVar5[0x4d];
        cVar18 = *pcVar5;
        iVar12 = (int)*(short *)(pcVar5 + 0x38);
        uVar9 = 0x5a;
      }
      else if (cVar6 == -0x25) {
        func_80032404(pcVar5[0x4c],pcVar5[0x4d],*pcVar5,0xdc,pcVar5 + 0x20,pcVar5 + 0x40,
                     (int)*(short *)(pcVar5 + 0x38),pcVar5 + 0x30,pcVar5);
        cVar6 = pcVar5[0x4c];
        cVar16 = pcVar5[0x4d];
        cVar18 = *pcVar5;
        iVar12 = (int)*(short *)(pcVar5 + 0x38);
        uVar9 = 100;
      }
      else {
        if (cVar6 != -0x22) goto LAB_80034290;
        func_80032404(pcVar5[0x4c],pcVar5[0x4d],*pcVar5,0xdf,pcVar5 + 0x20,pcVar5 + 0x40,
                     (int)*(short *)(pcVar5 + 0x38),pcVar5 + 0x30,pcVar5);
        cVar6 = pcVar5[0x4c];
        cVar16 = pcVar5[0x4d];
        cVar18 = *pcVar5;
        iVar12 = (int)*(short *)(pcVar5 + 0x38);
        uVar9 = 0x7e;
      }
      func_80032404(cVar6,cVar16,cVar18,uVar9,pcVar5 + 0x20,pcVar5 + 0x40,iVar12,pcVar5 + 0x30,pcVar5
                  );
    }
LAB_80034290:
    if (*(short *)(pcVar5 + 0x10) < 0x18) {
      *(short *)(pcVar5 + 0x38) = *(short *)(pcVar5 + 0x38) + -0x40;
      *(short *)(pcVar5 + 0x3c) = *(short *)(pcVar5 + 0x3c) + -0x40;
      sVar8 = *(short *)(pcVar5 + 0x3a) + -0x40;
    }
    else {
      *(short *)(pcVar5 + 0x38) = *(short *)(pcVar5 + 0x38) + 0x40;
      *(short *)(pcVar5 + 0x3c) = *(short *)(pcVar5 + 0x3c) + 0x40;
      sVar8 = *(short *)(pcVar5 + 0x3a) + 0x40;
    }
    *(short *)(pcVar5 + 0x3a) = sVar8;
    iVar12 = (int)*(short *)(pcVar5 + 0x10);
    bVar4 = iVar12 < 0x20;
    goto LAB_800351c0;
  case -0x35:
  case -0x32:
  case -0x2f:
  case -0x27:
  case -0x24:
  case -0x21:
  case -0x1e:
  case -0x19:
  case -0x14:
    iVar12 = *(int *)(D_801d1b9c + 0x1c);
    *(undefined4 *)(D_801d1b9c + 0x20) = *(undefined4 *)(*(int *)(D_801d1b9c + 0x1c) + 0x20);
    *(undefined4 *)(pcVar5 + 0x24) = *(undefined4 *)(iVar12 + 0x24);
    sVar8 = *(short *)(pcVar5 + 0x50);
    uVar9 = *(undefined4 *)(iVar12 + 0x28);
    *(short *)(pcVar5 + 0x50) = sVar8 + 0x20;
    *(undefined4 *)(pcVar5 + 0x28) = uVar9;
    if (-0x81 < (short)(sVar8 + 0x20)) {
      pcVar5[0x50] = -0x80;
      pcVar5[0x51] = -1;
    }
    if ((pcVar5[1] == -0x21) || (pcVar5[1] == -0x1e)) {
      iVar12 = rcos((int)*(short *)(pcVar5 + 0x50));
      iVar10 = rsin((int)*(short *)(pcVar5 + 0x50));
      iVar15 = *(int *)(*(int *)(pcVar5 + 0x1c) + 0x24);
      iVar14 = *(int *)(D_80063300 + 0x4def4) + (int)*(short *)(D_80063300 + 0x4e0ca);
    }
    else {
      iVar12 = rcos((int)*(short *)(pcVar5 + 0x50));
      iVar10 = rsin((int)*(short *)(pcVar5 + 0x50));
      iVar14 = *(int *)(D_80063300 + 0x4def4);
      iVar15 = *(int *)(*(int *)(pcVar5 + 0x1c) + 0x24);
    }
    iVar14 = iVar14 - iVar15;
    iVar15 = (iVar14 * iVar12) / iVar10;
    if (iVar10 == 0) {
      trap(0x1c00);
    }
    if ((iVar10 == -1) && (iVar14 * iVar12 == -0x80000000)) {
      trap(0x1800);
    }
    uVar7 = SquareRoot0(iVar15 * iVar15 + iVar14 * iVar14);
    *(undefined2 *)(pcVar5 + 0x3c) = uVar7;
    *(short *)(pcVar5 + 0x30) = -*(short *)(pcVar5 + 0x50);
    if (*(short *)(pcVar5 + 0x10) < 8) {
      *(short *)(pcVar5 + 0x12) = (short)((int)*(short *)(pcVar5 + 0x10) << 9);
    }
    break;
  case -0x34:
  case -0x31:
  case -0x2e:
  case -0x26:
  case -0x23:
  case -0x20:
  case -0x1d:
  case -0x18:
  case -0x13:
    psVar1 = (short *)(D_801d1b9c + 0x42);
    sVar8 = *(short *)(D_801d1b9c + 0x44);
    *(int *)(D_801d1b9c + 0x20) =
         *(int *)(D_801d1b9c + 0x20) + (int)*(short *)(D_801d1b9c + 0x40);
    *(int *)(pcVar5 + 0x24) = *(int *)(pcVar5 + 0x24) + (int)*psVar1;
    sVar11 = *(short *)(pcVar5 + 0x38);
    *(int *)(pcVar5 + 0x28) = *(int *)(pcVar5 + 0x28) + (int)sVar8;
    *(short *)(pcVar5 + 0x38) = sVar11 + *(short *)(pcVar5 + 0x50);
    *(short *)(pcVar5 + 0x3c) = sVar11 + *(short *)(pcVar5 + 0x50);
    if ((byte)pcVar5[5] < 3) {
      sVar8 = *(short *)(pcVar5 + 0x38) << 1;
    }
    else {
      sVar8 = *(short *)(pcVar5 + 0x3a) - *(short *)(pcVar5 + 0x50);
    }
    *(short *)(pcVar5 + 0x3a) = sVar8;
    pcVar5[5] = pcVar5[5] + '\x01';
    break;
  case -0x2b:
    uVar7 = *(undefined2 *)(D_801d1b9c + 0xc);
    uVar9 = *(undefined4 *)(D_801d1b9c + 0x18);
    cVar6 = D_801d1b9c[0x50];
    psVar1 = (short *)(D_801d1b9c + 0x3a);
    *(short *)(D_801d1b9c + 0x38) = *(short *)(D_801d1b9c + 0x38) + 0x60;
    *(short *)(pcVar5 + 0x3a) = *psVar1 + 0x60;
    func_8003333c(uVar7,uVar9,(int)cVar6);
    sVar8 = *(short *)(pcVar5 + 0x10);
    sVar11 = *(short *)(pcVar5 + 0x42) + -0x10;
    goto LAB_80034e1c;
  case -0x2a:
    iVar12 = *(int *)(D_801d1b9c + 0x1c);
    *(undefined4 *)(D_801d1b9c + 0x20) = *(undefined4 *)(*(int *)(D_801d1b9c + 0x1c) + 0x20);
    *(undefined4 *)(pcVar5 + 0x24) = *(undefined4 *)(iVar12 + 0x24);
    sVar11 = *(short *)(pcVar5 + 0x10);
    *(undefined4 *)(pcVar5 + 0x28) = *(undefined4 *)(iVar12 + 0x28);
    *(undefined2 *)(pcVar5 + 0x38) = *(undefined2 *)(*(int *)(pcVar5 + 0x1c) + 0x38);
    *(undefined2 *)(pcVar5 + 0x3a) = *(undefined2 *)(*(int *)(pcVar5 + 0x1c) + 0x3a);
    *(undefined2 *)(pcVar5 + 0x3c) = *(undefined2 *)(*(int *)(pcVar5 + 0x1c) + 0x3c);
    sVar8 = *(short *)(pcVar5 + 0x10);
    pcVar5[2] = (char)sVar11 +
                ((char)((ulonglong)((longlong)(int)sVar11 * 0x55555556) >> 0x20) -
                (char)(sVar11 >> 0xf)) * -3 | 0x80;
LAB_80034b68:
    if (sVar8 < 0x20) {
      *(short *)(pcVar5 + 0x12) = (short)((int)sVar8 << 7);
    }
    cVar6 = **(char **)(pcVar5 + 0x1c);
    if (cVar6 == -1) {
LAB_80034b8c:
      *pcVar5 = cVar6;
    }
    break;
  case -0x29:
    uVar7 = *(undefined2 *)(D_801d1b9c + 0xc);
    uVar9 = *(undefined4 *)(D_801d1b9c + 0x18);
    cVar6 = D_801d1b9c[0x50];
    psVar1 = (short *)(D_801d1b9c + 0x3a);
    *(short *)(D_801d1b9c + 0x38) = *(short *)(D_801d1b9c + 0x38) + 0x60;
    *(short *)(pcVar5 + 0x3a) = *psVar1 + 0x60;
    func_8003333c(uVar7,uVar9,(int)cVar6);
    sVar8 = *(short *)(pcVar5 + 0x10);
    sVar11 = *(short *)(pcVar5 + 0x42) + 2;
LAB_80034e1c:
    *(short *)(pcVar5 + 0x12) = sVar8 << 8;
    *(short *)(pcVar5 + 0x42) = sVar11;
    break;
  case -0x17:
    cVar6 = **(char **)(D_801d1b9c + 0x1c);
    if (cVar6 == -1) goto LAB_80034b8c;
    iVar12 = rcos((int)*(short *)(*(char **)(D_801d1b9c + 0x1c) + 0x34));
    iVar12 = *(short *)(*(int *)(pcVar5 + 0x1c) + 0x38) * iVar12;
    if (iVar12 < 0) {
      iVar12 = iVar12 + 0xfff;
    }
    iVar10 = rcos((int)*(short *)(*(int *)(pcVar5 + 0x1c) + 0x32));
    iVar10 = (iVar12 >> 0xc) * iVar10;
    if (iVar10 < 0) {
      iVar10 = iVar10 + 0xfff;
    }
    iVar14 = rsin((int)*(short *)(*(int *)(pcVar5 + 0x1c) + 0x32));
    iVar14 = (iVar12 >> 0xc) * iVar14;
    if (iVar14 < 0) {
      iVar14 = iVar14 + 0xfff;
    }
    rsin((int)*(short *)(*(int *)(pcVar5 + 0x1c) + 0x34));
    *(int *)(pcVar5 + 0x20) = *(int *)(*(int *)(pcVar5 + 0x1c) + 0x20) + (iVar10 >> 0xc);
    iVar12 = (int)*(short *)(*(int *)(pcVar5 + 0x1c) + 0x34);
LAB_8003516c:
    iVar12 = rsin(iVar12);
    iVar12 = *(short *)(*(int *)(pcVar5 + 0x1c) + 0x38) * iVar12;
    if (iVar12 < 0) {
      iVar12 = iVar12 + 0xfff;
    }
    *(int *)(pcVar5 + 0x24) = *(int *)(*(int *)(pcVar5 + 0x1c) + 0x24) + (iVar12 >> 0xc);
    iVar12 = (int)*(short *)(pcVar5 + 0x10);
    *(int *)(pcVar5 + 0x28) = *(int *)(*(int *)(pcVar5 + 0x1c) + 0x28) + (iVar14 >> 0xc);
    bVar4 = iVar12 < 0x20;
LAB_800351c0:
    if (bVar4) {
      *(short *)(pcVar5 + 0x12) = (short)(iVar12 << 7);
    }
    break;
  case -0x16:
    cVar6 = **(char **)(D_801d1b9c + 0x1c);
    if (cVar6 != -1) {
      iVar12 = rcos(*(short *)(*(char **)(D_801d1b9c + 0x1c) + 0x34) + 0x800);
      iVar12 = *(short *)(*(int *)(pcVar5 + 0x1c) + 0x38) * iVar12;
      if (iVar12 < 0) {
        iVar12 = iVar12 + 0xfff;
      }
      iVar10 = rcos((int)*(short *)(*(int *)(pcVar5 + 0x1c) + 0x32));
      iVar10 = (iVar12 >> 0xc) * iVar10;
      if (iVar10 < 0) {
        iVar10 = iVar10 + 0xfff;
      }
      iVar14 = rsin((int)*(short *)(*(int *)(pcVar5 + 0x1c) + 0x32));
      iVar14 = (iVar12 >> 0xc) * iVar14;
      if (iVar14 < 0) {
        iVar14 = iVar14 + 0xfff;
      }
      rsin(*(short *)(*(int *)(pcVar5 + 0x1c) + 0x34) + 0x800);
      *(int *)(pcVar5 + 0x20) = *(int *)(*(int *)(pcVar5 + 0x1c) + 0x20) + (iVar10 >> 0xc);
      iVar12 = *(short *)(*(int *)(pcVar5 + 0x1c) + 0x34) + 0x800;
      goto LAB_8003516c;
    }
    goto LAB_80034b8c;
  }
  return;
}
