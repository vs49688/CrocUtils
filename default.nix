{ stdenv, lib, fetchFromGitHub, cmake, cjson, version }:
let
  vsclib = fetchFromGitHub {
    owner  = "vs49688";
    repo   = "vsclib";
    rev    = "9084b9a6891d2029c8bca162b355d307d29b381f";
    sha256 = "sha256-pdwGlar7vjqaiVAJwwL7Bdq9cSpTRDE5L4hFn5g7LJw=";
  };
in
stdenv.mkDerivation {
  inherit version;

  pname = "crocutils";

  nativeBuildInputs = [ cmake ];

  enableParallelBuilding = true;

  src = builtins.filterSource (path: type: baseNameOf path != ".git") ./.;

  cmakeFlags = [ "-DCMAKE_BUILD_TYPE=MinSizeRel" ];

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
