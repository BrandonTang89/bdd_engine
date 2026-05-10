{
  description = "Development shell for bdd_engine";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs = { self, nixpkgs, ... }:
    let
      systems = [ "x86_64-linux" "aarch64-linux" "aarch64-darwin" ];
      forAllSystems = nixpkgs.lib.genAttrs systems;
    in {
      packages = forAllSystems (system:
        let
          pkgs = nixpkgs.legacyPackages.${system};
        in {
          default = pkgs.gcc15Stdenv.mkDerivation {
            pname = "bdd_engine";
            version = "0.1.0";
            src = ./.;

            nativeBuildInputs = [
              pkgs.cmake
              pkgs.ninja
            ];

            buildInputs = [
              pkgs.abseil-cpp
              pkgs.catch2_3
            ];

            cmakeFlags = [
              "-DCMAKE_BUILD_TYPE=Release"
            ];
          };
        });

      checks = forAllSystems (system: {
        default = self.packages.${system}.default.overrideAttrs (oldAttrs: {
          doCheck = true;
          cmakeFlags = (oldAttrs.cmakeFlags or []) ++ [ "-Bcmake_build_release" ];
          dontUseCmakeBuildDir = true;
          buildPhase = "cmake --build cmake_build_release -j $NIX_BUILD_CORES";
          checkPhase = "ctest --test-dir cmake_build_release";
          installPhase = "cmake --install cmake_build_release --prefix $out";
        });
      });

      apps = forAllSystems (system:
        let
          pkgs = nixpkgs.legacyPackages.${system};
        in {
          default = {
            type = "app";
            program = "${pkgs.writeShellScript "bdd_engine" ''
              exec ${pkgs.rlwrap}/bin/rlwrap ${self.packages.${system}.default}/bin/bdd_engine "$@"
            ''}";
          };
        });

      devShells = forAllSystems (system:
        let
          pkgs = nixpkgs.legacyPackages.${system};
        in {
          default = pkgs.mkShell {
            inputsFrom = [ self.packages.${system}.default ];
            packages = with pkgs; [
              gcc15
              rlwrap
            ];

            shellHook = ''
              if command -v git >/dev/null 2>&1; then
                REPO_ROOT="$(git rev-parse --show-toplevel 2>/dev/null || true)"
                if [ -n "$REPO_ROOT" ] && [ -d "$REPO_ROOT/scripts" ]; then
                  export PATH="$REPO_ROOT/scripts:$PATH"
                fi
              fi
              export CC=${pkgs.gcc15}/bin/gcc
              export CXX=${pkgs.gcc15}/bin/g++
            '';
          };
        });
    };
}
