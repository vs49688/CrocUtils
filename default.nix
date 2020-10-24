{ nixpkgs ? (import <nixpkgs> {})
, lib     ? nixpkgs.lib
, cmake   ? nixpkgs.cmake
, pkgs    ? nixpkgs
}:
pkgs.stdenv.mkDerivation {
  name = "CrocUtils";

  nativeBuildInputs = [ cmake ];

  enableParallelBuilding = true;

  src = builtins.filterSource (path: type: baseNameOf path != ".git") ./.;

  cmakeFlags = [ "-DCMAKE_BUILD_TYPE=MinSizeRel" ];

  meta = with lib; {
    description = "A small collection of utilities for Croc.";
    homepage    = "https://github.com/vs49688/CrocUtils";
    platforms   = platforms.all;
    license     = licenses.gpl2;
  };
}
