{
  description = "CrocUtils";

  outputs = { self, nixpkgs }: let
    version = if (self ? rev) then
      "1.3.0-${toString self.revCount}-g${builtins.substring 0 7 self.rev}"
    else
      "0.0.0-${self.lastModifiedDate}";
  in {
    packages.x86_64-linux = let
      pkgs = import nixpkgs { system = "x86_64-linux"; };

      vsclib = pkgs.fetchFromGitHub {
        owner  = "vs49688";
        repo   = "vsclib";
        rev    = "84f73a6e6de6100de21910274749b72fe5f788ea";
        sha256 = "sha256-HmnLaSha4L60Fh/gpodXs3C2clceTfFJqtPIF5PCTCY=";
      };

      musl64Packages = (import nixpkgs {
        system = "x86_64-linux";
        crossSystem = nixpkgs.lib.systems.examples.musl64;
      }).pkgsStatic;

      win64Packages = (import nixpkgs {
        system = "x86_64-linux";
        crossSystem = nixpkgs.lib.systems.examples.mingwW64;
      }).pkgsStatic;
    in rec {
      crocutils = pkgs.callPackage ./default.nix {
        inherit version vsclib;
      };

      default = crocutils;

      crocutils-musl64 = musl64Packages.callPackage ./default.nix {
        inherit version vsclib;
      };

      crocutils-win64 = win64Packages.callPackage ./default.nix {
        inherit version vsclib;
      };

      ci = pkgs.stdenvNoCC.mkDerivation {
        inherit version;

        pname = "crocutils-ci";

        dontUnpack = true;

        installPhase = ''
          mkdir -p $out

          cp "${crocutils-musl64}/bin/croctool" \
            "$out/croctool-${version}-${crocutils-musl64.stdenv.hostPlatform.system}"
          chmod 0755 "$out/croctool-${version}-${crocutils-musl64.stdenv.hostPlatform.system}"

          cp "${crocutils-win64}/bin/croctool.exe" \
            "$out/croctool-${version}-${crocutils-win64.stdenv.hostPlatform.system}.exe"
          chmod 0755 "$out/croctool-${version}-${crocutils-win64.stdenv.hostPlatform.system}.exe"

          cd "$out" && for i in *; do
            sha256sum -b "$i" > "$i.sha256"
          done
        '';
      };
    };
  };
}
