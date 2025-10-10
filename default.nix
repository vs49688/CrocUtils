{ stdenv, lib, fetchFromGitea, cmake, version, commitHash }:
let
  vsclib = fetchFromGitea {
    domain = "git.vs49688.net";
    owner  = "zane";
    repo   = "vsclib";
    rev    = "510b1c50f8efc572bc11bd44679ed228b94a9b15";
    hash   = "sha256-UCi4Uy1h1fE9OpR/4uXOtcO69/3TdG0PoCmwJ8n9S3c=";
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
