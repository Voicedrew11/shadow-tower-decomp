/*
 * VS.EXE: the round start and end sequence: rings of gouraud quads around both fighters, a fading floor quad and the fighters drawn on top (WIP).
 *
 * First attempt (one pass): a hand-checked transcription of the Ghidra
 * decompile with raw record offsets; structure fields are not named yet.

 *
 * Parked: no unit claims this file (config/units.toml) so VS.EXE stays a
 * partial image. Claiming 0x8002b078 completes VS's function coverage, and
 * kf build then runs the native PSYLINK link, which fails until the VS data
 * globals have defining objects (the same reason ST's last three functions
 * are held back).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <INLINE_C.H>
#include <vs/data.h>
#include <vs/buffer.h>
#include <vs/pad.h>

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

#define D_80063220 (*(s32 *)((u8 *)&DAT_80063220 + 0x0))
#define D_80063300 (*(s32 *)((u8 *)&DAT_80063300 + 0x0))
#define D_80063338 (*(s32 *)((u8 *)&DAT_80063338 + 0x0))
#define D_80063360 (*(s32 *)((u8 *)&DAT_80063360 + 0x0))
#define D_800633b0 (*(s32 *)((u8 *)&DAT_800633b0 + 0x0))
#define D_801603fc (*(s32 *)((u8 *)&DAT_801603c0 + 0x3c))
#define D_8016a4d0 (*(s32 *)((u8 *)&DAT_801624d0 + 0x8000))
#define D_8016a5b4 (*(s32 *)((u8 *)&DAT_801624d0 + 0x80e4))
#define D_8016a7d4 (*(s32 *)((u8 *)&DAT_801624d0 + 0x8304))
#define D_8018a4d0 (*(s32 *)((u8 *)&DAT_801624d0 + 0x28000))
#define D_8018ee68 (*(s32 *)((u8 *)&DAT_801624d0 + 0x2c998))
#define D_8018ef48 (*(s32 *)((u8 *)&DAT_801624d0 + 0x2ca78))
#define D_8018ef4c (*(s32 *)((u8 *)&DAT_801624d0 + 0x2ca7c))
#define D_8018ef50 (*(s32 *)((u8 *)&DAT_801624d0 + 0x2ca80))

/* WARNING: Removing unreachable block (ram,0x8002be68) */

