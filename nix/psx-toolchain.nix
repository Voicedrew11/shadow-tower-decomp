{ pkgs, sdkBuilder }:
let
      # Psy-Q PS-X Development System Release 3.0 kit; files dated 1995-04-08
      # despite the Archive.org item title. Inherited from King's Field II,
      # whose retail executables carry its RCS revisions. Shadow Tower links
      # Runtime Library 4.2 instead; this kit stays only as the analysis
      # corpus of the vendored-function seed and FID census.
      psyqKit = pkgs.fetchurl {
        name = "psyq-3.0-psx.zip";
        url = "https://archive.org/download/psyq_psx_toolchain_april_08_1994/psx.zip";
        hash = "sha256-QWJBY3zbAnO2vZNrCrVifIunIl7iuYBmzztQ+wnleJk=";
      };

      # Native Decompals rebuild used only as a practical code-generation probe.
      # It corresponds to GCC 2.6.0's PSX target, but is not evidence that this
      # rebuilt host binary (or either staged historical 2.6.0 binary) built KF.
      gcc260NativeArchive = pkgs.fetchurl {
        name = "decompals-old-gcc-0.17-gcc-2.6.0-psx.tar.gz";
        url = "https://github.com/decompals/old-gcc/releases/download/0.17/gcc-2.6.0-psx.tar.gz";
        hash = "sha256-NY2slJ8PVmr5xq+Tu6DUrATVbKnfgMWL1m2Bu7UN7Os=";
      };
      gcc260Native = pkgs.runCommand "decompals-gcc-2.6.0-psx-0.17" {
        nativeBuildInputs = [ pkgs.gnutar pkgs.gzip ];
      } ''
        mkdir -p "$out/bin"
        tar -xzf ${gcc260NativeArchive} -C "$out/bin"
        chmod +x "$out/bin"/*
      '';

      # Second native probe. GCC 2.5.7 is the closest available rebuild to the
      # 1994 Psy-Q compiler family; its text epilogue and load hoisting match
      # retail forms that 2.6.0 cannot emit, but it is still a probe.
      gcc257NativeArchive = pkgs.fetchurl {
        name = "decompals-old-gcc-0.17-gcc-2.5.7-psx.tar.gz";
        url = "https://github.com/decompals/old-gcc/releases/download/0.17/gcc-2.5.7-psx.tar.gz";
        hash = "sha256-DbH7QDx2blGwlDVLmhNqbB8BRXqkHMyiisRXt8tn2HE=";
      };
      gcc257Native = pkgs.runCommand "decompals-gcc-2.5.7-psx-0.17" {
        nativeBuildInputs = [ pkgs.gnutar pkgs.gzip ];
      } ''
        mkdir -p "$out/bin"
        tar -xzf ${gcc257NativeArchive} -C "$out/bin"
        chmod +x "$out/bin"/*
      '';

      gcc257SourceArchive = pkgs.fetchurl {
        url = "https://www.nic.funet.fi/index/gnu/funet/historical-funet-gnu-area-from-early-1990s/gcc-2.5.7.tar.gz";
        hash = "sha256-Y0W+QiNeXsTESNZYflUoPaPuV3YdtdINegpI1jmH/I4=";
      };
      gcc257Headers = pkgs.runCommand "gcc-2.5.7-mips-headers" {
        nativeBuildInputs = [ pkgs.gnutar pkgs.gzip ];
      } ''
        mkdir -p "$out/include"
        tar -xzf ${gcc257SourceArchive} --strip-components=1 -C "$out/include" \
          gcc-2.5.7/gstdarg.h gcc-2.5.7/va-mips.h
        mv "$out/include/gstdarg.h" "$out/include/stdarg.h"
      '';

      # The complete pinned Psy-Q 3.0 kit tree (analysis corpus only).
      psyqSdk = pkgs.runCommand "shadow-tower-psyq-3.0-sdk" {
        nativeBuildInputs = with pkgs; [
          binutils
          coreutils
          file
          p7zip
          python3
          unar
        ];
      } ''
        export PSYQ_KIT_ZIP="${psyqKit}"
        python3 ${sdkBuilder} \
          --work-dir "$TMPDIR/toolchain-work" \
          --stage-dir "$out"
      '';

      # Standalone allocator from the Psy-Q Release 2.5 floppies; Runtime
      # Library 4.2 ships no MALLOC.OBJ. Its 0x60c text matches, relocation
      # fields masked, the allocator that STR, TITLE, OPTXT and END link
      # directly after NONE3.OBJ; the original distribution remains unproved.
      psyq25Floppies = pkgs.fetchurl {
        name = "psyq-release-2.5-floppies.rar";
        url = "https://archive.org/download/ps1_sdks/Floppies.rar";
        hash = "sha256-SaLzzryjqEIclPHeQ7nSDnN1CwQrCDHdKxcy0o+Ud4M=";
      };
      psyqMallocObj = pkgs.runCommand "shadow-tower-psyq-release-2.5-malloc" {
        nativeBuildInputs = with pkgs; [ coreutils findutils unar ];
      } ''
        mkdir -p "$TMPDIR/floppies" "$out"
        unar -quiet -force-overwrite -output-directory "$TMPDIR/floppies" ${psyq25Floppies}
        malloc_obj="$(find "$TMPDIR/floppies" -type f -path '*/isa board/PSXLIB/LIB/MALLOC.OBJ' -print -quit)"
        test -n "$malloc_obj"
        test "$(sha256sum "$malloc_obj" | cut -d' ' -f1)" = \
          "628e405fd0e3acfff2ce9d4a15d481f0aa36398c14e9eae0a82b7ff0a86a74c9"
        cp "$malloc_obj" "$out/MALLOC.OBJ"
      '';

      # Sony's Programmer Tool Runtime Library 4.2: the LIB and INCLUDE trees
      # and the two bundled 4.2.1 patch archives, unchanged. Every SDK object
      # linked into the seven SLUS-00863 programs matches an object of these
      # archives with relocation fields masked, and their RCS strings
      # (sys.c 1.140 1998/01/12, bios.c 1.86, intr.c 1.76) are the retail ones.
      psyq42Archive = pkgs.fetchurl {
        name = "psx-runtime-library-4.2.7z";
        url = "https://archive.org/download/play-station-programmer-tool-runtime-library-version-4.2.7z/PlayStation_Programmer_Tool_-_Runtime_Library_Version_4.2.7z";
        hash = "sha256-5PWmeF6YEFHZEMNTSg8F6h1PbvT7z7HQnbk+ymIj5f4=";
      };
      # `psyq-4.2/` keeps the extracted tree byte-identical. `lib/` is the
      # linker's archive directory: the 4.2 archives with LIBPAD.LIB taken from
      # the bundled 4.2.1 pad patch (LIB/42PATCH/J421PD.ZIP), whose PADMAIN,
      # PADIF and WAITRC2 members are the ones the retail programs link.
      # `include-lf/` is INCLUDE with CRLF line endings converted to LF.
      psyq42Sdk = pkgs.runCommand "shadow-tower-psyq-4.2-runtime" {
        nativeBuildInputs = with pkgs; [ coreutils findutils p7zip unzip ];
      } ''
        mkdir -p "$out/psyq-4.2" "$out/lib" "$out/include-lf"
        7z x -bd -o"$out/psyq-4.2" ${psyq42Archive} >/dev/null
        check() {
          test "$(sha256sum "$1" | cut -d' ' -f1)" = "$2" || { echo "$1: unexpected hash" >&2; exit 1; }
        }
        check "$out/psyq-4.2/LIB/LIBPAD.LIB" 91b29e9cfa082b0d4981d7cd15935e9096cb4442a09358a367bc05e29f066369
        check "$out/psyq-4.2/LIB/42PATCH/J421PD.ZIP" c3d2a9b3c2d077bef76397f36945207febbed71b4416729dc533d5be215d5bd3
        cp "$out/psyq-4.2/LIB/"*.LIB "$out/lib/"
        unzip -p "$out/psyq-4.2/LIB/42PATCH/J421PD.ZIP" LIBPAD.LIB > "$out/lib/LIBPAD.LIB"
        check "$out/lib/LIBPAD.LIB" eb41454b3678fdeee1065ff2c892882cdeadd1d04c9d8ad0c7a7c8ba41012fc4
        (cd "$out/psyq-4.2/INCLUDE" && find . -type f) | while read -r header; do
          mkdir -p "$out/include-lf/$(dirname "$header")"
          tr -d '\r' < "$out/psyq-4.2/INCLUDE/$header" > "$out/include-lf/$header"
        done
      '';

      # The Psy-Q/SDevTC 4.5 developer kit as preserved on Archive.org
      # (`ps1_sdks`, a 1999 floppy-set release of the 1998 tools). Only the
      # files the executable chain needs are taken, all DOS programs:
      # BIN/DOS/ASPSXD.EXE, the DOS build of SDevTC ASPSX 2.79 (1998-05-22,
      # Watcom C with the FlashTek X-32VM DOS extender bound in); CPE2X 1.5
      # (1998-01-23, 16-bit real mode), whose /CA option writes the "North
      # America area" title that every retail header carries; and the
      # DMPSX 3.0 (BIN/DOS/DMPSX.EXE, 1999-01-30, Borland C++ real mode), the GTE
      # post-processor for Runtime Library 4.2's INLINE_C.H macros; and the
      # NONE3.OBJ overlay startup that begins the text of STR, ST, TITLE,
      # OPTXT, VS and END. The kit's DOS PSYLINK 2.74D (BIN/DOS/PSYLINKD.EXE)
      # is an LX image left without its X-32 loader, so no DOS extender
      # starts it; the linker comes from SDevTC 2.4 below.
      psyq45KitArchive = pkgs.fetchurl {
        name = "psyq-sdevtc-4.5-mfd.zip";
        url = "https://archive.org/download/ps1_sdks/PSYQ.SDevTC.Developers.Toolkit.For.PSX.v4.5-MFD.zip";
        hash = "sha256-5+Qmnvz1SC2cTFtkYQ3U5Nr0v8AOvgYAWXEAl1dI5Ss=";
      };
      psyq45Tools = pkgs.runCommand "shadow-tower-psyq-4.5-dos-tools" {
        nativeBuildInputs = with pkgs; [ coreutils findutils unar unzip ];
      } ''
        mkdir -p "$TMPDIR/kit" "$out"
        cd "$TMPDIR/kit"
        unzip -q ${psyq45KitArchive}
        cd PSYQ.SDevTC.Developers.Toolkit.For.PSX.v4.5-MFD
        for volume in mfdpsx0*.zip; do unzip -qo "$volume"; done
        unar -quiet -force-overwrite -output-directory "$TMPDIR/tree" mfdpsx.rar
        root="$TMPDIR/tree/mfdpsx"
        install -m 0644 "$root/BIN/DOS/ASPSXD.EXE" "$root/BIN/DOS/DMPSX.EXE" "$root/BIN/CPE2X.EXE"           "$root/LIB/NONE3.OBJ" "$out/"
        check() {
          test "$(sha256sum "$out/$1" | cut -d' ' -f1)" = "$2" || { echo "$1: unexpected hash" >&2; exit 1; }
        }
        check ASPSXD.EXE 29018ea7ef45e29d4de598f23b998f3431eacd2f0c5e9c32043212d7f9ba6dcf
        check DMPSX.EXE d2e4febf99665a4f867d0dc44db4e0e37e1dadb6e0a2256f41d4e7f3cae41ee0
        check CPE2X.EXE 777caca88327cd18d4f92b43439575c1759381690d49f14177805a94b8ecbef6
        check NONE3.OBJ 063efc868877ca84fcbe1fdc3afe9656e6c7933cb68156e08d04d35b76166899
      '';

      # SN Systems' SDevTC 2.4 CD (Japan, files dated 1998-06-11; redump
      # image in Archive.org `ps1_sdks`). Only DOS/PSYLINKD.EXE is taken:
      # PSYLINK 2.73D, the DOS build of the linker (Watcom C, FlashTek X-32VM
      # bound in). It links PSX.EXE byte-identically, and its CPE, SYM and MAP
      # output equals the Win32 PSYLINK 2.74 of the 4.5 kit on test links that
      # take every member of each program's libraries.
      sdevtc24Archive = pkgs.fetchurl {
        name = "sdevtc-2.4-japan-redump.zip";
        url = "https://archive.org/download/ps1_sdks/SDevTC%20-%20Version%202.4%20%28Japan%29_redump.zip";
        hash = "sha256-BMRsMU2HuXRVX4rG0VZKnMKrl7GKvIT75mAZi88cfcU=";
      };
      sdevtc24Linker = pkgs.runCommand "shadow-tower-sdevtc-2.4-psylinkd" {
        nativeBuildInputs = with pkgs; [ bchunk coreutils p7zip unzip ];
      } ''
        mkdir -p "$TMPDIR/cd" "$out"
        cd "$TMPDIR/cd"
        unzip -q ${sdevtc24Archive}
        bchunk "SDevTC - Version 2.4 (Japan).bin" "SDevTC - Version 2.4 (Japan).cue" track >/dev/null
        7z x -bd -oiso track01.iso DOS/PSYLINKD.EXE >/dev/null
        install -m 0644 iso/DOS/PSYLINKD.EXE "$out/"
        test "$(sha256sum "$out/PSYLINKD.EXE" | cut -d' ' -f1)" = \
          "2014db126ceaea6d287067ea3f829eea1745836fca570f22ef379eaff7832dd2" \
          || { echo "PSYLINKD.EXE: unexpected hash" >&2; exit 1; }
      '';

      # Decompals rebuild of the SN Systems GCC 2.7.2 PSX compiler
      # ("cygnus-2.7.2-970404 SN32"). With -O2 it reproduces sampled Shadow
      # Tower game functions exactly; it is still a rebuild, not the original
      # Win32 CC1PSX.EXE.
      gcc272CdkNativeArchive = pkgs.fetchurl {
        name = "decompals-old-gcc-0.17-gcc-2.7.2-cdk.tar.gz";
        url = "https://github.com/decompals/old-gcc/releases/download/0.17/gcc-2.7.2-cdk.tar.gz";
        hash = "sha256-QrsN+W2xGptdLiPXi9yWJ5H0AEYoDT02Dak/5e728Ls=";
      };
      gcc272CdkNative = pkgs.runCommand "decompals-gcc-2.7.2-cdk-0.17" {
        nativeBuildInputs = [ pkgs.gnutar pkgs.gzip ];
      } ''
        mkdir -p "$out/bin"
        tar -xzf ${gcc272CdkNativeArchive} -C "$out/bin" cc1 cpp
        chmod +x "$out/bin"/*
      '';

      cc1psx272cdk = pkgs.writeShellApplication {
        name = "cc1psx-272cdk";
        text = ''exec ${gcc272CdkNative}/bin/cc1 "$@"'';
      };

      cpppsx272cdk = pkgs.writeShellApplication {
        name = "cpppsx-272cdk";
        text = ''exec ${gcc272CdkNative}/bin/cpp "$@"'';
      };

      cc1psx260 = pkgs.writeShellApplication {
        name = "cc1psx-260";
        text = ''exec ${gcc260Native}/bin/cc1 "$@"'';
      };

      cpppsx260 = pkgs.writeShellApplication {
        name = "cpppsx-260";
        text = ''exec ${gcc260Native}/bin/cpp "$@"'';
      };

      cc1psx257 = pkgs.writeShellApplication {
        name = "cc1psx-257";
        text = ''exec ${gcc257Native}/bin/cc1 "$@"'';
      };

      cpppsx257 = pkgs.writeShellApplication {
        name = "cpppsx-257";
        text = ''exec ${gcc257Native}/bin/cpp "$@"'';
      };

in {
  inherit psyqSdk psyq42Sdk psyq45Tools sdevtc24Linker psyqMallocObj
    gcc272CdkNative cc1psx272cdk cpppsx272cdk
    gcc257Native gcc257Headers gcc260Native cc1psx257 cpppsx257 cc1psx260 cpppsx260;
}
