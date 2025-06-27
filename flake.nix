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
          ruby
          boost
        ];
        nativeBuildInputs = with pkgs; [
          cmake
          gcc

          clang-tools
          pkg-config
          upx
          binutils
        ];
      in 
      {
        packages = {
          # Main application package
          default = pkgs.stdenv.mkDerivation {
            name = "malestrom-cpp";
            src = pkgs.lib.sourceByRegex ./. [
              "^src.*"
              "^deps.*"
              "^test.*"
              "^CMakeLists.txt"
            ];

            buildInputs = buildInputs;
            nativeBuildInputs = nativeBuildInputs;
            doCheck = true;

            installPhase = ''
              make install
              strip -s $out/bin/server
            '';
          };

          test = pkgs.stdenv.mkDerivation {
            pname = "malestrom-cpp-test";
            version = "1.0.0";
            src = pkgs.lib.sourceByRegex ./. [
              "^src.*"
              "^deps.*"
              "^test.*"
              "CMakeLists.txt"
            ];

            buildInputs = buildInputs;
            nativeBuildInputs = nativeBuildInputs;
          };
        };

        # Development shell
        devShells.default = pkgs.mkShell {
          buildInputs = [

          ] ++ buildInputs;
          nativeBuildInputs = with pkgs; [
            docker
            fish
            libcxx
          ] ++ nativeBuildInputs;

          shellHook = ''
            export MAELSTROM_DIR=${pkgs.maelstrom-clj}
            cat > compile_commands.json << EOF
            [
              {
                "directory": "$PWD",
                "command": "g++ -Og -g3 -ggdb -Wall -pedantic -std=c++23 -I./src -o src/%.o src/%.cpp",
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