/* Not exact (one attempt; 18.2%): first divergence at +0x0, the frame and the initializer copies (retail copies its .rodata tables into locals; this reads them as externs); the GTE blocks are reconstructed from the retail cop2 sequence. */
ADDRESS(0x8002b078, 0xf70)
undefined4 func_8002b078(int param_1)
{
  undefined1 *puVar1;
  undefined2 uVar2;
  char cVar3;
  short sVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  undefined *puVar9;
  uint uVar10;
  int iVar11;
  undefined *puVar12;
  uint *puVar13;
  int iVar14;
  undefined *puVar15;
  short *psVar16;
  int *piVar17;
  undefined **ppuVar18;
  short sVar19;
  int *piVar20;
  uint *puVar21;
  short *psVar22;
  short *psVar23;
  short *psVar24;
  undefined4 *puVar25;
  short local_210 [60];
  undefined2 local_198;
  undefined2 local_196;
  short local_194;
  short local_190 [58];
  undefined1 auStack_11c [4];
  undefined2 local_118;
  undefined2 local_116;
  short local_114;
  undefined1 local_110 [4];
  undefined1 auStack_10c [4];
  undefined1 auStack_108 [4];
  undefined1 auStack_104 [6];
  undefined1 local_fe;
  undefined1 local_fa;
  undefined1 local_f6;
  undefined1 local_f2;
  undefined2 local_f0;
  undefined2 local_ee;
  undefined2 local_ec;
  undefined2 local_ea;
  undefined2 local_e8;
  undefined2 local_e6;
  undefined2 local_e4;
  undefined2 local_e2;
  undefined2 local_e0;
  int local_dc;
  int local_d8;
  int local_d4;
  byte local_d0;
  byte local_cf;
  byte local_ce;
  byte local_cc;
  byte local_cb;
  byte local_ca;
  int local_c8 [16];
  int local_88 [17];
  int local_44;
  undefined4 *local_40;
  int local_38;
  uint local_34;
  undefined *local_30;
  undefined *local_2c;

  puVar1 = local_110 + 3;
  /* unaligned copy (decompile noise dropped) */
  *(u32 *)local_110 = (u32)(DAT_8001147c);
  puVar1 = auStack_10c + 3;
  /* unaligned copy (decompile noise dropped) */
  *(u32 *)auStack_10c = (u32)(DAT_80011480);
  puVar1 = auStack_108 + 3;
  /* unaligned copy (decompile noise dropped) */
  *(u32 *)auStack_108 = (u32)(DAT_80011484);
  puVar1 = auStack_104 + 3;
  /* unaligned copy (decompile noise dropped) */
  (*(T4 *)((u8 *)&auStack_104 + 0)) = DAT_80011488;
  piVar20 = local_c8;
  piVar17 = &DAT_8001148c;
  do {
    iVar8 = piVar17[1];
    iVar11 = piVar17[2];
    iVar14 = piVar17[3];
    *piVar20 = *piVar17;
    piVar20[1] = iVar8;
    piVar20[2] = iVar11;
    piVar20[3] = iVar14;
    piVar17 = piVar17 + 4;
    piVar20 = piVar20 + 4;
  } while (piVar17 != &DAT_800114cc);
  piVar17 = local_88;
  ppuVar18 = (undefined **)&DAT_800114cc;
  do {
    puVar9 = ppuVar18[1];
    puVar12 = ppuVar18[2];
    puVar15 = ppuVar18[3];
    *piVar17 = (int)*ppuVar18;
    piVar17[1] = (int)puVar9;
    piVar17[2] = (int)puVar12;
    piVar17[3] = (int)puVar15;
    ppuVar18 = ppuVar18 + 4;
    piVar17 = piVar17 + 4;
  } while (ppuVar18 != &DAT_8001150c);
  if (0x78 < param_1) {
    return 1;
  }
  if (param_1 < 0x5b) {
    if (param_1 < 0x47) {
      if (param_1 < 0x29) {
        iVar8 = param_1 + -10;
      }
      else {
        iVar8 = 0x30 - param_1;
      }
      func_8002a9c0(D_80063338 + 0x10,iVar8 * 2,0x32 - param_1);
    }
    else {
      iVar11 = param_1 + -0x46;
      iVar8 = iVar11 * 2;
      func_8002a360(7,iVar8);
      func_8002a124(0x1e,iVar11);
      func_80029b10(iVar8);
      func_80029f28(D_801603fc % 0x60,iVar8);
      func_8002ad40(iVar11);
    }
  }
  else {
    func_8002a360(7,(0x6a - param_1) * 2);
    func_8002a360(3,(param_1 + -100) * 2);
    func_8002a124(0x1e,0x20);
    func_80029b10(0x20);
    func_80029f28(D_801603fc % 0x60,0x20);
    func_8002ad40(0x20);
  }
  if (0x38 < param_1 - 0x14U) goto code_r0x8002bb8c;
  if (param_1 < 0x2d) {
    puVar9 = (undefined *)((param_1 - 0x14U) * 6);
    local_ce = 0xff;
    local_cf = 0xff;
    local_d0 = 0xff;
    local_ca = 0xff;
code_r0x8002b3a4:
    local_cb = local_ca;
    local_cc = local_ca;
  }
  else {
    if (0x3c < param_1) {
      puVar9 = (undefined *)0x96;
      local_ce = 0;
      local_cf = 0;
      local_d0 = 0;
      if ((param_1 + -0x3d) * -0x10 + 0xff < 0) {
        local_ca = 0;
      }
      else {
        local_ca = ~((char)(param_1 + -0x3d) * '\x10');
      }
      goto code_r0x8002b3a4;
    }
    puVar9 = (undefined *)0x96;
    if ((param_1 + -0x2d) * -0x10 + 0xff < 0) {
      local_ce = 0;
    }
    else {
      local_ce = ~((char)(param_1 + -0x2d) * '\x10');
    }
    local_ca = 0xff;
    local_cb = 0xff;
    local_cc = 0xff;
    local_cf = local_ce;
    local_d0 = local_ce;
  }
  local_44 = 0;
  local_40 = &D_800633b0;
  local_cc = local_ca;
  local_cb = local_ca;
  do {
    sVar4 = *(short *)(local_40 + 0x13833);
    iVar8 = 500;
    if (((399 < sVar4) && (iVar8 = 1000, 899 < sVar4)) && (iVar8 = 2000, sVar4 < 0x578)) {
      iVar8 = 0x5dc;
    }
    iVar14 = 0;
    psVar16 = local_210;
    psVar22 = local_210;
    iVar11 = 0x80;
    do {
      iVar5 = *(int *)(psVar16 + 0xc4);
      psVar22[1] = 0;
      *psVar22 = (short)((iVar8 * iVar5 * (int)puVar9) / 0x64000);
      psVar22[2] = (short)((iVar8 * *(int *)(psVar16 + 0xa4) * (int)puVar9) / 0x64000);
      *(short *)((int)local_210 + iVar11) =
           (short)((iVar8 * *(int *)(psVar16 + 0xc4) * (int)puVar9) / 0x64000);
      *(undefined2 *)((int)local_210 + iVar11 + 2) = (undefined2)D_8018ef4c;
      iVar14 = iVar14 + 1;
      psVar22 = psVar22 + 4;
      *(short *)((int)local_210 + iVar11 + 4) =
           (short)((iVar8 * *(int *)(psVar16 + 0xa4) * (int)puVar9) / 0x64000);
      psVar16 = psVar16 + 2;
      iVar11 = iVar11 + 8;
    } while (iVar14 < 0x10);
    local_d8 = -D_8018ef4c;
    local_dc = local_40[0x137bc] - D_8018ef48;
    local_f0 = 0x1000;
    local_ee = 0;
    local_ec = 0;
    local_ea = 0;
    local_e8 = 0x1000;
    local_e6 = 0;
    local_e4 = 0;
    local_e2 = 0;
    local_e0 = 0x1000;
    local_d4 = local_40[0x137be] - D_8018ef50;
    local_30 = puVar9;
    CompMatrix(&D_8018ee68,&local_f0,&local_f0);
    gte_SetRotMatrix((MATRIX *)&local_f0);
    gte_SetTransMatrix((MATRIX *)&local_f0);
    iVar8 = 0;
    psVar16 = local_210;
    puVar9 = local_30;
    do {
      gte_ldv3c(psVar16);
      gte_rtpt();
      gte_stsxy3((long *)psVar16, (long *)(psVar16 + 4), (long *)(psVar16 + 8));
      gte_stsz3((long *)(psVar16 + 2), (long *)((short *)(psVar16 + 2) + 4), (long *)((short *)(psVar16 + 2) + 8));
      iVar8 = iVar8 + 3;
      psVar16 = psVar16 + 0xc;
    } while (iVar8 < 0x1e);
    gte_ldv01((SVECTOR *)&local_118, (SVECTOR *)local_190);
    gte_rtpt();
    gte_stsxy01((long *)&local_118, (long *)local_190);
    gte_stsz3((long *)(auStack_11c), (long *)((short *)(auStack_11c) + 4), (long *)((short *)(auStack_11c) + 8));
    iVar8 = 0;
    local_34 = 0xff000000;
    iVar5 = 8;
    iVar14 = 0x88;
    iVar11 = 0x80;
    local_30 = puVar9;
    psVar16 = local_210;
    do {
      puVar21 = *(uint **)(D_8016a7d4 + 8);
      *(uint **)(D_8016a7d4 + 8) = puVar21 + 9;
      SetPolyG4(puVar21);
      if (0x2d < param_1) {
        *(byte *)((int)puVar21 + 7) = *(byte *)((int)puVar21 + 7) | 2;
      }
      if (iVar8 < 0xf) {
        *(undefined2 *)(puVar21 + 2) = *(undefined2 *)((int)local_210 + iVar11);
        *(undefined2 *)((int)puVar21 + 10) = *(undefined2 *)((int)local_210 + iVar11 + 2);
        *(undefined2 *)(puVar21 + 4) = *(undefined2 *)((int)local_210 + iVar14);
        *(undefined2 *)((int)puVar21 + 0x12) = *(undefined2 *)((int)local_210 + iVar14 + 2);
        *(short *)(puVar21 + 6) = *psVar16;
        *(short *)((int)puVar21 + 0x1a) = psVar16[1];
        *(undefined2 *)(puVar21 + 8) = *(undefined2 *)((int)local_210 + iVar5);
        *(undefined2 *)((int)puVar21 + 0x22) = *(undefined2 *)((int)local_210 + iVar5 + 2);
        uVar10 = (int)*(short *)((int)local_210 + iVar11 + 4) +
                 (int)*(short *)((int)local_210 + iVar14 + 4) + (int)psVar16[2] +
                 (int)*(short *)((int)local_210 + iVar5 + 4);
        iVar6 = (uint)local_ce - (uint)(byte)local_110[iVar8];
        iVar7 = 0;
        if (-1 < iVar6) {
          iVar7 = iVar6;
        }
        local_fe = (char)iVar7;
        iVar6 = (uint)local_ce - (uint)(byte)local_110[iVar8 + 1];
        iVar7 = 0;
        if (-1 < iVar6) {
          iVar7 = iVar6;
        }
        local_fa = (char)iVar7;
        iVar7 = (uint)local_ca - (uint)(byte)local_110[iVar8];
        local_f6 = 0;
        if (-1 < iVar7) {
          local_f6 = (undefined1)iVar7;
        }
        iVar7 = (uint)local_ca - (uint)(byte)local_110[iVar8 + 1];
      }
      else {
        *(undefined2 *)(puVar21 + 2) = local_118;
        *(undefined2 *)((int)puVar21 + 10) = local_116;
        *(short *)(puVar21 + 4) = local_190[0];
        *(short *)((int)puVar21 + 0x12) = local_190[1];
        *(undefined2 *)(puVar21 + 6) = local_198;
        *(undefined2 *)((int)puVar21 + 0x1a) = local_196;
        *(short *)(puVar21 + 8) = local_210[0];
        *(short *)((int)puVar21 + 0x22) = local_210[1];
        uVar10 = (int)local_114 + (int)local_190[2] + (int)local_194 + (int)local_210[2];
        iVar6 = (uint)local_ce - (uint)(byte)local_110[iVar8];
        iVar7 = 0;
        if (-1 < iVar6) {
          iVar7 = iVar6;
        }
        iVar6 = (uint)local_ce - ((uint)local_110 & 0xff);
        local_fe = (char)iVar7;
        iVar7 = 0;
        if (-1 < iVar6) {
          iVar7 = iVar6;
        }
        local_fa = (char)iVar7;
        iVar7 = (uint)local_ca - (uint)(byte)local_110[iVar8];
        local_f6 = 0;
        if (-1 < iVar7) {
          local_f6 = (undefined1)iVar7;
        }
        iVar7 = (uint)local_ca - ((uint)local_110 & 0xff);
      }
      iVar6 = 0;
      if (-1 < iVar7) {
        iVar6 = iVar7;
      }
      local_f2 = (undefined1)iVar6;
      *(byte *)(puVar21 + 1) = local_cc;
      *(byte *)((int)puVar21 + 5) = local_cb;
      *(undefined1 *)((int)puVar21 + 6) = local_f6;
      *(byte *)(puVar21 + 3) = local_cc;
      *(byte *)((int)puVar21 + 0xd) = local_cb;
      *(undefined1 *)((int)puVar21 + 0xe) = local_f2;
      *(byte *)(puVar21 + 5) = local_d0;
      *(byte *)((int)puVar21 + 0x15) = local_cf;
      *(undefined1 *)((int)puVar21 + 0x16) = local_fe;
      *(byte *)(puVar21 + 7) = local_d0;
      *(byte *)((int)puVar21 + 0x1d) = local_cf;
      iVar7 = (int)((uVar10 >> 7) << 0x10) >> 0xe;
      *(undefined1 *)((int)puVar21 + 0x1e) = local_fa;
      *puVar21 = *puVar21 & local_34 | *(uint *)(iVar7 + D_8016a5b4) & 0xffffff;
      puVar13 = (uint *)(iVar7 + D_8016a5b4);
      *puVar13 = *puVar13 & local_34 | (uint)puVar21 & 0xffffff;
      puVar21 = *(uint **)(D_8016a7d4 + 8);
      *(uint **)(D_8016a7d4 + 8) = puVar21 + 8;
      SetPolyFT3(puVar21);
      if (iVar8 == 0xf) {
        uVar10 = (int)local_114 + (int)local_190[2] + (int)local_194 + (int)local_210[2];
      }
      else {
        uVar10 = (int)*(short *)((int)local_210 + iVar11 + 4) +
                 (int)*(short *)((int)local_210 + iVar14 + 4) + (int)psVar16[2] +
                 (int)*(short *)((int)local_210 + iVar5 + 4);
      }
      iVar5 = iVar5 + 8;
      psVar16 = psVar16 + 4;
      iVar14 = iVar14 + 8;
      *(undefined2 *)((int)puVar21 + 0xe) = 0x7f81;
      iVar7 = (int)((uVar10 >> 7) << 0x10) >> 0xe;
      *(undefined2 *)(puVar21 + 2) = 0;
      *(undefined2 *)((int)puVar21 + 10) = 0;
      *(undefined2 *)(puVar21 + 4) = 0;
      *(undefined2 *)((int)puVar21 + 0x12) = 0;
      *(undefined2 *)(puVar21 + 6) = 0;
      *(undefined2 *)((int)puVar21 + 0x1a) = 0;
      *(undefined1 *)(puVar21 + 3) = 0;
      *(undefined1 *)((int)puVar21 + 0xd) = 0;
      *(undefined1 *)(puVar21 + 5) = 0;
      *(undefined1 *)((int)puVar21 + 0x15) = 0;
      *(undefined1 *)(puVar21 + 7) = 0;
      *(undefined1 *)((int)puVar21 + 0x1d) = 0;
      *(undefined2 *)((int)puVar21 + 0x16) = 0x38;
      *(undefined1 *)(puVar21 + 1) = 0;
      *(undefined1 *)((int)puVar21 + 5) = 0;
      *(undefined1 *)((int)puVar21 + 6) = 0;
      *puVar21 = *puVar21 & local_34 | *(uint *)(iVar7 + D_8016a5b4) & 0xffffff;
      iVar8 = iVar8 + 1;
      puVar13 = (uint *)(iVar7 + D_8016a5b4);
      *puVar13 = *puVar13 & local_34 | (uint)puVar21 & 0xffffff;
      iVar11 = iVar11 + 8;
    } while (iVar8 < 0x10);
    local_40 = local_40 + 0x1383a;
    local_44 = local_44 + 1;
    puVar9 = local_30;
  } while (local_44 < 2);
code_r0x8002bb8c:
  if (param_1 < 100) {
    iVar11 = 0;
    uVar10 = 0x18;
    iVar8 = 0x10;
    iVar14 = 8;
    puVar25 = &D_800633b0;
    puVar9 = &D_8016a4d0;
    puVar12 = &D_8018a4d0;
    psVar16 = local_210;
    do {
      sVar4 = *(short *)(puVar25 + 0x13833);
      sVar19 = 0x271;
      if (((399 < sVar4) && (sVar19 = 0x4e2, 899 < sVar4)) && (sVar19 = 0x9c4, sVar4 < 0x578)) {
        sVar19 = 0x753;
      }
      sVar4 = -sVar19;
      psVar23 = (short *)((int)local_210 + iVar14);
      psVar24 = (short *)((int)local_210 + iVar8);
      psVar22 = (short *)((int)local_210 + uVar10);
      *psVar16 = sVar4;
      psVar16[1] = 0;
      psVar16[2] = sVar4;
      *psVar23 = sVar19;
      *(undefined2 *)((int)local_210 + iVar14 + 2) = 0;
      *(short *)((int)local_210 + iVar14 + 4) = sVar4;
      *psVar24 = sVar4;
      *(undefined2 *)((int)local_210 + iVar8 + 2) = 0;
      *(short *)((int)local_210 + iVar8 + 4) = sVar19;
      *psVar22 = sVar19;
      *(undefined2 *)((int)local_210 + uVar10 + 2) = 0;
      *(short *)((int)local_210 + uVar10 + 4) = sVar19;
      local_dc = puVar25[0x137bc] - *(int *)(puVar12 + 0x4a78);
      local_d8 = -*(int *)(puVar12 + 0x4a7c);
      local_f0 = 0x1000;
      local_ee = 0;
      local_ec = 0;
      local_ea = 0;
      local_e8 = 0x1000;
      local_e6 = 0;
      local_e4 = 0;
      local_e2 = 0;
      local_e0 = 0x1000;
      local_d4 = puVar25[0x137be] - *(int *)(puVar12 + 0x4a80);
      local_38 = iVar8;
      local_34 = uVar10;
      local_30 = puVar9;
      local_2c = puVar12;
      CompMatrix(&D_8018ee68,&local_f0,&local_f0);
      gte_SetRotMatrix((MATRIX *)&local_f0);
      gte_SetTransMatrix((MATRIX *)&local_f0);
      gte_ldv3c(psVar16);
      gte_rtpt();
      gte_stsxy3((long *)psVar16, (long *)psVar23, (long *)psVar24);
      gte_ldv0((SVECTOR *)psVar22);
      gte_rtps();
      gte_stsxy((long *)psVar22);
      puVar21 = *(uint **)(*(int *)(local_30 + 0x304) + 8);
      *(uint **)(*(int *)(local_30 + 0x304) + 8) = puVar21 + 10;
      *(undefined1 *)((int)puVar21 + 3) = 9;
                    /* Probable PsyQ macro: setPolyFT4() + setSemiTrans(polyFT4, 1) */
      *(undefined1 *)((int)puVar21 + 7) = 0x2e;
      *(short *)(puVar21 + 2) = *psVar16;
      *(short *)((int)puVar21 + 10) = psVar16[1];
      *(short *)(puVar21 + 4) = *psVar23;
      *(undefined2 *)((int)puVar21 + 0x12) = *(undefined2 *)((int)local_210 + iVar14 + 2);
      *(short *)(puVar21 + 6) = *psVar24;
      *(undefined2 *)((int)puVar21 + 0x1a) = *(undefined2 *)((int)local_210 + iVar8 + 2);
      *(short *)(puVar21 + 8) = *psVar22;
      uVar2 = *(undefined2 *)((int)local_210 + uVar10 + 2);
      *(undefined1 *)((int)puVar21 + 0xd) = 0x80;
      *(undefined1 *)((int)puVar21 + 0x15) = 0x80;
      *(undefined1 *)((int)puVar21 + 0x1d) = 0xfd;
      *(undefined1 *)((int)puVar21 + 0x25) = 0xfd;
      *(undefined2 *)((int)puVar21 + 0xe) = 0x7820;
      *(undefined1 *)(puVar21 + 3) = 4;
      *(undefined1 *)(puVar21 + 5) = 0x7f;
      *(undefined1 *)(puVar21 + 7) = 4;
      *(undefined1 *)(puVar21 + 9) = 0x7f;
      *(undefined2 *)((int)puVar21 + 0x16) = 0x3d;
      *(undefined2 *)((int)puVar21 + 0x22) = uVar2;
      cVar3 = (char)(param_1 << 2);
      if (param_1 < 0x2e) {
LAB_8002be6c:
        *(char *)(puVar21 + 1) = cVar3;
        *(char *)((int)puVar21 + 5) = cVar3;
        *(char *)((int)puVar21 + 6) = cVar3;
      }
      else {
        if (param_1 < 0x4e) {
          cVar3 = ('M' - (char)param_1) * '\b';
          goto LAB_8002be6c;
        }
        *(undefined1 *)(puVar21 + 1) = 0;
        *(undefined1 *)((int)puVar21 + 5) = 0;
        *(undefined1 *)((int)puVar21 + 6) = 0;
      }
      psVar16 = psVar16 + 4;
      uVar10 = local_34 + 8;
      iVar8 = local_38 + 8;
      iVar14 = iVar14 + 8;
                    /* Probable PsyQ macro: addPrim(). */
      *puVar21 = *puVar21 & 0xff000000 | *(uint *)(*(int *)(local_30 + 0xe4) + 0x3fec) & 0xffffff;
      iVar11 = iVar11 + 1;
      *(uint *)(*(int *)(local_30 + 0xe4) + 0x3fec) =
           *(uint *)(*(int *)(local_30 + 0xe4) + 0x3fec) & 0xff000000 | (uint)puVar21 & 0xffffff;
      puVar25 = puVar25 + 0x1383a;
      puVar9 = local_30;
      puVar12 = local_2c;
    } while (iVar11 < 2);
  }
  if (0x2d < param_1) {
    puVar25 = &D_800633b0;
    iVar8 = 0;
    do {
      D_80063300 = puVar25;
      if (*(short *)(puVar25 + 0x137c4) == 1) {
        func_80025204();
      }
      else if (*(short *)(puVar25 + 0x137c4) == 2) {
        D_80063360 = 0;
        func_80028914();
      }
      iVar11 = iVar8 + 1;
      *(ushort *)((int)D_80063300 + 0x4deea) =
           *(short *)((int)D_80063300 + 0x4deea) +
           *(short *)(D_80063300 + 0x137bb) * (short)D_80063220 & 0xfff;
      func_8002bfe8(iVar8,0x1000);
      puVar25 = puVar25 + 0x1383a;
      iVar8 = iVar11;
    } while (iVar11 < 2);
  }
  return 0;
}
