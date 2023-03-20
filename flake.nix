{
  description = "CrocUtils";

  outputs = { self, nixpkgs }: let
    version = if (self ? shortRev) then
      "1.3.0-${toString self.revCount}-g${self.shortRev}"
    else
      "0.0.0-${self.lastModifiedDate}";

    commitHash = if (self ? rev) then self.rev else "unknown";
  in {
    packages.x86_64-linux = let
      pkgs = import nixpkgs { system = "x86_64-linux"; };

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
        inherit version commitHash;
      };

      default = crocutils;

      crocutils-musl64 = musl64Packages.callPackage ./default.nix {
        inherit version commitHash;
      };

      crocutils-win64 = win64Packages.callPackage ./default.nix {
        inherit version commitHash;
      };

      ci = pkgs.stdenvNoCC.mkDerivation {
        inherit version commitHash;

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

    packages.x86_64-darwin = let
      pkgs = import nixpkgs { system = "x86_64-darwin"; };
    in rec {
      crocutils = pkgs.callPackage ./default.nix {
        inherit version commitHash;
      };

      default = crocutils;
    };

    packages.aarch64-darwin = let
      pkgs = import nixpkgs { system = "aarch64-darwin"; };
    in rec {
      crocutils = pkgs.callPackage ./default.nix {
        inherit version commitHash;
      };

      default = crocutils;
    };
  };
}
