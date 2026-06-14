{
  description = "caos - cellular automata experiments from Langton (1990)";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs = { self, nixpkgs }:
    let
      systems = [ "x86_64-linux" "aarch64-linux" "x86_64-darwin" "aarch64-darwin" ];
      forAll = f: nixpkgs.lib.genAttrs systems (s: f nixpkgs.legacyPackages.${s});
    in {
      devShells = forAll (pkgs: {
        default = pkgs.mkShell {
          packages = [ pkgs.gcc pkgs.gnumake pkgs.gnuplot ];
        };
      });

      packages = forAll (pkgs: {
        default = pkgs.stdenv.mkDerivation {
          pname = "caos";
          version = "0.1.0";
          src = ./.;
          installPhase = ''
            mkdir -p $out/bin
            cp caos $out/bin/
          '';
        };
      });
    };
}
