# XMvsSF 3v3 + DHC core (unofficial)

The `xmvsf3v3` branch of this fork is FinalBurn Neo's libretro port with the changes used by
XMvsSF 3v3 + DHC, a fan modification of X-Men Vs. Street Fighter (Capcom, 1996). It is not
part of FinalBurn Neo, and the FBNeo team does not support it.

## Reporting problems

Please don't take them to FinalBurn Neo's GitHub, forum or Discord. Use the feedback form on
the release page instead: https://www.morphus56k.com/blog/xmvsf-three-on-three

## What the branch adds

- `xmvsf6pd`, a clone of `xmvsf` (Europe 961004) for two or six players: six independent
  controllers, and the memory window the three-member teams use.
- HUD portrait colours for all eighteen colours, decoded from the game's packed colour table.
- An optional 512x224 (16:9) view, core option `fbneo-xmvsf-expanded-view`, applied when
  content is loaded.
- `src/burner/libretro/Makefile.xmv6diag`, a build with only the CPS-1 and CPS-2 drivers.

The CPS-2 changes switch on only for `xmvsf6pd`.

## Building

    make -C src/burner/libretro SUBSET=xmv6diag REGEN_HEADERS=1                 # Linux
    make -C src/burner/libretro SUBSET=xmv6diag REGEN_HEADERS=1 platform=win    # Windows
    make -C src/burner/libretro SUBSET=xmv6diag REGEN_HEADERS=1 platform=osx    # macOS

The result is `fbneo_xmv6diag_libretro`, which names itself "FinalBurn Neo (XMvsSF 3v3 +
DHC)". `REGEN_HEADERS=1` (needs perl) regenerates `src/dep/generated/driverlist.h` and
`gamelist.txt` so that `xmvsf6pd` is listed; the copies in the repository are FinalBurn Neo's,
so they show as modified after this build. FinalBurn Neo's usual build
(`make -C src/burner/libretro`) still works.

## Branches, tags and releases

- `xmvsf3v3` (the default branch): the changes, with FinalBurn Neo's master merged in.
- `master`: libretro/FBNeo's master, unchanged.
- `xmvsf3v3-v1.1.1`: the exact source of the cores in the XMvsSF 3v3 + DHC v1.1.1 packages.
  Its [release](https://github.com/plasx/FBNeo-xmvsf3v3/releases/tag/xmvsf3v3-v1.1.1) has the
  source archives.
- `xmvsf3v3-v1.1.0`: the source of the cores in the v1.1.0 packages, which v1.1.1 replaces. With
  RetroArch's SOCD option on, that core let FinalBurn Neo's player-3 stick cleaning act on the
  byte that holds P1's and P2's kicks, so P1's LK and MK pressed on the same frame didn't both
  reach the two-player game. Its
  [release](https://github.com/plasx/FBNeo-xmvsf3v3/releases/tag/xmvsf3v3-v1.1.0) has the source
  archives.
- `xmvsf3v3-v1.0.0`: the source of the cores in the v1.0.0 and v1.0.0-r2 packages, which are
  withdrawn.

## No ROMs

This repository has no ROM images, CPS-2 keys or other game data, and none can be provided.
The XMvsSF 3v3 + DHC packages patch your own `xmvsf.zip`.

## Licence

FinalBurn Neo's licence, [src/license.txt](src/license.txt), covers this fork unchanged: no
selling or other profit, source changes made public (this repository publishes them), the
licence text kept in full, no ROMs, and no donations. The XMvsSF 3v3 + DHC changes are
published under the same terms. X-Men and Marvel names belong to Marvel; Street Fighter and
Capcom names belong to Capcom.
