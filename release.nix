##
# Nix derivation for CI purposes.
# Invoke as:
#   nix-build release.nix \
#       --argstr gitDescribe $(git describe)
##
{ nixpkgs ? import <nixpkgs> {}
, gitDescribe
}:
let
  systems    = nixpkgs.lib.systems.examples;
  baseTarget = nixpkgs.callPackage ./default.nix { inherit nixpkgs; };

  targets = {
    musl64Static = baseTarget.override {
      pkgs = (import <nixpkgs> { crossSystem = systems.musl64; }).pkgsStatic;
    };

    win64Static = baseTarget.override {
      pkgs = (import <nixpkgs> { crossSystem = systems.mingwW64; }).pkgsStatic;
    };
  };
in
nixpkgs.stdenv.mkDerivation {
  name = "CrocUtils-CI";

  dontUnpack = true;

  installPhase = ''
    mkdir -p "$out"

    cp "${targets.musl64Static}/bin/croctool" \
      "$out/croctool-${gitDescribe}-${targets.musl64Static.stdenv.hostPlatform.system}"

    cp "${targets.win64Static}/bin/croctool.exe" \
      "$out/croctool-${gitDescribe}-${targets.win64Static.stdenv.hostPlatform.system}.exe"

    cd "$out" && for i in *; do
      sha256sum -b "$i" > "$i.sha256"
    done
  '';
}
