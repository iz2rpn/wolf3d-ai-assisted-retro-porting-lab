# Third-party notices and asset policy

WOLF3D – AI-Assisted Retro Porting Lab analyzes and patches upstream projects but does not relicense
them. The source-fetch script pins each dependency to a specific commit.

Original WOLF3D lab work is dedicated under CC0-1.0. That dedication applies
only to rights held by the project author and contributors; it cannot remove the
licenses, notices, trademarks, or asset rights listed below. See `LICENSE` for
the path-level mapping and `LICENSES/` for included legal texts.

| Component | Upstream | License evidence in the fetched tree | Repository policy |
|---|---|---|---|
| Wolf4SDL | `github.com/lazd/wolf4sdl` | `license-gpl.txt`, `license-id.txt`, `license-mame.txt`, and the license section in `README.txt` | The Z6S port follows the GPL v2 path. Preserve all upstream notices. |
| Wolfenstein 3D historical source | `github.com/id-Software/wolf3d` | Historical limited-use agreement in `WOLFSRC/README/LICENSE.DOC` | Reference-only checkout. It is never treated as game-data permission. |
| SDL 1.2 | `github.com/libsdl-org/SDL-1.2` | GNU LGPL 2.1 text in `COPYING` | Preserve the license and satisfy source/relinking obligations for distributions. |
| tinyalsa | `github.com/tinyalsa/tinyalsa` | BSD-style notice in `NOTICE` | Preserve the notice and disclaimer. |
| MAME OPL emulator | bundled by Wolf4SDL | Separate notice in `license-mame.txt` | Preserve its notice; do not imply that the root license replaces it. |
| BusyBox 1.21.1 | recovered from the device | The binary identifies itself as GPLv2 | Excluded from Git. Do not redistribute it without corresponding source and required notices. |
| NOVA3D firmware UI and `libzkgui.so` | recovered from the device | No redistributable license was found | Excluded from Git and public releases. |

## Game data

Wolfenstein 3D data files are copyrighted assets and are not covered by the
source-code licenses. This repository does not distribute `.WL1`, `.WL6`, the
commercial DOS executable, or derived archives. Users must supply legally
obtained data files themselves. The registered WL6 case study was tested with
the project owner's original copy; those files remain local and private.

## Trademarks

Wolfenstein 3D and related names and marks belong to their respective owners.
WOLF3D – AI-Assisted Retro Porting Lab is an independent interoperability and software-modernization
project and is not endorsed by id Software, Bethesda, OpenAI, or NOVA3D.

## Release rule

Before publishing a commit or release, run:

```bash
python tools/retroport.py check-release --root .
```

The command is intentionally conservative. A clean result is necessary but is
not a substitute for legal review.
