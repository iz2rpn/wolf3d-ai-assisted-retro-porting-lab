#!/usr/bin/env bash
set -euo pipefail

ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
THIRD="$ROOT/third_party"
mkdir -p "$THIRD"

clone_at()
{
  local name="$1" url="$2" commit="$3"
  local dir="$THIRD/$name"
  if [ ! -d "$dir/.git" ]; then
    echo "==> Cloning $name"
    git -c core.autocrlf=false clone "$url" "$dir"
    git -C "$dir" checkout --detach "$commit"
  fi
  local actual
  actual="$(git -C "$dir" rev-parse HEAD)"
  if [ "$actual" != "$commit" ]; then
    echo "$name is at $actual, expected $commit; refusing to overwrite it." >&2
    exit 1
  fi
}

apply_once()
{
  local repo="$1" patch="$2" sentinel_file="$3" sentinel="$4"
  if grep -Fq "$sentinel" "$repo/$sentinel_file"; then
    echo "==> Already applied: $(basename "$patch")"
  elif git -C "$repo" apply --check "$patch"; then
    git -C "$repo" apply "$patch"
    echo "==> Applied $(basename "$patch")"
  else
    echo "Patch does not match clean or patched sources: $patch" >&2
    exit 1
  fi
}

clone_at wolf4sdl https://github.com/lazd/wolf4sdl.git \
  3d41ccce8f8fecbed83aa9d8d42734c2c7e62374
clone_at id-wolf3d https://github.com/id-Software/wolf3d.git \
  05167784ef009d0d0daefe8d012b027f39dc8541
clone_at SDL-1.2.15 https://github.com/libsdl-org/SDL-1.2.git \
  457d4e55ffe1b6ad4c4fa4559dbda8360bf8253d
clone_at tinyalsa https://github.com/tinyalsa/tinyalsa.git \
  e43025bbf702eb7dd8edd48c1eb50530c60f1de8

apply_once "$THIRD/wolf4sdl" "$ROOT/patches/wolf4sdl-z6s.patch" \
  id_in.cpp Mix_Z6S_Pump
apply_once "$THIRD/wolf4sdl" "$ROOT/patches/wolf4sdl-z6s-native-video.patch" \
  id_vl.cpp VL_Z6SPresent
apply_once "$THIRD/SDL-1.2.15" "$ROOT/patches/sdl12-z6s.patch" \
  src/video/fbcon/SDL_fbevents.c SDL_FBCON_NO_CONSOLE
apply_once "$THIRD/tinyalsa" "$ROOT/patches/tinyalsa-no-plugins.patch" \
  src/pcm.c 'Z6S minimal build does not link tinyalsa plugin helpers'

echo "Sources are pinned and ready."
