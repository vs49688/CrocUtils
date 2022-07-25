{ stdenv, lib, fetchFromGitHub, cmake, cjson, version, commitHash }:
let
  vsclib = fetchFromGitHub {
    owner  = "vs49688";
    repo   = "vsclib";
    rev    = "76f0994ddfaf6c0f7ec0f91708190681e51de6b0";
    sha256 = "sha256-ZNuOmubLzZ8eSYFaVWzeta4HNyLO/1go/jPskWY53g4=";
  };
in
stdenv.mkDerivation {
  inherit version;

  pname = "crocutils";

  nativeBuildInputs = [ cmake ];

  enableParallelBuilding = true;

  src = builtins.filterSource (path: type: baseNameOf path != ".git") ./.;

  cmakeFlags = [
    "-DCMAKE_BUILD_TYPE=MinSizeRel"
    "-DCROCTOOL_VERSION_STRING=${version}"
    "-DCROCTOOL_COMMIT_HASH=${commitHash}"
  ];

  preConfigure = ''
    rm -rf vsclib
    ln -s ${vsclib} vsclib

    rm -rf cJSON
    ln -s ${cjson.src} cJSON
  '';

  doCheck = true;
  checkPhase = ''
    pushd ../tests
    ../build/tests/libcroc_tests
    popd
  '';

  meta = with lib; {
    description = "A small collection of utilities for Croc.";
    homepage    = "https://github.com/vs49688/CrocUtils";
    platforms   = platforms.all;
    license     = licenses.gpl2;
  };
}
