{
  description = "Development shell for bdd_engine";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs = { self, nixpkgs, ... }:
    let
      systems = [ "x86_64-linux" "aarch64-linux" ];
      forAllSystems = f: nixpkgs.lib.genAttrs systems (system: f system);
    in {
      packages = forAllSystems (system:
        let
          pkgs = import nixpkgs { inherit system; };
        in {
          default = pkgs.gcc14Stdenv.mkDerivation {
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

      apps = forAllSystems (system: {
        default = {
          type = "app";
          program = "${self.packages.${system}.default}/bin/bdd_engine";
        };
      });

      devShells = forAllSystems (system:
        let
          pkgs = import nixpkgs { inherit system; };
        in {
          default = pkgs.mkShell {
            inputsFrom = [ self.packages.${system}.default ];
            packages = with pkgs; [
              gcc14
            ];

            shellHook = ''
              if command -v git >/dev/null 2>&1; then
                REPO_ROOT="$(git rev-parse --show-toplevel 2>/dev/null || true)"
                if [ -n "$REPO_ROOT" ] && [ -d "$REPO_ROOT/scripts" ]; then
                  export PATH="$REPO_ROOT/scripts:$PATH"
                fi
              fi
              export CC=${pkgs.gcc14}/bin/gcc
              export CXX=${pkgs.gcc14}/bin/g++
            '';
          };
        });
    };
}
