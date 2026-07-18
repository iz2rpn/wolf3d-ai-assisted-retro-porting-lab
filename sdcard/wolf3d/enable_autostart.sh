#!/bin/sh
set -eu
ROOT=/mnt/extsd/wolf3d
BUSYBOX=/mnt/extsd/busybox

"$BUSYBOX" rm -f "$ROOT/stop"
"$BUSYBOX" cp "$ROOT/EasyUI.wolf3d_autostart.cfg" /mnt/extsd/EasyUI.cfg
"$BUSYBOX" chmod 755 "$ROOT/run_wolf3d.sh" \
  "$ROOT/enable_autostart.sh" "$ROOT/disable_autostart.sh" \
  "$ROOT/test_once.sh" "$ROOT/wolf3d-z6s-wl1" \
  "$ROOT/wolf3d-z6s-wl6" "$ROOT/libwolf_autostart.so"
"$BUSYBOX" sync
echo "Wolf3D autostart enabled; reboot the LCD."
