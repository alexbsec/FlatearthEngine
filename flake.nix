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
          xorg.libXdmcp
          xorg.libXau
        ];


      in {
        devShells.default = pkgs.mkShell {
          name = "flatearth-dev-shell";

          buildInputs = nativeBuildInputs;

shellHook = ''
  echo "[Flatearth] Dev shell pronta 🚀"
  echo "Use: build-engine | build-tests | run-tests"

  export PKG_CONFIG_PATH="${pkgs.xorg.libxcb.dev}/lib/pkgconfig:${pkgs.xorg.libX11.dev}/lib/pkgconfig:$PKG_CONFIG_PATH"
  export CMAKE_PREFIX_PATH="${pkgs.xorg.libxcb}:${pkgs.xorg.libX11}:${pkgs.xorg.libXdmcp}:${pkgs.xorg.libXau}:${pkgs.fmt}:${pkgs.vulkan-loader}:${pkgs.vulkan-headers}:${pkgs.vulkan-tools}:${pkgs.pkg-config}:${pkgs.gcc}:${pkgs.cmake}"
  export LIBRARY_PATH="${pkgs.xorg.libXdmcp}/lib:${pkgs.xorg.libXau}/lib:$LIBRARY_PATH"
  export NIX_LDFLAGS="-L${pkgs.xorg.libXdmcp}/lib -L${pkgs.xorg.libXau}/lib"

  alias build-engine='cmake -S engine -B engine/build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON && cmake --build engine/build'
  alias build-tests='cmake -S tests -B tests/build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON && cmake --build tests/build'
  alias run-tests='./tests/build/flatearth_tests'
'';


        };
      });
}

