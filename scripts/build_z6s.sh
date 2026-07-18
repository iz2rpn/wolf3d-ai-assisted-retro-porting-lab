#!/usr/bin/env bash
set -euo pipefail

# Cross-build Wolf4SDL for the ARM926EJ-S used by the NOVA3D Z6S.
# Run from Ubuntu/WSL after installing the packages listed in docs/03_TOOLCHAIN.md.

ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
PROFILE="${1:-wl1}"
CROSS="${CROSS_COMPILE:-arm-linux-gnueabi-}"
JOBS="${JOBS:-$(nproc 2>/dev/null || echo 2)}"

SDL_SRC="$ROOT/third_party/SDL-1.2.15"
WOLF_SRC="$ROOT/third_party/wolf4sdl"
TINY_SRC="$ROOT/third_party/tinyalsa"
PORT="$ROOT/port/z6s"
BUILD="$ROOT/build/z6s/$PROFILE"
# SDL 1.2's old libtool cannot configure below a path containing spaces.
# Keep its generated tree on the WSL ext4 filesystem and expose the sources
# through a space-free symlink. Wolf4SDL itself can still build in the project.
WSL_BUILD_ROOT="${Z6S_WSL_BUILD_ROOT:-/var/tmp/z6s-wolf3d-${UID}}"
SDL_SOURCE_ALIAS="$WSL_BUILD_ROOT/SDL-1.2.15"
SDL_BUILD="$WSL_BUILD_ROOT/sdl-build"
SDL_PREFIX="$WSL_BUILD_ROOT/sysroot"
OBJ="$BUILD/obj"
DIST="$ROOT/dist/z6s/$PROFILE"

case "$PROFILE" in
  wl1) VERSION_FLAGS="-DUPLOAD -DCARMACIZED" ;;
  wl6) VERSION_FLAGS="-DGOODTIMES -DCARMACIZED" ;;
  *)
    echo "Usage: $0 [wl1|wl6]" >&2
    exit 2
    ;;
esac

for tool in "${CROSS}gcc" "${CROSS}g++" "${CROSS}readelf" make autoconf; do
  command -v "$tool" >/dev/null || {
    echo "Missing tool: $tool (see docs/03_TOOLCHAIN.md)" >&2
    exit 1
  }
done

ARCH_FLAGS="-march=armv5te -mtune=arm926ej-s -mfloat-abi=soft"
SIZE_FLAGS="-Os -ffunction-sections -fdata-sections -fno-strict-aliasing"
COMMON_FLAGS="$ARCH_FLAGS $SIZE_FLAGS -fno-stack-protector"

mkdir -p "$WSL_BUILD_ROOT" "$SDL_BUILD" "$SDL_PREFIX" "$OBJ" "$DIST"
ln -sfn "$SDL_SRC" "$SDL_SOURCE_ALIAS"

echo "==> Preparing SDL 1.2 build system"
# A source checkout made by Git for Windows may contain CRLF build metadata.
# GCC accepts CRLF C sources, but old autoconf/libtool scripts do not.
find "$SDL_SRC" -type f \( -name '*.sh' -o -name '*.in' -o -name '*.m4' \
  -o -name 'config.guess' -o -name 'config.sub' \) \
  -print0 | while IFS= read -r -d '' file; do
    if grep -q $'\r$' "$file"; then sed -i 's/\r$//' "$file"; fi
  done
find "$SDL_SRC/build-scripts" -type f -print0 | \
  while IFS= read -r -d '' file; do
    if grep -q $'\r$' "$file"; then sed -i 's/\r$//' "$file"; fi
  done
if [ ! -x "$SDL_SRC/configure" ] || [ "$SDL_SRC/configure.in" -nt "$SDL_SRC/configure" ]; then
  (cd "$SDL_SRC" && ./autogen.sh)
fi

SDL_CONFIG_TAG="z6s-sdl-armv5-fbcon-joystick-v2"
if [ ! -f "$SDL_BUILD/.z6s-config-tag" ] || \
   [ "$(cat "$SDL_BUILD/.z6s-config-tag" 2>/dev/null || true)" != "$SDL_CONFIG_TAG" ]; then
  echo "==> Configuring minimal static SDL 1.2"
  rm -rf "$SDL_BUILD"
  mkdir -p "$SDL_BUILD"
  (cd "$SDL_BUILD" && \
    CC="${CROSS}gcc" AR="${CROSS}ar" RANLIB="${CROSS}ranlib" \
    CFLAGS="$COMMON_FLAGS" \
    "$SDL_SOURCE_ALIAS/configure" \
      --host=arm-linux-gnueabi \
      --prefix="$SDL_PREFIX" \
      --disable-shared \
      --enable-static \
      --disable-audio \
      --enable-joystick \
      --disable-cdrom \
      --disable-threads \
      --enable-timers \
      --disable-video-x11 \
      --disable-video-opengl \
      --disable-video-dga \
      --disable-video-directfb \
      --enable-video-fbcon)
  printf '%s\n' "$SDL_CONFIG_TAG" > "$SDL_BUILD/.z6s-config-tag"
