#!/bin/sh
set -u

ROOT=/mnt/extsd/wolf3d
BUSYBOX=/mnt/extsd/busybox
DATA=$ROOT/data
LOG=$ROOT/wolf3d.log
STOP=$ROOT/stop
GOV_STATE=$ROOT/governors.before
CLEANED=0

log()
{
  echo "$*" >> "$LOG"
}

ensure_zkgui()
{
  if ! "$BUSYBOX" pidof zkgui >/dev/null 2>&1; then
    log "Starting /bin/zkgui to initialize the LCD controller"
    /bin/setprop ctl.start zkswe >> "$LOG" 2>&1 || true
    "$BUSYBOX" sleep 4
  fi
}

hold_zkgui()
{
  for pid in $("$BUSYBOX" pidof zkgui 2>/dev/null); do
    log "Holding vendor HMI pid $pid"
    "$BUSYBOX" kill -STOP "$pid" >> "$LOG" 2>&1 || true
  done
}

resume_zkgui()
{
  for pid in $("$BUSYBOX" pidof zkgui 2>/dev/null); do
    log "Resuming vendor HMI pid $pid"
    "$BUSYBOX" kill -CONT "$pid" >> "$LOG" 2>&1 || true
  done
}

set_cpu_performance()
{
  : > "$GOV_STATE"
  for gov in /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor; do
    [ -e "$gov" ] || continue
    old=$("$BUSYBOX" cat "$gov" 2>/dev/null || echo unknown)
    echo "$gov $old" >> "$GOV_STATE"
    echo performance > "$gov" 2>> "$LOG" || true
  done
}

restore_cpu_governors()
{
  [ -f "$GOV_STATE" ] || return 0
  while read gov old; do
    [ -e "$gov" ] || continue
    [ "$old" = unknown ] || echo "$old" > "$gov" 2>> "$LOG" || true
  done < "$GOV_STATE"
}

cleanup()
{
  [ "$CLEANED" = 0 ] || return 0
  CLEANED=1
  restore_cpu_governors
  resume_zkgui
}

echo "Z6S Wolf3D launcher" > "$LOG"
: > "$ROOT/audio.log"
: > "$ROOT/input.log"
: > "$ROOT/performance.log"
date >> "$LOG" 2>/dev/null || true
trap cleanup EXIT INT TERM HUP

if [ -f "$STOP" ]; then
  log "Stop sentinel present; leaving the vendor HMI active"
  exit 0
fi
if [ -f "$DATA/AUDIOHED.WL6" ] && [ -f "$DATA/AUDIOT.WL6" ] &&
   [ -f "$DATA/GAMEMAPS.WL6" ] && [ -f "$DATA/MAPHEAD.WL6" ] &&
   [ -f "$DATA/VGADICT.WL6" ] && [ -f "$DATA/VGAGRAPH.WL6" ] &&
   [ -f "$DATA/VGAHEAD.WL6" ] && [ -f "$DATA/VSWAP.WL6" ]; then
  BIN=$ROOT/wolf3d-z6s-wl6
  EDITION="WL6 completa"
elif [ -f "$DATA/VSWAP.WL1" ]; then
  BIN=$ROOT/wolf3d-z6s-wl1
  EDITION="WL1 shareware"
else
  log "Missing a complete WL1 or WL6 data set in $DATA"
  exit 1
fi
if [ ! -x "$BIN" ]; then
  log "Missing executable for $EDITION: $BIN"
  exit 1
fi

ensure_zkgui
hold_zkgui
set_cpu_performance
"$BUSYBOX" chmod 666 /dev/fb0 /dev/input/event* >> "$LOG" 2>&1 || true
FB_SIZE=$("$BUSYBOX" cat /sys/class/graphics/fb0/virtual_size 2>/dev/null || echo unknown)
FB_BPP=$("$BUSYBOX" cat /sys/class/graphics/fb0/bits_per_pixel 2>/dev/null || echo unknown)
log "Framebuffer before launch: $FB_SIZE, ${FB_BPP}bpp"

cd "$DATA" || exit 1
export HOME=$ROOT
export SDL_VIDEODRIVER=fbcon
export SDL_FBDEV=/dev/fb0
export SDL_FBCON_NO_CONSOLE=1
export SDL_FB_BROKEN_MODES=1
export SDL_NOMOUSE=1
export SDL_MOUSEDRV=dummy
export Z6S_FORCE_OPAQUE=1
export Z6S_NATIVE_WIDTH=480
export Z6S_NATIVE_HEIGHT=272
export Z6S_METRICS_LOG=$ROOT/performance.log
export Z6S_FORCE_AUDIO=1
export Z6S_ALSA_PCM=/dev/snd/pcmC0D0p
export Z6S_TOUCH_INPUT=${Z6S_TOUCH_INPUT:-1}

while [ ! -f "$STOP" ]; do
  if "$BUSYBOX" grep -q 'Handlers=.*kbd' /proc/bus/input/devices 2>/dev/null; then
    export Z6S_DEMO_MODE=0
    log "Keyboard detected: original logo waits for input"
  else
    export Z6S_DEMO_MODE=1
    log "No keyboard detected: logo, main menu, then automatic demo"
  fi
  log "Starting Wolf3D $EDITION: 320x200 logical -> 480x272 native, 32 bpp, 11025 Hz"
  "$BIN" --resf 320 200 --bits 32 --samplerate 11025 \
    --audiobuffer 512 --nodblbuf --joystick -1 --configdir "$ROOT" \
    >> "$LOG" 2>&1 || true
  [ "${Z6S_RUN_ONCE:-0}" = 1 ] && break
  [ -f "$STOP" ] && break
  log "Wolf3D exited; restarting in one second"
  "$BUSYBOX" sleep 1
done

cleanup
trap - EXIT INT TERM HUP
log "Launcher finished"
