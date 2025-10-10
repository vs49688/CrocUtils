{ stdenv, lib, fetchFromGitHub, fetchFromGitea, cmake, perl, version, commitHash }:
let
  vsclib = fetchFromGitea {
    domain = "git.vs49688.net";
    owner  = "zane";
    repo   = "vsclib";
    rev    = "510b1c50f8efc572bc11bd44679ed228b94a9b15";
    hash   = "sha256-UCi4Uy1h1fE9OpR/4uXOtcO69/3TdG0PoCmwJ8n9S3c=";
  };

  brender = fetchFromGitHub {
    owner  = "BlazingRenderer";
    repo   = "BRender";
    rev    = "db5b7b81ec0536393f2d01e55c4d230afc62ab8f";
    hash   = "sha256-wadNdyaW7kD9ooD0i8v1xtEpyBvSfOWOVef8RYZ2ee4=";
  };
in
stdenv.mkDerivation {
  inherit version;

  pname = "crocutils";

  nativeBuildInputs = [
    cmake
    perl
  ];

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

    rm -rf brender
    ln -s ${brender} brender
  '';

  doCheck = true;
  checkPhase = ''
    pushd ../tests
    ../build/tests/libcroc_tests
    popd
  '';

  passthru.commitHash = commitHash;

  meta = with lib; {
    description = "A small collection of utilities for Croc.";
    homepage    = "https://github.com/vs49688/CrocUtils";
    platforms   = platforms.all;
    license     = licenses.gpl2;
  };
}
