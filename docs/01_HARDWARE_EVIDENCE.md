# 1. Hardware evidence and confidence

## Target identity

The following values come from the recovered SD backup, earlier ADB logs, ELF
inspection, and the working port. They are not generic assumptions about ARM
Linux devices.

| Item | Value | Evidence status |
|---|---|---|
| CPU | ARM926EJ-S, ARMv5TEJ | recovered and consistent with the ELF |
| Kernel | Linux 3.10.65 | recovered from the device |
| Vendor process | musl userspace | verified from recovered dependencies |
| Physical display | 480x272, 32 bpp, 1,920-byte pitch | verified by the prior port |
| Framebuffer | `/dev/fb0`, 1,044,480 mapped bytes | verified by the prior port |
| Touch | Goodix, previously `/dev/input/event1` | verified by the prior port |
| Audio | `/dev/snd/pcmC0D0p`, 11,025 Hz stereo | opened by the prior port |
| Diagnostics | ADB over USB | previously verified |

The mapped framebuffer is exactly two 480x272x4 frames. That does not prove the
second frame can be selected through a standard pan API; the port uses SDL
fbcon and leaves scanout behavior to the vendor driver.

## ABI contract

```text
ELF32 little-endian
Machine: ARM
EABI: Version 5
Float ABI: soft-float
ISA: ARMv5TE
Game link: static
```

An `armhf` binary is invalid for this CPU because the ABI expects hardware
floating-point registers. The correct toolchain target is
`arm-linux-gnueabi`, not `arm-linux-gnueabihf`.

The EasyUI proxy is a separate case: it is loaded inside the firmware's musl
process. It is built with `-nostdlib`, imports only `dlopen` and `dlsym`, and
uses direct ARM syscalls for process operations.

## Recovered backup

The private backup originally contained 803 files and 47,249,216 bytes. A
source-to-backup SHA-256 comparison matched all 803 files. The old executable,
WL1/WL6 data, EasyUI resources, and device logs remain outside the public
release and are not overwritten by repository reconstruction.

## Physical validation history

The first reconstructed build launched on the device and confirmed Goodix touch
and the target PCM node, but exposed three issues: a non-full-screen 320x200
image, a black 3D demo, and `EBADFD` on the first PCM write.

The corrected build uses one 480x272x32 physical mode, presents every screen
through an opaque scaler, and explicitly prepares the PCM with one bounded
recovery attempt. On 2026-07-18 the device owner reported correct, smooth
operation on the physical LCD and published a
[hardware demonstration](https://www.youtube.com/shorts/TgmA7cbyw-s).

This is strong qualitative evidence. Numerical startup, presentation-rate, and
resident-memory values require the newly instrumented `performance.log` and
remain distinct from the owner's observation.

## Remaining hardware-specific checks

- Capture the fresh instrumented log after at least 25 seconds of demo playback.
- Verify whether a physical connector supports powered USB host/OTG. ADB only
  proves device/gadget mode.
- Identify the amplifier or documentation before assuming speaker impedance and
  power. `SPK+`/`SPK-` likely indicates a bridged output but does not prove its
  safe load.
- Continue capability-based input discovery because event numbers can change.
