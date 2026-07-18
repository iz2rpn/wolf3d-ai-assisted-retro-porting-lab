# 3. Reproducible toolchain

## Host environment

Ubuntu supplies a complete ARM EABI soft-float cross-toolchain. Linux or WSL
also keeps legacy Autoconf behavior independent of the Windows `PATH`.

Environment used for the tracked builds:

```text
WSL: Ubuntu 26.04 LTS
Cross GCC: 15.2.0
binutils: 2.46
Target: arm-linux-gnueabi
```

Install dependencies on Ubuntu:

```bash
sudo apt-get update
sudo apt-get install -y \
  gcc-arm-linux-gnueabi g++-arm-linux-gnueabi \
  binutils-arm-linux-gnueabi libc6-dev-armel-cross \
  make autoconf file patch git ca-certificates qemu-user
```

On Windows, first install the WSL distribution:

```powershell
wsl --install -d Ubuntu-26.04 --no-launch
```

## Critical flags

```text
-march=armv5te
-mtune=arm926ej-s
-mfloat-abi=soft
-Os
-ffunction-sections -fdata-sections
-Wl,--gc-sections
-static
```

`-Os` reduces code and instruction-cache pressure. Separate sections allow the
linker to remove unused code. Static linking avoids dependence on the firmware's
musl libraries for the standalone game.

## Paths containing spaces

SDL 1.2's old libtool cannot configure reliably below a path containing spaces.
`build_z6s.sh` creates a temporary alias and SDL build directory below
`/var/tmp/z6s-wolf3d-UID`. Source, Wolf objects, and final output remain in the
project. This workaround is automatic.

## Clean reconstruction

```bash
bash scripts/fetch_sources.sh
bash scripts/verify_patches.sh
```

The clone uses `core.autocrlf=false`; the build also normalizes legacy Autoconf
scripts if a Windows checkout introduced CRLF endings.
