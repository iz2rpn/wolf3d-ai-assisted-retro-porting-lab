# Building and verification

## Host requirements

The supported build host is Linux or WSL. The tested reconstruction environment
uses Ubuntu 26.04, GCC cross 15.2.0, binutils 2.46, and the
`arm-linux-gnueabi` target. Python 3.11+ runs the WOLF3D lab tooling and tests.

On Ubuntu:

```bash
sudo apt-get update
sudo apt-get install -y \
  gcc-arm-linux-gnueabi g++-arm-linux-gnueabi \
  binutils-arm-linux-gnueabi libc6-dev-armel-cross \
  make autoconf file patch git ca-certificates qemu-user
```

## Reconstruct upstream source

```bash
bash scripts/fetch_sources.sh
bash scripts/verify_patches.sh
```

The fetch script uses exact commits. It refuses to overwrite a checkout at an
unexpected revision. The verification script rebuilds clean patched trees and
compares the expected modified files semantically, including normalization of
legacy CRLF line endings.

## Supply game data locally

No game data is included or downloaded. Copy a compatible, lawfully obtained
data set to the local ignored deployment directory.

WL1 and WL6 are distinct compile-time profiles. A registered WL6 set requires
all eight files with the `.WL6` suffix: `AUDIOHED`, `AUDIOT`, `GAMEMAPS`,
`MAPHEAD`, `VGADICT`, `VGAGRAPH`, `VGAHEAD`, and `VSWAP`. Never make a WL6 set
by renaming WL1 files.

## Cross-build

```bash
bash scripts/build_z6s.sh wl1
bash scripts/build_z6s.sh wl6
```

Important flags:

```text
-march=armv5te -mtune=arm926ej-s -mfloat-abi=soft
-Os -ffunction-sections -fdata-sections -Wl,--gc-sections -static
```

Expected outputs are generated under ignored `dist/z6s/<profile>/`. The script
checks for ELF32 ARM EABI5 soft-float, a static game executable, and an
autostart proxy whose unresolved surface is limited to `dlopen` and `dlsym`.

The SDL 1.2 build system cannot configure reliably below a path containing
spaces. `build_z6s.sh` creates a deterministic alias below `/var/tmp`; no manual
source relocation is required.

## Automated checks

```bash
python3 -m unittest -v
python3 -m py_compile tools/retroport.py
bash scripts/verify_patches.sh
bash scripts/test_qemu.sh
python3 tools/retroport.py check-release
```

QEMU validates ARM startup, argument handling, data loading, and the
keyboard-free demo path for both profiles. It deliberately cannot validate the
vendor framebuffer, Goodix input, USB host mode, physical audio, or timing.

## Generate evidence

```bash
python3 tools/retroport.py analyze \
  --json reports/wolf3d-evidence.json \
  --markdown reports/wolf3d-evidence.md

python3 tools/retroport.py benchmark \
  --binary wl1=dist/z6s/wl1/wolf3d-z6s \
  --binary wl6=dist/z6s/wl6/wolf3d-z6s \
  --output benchmarks/runs/local.json
```

After a device run, copy `performance.log` without editing it and parse it:

```bash
python3 tools/retroport.py benchmark \
  --binary wl1=dist/z6s/wl1/wolf3d-z6s \
  --device-log path/to/performance.log \
  --output benchmarks/runs/device.json
```

The log contains first-present time and five-second presentation windows. The
reported rate is completed presentation calls per second, not GPU timing or a
claim about internal renderer throughput.

## Physical validation checklist

- Verify one fixed 480x272x32 mode from logo through demo.
- Confirm no black transition when the 3D demo starts.
- Capture at least three telemetry windows after startup.
- Check keyboard enumeration in `/proc/bus/input/devices`.
- Test touch separately from keyboard input.
- Inspect `audio.log` before connecting a speaker.
- Verify that exit restores the CPU governor and resumes the vendor UI.
- Remove the SD only after power-off.
