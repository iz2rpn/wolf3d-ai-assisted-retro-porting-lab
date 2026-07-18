# 5. Video: fixed scanout, scaling, and alpha

## Resolution strategy

The panel remains at 480x272x32 while Wolf3D renders its palettized scene at
320x200. Rendering the game directly at the panel resolution would process 104%
more logical pixels without adding detail to the original textures.

`VL_Z6SPresent()` converts and scales every logical frame across the full native
surface. X/Y maps are computed once; the hot loop uses palette lookups and
copies. A 522,240-byte intermediate buffer prevents scanout from observing a
frame while it is being assembled row by row.

Runtime contract:

```text
--resf 320 200 --bits 32
SDL_VIDEODRIVER=fbcon
SDL_FBDEV=/dev/fb0
SDL_FBCON_NO_CONSOLE=1
SDL_FB_BROKEN_MODES=1
Z6S_NATIVE_WIDTH=480
Z6S_NATIVE_HEIGHT=272
```

`SDL_FB_BROKEN_MODES=1` prevents alternative mode probing. fbcon double
buffering is disabled because it would alter `yres_virtual`. The logo, menu,
fades, 3D renderer, and demos all use the same presenter.

## SDL fbcon adaptation

SDL 1.2 normally opens a Linux virtual console and selects medium-raw keyboard
mode. This firmware exposes no usable VT. With `SDL_FBCON_NO_CONSOLE=1`, the
patched backend skips console ownership; the independent evdev adapter supplies
input.

## Black-screen root cause

Early `/dev/fb0` dumps contained correct RGB values while the physical panel was
black. On this vendor format, the high byte of each 32-bit pixel behaves as
alpha; SDL's palettized blitter left it at zero. Every converted color now sets:

```c
pixel |= 0xff000000U;
```

The final copy honors `screen->pitch`. The earlier `VL_Z6SForceOpaque()` remains
only as a fallback for unexpected video formats.

## Low-overhead telemetry

When `Z6S_METRICS_LOG` is set, the presenter records:

- one `sdl_first_present` event;
- one `present_window` record every five seconds;
- completed presentations, elapsed time, integer `fps_milli`, mean present
  interval, and process `maxrss_kb`.

No file output occurs per frame. `fps_milli` is a presentation-call rate, not a
claim about panel refresh, GPU execution, or end-to-end latency.

## Device diagnostics

If the process is alive but the screen is black:

```sh
/mnt/extsd/busybox pidof wolf3d-z6s-wl1 wolf3d-z6s-wl6
/mnt/extsd/busybox tail -100 /mnt/extsd/wolf3d/wolf3d.log
/mnt/extsd/busybox dd if=/dev/fb0 of=/mnt/extsd/wolf3d/fb-test.raw \
  bs=522240 count=2
```

Preserve the dump before changing code. It separates rendering failure from
pixel-format or scanout failure.
