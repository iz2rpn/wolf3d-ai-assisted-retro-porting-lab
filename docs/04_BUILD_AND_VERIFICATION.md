# 4. Build and verification

## Data profiles

```bash
bash scripts/build_z6s.sh wl1
bash scripts/build_z6s.sh wl6
```

- `wl1` builds the shareware data profile.
- `wl6` builds the registered profile. Its eight original data files must come
  from the user's own lawful copy and are never included in this repository.

The local launcher selects WL6 only when all eight expected `.WL6` files exist;
otherwise it falls back to a complete compatible WL1 set. Renaming files between
profiles is invalid.

## Build stages

1. Build static SDL 1.2 with fbcon and only required subsystems.
2. Build Wolf4SDL with the selected data profile and `Z6S_TARGET`.
3. Add the Z6S evdev backend.
4. Add the minimal mixer and required tinyalsa PCM modules.
5. Link statically, strip symbols, and inspect the ABI.
6. Build the `-nostdlib` EasyUI proxy.
7. Package generated output only into ignored local directories.

## ELF checks

```bash
arm-linux-gnueabi-readelf -h dist/z6s/wl1/wolf3d-z6s
arm-linux-gnueabi-readelf -d dist/z6s/wl1/wolf3d-z6s
arm-linux-gnueabi-readelf -Ws dist/z6s/wl1/libwolf_autostart.so
```

The game must report EABI5 soft-float and no dynamic section. The proxy must
export `onEasyUIInit`/`onEasyUIDeinit` and leave only `dlopen`/`dlsym` unresolved.

## QEMU coverage

`scripts/test_qemu.sh` checks both `--help` paths, then runs WL1 and WL6 for 20
seconds with a dummy SDL driver and compatible local data. It intentionally does
not pass `--nowait`; the no-keyboard policy exercises logo/menu/demo startup.
Missing `/dev/snd` is expected under QEMU and must degrade to silent operation.

QEMU cannot validate the vendor framebuffer, Goodix input, USB host capability,
speaker output, or real timing. Those are separate physical tests.

## Full verification set

```bash
python3 -m unittest -v
python3 -m py_compile tools/retroport.py
bash scripts/verify_patches.sh
bash scripts/build_z6s.sh wl1
bash scripts/build_z6s.sh wl6
bash scripts/test_qemu.sh
python3 tools/retroport.py check-release
```

See [../BUILDING.md](../BUILDING.md) for end-to-end instructions and
[../BENCHMARKS.md](../BENCHMARKS.md) for measurement semantics.
