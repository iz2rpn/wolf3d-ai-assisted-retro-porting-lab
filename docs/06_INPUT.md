# 6. USB keyboard and touch

## Keyboard backend

The adapter opens `/dev/input/event0` through `event15` in non-blocking mode,
reads capabilities with `EVIOCGBIT`, and recognizes a keyboard by `KEY_A` and
`KEY_ENTER`. It rescans every two seconds, allowing a keyboard to be connected
after startup and recovering from disconnects.

Linux keys become SDL 1.2 events in the game thread. The adapter adds no daemon,
worker thread, or mutex.

| Action | Key |
|---|---|
| Move | Arrow keys |
| Fire | Ctrl |
| Open/use | Space |
| Run | Shift |
| Strafe | Alt + arrow |
| Weapons | 1-4 |
| Menu/back | Escape |
| Confirm | Enter |

Detection is logged to `/mnt/extsd/wolf3d/input.log`.

## Startup without a keyboard

Before each run, the launcher searches `/proc/bus/input/devices` for a keyboard
handler:

- keyboard present: `Z6S_DEMO_MODE=0`, preserving interactive startup;
- keyboard absent: `Z6S_DEMO_MODE=1`, showing the logo for three seconds and the
  main menu for five seconds before starting a demo;
- input during the timed menu makes it interactive immediately.

The engine continues rescanning devices, so a keyboard connected later can take
control without a restart.

## USB limitation

The software is ready for an evdev keyboard, but the physical connector must
support host mode and supply power. ADB uses the opposite USB direction. If no
keyboard appears, inspect:

```sh
cat /proc/bus/input/devices
ls -l /dev/input/event*
dmesg | tail -80
```

If the kernel does not enumerate a new input device, the issue precedes the
Wolf3D key mapping; use the correct OTG/host port or a powered hub.

## Touch menus and gameplay

Touch devices are recognized from `ABS_X/Y` or multitouch position capabilities.
Raw ranges come from `EVIOCGABS`, so Goodix resolution is not hard-coded.

Menus accept a selection after a released tap with at most 18 pixels of movement
and less than two seconds of duration. Gameplay uses an invisible 4x3 grid:

| Row | Zone 1 | Zone 2 | Zone 3 | Zone 4 |
|---|---|---|---|---|
| Top | Weapon 1 | Weapon 2 | Weapon 3 | Weapon 4 |
| Middle | Left | Forward | Right | Use |
| Bottom | Escape | Fire | Back | Enter |

Keyboard input always remains enabled. `Z6S_TOUCH_INPUT=0` disables touch only.
Orientation overrides are `Z6S_TOUCH_SWAP_XY`, `Z6S_TOUCH_INVERT_X`, and
`Z6S_TOUCH_INVERT_Y`.
