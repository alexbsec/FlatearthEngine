{
  description = "Flatearth Engine Dev Shell (Linux Only)";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixpkgs-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };

        nativeBuildInputs = with pkgs; [
          gcc
          cmake
          ninja
          gdb
          vulkan-headers
          vulkan-loader
          vulkan-tools
          pkg-config
          fmt
          xorg.libX11
          xorg.libxcb
          xorg.xcbutilkeysyms
        ];

      in {
        devShells.default = pkgs.mkShell {
          name = "flatearth-dev-shell";

          buildInputs = nativeBuildInputs;

          shellHook = ''
            echo "[Flatearth] Dev shell pronta 🚀"
            echo "Use: build-engine | build-tests | run-tests"

            # Comandos auxiliares
            alias build-engine='cmake -S engine -B engine/build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON && cmake --build engine/build'
            alias build-tests='cmake -S tests -B tests/build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON && cmake --build tests/build'
            alias run-tests='./tests/build/flatearth_tests'
          '';
        };
      });
}

