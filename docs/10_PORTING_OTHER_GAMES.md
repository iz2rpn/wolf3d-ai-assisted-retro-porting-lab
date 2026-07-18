# 10. Porting other open-source games

## 1. Separate engine from assets

Identify the code license, asset license and format, runtime dependencies, and
supported architectures before compiling. An open-source engine does not make
ROMs, WADs, PAKs, or commercial data redistributable. Prefer fully libre assets
or data the user supplies locally.

## 2. Find an existing portable substrate

Directly porting the DOS Wolf3D source would require replacing x86 assembly,
VGA, interrupts, and Sound Blaster access. Wolf4SDL had already isolated those
layers. For another game, prefer an existing software-rendered Linux/SDL port
over a modern OpenGL-only engine.

## 3. Write the target contract

```text
CPU: ARMv5TE, soft-float
OS: Linux 3.10.65
standalone ABI: static executable
video: /dev/fb0, 480x272x32
input: evdev
audio: ALSA pcmC0D0p
starting budget: 320x200, 11/22 kHz, few or no threads
```

Use `retroport.toml` and `tools/retroport.py analyze` to turn this contract into
versioned evidence rather than leaving it in chat history.

## 4. Build the smallest static executable first

Start with `--help` or one full-screen frame. Inspect the ABI with `readelf`,
then add assets, input, and audio one subsystem at a time. A black screen after
four simultaneous changes provides little diagnostic value.

## 5. Keep four small adapters

1. `platform_video`: mode, pitch, format, and present;
2. `platform_input`: evdev discovery and mapping;
3. `platform_audio`: PCM format and pump;
4. `platform_boot`: launcher, cleanup, and recovery.

The engine should know as little as possible about the appliance. Limit target
macros to integration points.

## 6. Make changes reconstructable

- pin exact commits, not branches;
- store patches that apply to a clean tree;
- version compiler flags and source lists;
- automate ABI checks;
- distinguish QEMU, physical, and still-unknown results;
- run a release guard against the Git index.

## 7. Optimize in evidence order

1. establish a baseline;
2. reduce logical resolution only when it matters;
3. reduce sample rate/channels if audio is measured as costly;
4. link out unused platform backends;
5. avoid threads on fragile firmware unless justified;
6. measure CPU/RAM/timing again;
7. optimize engine algorithms last.

## 8. Keep deployment reversible

Do not replace internal `/res` or `/bin` files. Prefer external storage, a safe
configuration, stop scripts, and removal after power-off. Every port should have
an ADB recovery path and a documented rollback.

## Checklist

- [ ] correct ARMv5 soft-float ELF
- [ ] no unavailable dynamic dependencies
- [ ] legally sourced, profile-compatible assets
- [ ] QEMU or parser smoke test
- [ ] visible framebuffer with correct pitch/format
- [ ] keyboard capability visible to the kernel
- [ ] touch can be disabled independently
- [ ] PCM does not enter a repeated error loop
- [ ] HMI and CPU governor restore on exit
- [ ] release guard and recovery procedure pass
