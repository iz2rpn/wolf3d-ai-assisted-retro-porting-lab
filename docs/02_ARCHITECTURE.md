# 2. Target port architecture

## Boot flow

1. EasyUI reads `/mnt/extsd/EasyUI.cfg`.
2. It loads `libwolf_autostart.so` as a startup library.
3. The proxy loads the original `/res/lib/libzkgui.so` and forwards
   `onEasyUIInit`, preserving vendor hardware initialization.
4. A child waits eight seconds and starts `run_wolf3d.sh` with BusyBox.
5. The launcher pauses `zkgui`, temporarily selects the performance governor,
   detects data/input policy, and starts Wolf4SDL.
6. On exit it restores the previous governor and resumes the HMI.

Internal firmware is not modified. Removing the SD while powered off restores
the appliance's normal internal boot path.

## Source layout

```text
reconstructed_wolf3d/
|-- .codex/skills/retroport-ai/  GPT-5.6/Codex workflow
|-- docs/                        target engineering notes
|-- patches/                     reproducible upstream adaptations
|-- port/z6s/
|   |-- autostart/               libc-free EasyUI proxy
|   |-- include/SDL_mixer.h      minimal required mixer API
|   `-- src/
|       |-- sdl_mixer_z6s.cpp    synchronous mixer and tinyalsa pump
|       `-- z6s_input.cpp        evdev keyboard and touch
|-- scripts/                     fetch, build, verify, and QEMU checks
|-- tools/retroport.py           deterministic evidence tool
|-- sdcard/                      public launcher/config skeleton only
`-- third_party/                 ignored reconstructed upstream trees
```

## Why Wolf4SDL

The id Software source preserves the original DOS implementation, including x86
assembly, VGA, interrupts, and Sound Blaster code. It is invaluable reference
material but not a direct ARM/Linux base. Wolf4SDL already replaces those
interfaces with portable C/C++ and SDL, keeping the Z6S changes focused on
video, input, audio, and boot.

## Fixed upstream revisions

| Project | Commit |
|---|---|
| Wolf4SDL | `3d41ccce8f8fecbed83aa9d8d42734c2c7e62374` |
| id Software wolf3d | `05167784ef009d0d0daefe8d012b027f39dc8541` |
| SDL 1.2 | `457d4e55ffe1b6ad4c4fa4559dbda8360bf8253d` |
| tinyalsa | `e43025bbf702eb7dd8edd48c1eb50530c60f1de8` |

Run `scripts/fetch_sources.sh` to recreate `third_party`. It refuses to overwrite
a checkout on an unexpected commit. Run `scripts/verify_patches.sh` to confirm
the patch stack reproduces the expected semantic modifications.

For the product-level architecture, see [../ARCHITECTURE.md](../ARCHITECTURE.md).
