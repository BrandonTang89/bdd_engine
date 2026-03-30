{
  description = "Development shell for bdd_engine";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs = { nixpkgs, ... }:
    let
      systems = [ "x86_64-linux" "aarch64-linux" ];
      forAllSystems = f: nixpkgs.lib.genAttrs systems (system: f system);
    in {
      devShells = forAllSystems (system:
        let
          pkgs = import nixpkgs { inherit system; };
        in {
          default = pkgs.mkShell {
            packages = with pkgs; [
              gcc14
              cmake
              ninja
              abseil-cpp
              catch2_3
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
