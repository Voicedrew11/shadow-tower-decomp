/*
 * VS.EXE: effect spawn (VS's copy of ST.EXE's func_80036484): allocates an effect record and sets it up per kind.
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

/* func_80032404's switch table. */
JUMPTABLES(0x80011934, 0x300)

#define D_800b147e (*(s32 *)((u8 *)&DAT_800633b0 + 0x4e0ce))

/* WARNING: Removing unreachable block (ram,0x80032fcc) */

/* Not exact (one attempt; 14.3%): first divergence at +0x0, the prologue: retail keeps the five register arguments in other saved registers and points $s0 at the first stacked argument; this transcription merges case tails Ghidra joined, so it is 0x28c bytes shorter. */
ADDRESS(0x80032404, 0xf38)
undefined1 * func_80032404(uint param_1, uint param_2, undefined1 param_3, undefined1 param_4, undefined4 *param_5, ...)
{
  int *args = (int *)(&param_5 + 1);
  undefined1 *puVar1;
  ushort uVar2;
  ushort uVar3;
  undefined1 uVar4;
  char cVar5;
  byte bVar6;
  undefined2 uVar7;
  undefined1 *puVar8;
  uint uVar9;
  int iVar10;
  undefined4 uVar11;
  uint uVar12;
  uint uVar13;
  undefined **ppuVar14;
  int iVar15;
  uint uVar16;
  uint uVar17;
  undefined4 uVar18;
  
  uVar16 = param_1;
  uVar17 = param_2;
  uVar18 = func_800322f0();
  puVar8 = (undefined1 *)uVar18;
  if (puVar8 == (undefined1 *)0x0) {
    return (undefined1 *)0x0;
  }
  *puVar8 = param_3;
  puVar8[1] = param_4;
  if (param_5 != (undefined4 *)0x0) {
    uVar11 = *(undefined4 *)((int)param_5 + 4);
    uVar18 = *param_5;
    uVar16 = *(uint *)(param_5 + 1);
    uVar17 = *(uint *)((int)param_5 + 0xc);
    *(undefined4 *)(puVar8 + 0x20) = *(undefined4 *)param_5;
    *(undefined4 *)(puVar8 + 0x24) = uVar11;
    *(uint *)(puVar8 + 0x28) = uVar16;
    *(uint *)(puVar8 + 0x2c) = uVar17;
  }
  if (args[0] == 0) {
    *(undefined2 *)(puVar8 + 0x44) = 0;
    *(undefined2 *)(puVar8 + 0x42) = 0;
    *(undefined2 *)(puVar8 + 0x40) = 0;
  }
  else {
    *(C8 *)(puVar8 + 0x40) = *(C8 *)(args[0]);
  }
  *(undefined2 *)(puVar8 + 0x3c) = 0x1000;
  *(undefined2 *)(puVar8 + 0x3a) = 0x1000;
  *(undefined2 *)(puVar8 + 0x38) = 0x1000;
  puVar8[6] = 1;
  puVar8[9] = 0xff;
  puVar8[5] = 0;
  puVar8[4] = 0;
  *(undefined2 *)(puVar8 + 0x34) = 0;
  *(undefined2 *)(puVar8 + 0x32) = 0;
  *(undefined2 *)(puVar8 + 0x30) = 0;
  *(undefined2 *)(puVar8 + 0x14) = 0;
  puVar8[3] = 0;
  puVar8[8] = 0;
  *(undefined2 *)(puVar8 + 0x12) = 0;
  puVar8[7] = 0;
  *(undefined2 *)(puVar8 + 0x10) = 0xffff;
  *(undefined2 *)(puVar8 + 0xe) = 0;
  *(undefined4 *)(puVar8 + 0x1c) = 0;
  puVar8[0x4c] = (char)param_1;
  uVar13 = (byte)puVar8[1] - 0x30;
  puVar8[0x4d] = (char)param_2;
  if (0xbf < uVar13) {
switchD_80032548_caseD_3b:
    *puVar8 = 0xff;
    return puVar8;
  }
  ppuVar14 = &DAT_80011934 + uVar13;
  switch(puVar8[1]) {
  case 0x30:
  case 0x38:
    *(undefined2 *)(puVar8 + 0x10) = 0x60;
    puVar8[2] = 0x80;
    uVar4 = 7;
    goto LAB_8003258c;
  default:
    puVar8[6] = 5;
    uVar7 = 0x40;
    if (puVar8[1] == -0x79) {
      uVar7 = 0x80;
    }
    *(undefined2 *)(puVar8 + 0x10) = uVar7;
    iVar10 = rand();
    uVar16 = iVar10 * 3;
    if ((int)uVar16 < 0) {
      uVar16 = uVar16 + 0x7fff;
    }
    bVar6 = (byte)(uVar16 >> 0xf);
    if (2 < (uVar16 >> 0xf & 0xff)) {
      bVar6 = 2;
    }
    cVar5 = puVar8[1];
    if (cVar5 == '`') {
      bVar6 = bVar6 + 4;
    }
    if (cVar5 == -0x67) {
      bVar6 = bVar6 + 3;
    }
    if (cVar5 == 'y') {
      bVar6 = bVar6 + 2;
    }
    if (cVar5 == -0x7b) {
      bVar6 = bVar6 + 4;
    }
    if (cVar5 == -0x5d) {
      bVar6 = bVar6 + 2;
    }
    cVar5 = puVar8[1];
    puVar8[2] = bVar6 | 0x80;
    if ((cVar5 == '4') ||
       ((uVar4 = 0xe, cVar5 != -0x71 &&
        (((cVar5 == -0x7b || (cVar5 == 'K')) || (uVar4 = 0xf, cVar5 == 's')))))) {
      uVar4 = 0xe;
    }
    puVar8[7] = uVar4;
    *(undefined2 *)(puVar8 + 0x12) = 0x1000;
    iVar10 = (int)args[1];
    *(short *)(puVar8 + 0x3c) = args[1];
    *(short *)(puVar8 + 0x3a) = args[1];
    *(short *)(puVar8 + 0x38) = args[1];
    if (iVar10 < 0) {
      iVar10 = iVar10 + 7;
    }
    iVar15 = (int)*(short *)(puVar8 + 0x38);
    *(short *)(puVar8 + 0xc) = (short)(iVar10 >> 3);
    if (iVar15 < 0) {
      iVar15 = iVar15 + 7;
    }
    cVar5 = puVar8[1];
    *(uint *)(puVar8 + 0x18) = iVar15 >> 3 | 0x80000000;
    if (((cVar5 == 'V') || (cVar5 == '_')) || ((cVar5 == 'y' || (cVar5 == 'p')))) {
      iVar10 = rand();
      cVar5 = (char)(iVar10 >> 9) + -0x80;
    }
    else {
      iVar10 = rand();
      cVar5 = (char)(iVar10 >> 0xb) + '\x10';
    }
    puVar8[0x50] = cVar5;
    puVar8[8] = (undefined1)args[2];
    break;
  case 0x35:
  case 0x53:
  case 0x82:
  case 0x8d:
    uVar7 = 0x60;
    goto LAB_800329a0;
  case 0x3a:
    uVar7 = 0x20;
    goto LAB_80033064;
  case 0x3b:
  case 0x44:
  case 0x4c:
  case 0x4e:
  case 0x50:
  case 0x54:
  case 0x5b:
  case 0x9a:
  case 0xaa:
  case 0xab:
  case 0xac:
  case 0xad:
  case 0xae:
  case 0xaf:
  case 0xb0:
  case 0xb1:
  case 0xb2:
  case 0xb3:
  case 0xb4:
  case 0xb5:
  case 0xb6:
  case 0xb7:
  case 0xb8:
  case 0xb9:
  case 0xba:
  case 0xbb:
  case 0xbc:
  case 0xbd:
  case 0xbe:
  case 0xbf:
  case 0xc0:
  case 0xc1:
  case 0xc2:
  case 0xc3:
  case 0xc4:
  case 0xc5:
  case 0xc6:
  case 199:
    goto switchD_80032548_caseD_3b;
  case 0x3d:
  case 0x46:
  case 0x52:
  case 0x5a:
  case 100:
  case 0x7e:
  case 0x81:
  case 0x86:
  case 0x8a:
    puVar8[6] = 5;
    *(undefined2 *)(puVar8 + 0x10) = 0x20;
    puVar8[2] = 0x80;
    puVar8[7] = 0xd;
    *(undefined2 *)(puVar8 + 0x12) = 0x1000;
    *(short *)(puVar8 + 0x3c) = args[1];
    *(short *)(puVar8 + 0x3a) = args[1];
    *(short *)(puVar8 + 0x38) = args[1];
    *(C8 *)(puVar8 + 0x30) = *(C8 *)(args[2]);
    if ((puVar8[1] == '~') || (puVar8[1] == -0x7f)) {
      iVar10 = (short)(&D_800b147e)[(uint)(byte)puVar8[0x4c] * 0x27074] + 0x640;
    }
    else {
      iVar10 = (int)(short)(&D_800b147e)[(uint)(byte)puVar8[0x4c] * 0x27074];
    }
    *(int *)(puVar8 + 0x24) = iVar10;
    *(undefined2 *)(puVar8 + 0x32) = 0;
    *(undefined2 *)(puVar8 + 0x40) = 0;
    *(undefined2 *)(puVar8 + 0x42) = 0;
    *(undefined2 *)(puVar8 + 0x44) = 0;
    puVar8[8] = 1;
    break;
  case 0x3e:
  case 0x41:
  case 0x48:
  case 0x5e:
  case 0x6c:
  case 0x71:
  case 0x7a:
  case 0x89:
  case 0x93:
  case 0xa4:
    *(undefined2 *)(puVar8 + 0x10) = 0x40;
    puVar8[2] = 0x83;
    puVar8[6] = 1;
    if (puVar8[1] == '^') {
      puVar8[2] = 0x80;
    }
    if ((byte)puVar8[1] == 0xa4) {
      puVar8[2] = 0x85;
    }
    puVar8[7] = 0xd;
    *(undefined2 *)(puVar8 + 0x12) = 0x1000;
    *(short *)(puVar8 + 0x3c) = args[1];
    *(short *)(puVar8 + 0x3a) = args[1];
    *(short *)(puVar8 + 0x38) = args[1];
    *(C8 *)(puVar8 + 0x30) = *(C8 *)(args[2]);
    uVar2 = *(ushort *)(puVar8 + 0x38);
    uVar3 = *(ushort *)(puVar8 + 0x38);
    puVar8[8] = 1;
    goto LAB_80032f3c;
  case 0x40:
  case 0x88:
  case 0x8b:
  case 0xa2:
  case 0xa8:
    uVar16 = (uint)(byte)puVar8[1];
    puVar8[6] = 1;
    *(undefined2 *)(puVar8 + 0x10) = 0x40;
    puVar8[2] = 0x80;
    if (uVar16 == 0xa8) {
      puVar8[2] = 0x84;
    }
    uVar4 = 0xd;
    if ((byte)puVar8[1] == 0x40) {
      puVar8[2] = 0x84;
      uVar4 = 0xe;
      uVar16 = 0x40;
    }
    puVar8[7] = uVar4;
    *(undefined2 *)(puVar8 + 0x12) = 0x1000;
    *(short *)(puVar8 + 0x3c) = args[1];
    *(short *)(puVar8 + 0x3a) = args[1];
    *(short *)(puVar8 + 0x38) = args[1];
    *(C8 *)(puVar8 + 0x30) = *(C8 *)(args[2]);
    puVar8[8] = 1;
    *(short *)(puVar8 + 0xc) =
         (short)(((int)((uint)*(ushort *)(puVar8 + 0x38) << 0x10) >> 0x10) -
                 ((int)((uint)*(ushort *)(puVar8 + 0x38) << 0x10) >> 0x1f) >> 1);
    *(int *)(puVar8 + 0x18) =
         ((int)((uint)*(ushort *)(puVar8 + 0x3a) << 0x10) >> 0x10) -
         ((int)((uint)*(ushort *)(puVar8 + 0x3a) << 0x10) >> 0x1f) >> 1;
    break;
  case 0x55:
    *(undefined2 *)(puVar8 + 0x10) = 0x40;
    uVar4 = 0x86;
    goto LAB_80032d74;
  case 0x57:
  case 0x58:
  case 0x5c:
  case 0x61:
  case 0x62:
  case 99:
  case 0x67:
  case 0x68:
  case 0x74:
  case 0x8c:
  case 0x95:
  case 0x9c:
  case 0x9e:
    *(undefined2 *)(puVar8 + 0x10) = 0x40;
    puVar8[2] = 0x83;
    puVar8[7] = 0xf;
    puVar8[6] = 1;
    *(undefined2 *)(puVar8 + 0x12) = 0x1000;
    iVar10 = args[1] * 0x600;
    *(short *)(puVar8 + 0x3c) = args[1];
    *(short *)(puVar8 + 0x3a) = args[1];
    *(short *)(puVar8 + 0x38) = args[1];
    if (iVar10 < 0) {
      iVar10 = iVar10 + 0xfff;
    }
    *(short *)(puVar8 + 0xc) = (short)(iVar10 >> 0xc);
    iVar10 = *(short *)(puVar8 + 0x38) * 0x600;
    if (iVar10 < 0) {
      iVar10 = iVar10 + 0xfff;
    }
    *(uint *)(puVar8 + 0x18) = iVar10 >> 0xc | 0x80000000;
    ((undefined1 * (*)())func_80032404)(puVar8[0x4c],puVar8[0x4d],*puVar8,0xd6,puVar8 + 0x20,puVar8 + 0x40,
                 (int)*(short *)(puVar8 + 0x38),puVar8);
    puVar8[8] = 1;
    break;
  case 0x59:
    *(undefined2 *)(puVar8 + 0x10) = 0x40;
    uVar4 = 0x83;
    goto LAB_80032d74;
  case 0x5d:
    *(undefined2 *)(puVar8 + 0x10) = 0x40;
    uVar4 = 0x84;
    goto LAB_8003306c;
  case 0x65:
  case 0x75:
  case 0x77:
  case 0x80:
  case 0xa6:
    puVar8[6] = 5;
    *(undefined2 *)(puVar8 + 0x10) = 0x40;
    puVar8[2] = 0x80;
    puVar8[7] = 0xd;
    *(undefined2 *)(puVar8 + 0x12) = 0x1000;
    *(short *)(puVar8 + 0x3c) = args[1];
    *(short *)(puVar8 + 0x3a) = args[1];
    *(short *)(puVar8 + 0x38) = args[1];
    *(C8 *)(puVar8 + 0x30) = *(C8 *)(args[2]);
    iVar10 = (int)*(short *)(puVar8 + 0x38);
    *(undefined2 *)(puVar8 + 0x30) = 0;
    *(undefined2 *)(puVar8 + 0x32) = 0;
    *(undefined2 *)(puVar8 + 0x34) = 0;
    if (iVar10 < 0) {
      iVar10 = iVar10 + 7;
    }
    iVar15 = (int)*(short *)(puVar8 + 0x38);
    *(short *)(puVar8 + 0xc) = (short)(iVar10 >> 3);
    if (iVar15 < 0) {
      iVar15 = iVar15 + 7;
    }
    *(uint *)(puVar8 + 0x18) = iVar15 >> 3 | 0x80000000;
    goto LAB_80032e98;
  case 0x66:
    *(undefined2 *)(puVar8 + 0x10) = 0x60;
    uVar4 = 0x80;
    goto LAB_80032584;
  case 0x69:
    *(undefined2 *)(puVar8 + 0x10) = 0x40;
    uVar4 = 0x84;
    goto LAB_80032edc;
  case 0x76:
    *(undefined2 *)(puVar8 + 0x10) = 0x40;
    uVar4 = 0x81;
LAB_80032edc:
    puVar8[2] = uVar4;
    puVar8[7] = 0xd;
    puVar8[6] = 1;
    *(undefined2 *)(puVar8 + 0x12) = 0x1000;
    *(short *)(puVar8 + 0x3c) = args[1];
    *(short *)(puVar8 + 0x3a) = args[1];
    *(short *)(puVar8 + 0x38) = args[1];
    *(C8 *)(puVar8 + 0x30) = *(C8 *)(args[2]);
    uVar2 = *(ushort *)(puVar8 + 0x38);
    uVar3 = *(ushort *)(puVar8 + 0x38);
    puVar8[8] = 1;
LAB_80032f3c:
    *(short *)(puVar8 + 0xc) =
         (short)(((int)((uint)uVar2 << 0x10) >> 0x10) - ((int)((uint)uVar2 << 0x10) >> 0x1f) >> 1);
    *(uint *)(puVar8 + 0x18) =
         ((int)((uint)uVar3 << 0x10) >> 0x10) - ((int)((uint)uVar3 << 0x10) >> 0x1f) >> 1 |
         0x80000000;
    break;
  case 0x78:
  case 0x79:
    uVar7 = 0x40;
LAB_80033064:
    *(undefined2 *)(puVar8 + 0x10) = uVar7;
    uVar4 = 0x80;
LAB_8003306c:
    puVar8[2] = uVar4;
    puVar8[6] = 1;
    puVar8[7] = 0;
    goto LAB_8003307c;
  case 0x7c:
  case 0x83:
  case 0x84:
  case 0x8e:
  case 0x91:
  case 0x96:
  case 0xa0:
  case 0xa7:
  case 0xa9:
    *(undefined2 *)(puVar8 + 0x10) = 0x40;
    puVar8[2] = 0x80;
    puVar8[7] = 0xd;
    puVar8[6] = 1;
    *(undefined2 *)(puVar8 + 0x12) = 0x1000;
    *(short *)(puVar8 + 0x3c) = args[1];
    *(short *)(puVar8 + 0x3a) = args[1];
    *(short *)(puVar8 + 0x38) = args[1];
    *(C8 *)(puVar8 + 0x30) = *(C8 *)(args[2]);
    iVar10 = rcos((int)*(short *)(puVar8 + 0x34));
    iVar10 = *(short *)(puVar8 + 0x38) * iVar10;
    if (iVar10 < 0) {
      iVar10 = iVar10 + 0xfff;
    }
    iVar15 = *(int *)(puVar8 + 0x20) + (iVar10 >> 0xc);
    iVar10 = rsin((int)*(short *)(puVar8 + 0x34));
    iVar10 = *(short *)(puVar8 + 0x38) * iVar10;
    if (iVar10 < 0) {
      iVar10 = iVar10 + 0xfff;
    }
    iVar10 = *(int *)(puVar8 + 0x24) + (iVar10 >> 0xc);
    rcos(*(short *)(puVar8 + 0x34) + 0x800);
    rsin(*(short *)(puVar8 + 0x34) + 0x800);
    ((undefined1 * (*)())func_80032404)(puVar8[0x4c],puVar8[0x4d],*puVar8,0xe9,puVar8 + 0x20,puVar8 + 0x40,
                 (int)*(short *)(puVar8 + 0x38),puVar8,iVar15,iVar10);
    ((undefined1 * (*)())func_80032404)(puVar8[0x4c],puVar8[0x4d],*puVar8,0xea,puVar8 + 0x20,puVar8 + 0x40,
                 (int)*(short *)(puVar8 + 0x38),puVar8);
    puVar8[8] = 1;
    break;
  case 0x7d:
  case 0xa1:
    *(undefined2 *)(puVar8 + 0x10) = 0x60;
    uVar4 = 0x84;
LAB_80032584:
    puVar8[2] = uVar4;
    uVar4 = 5;
LAB_8003258c:
    puVar8[7] = uVar4;
    puVar8[9] = 0x33;
    *(undefined2 *)(puVar8 + 0x12) = 0x1000;
    *(short *)(puVar8 + 0x3c) = args[1];
    *(short *)(puVar8 + 0x3a) = args[1];
    *(short *)(puVar8 + 0x38) = args[1];
    *(C8 *)(puVar8 + 0x30) = *(C8 *)(args[2]);
    iVar10 = *(short *)(puVar8 + 0x38) * 0x380;
    if (iVar10 < 0) {
      iVar10 = iVar10 + 0xfff;
    }
    *(short *)(puVar8 + 0xc) = (short)(iVar10 >> 0xc);
    iVar10 = *(short *)(puVar8 + 0x38) * 0x380;
    if (iVar10 < 0) {
      iVar10 = iVar10 + 0xfff;
    }
    *(uint *)(puVar8 + 0x18) = iVar10 >> 0xc | 0x80000000;
    puVar8[8] = (undefined1)args[3];
    break;
  case 0x97:
    *(undefined2 *)(puVar8 + 0x10) = 0x40;
    uVar4 = 0x84;
LAB_80032d74:
    puVar8[2] = uVar4;
    puVar8[7] = 0xf;
    puVar8[6] = 1;
    *(undefined2 *)(puVar8 + 0x12) = 0x1000;
    iVar10 = args[1] * 0x600;
    *(short *)(puVar8 + 0x3c) = args[1];
    *(short *)(puVar8 + 0x3a) = args[1];
    *(short *)(puVar8 + 0x38) = args[1];
    if (iVar10 < 0) {
      iVar10 = iVar10 + 0xfff;
    }
    *(short *)(puVar8 + 0xc) = (short)(iVar10 >> 0xc);
    iVar10 = *(short *)(puVar8 + 0x38) * 0x600;
    if (iVar10 < 0) {
      iVar10 = iVar10 + 0xfff;
    }
    *(uint *)(puVar8 + 0x18) = iVar10 >> 0xc | 0x80000000;
    puVar8[8] = 1;
    break;
  case 200:
  case 0xd3:
  case 0xe4:
  case 0xee:
    *(undefined2 *)(puVar8 + 0x10) = 0x20;
    puVar8[2] = 0x82;
    puVar8[7] = 0xd;
    puVar8[6] = 1;
    goto LAB_8003307c;
  case 0xc9:
  case 0xd4:
  case 0xe5:
  case 0xef:
    *(undefined2 *)(puVar8 + 0x10) = 0x60;
    uVar4 = 0x80;
    goto LAB_800329a8;
  case 0xca:
  case 0xcd:
  case 0xd0:
  case 0xd8:
  case 0xdb:
  case 0xde:
  case 0xe1:
  case 0xe6:
  case 0xeb:
    *(undefined2 *)(puVar8 + 0x10) = 0x24;
    puVar8[2] = 0x82;
    puVar8[7] = 0xd;
    puVar8[6] = 1;
LAB_8003307c:
    *(undefined2 *)(puVar8 + 0x12) = 0x1000;
    *(short *)(puVar8 + 0x3c) = args[1];
    *(short *)(puVar8 + 0x3a) = args[1];
    *(short *)(puVar8 + 0x38) = args[1];
    *(C8 *)(puVar8 + 0x30) = *(C8 *)(args[2]);
    iVar10 = (int)*(short *)(puVar8 + 0x38);
    if (iVar10 < 0) {
      iVar10 = iVar10 + 7;
    }
    iVar15 = (int)*(short *)(puVar8 + 0x38);
    *(short *)(puVar8 + 0xc) = (short)(iVar10 >> 3);
    if (iVar15 < 0) {
      iVar15 = iVar15 + 7;
    }
    *(uint *)(puVar8 + 0x18) = iVar15 >> 3 | 0x80000000;
    puVar8[8] = 1;
    break;
  case 0xcb:
  case 0xce:
  case 0xd1:
  case 0xd9:
  case 0xdc:
  case 0xdf:
  case 0xe2:
  case 0xe7:
  case 0xec:
    uVar7 = 0x20;
LAB_800329a0:
    *(undefined2 *)(puVar8 + 0x10) = uVar7;
    uVar4 = 0x81;
LAB_800329a8:
    puVar8[2] = uVar4;
    puVar8[7] = 0xd;
    puVar8[6] = 1;
    *(undefined2 *)(puVar8 + 0x12) = 0x1000;
    *(short *)(puVar8 + 0x3c) = args[1];
    *(short *)(puVar8 + 0x3a) = args[1];
    *(short *)(puVar8 + 0x38) = args[1];
    *(C8 *)(puVar8 + 0x30) = *(C8 *)(args[2]);
    iVar10 = (int)*(short *)(puVar8 + 0x38);
    if (iVar10 < 0) {
      iVar10 = iVar10 + 7;
    }
    iVar15 = (int)*(short *)(puVar8 + 0x38);
    *(short *)(puVar8 + 0xc) = (short)(iVar10 >> 3);
    if (iVar15 < 0) {
      iVar15 = iVar15 + 7;
    }
    *(uint *)(puVar8 + 0x18) = iVar15 >> 3 | 0x80000000;
    *(undefined2 *)(puVar8 + 0x50) = 0xfc80;
    puVar8[8] = 1;
    *(undefined4 *)(puVar8 + 0x1c) = args[3];
    break;
  case 0xcc:
  case 0xcf:
  case 0xd2:
  case 0xda:
  case 0xdd:
  case 0xe0:
  case 0xe3:
  case 0xe8:
  case 0xed:
    puVar8[6] = 5;
    puVar8[9] = 0x33;
    *(undefined2 *)(puVar8 + 0x12) = 0x1000;
    puVar8[7] = args[4] | 4;
    puVar8[2] = 0x83;
    *(short *)(puVar8 + 0x3c) = args[1];
    *(short *)(puVar8 + 0x3a) = args[1];
    *(short *)(puVar8 + 0x38) = args[1];
    *(undefined2 *)(puVar8 + 0x50) = (undefined2)args[2];
    puVar8[8] = 1;
    *(undefined2 *)(puVar8 + 0x10) = (undefined2)args[3];
    break;
  case 0xd5:
    puVar8[6] = 5;
    *(undefined2 *)(puVar8 + 0x10) = 0x10;
    iVar10 = rand();
    uVar16 = iVar10 * 3;
    if ((int)uVar16 < 0) {
      uVar16 = uVar16 + 0x7fff;
    }
    bVar6 = (char)(uVar16 >> 0xf) + 3;
    if (2 < (uVar16 >> 0xf & 0xff)) {
      bVar6 = 5;
    }
    puVar8[2] = bVar6 | 0x80;
    puVar8[7] = 0xf;
    *(undefined2 *)(puVar8 + 0x12) = 0x1000;
    *(short *)(puVar8 + 0x3c) = args[1];
    *(short *)(puVar8 + 0x3a) = args[1];
    *(short *)(puVar8 + 0x38) = args[1];
    iVar10 = rand();
    puVar8[0x50] = (char)(iVar10 >> 0xb) + '\x10';
    goto LAB_80032e98;
  case 0xd6:
    puVar8[6] = 5;
    *(undefined2 *)(puVar8 + 0x10) = 0x40;
    puVar8[2] = 0x80;
    uVar4 = 0xf;
    goto LAB_800332d0;
  case 0xd7:
    puVar8[6] = 5;
    *(undefined2 *)(puVar8 + 0x10) = 0x10;
    iVar10 = rand();
    uVar16 = iVar10 * 3;
    if ((int)uVar16 < 0) {
      uVar16 = uVar16 + 0x7fff;
    }
    bVar6 = (byte)(uVar16 >> 0xf) | 0x80;
    if (2 < (uVar16 >> 0xf & 0xff)) {
      bVar6 = 0x82;
    }
    puVar8[2] = bVar6;
    puVar8[7] = 0xf;
    *(undefined2 *)(puVar8 + 0x12) = 0x1000;
    *(short *)(puVar8 + 0x3c) = args[1];
    *(short *)(puVar8 + 0x3a) = args[1];
    *(short *)(puVar8 + 0x38) = args[1];
    iVar10 = rand();
    puVar8[0x50] = (char)(iVar10 >> 0xb) + '\x10';
LAB_80032e98:
    puVar8[8] = 1;
    break;
  case 0xe9:
  case 0xea:
    puVar8[6] = 5;
    *(undefined2 *)(puVar8 + 0x10) = 0x40;
    puVar8[2] = 0x83;
    uVar4 = 0xd;
LAB_800332d0:
    puVar8[7] = uVar4;
    *(undefined2 *)(puVar8 + 0x12) = 0x1000;
    *(short *)(puVar8 + 0x3c) = args[1];
    *(short *)(puVar8 + 0x3a) = args[1];
    *(short *)(puVar8 + 0x38) = args[1];
    puVar8[8] = 1;
    *(uint *)(puVar8 + 0x1c) = args[2];
  }
  return puVar8;
}
