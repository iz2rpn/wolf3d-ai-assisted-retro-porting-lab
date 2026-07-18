#!/bin/sh
set -u
ROOT=/mnt/extsd/wolf3d
BUSYBOX=/mnt/extsd/busybox

"$BUSYBOX" rm -f "$ROOT/stop"
Z6S_RUN_ONCE=1 "$ROOT/run_wolf3d.sh"
"$BUSYBOX" touch "$ROOT/stop"
echo "One-shot test complete. See /mnt/extsd/wolf3d/wolf3d.log."
