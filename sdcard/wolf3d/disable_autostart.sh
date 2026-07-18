#!/bin/sh
set -u
ROOT=/mnt/extsd/wolf3d
BUSYBOX=/mnt/extsd/busybox

"$BUSYBOX" touch "$ROOT/stop"
"$BUSYBOX" cp "$ROOT/EasyUI.safe.cfg" /mnt/extsd/EasyUI.cfg
"$BUSYBOX" killall wolf3d-z6s-wl1 2>/dev/null || true
"$BUSYBOX" killall wolf3d-z6s-wl6 2>/dev/null || true
for pid in $("$BUSYBOX" pidof zkgui 2>/dev/null); do
  "$BUSYBOX" kill -CONT "$pid" 2>/dev/null || true
done
"$BUSYBOX" sync
echo "Wolf3D autostart disabled; reboot the LCD."
