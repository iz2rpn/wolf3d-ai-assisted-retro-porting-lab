#!/usr/bin/env bash
set -euo pipefail

ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
QEMU="${QEMU_ARM:-qemu-arm}"
WL1="$ROOT/dist/z6s/wl1/wolf3d-z6s"
WL6="$ROOT/dist/z6s/wl6/wolf3d-z6s"
DATA="$ROOT/sdcard/wolf3d/data"
TMP="$(mktemp -d /tmp/z6s-qemu-XXXXXX)"

check_parser()
{
  local binary="$1"
  local output="$TMP/help.txt"
  "$QEMU" "$binary" --help >"$output" 2>&1 || true
  grep -q "Usage: Wolf4SDL" "$output"
}

check_parser "$WL1"
check_parser "$WL6"

smoke_game()
{
  local label="$1"
  local binary="$2"
  local status

  set +e
  (
    cd "$DATA"
    timeout 20 env \
      SDL_VIDEODRIVER=dummy \
      HOME="$TMP" \
      Z6S_DEMO_MODE=1 \
      Z6S_FORCE_AUDIO=0 \
      Z6S_FORCE_OPAQUE=0 \
      Z6S_TOUCH_INPUT=0 \
      "$QEMU" "$binary" \
        --resf 320 200 --bits 32 --nodblbuf --joystick -1 \
        --configdir "$TMP"
  )
  status=$?
  set -e

  if [ "$status" -ne 124 ]; then
    echo "QEMU $label smoke test failed: exit status $status" >&2
    exit 1
  fi
}

smoke_game WL1 "$WL1"
smoke_game WL6 "$WL6"

echo "QEMU OK: both WL1 and WL6 survived 20 s in no-keyboard mode."
echo "Temporary configuration kept at: $TMP"
