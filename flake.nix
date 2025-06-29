{
  description = "maelstrom-cpp";
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs {
          inherit system;
          overlays = [
            (self: super: {
              gradle = super.gradle.override {
                jdk = super.jdk11;
              };
            })
          ];
          config.allowUnsupportedSystem = true;
          config.allowUnfree = true;
        };

        buildInputs = with pkgs; [
          maelstrom-clj
          gcc
          glibc.dev
          libcxx.dev
        ];
        nativeBuildInputs = with pkgs; [
          clang-tools
        ];
      in 
      {
        devShells.default = pkgs.mkShell {
          buildInputs = [
          ] ++ buildInputs;
          nativeBuildInputs = with pkgs; [
            docker
            fish
            gdb
          ] ++ nativeBuildInputs;

          shellHook = ''
            export MAELSTROM_DIR=${pkgs.maelstrom-clj}
            export GLIBC_INCLUDE=${pkgs.glibc.dev}/include
            cat > compile_commands.json << EOF
            [
              {
                "directory": "$PWD",
                "command": "g++ -Og -g3 -ggdb -Wall -pedantic -std=c++23 -I./src -I${pkgs.libcxx.dev}/include/c++/v1 -I${pkgs.glibc.dev}/include -I${pkgs.nlohmann_json}/include -o src/%.o src/%.cpp",
                "file": "src/.*\\\\.cpp$"
              },
            ]
            EOF
            fish
          '';
        };
      }
    );
}
