{
  description = "Hyprland plugin: scroll_move_natural";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    hyprland.url = "github:hyprwm/Hyprland/v0.56.0";
  };

  outputs = { self, nixpkgs, hyprland }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs { inherit system; };
    in
    {
      packages.${system}.default = pkgs.stdenv.mkDerivation {
        pname = "scroll-move-natural";
        version = "1.0";
        src = ./.;

        nativeBuildInputs = [ pkgs.pkg-config ];

        buildInputs = [
          hyprland.packages.${system}.hyprland.dev
          hyprland.packages.${system}.hyprland

          pkgs.aquamarine
          pkgs.hyprcursor
          pkgs.hyprgraphics
          pkgs.hyprlang
          pkgs.hyprutils
          pkgs.libdrm
          pkgs.libinput
          pkgs.cairo
          pkgs.pixman
          pkgs.pango
          pkgs.wayland
          pkgs.wayland-protocols
          pkgs.libxkbcommon
          pkgs.libxcb
          pkgs.xcbutil
          pkgs.xcbutilerrors
          pkgs.xcbutilrenderutil
          pkgs.libxcb-wm
          pkgs.xwayland
          pkgs.libei
          pkgs.libGL
          pkgs.mesa
          pkgs.systemd.dev
        ];

        buildPhase = ''
          runHook preBuild
          make CXX="$CXX"
          runHook postBuild
        '';

        installPhase = ''
          runHook preInstall
          mkdir -p $out/lib
          cp scroll-move-natural.so $out/lib/
          runHook postInstall
        '';
      };

      devShells.${system}.default = pkgs.mkShell {
        inputsFrom = [ self.packages.${system}.default ];
      };
    };
}
