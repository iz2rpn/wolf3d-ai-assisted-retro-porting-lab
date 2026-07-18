#!/usr/bin/env bash
set -euo pipefail

ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
TMP="$(mktemp -d /tmp/retroport-patches-XXXXXX)"
trap 'rm -rf "$TMP"' EXIT HUP INT TERM

archive_repo()
{
  local source="$1" destination="$2"
  mkdir -p "$destination"
  git -C "$source" archive HEAD | tar -x -C "$destination"
}

echo "==> Verifying Wolf4SDL patch sequence"
archive_repo "$ROOT/third_party/wolf4sdl" "$TMP/wolf4sdl"
git -C "$TMP/wolf4sdl" apply "$ROOT/patches/wolf4sdl-z6s.patch"
git -C "$TMP/wolf4sdl" apply --check \
  "$ROOT/patches/wolf4sdl-z6s-native-video.patch"
git -C "$TMP/wolf4sdl" apply \
  "$ROOT/patches/wolf4sdl-z6s-native-video.patch"

for file in id_in.cpp id_sd.cpp id_vh.cpp id_vl.cpp id_vl.h \
            wl_draw.cpp wl_main.cpp wl_menu.cpp; do
  # Git for Windows may materialize the working copy with CRLF. Compare the
  # semantic patch result without allowing line-ending churn to hide changes.
  cmp <(sed 's/\r$//' "$TMP/wolf4sdl/$file") \
      <(sed 's/\r$//' "$ROOT/third_party/wolf4sdl/$file")
done

echo "==> Verifying SDL 1.2 patch"
archive_repo "$ROOT/third_party/SDL-1.2.15" "$TMP/sdl"
git -C "$TMP/sdl" apply --check "$ROOT/patches/sdl12-z6s.patch"

echo "==> Verifying tinyalsa patch"
archive_repo "$ROOT/third_party/tinyalsa" "$TMP/tinyalsa"
git -C "$TMP/tinyalsa" apply --check "$ROOT/patches/tinyalsa-no-plugins.patch"

echo "Patch verification passed."
