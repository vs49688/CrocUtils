{
  description = "CrocUtils";

  inputs.nixpkgs.url = github:NixOS/nixpkgs;

  outputs = { self, nixpkgs }: let
    version = if (self ? shortRev) then
      "1.4.0-${toString self.revCount}-g${self.shortRev}"
    else
      "0.0.0-${self.lastModifiedDate}";

    commitHash = if (self ? rev) then self.rev else "unknown";

    mkShells = packages: builtins.mapAttrs (k: v: v.overrideAttrs(old: {
      hardeningDisable = [ "all" ];
    })) packages;
  in {
    packages.x86_64-linux = let
      pkgs = nixpkgs.legacyPackages.x86_64-linux;
    in rec {
      crocutils = pkgs.callPackage ./default.nix {
        inherit version commitHash;
      };

      default = crocutils;

      crocutils-musl64 = pkgs.pkgsCross.musl64.pkgsStatic.callPackage ./default.nix {
        inherit version commitHash;
      };

      crocutils-win64 = pkgs.pkgsCross.mingwW64.callPackage ./default.nix {
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
      pkgs = nixpkgs.legacyPackages.x86_64-darwin;
    in rec {
      crocutils = pkgs.callPackage ./default.nix {
        inherit version commitHash;
      };

      default = crocutils;
    };

    packages.aarch64-darwin = let
      pkgs = nixpkgs.legacyPackages.aarch64-darwin;
    in rec {
      crocutils = pkgs.callPackage ./default.nix {
        inherit version commitHash;
      };

      default = crocutils;
    };

    devShells.x86_64-linux   = mkShells self.packages.x86_64-linux;
    devShells.x86_64-darwin  = mkShells self.packages.x86_64-darwin;
    devShells.aarch64-darwin = mkShells self.packages.x86_64-darwin;
  };
}
