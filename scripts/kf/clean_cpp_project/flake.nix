{
  description = "Shadow Tower C++ source and PlayStation build";
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/64c08a7ca051951c8eae34e3e3cb1e202fe36786";
  outputs = { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs { inherit system; config.allowUnfree = true; };
      sdk = import ./nix/psx-toolchain.nix { inherit pkgs; sdkBuilder = ./scripts/create-toolchain.py; };
      emulator = import ./nix/pcsx-redux.nix { inherit pkgs; };
      python = pkgs.python3.withPackages (p: [ p.pyelftools ]);
      binutils = pkgs.pkgsCross.mipsel-linux-gnu.buildPackages.binutils;
      environment = {
        PSYQ_SDK = "${sdk.psyqSdk}/psyq-3.0";
        PSYQ_INCLUDE = "${sdk.psyq42Sdk}/include-lf";
        PSYQ_LIB = "${sdk.psyq42Sdk}/lib";
        PSYQ_MALLOC_OBJ = "${sdk.psyqMallocObj}/MALLOC.OBJ";
        PSYQ_STARTUP_OBJ = "${sdk.psyq45Tools}/NONE3.OBJ";
        PSYQ_ASPSX = "${sdk.psyq45Tools}/ASPSXD.EXE";
        PSYQ_PSYLINK = "${sdk.sdevtc24Linker}/PSYLINKD.EXE";
        PSYQ_CPE2X = "${sdk.psyq45Tools}/CPE2X.EXE";
        MIPS_LD = "${binutils}/bin/mipsel-unknown-linux-gnu-ld";
      };
      buildTools = [ python pkgs.llvmPackages.clang-unwrapped binutils pkgs.dosbox-x ];
      game = pkgs.stdenvNoCC.mkDerivation (environment // {
        pname = "kings-field-cpp";
        version = "1.0";
        src = pkgs.lib.cleanSourceWith {
          src = ./.;
          filter = path: type: !(builtins.elem (baseNameOf path)
            [ ".git" "build" "result" "__pycache__" "codecs" ]);
        };
        nativeBuildInputs = buildTools;
        dontConfigure = true;
        buildPhase = ''
          runHook preBuild
          python3 build.py
          runHook postBuild
        '';
        installPhase = ''
          mkdir -p "$out/link"
          cp build/psx/PSX.EXE build/str/STR.EXE build/st/ST.EXE build/title/TITLE.EXE build/optxt/OPTXT.EXE build/vs/VS.EXE build/end/END.EXE "$out/"
          cp build/psx/PSX.CPE build/str/STR.CPE build/st/ST.CPE build/title/TITLE.CPE build/optxt/OPTXT.CPE build/vs/VS.CPE build/end/END.CPE "$out/link/"
        '';
      });
      runner = pkgs.writeShellApplication {
        name = "kings-field";
        runtimeInputs = [ pkgs.python3 emulator ];
        text = ''
          export PCSX_REDUX_BIN="${emulator}/bin/pcsx-redux"
          export PCSX_REDUX_BIOS="${emulator.openbios}"
          exec python3 ${./scripts/psxbuild/disc.py} --executables ${game} "$@"
        '';
      };
      retailRunner = pkgs.writeShellApplication {
        name = "kf-run-retail";
        runtimeInputs = [ pkgs.python3 emulator ];
        text = ''
          export PCSX_REDUX_BIN="${emulator}/bin/pcsx-redux"
          export PCSX_REDUX_BIOS="${emulator.openbios}"
          exec python3 ${./scripts/psxbuild/disc.py} --retail "$@"
        '';
      };
    in {
      packages.${system} = { inherit game; default = game; };
      apps.${system} = {
        default = { type = "app"; program = "${runner}/bin/kings-field"; };
        retail = { type = "app"; program = "${retailRunner}/bin/kf-run-retail"; };
      };
      devShells.${system} = {
        default = pkgs.mkShell (environment // { packages = buildTools ++ [ retailRunner ]; });
        codecs = pkgs.mkShell { packages = [ pkgs.cargo pkgs.rustc ]; };
      };
    };
}