fi

echo "==> Building SDL 1.2 fbcon backend"
make -C "$SDL_BUILD" -j"$JOBS"
make -C "$SDL_BUILD" install

WOLF_SOURCES=(
  mame/fmopl.cpp
  id_ca.cpp id_in.cpp id_pm.cpp id_sd.cpp id_us_1.cpp
  id_vh.cpp id_vl.cpp signon.cpp
  wl_act1.cpp wl_act2.cpp wl_agent.cpp wl_atmos.cpp wl_cloudsky.cpp
  wl_debug.cpp wl_draw.cpp wl_floorceiling.cpp wl_game.cpp wl_inter.cpp
  wl_main.cpp wl_menu.cpp wl_parallax.cpp wl_play.cpp wl_state.cpp wl_text.cpp
)

PORT_SOURCES=(
  "$PORT/src/z6s_input.cpp"
  "$PORT/src/sdl_mixer_z6s.cpp"
)

TINY_SOURCES=(
  "$TINY_SRC/src/limits.c"
  "$TINY_SRC/src/pcm.c"
  "$TINY_SRC/src/pcm_hw.c"
)

INCLUDES=(
  "-I$WOLF_SRC"
  "-I$PORT/include"
  "-I$SDL_PREFIX/include/SDL"
  "-I$TINY_SRC/include"
  "-I$TINY_SRC/src"
)

DEFINES=(
  -DZ6S_TARGET
  -DVERSIONALREADYCHOSEN
  $VERSION_FLAGS
)

objects=()

compile_cpp() {
  local src="$1"
  local key="${src#"$ROOT/"}"
  key="${key//\//_}"
  local out="$OBJ/${key%.cpp}.o"
  objects+=("$out")
  echo "==> CXX ${src#"$ROOT/"}"
  "${CROSS}g++" $COMMON_FLAGS -std=gnu++98 -fno-exceptions \
    -fno-rtti "${INCLUDES[@]}" "${DEFINES[@]}" -c "$src" -o "$out"
}

compile_c() {
  local src="$1"
  local key="${src#"$ROOT/"}"
  key="${key//\//_}"
  local out="$OBJ/${key%.c}.o"
  objects+=("$out")
  echo "==> CC  ${src#"$ROOT/"}"
  "${CROSS}gcc" $COMMON_FLAGS -std=gnu99 "${INCLUDES[@]}" \
    "${DEFINES[@]}" -c "$src" -o "$out"
}

for rel in "${WOLF_SOURCES[@]}"; do compile_cpp "$WOLF_SRC/$rel"; done
for src in "${PORT_SOURCES[@]}"; do compile_cpp "$src"; done
for src in "${TINY_SOURCES[@]}"; do compile_c "$src"; done

echo "==> Linking static ARM executable"
"${CROSS}g++" $ARCH_FLAGS -static -Wl,--gc-sections -Wl,--build-id=none \
  -Wl,-s -o "$DIST/wolf3d-z6s" "${objects[@]}" \
  "$SDL_PREFIX/lib/libSDL.a" -lm -ldl

echo "==> Building musl-safe EasyUI proxy (no libc dependency)"
"${CROSS}gcc" $ARCH_FLAGS -Os -fPIC -fomit-frame-pointer \
  -ffunction-sections -fdata-sections -fno-stack-protector -fno-builtin \
  -ffreestanding -nostdlib -shared "$PORT/autostart/libwolf_autostart.c" \
  -Wl,--gc-sections -Wl,--hash-style=sysv -Wl,--build-id=none \
  -Wl,-soname,libwolf_autostart.so -o "$DIST/libwolf_autostart.so"

echo "==> ELF verification"
"${CROSS}readelf" -h "$DIST/wolf3d-z6s" | \
  grep -E 'Class:|Machine:|Flags:'
"${CROSS}readelf" -d "$DIST/wolf3d-z6s" | \
  grep NEEDED && { echo "ERROR: game must be static" >&2; exit 1; } || true
"${CROSS}readelf" -Ws "$DIST/libwolf_autostart.so" | \
  grep -E 'UND.*(dlopen|dlsym)|onEasyUI(Init|Deinit)'

echo "==> Done: $DIST"
