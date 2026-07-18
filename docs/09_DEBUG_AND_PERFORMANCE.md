# 9. Debugging and performance

## Diagnostic order

1. Process: is either game executable alive?
2. Boot log: did the proxy and launcher run?
3. Data: does the compiled WL1/WL6 profile match the supplied files?
4. Video: are native mode, pitch, and alpha correct?
5. Input: which evdev capabilities were discovered?
6. Audio: did the PCM open, prepare, and write without repeated recovery?

Change one layer per test so results remain comparable.

## Useful device commands

```sh
cat /proc/cpuinfo
cat /proc/meminfo
cat /proc/bus/input/devices
ls -l /dev/fb0 /dev/input/event* /dev/snd/*
cat /sys/class/graphics/fb0/virtual_size
cat /sys/class/graphics/fb0/bits_per_pixel
ps
dmesg | tail -100
```

The expected video message is:

```text
Z6S video scaler: 320x200 logical -> 480x272x32 native
```

The launcher truncates session-specific input, audio, and performance logs so a
new run does not silently mix old and new evidence.

## Existing optimizations

- 320x200 logical rendering with fixed 480x272x32 scanout;
- precomputed X/Y scaling maps and no mode enumeration;
- 11,025 Hz audio, eight channels, and one main-loop period at a time;
- no audio/input worker threads;
- non-blocking input with two-second hot-plug scans;
- ARMv5TE-specific `-Os` build and section garbage collection;
- SDL without X11, OpenGL, generic audio, or CD-ROM backends;
- performance governor only while the game runs;
- five-second integer telemetry windows instead of per-frame logging.

## Measure before changing code

```sh
pid=$(pidof wolf3d-z6s-wl1 wolf3d-z6s-wl6)
cat /proc/${pid%% *}/status
cat /mnt/extsd/wolf3d/performance.log
```

Copy the raw log without editing it and parse it on the host with
`tools/retroport.py benchmark --device-log ...`. A presentation-call rate is a
regression signal, not automatically the display refresh rate.

If CPU is limiting, measure the mixer and presenter before lowering quality. If
audio underruns while CPU is free, compare a larger audio buffer. If control
latency rises, restore the 512-byte default. Record every changed variable.

## Compiler warnings

Wolf4SDL is historical code and modern GCC reports ignored return values and old
format patterns. The build does not hide them with `-w`, but it also does not use
`-Werror` because pre-existing warnings must not block reproducibility. New
project-authored code should remain warning-clean where practical.
