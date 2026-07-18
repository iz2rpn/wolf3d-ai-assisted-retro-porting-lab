# RetroPort AI evidence report

Generated: `2026-07-18T20:15:49Z`
Target: Linux ARMv5TE soft-float embedded framebuffer device

## Deterministic scope

- 18 files, 2381 lines, 70100 bytes scanned.
- 158 portability evidence items.
- 4 patch files and 4 pinned dependencies.
- Release guard: PASS.

This report contains evidence, not an autonomous refactoring decision.
GPT-5.6 may explain and prioritize it, but every proposed patch requires
human review and deterministic verification.

## Pinned dependencies

| Name | Commit | Upstream |
|---|---|---|
| wolf4sdl | `3d41ccce8f8fecbed83aa9d8d42734c2c7e62374` | `https://github.com/lazd/wolf4sdl.git` |
| id-wolf3d | `05167784ef009d0d0daefe8d012b027f39dc8541` | `https://github.com/id-Software/wolf3d.git` |
| SDL-1.2.15 | `457d4e55ffe1b6ad4c4fa4559dbda8360bf8253d` | `https://github.com/libsdl-org/SDL-1.2.git` |
| tinyalsa | `e43025bbf702eb7dd8edd48c1eb50530c60f1de8` | `https://github.com/tinyalsa/tinyalsa.git` |

## Patch surface

| Patch | Files | Hunks | + | - |
|---|---:|---:|---:|---:|
| `patches/sdl12-z6s.patch` | 1 | 2 | 10 | 0 |
| `patches/tinyalsa-no-plugins.patch` | 1 | 1 | 5 | 1 |
| `patches/wolf4sdl-z6s-native-video.patch` | 4 | 14 | 195 | 18 |
| `patches/wolf4sdl-z6s.patch` | 7 | 19 | 189 | 1 |

## Entry points

| Kind | Location | Evidence |
|---|---|---|
| audio-pump | `patches/wolf4sdl-z6s.patch:26` | `+        Mix_Z6S_Pump();` |
| audio-pump | `patches/wolf4sdl-z6s.patch:54` | `+    Mix_Z6S_Pump();` |
| audio-pump | `patches/wolf4sdl-z6s.patch:72` | `+           is absent. Mix_Z6S_Pump() becomes a no-op until audio is available. */` |
| audio-pump | `port/z6s/include/SDL_mixer.h:55` | `void Mix_Z6S_Pump(void);` |
| audio-pump | `port/z6s/src/sdl_mixer_z6s.cpp:457` | `extern "C" void Mix_Z6S_Pump(void)` |
| frame-presenter | `patches/wolf4sdl-z6s-native-video.patch:12` | `+    VL_Z6SPresent(screenBuffer);` |
| frame-presenter | `patches/wolf4sdl-z6s-native-video.patch:30` | `+    VL_Z6SPresent(source);` |
| frame-presenter | `patches/wolf4sdl-z6s-native-video.patch:150` | `+void VL_Z6SPresent(SDL_Surface *source)` |
| frame-presenter | `patches/wolf4sdl-z6s-native-video.patch:269` | `+    VL_Z6SPresent(screenBuffer);` |
| frame-presenter | `patches/wolf4sdl-z6s-native-video.patch:282` | `+        VL_Z6SPresent(curSurface);` |
| frame-presenter | `patches/wolf4sdl-z6s-native-video.patch:298` | `+            VL_Z6SPresent(curSurface);` |
| frame-presenter | `patches/wolf4sdl-z6s-native-video.patch:313` | `+void VL_Z6SPresent(SDL_Surface *source);` |
| frame-presenter | `patches/wolf4sdl-z6s-native-video.patch:325` | `+        VL_Z6SPresent(screenBuffer);` |
| input-pump | `patches/wolf4sdl-z6s.patch:25` | `+        Z6S_InputPump();` |
| input-pump | `patches/wolf4sdl-z6s.patch:53` | `+    Z6S_InputPump();` |
| input-pump | `port/z6s/include/z6s_input.h:10` | `void Z6S_InputPump(void);` |
| input-pump | `port/z6s/src/z6s_input.cpp:365` | `void Z6S_InputPump(void)` |
| shared-library-hook | `port/z6s/autostart/libwolf_autostart.c:146` | `int onEasyUIInit(void *a0, void *a1, void *a2, void *a3)` |
| shell-entry | `scripts/build_z6s.sh:1` | `#!/usr/bin/env bash` |
| shell-entry | `scripts/clean_z6s.sh:1` | `#!/usr/bin/env bash` |
| shell-entry | `scripts/fetch_sources.sh:1` | `#!/usr/bin/env bash` |
| shell-entry | `scripts/test_qemu.sh:1` | `#!/usr/bin/env bash` |
| shell-entry | `scripts/verify_patches.sh:1` | `#!/usr/bin/env bash` |
| shell-entry | `sdcard/wolf3d/disable_autostart.sh:1` | `#!/bin/sh` |
| shell-entry | `sdcard/wolf3d/enable_autostart.sh:1` | `#!/bin/sh` |
| shell-entry | `sdcard/wolf3d/run_wolf3d.sh:1` | `#!/bin/sh` |
| shell-entry | `sdcard/wolf3d/test_once.sh:1` | `#!/bin/sh` |

## Portability evidence

| Severity | Category | Rule | Location | Evidence |
|---|---|---|---|---|
| low | configuration | environment-configuration | `patches/sdl12-z6s.patch:11` | `+    if ( SDL_getenv("SDL_FBCON_NO_CONSOLE") != NULL ) {` |
| low | configuration | environment-configuration | `patches/sdl12-z6s.patch:22` | `+    if ( SDL_getenv("SDL_FBCON_NO_CONSOLE") != NULL ) {` |
| high | hardware-boundary | absolute-kernel-path | `patches/sdl12-z6s.patch:28` | `static const char * const tty0[] = { "/dev/tty0", "/dev/vc/0", NULL };` |
| medium | rendering | sdl12-framebuffer-api | `patches/wolf4sdl-z6s-native-video.patch:8` | `-	SDL_BlitSurface(screenBuffer, NULL, screen, NULL);` |
| medium | rendering | sdl12-framebuffer-api | `patches/wolf4sdl-z6s-native-video.patch:14` | `+	SDL_BlitSurface(screenBuffer, NULL, screen, NULL);` |
| medium | rendering | sdl12-framebuffer-api | `patches/wolf4sdl-z6s-native-video.patch:15` | `SDL_Flip(screen);` |
| medium | rendering | sdl12-framebuffer-api | `patches/wolf4sdl-z6s-native-video.patch:39` | `SDL_BlitSurface(source, NULL, screen, NULL);` |
| medium | rendering | sdl12-framebuffer-api | `patches/wolf4sdl-z6s-native-video.patch:43` | `SDL_Flip(screen);` |
| medium | rendering | sdl12-framebuffer-api | `patches/wolf4sdl-z6s-native-video.patch:53` | `SDL_Flip(screen);` |
| medium | rendering | sdl12-framebuffer-api | `patches/wolf4sdl-z6s-native-video.patch:59` | `SDL_BlitSurface(source, NULL, screen, NULL);` |
| medium | rendering | sdl12-framebuffer-api | `patches/wolf4sdl-z6s-native-video.patch:63` | `SDL_Flip(screen);` |
| low | configuration | environment-configuration | `patches/wolf4sdl-z6s-native-video.patch:96` | `+    const char *path = getenv("Z6S_METRICS_LOG");` |
| medium | rendering | sdl12-framebuffer-api | `patches/wolf4sdl-z6s-native-video.patch:161` | `+        SDL_BlitSurface(source, NULL, screen, NULL);` |
| medium | rendering | sdl12-framebuffer-api | `patches/wolf4sdl-z6s-native-video.patch:163` | `+        SDL_Flip(screen);` |
| medium | rendering | sdl12-framebuffer-api | `patches/wolf4sdl-z6s-native-video.patch:225` | `+    SDL_Flip(screen);` |
| low | configuration | environment-configuration | `patches/wolf4sdl-z6s-native-video.patch:240` | `+    const char *z6sWidthEnv = getenv("Z6S_NATIVE_WIDTH");` |
| low | rendering | fixed-render-dimensions | `patches/wolf4sdl-z6s-native-video.patch:240` | `+    const char *z6sWidthEnv = getenv("Z6S_NATIVE_WIDTH");` |
| low | configuration | environment-configuration | `patches/wolf4sdl-z6s-native-video.patch:241` | `+    const char *z6sHeightEnv = getenv("Z6S_NATIVE_HEIGHT");` |
| low | rendering | fixed-render-dimensions | `patches/wolf4sdl-z6s-native-video.patch:241` | `+    const char *z6sHeightEnv = getenv("Z6S_NATIVE_HEIGHT");` |
| medium | rendering | sdl12-framebuffer-api | `patches/wolf4sdl-z6s-native-video.patch:251` | `+    screen = SDL_SetVideoMode(z6sNativeWidth, z6sNativeHeight, screenBits,` |
| medium | rendering | sdl12-framebuffer-api | `patches/wolf4sdl-z6s-native-video.patch:254` | `screen = SDL_SetVideoMode(screenWidth, screenHeight, screenBits,` |
| medium | rendering | sdl12-framebuffer-api | `patches/wolf4sdl-z6s-native-video.patch:278` | `-        SDL_BlitSurface(curSurface, NULL, screen, NULL);` |
| medium | rendering | sdl12-framebuffer-api | `patches/wolf4sdl-z6s-native-video.patch:284` | `+        SDL_BlitSurface(curSurface, NULL, screen, NULL);` |
| medium | rendering | sdl12-framebuffer-api | `patches/wolf4sdl-z6s-native-video.patch:285` | `SDL_Flip(screen);` |
| medium | rendering | sdl12-framebuffer-api | `patches/wolf4sdl-z6s-native-video.patch:294` | `-            SDL_BlitSurface(curSurface, NULL, screen, NULL);` |
| medium | rendering | sdl12-framebuffer-api | `patches/wolf4sdl-z6s-native-video.patch:300` | `+            SDL_BlitSurface(curSurface, NULL, screen, NULL);` |
| medium | rendering | sdl12-framebuffer-api | `patches/wolf4sdl-z6s-native-video.patch:301` | `SDL_Flip(screen);` |
| medium | rendering | sdl12-framebuffer-api | `patches/wolf4sdl-z6s-native-video.patch:327` | `SDL_BlitSurface(screenBuffer, NULL, screen, NULL);` |
| medium | rendering | sdl12-framebuffer-api | `patches/wolf4sdl-z6s-native-video.patch:328` | `SDL_Flip(screen);` |
| medium | rendering | sdl12-framebuffer-api | `patches/wolf4sdl-z6s.patch:83` | `SDL_BlitSurface(screenBuffer, NULL, screen, NULL);` |
| medium | rendering | sdl12-framebuffer-api | `patches/wolf4sdl-z6s.patch:87` | `SDL_Flip(screen);` |
| medium | rendering | sdl12-framebuffer-api | `patches/wolf4sdl-z6s.patch:93` | `SDL_BlitSurface(source, NULL, screen, NULL);` |
| medium | rendering | sdl12-framebuffer-api | `patches/wolf4sdl-z6s.patch:97` | `SDL_Flip(screen);` |
| medium | rendering | sdl12-framebuffer-api | `patches/wolf4sdl-z6s.patch:107` | `SDL_Flip(screen);` |
| medium | rendering | sdl12-framebuffer-api | `patches/wolf4sdl-z6s.patch:113` | `SDL_BlitSurface(source, NULL, screen, NULL);` |
| medium | rendering | sdl12-framebuffer-api | `patches/wolf4sdl-z6s.patch:117` | `SDL_Flip(screen);` |
| low | configuration | environment-configuration | `patches/wolf4sdl-z6s.patch:137` | `+        const char *value = getenv("Z6S_FORCE_OPAQUE");` |
| medium | rendering | sdl12-framebuffer-api | `patches/wolf4sdl-z6s.patch:162` | `SDL_BlitSurface(curSurface, NULL, screen, NULL);` |
| medium | rendering | sdl12-framebuffer-api | `patches/wolf4sdl-z6s.patch:166` | `SDL_Flip(screen);` |
| medium | rendering | sdl12-framebuffer-api | `patches/wolf4sdl-z6s.patch:172` | `SDL_BlitSurface(curSurface, NULL, screen, NULL);` |
| medium | rendering | sdl12-framebuffer-api | `patches/wolf4sdl-z6s.patch:176` | `SDL_Flip(screen);` |
| low | configuration | environment-configuration | `patches/wolf4sdl-z6s.patch:204` | `+    const char *z6sForceAudio = getenv("Z6S_FORCE_AUDIO");` |
| low | configuration | environment-configuration | `patches/wolf4sdl-z6s.patch:223` | `+        const char *demoMode = getenv("Z6S_DEMO_MODE");` |
| low | configuration | environment-configuration | `patches/wolf4sdl-z6s.patch:256` | `+        const char *z6sDemoMode = getenv("Z6S_DEMO_MODE");` |
| critical | abi | inline-arm-syscall | `port/z6s/autostart/libwolf_autostart.c:19` | `#define SYS_fork      2` |
| critical | abi | inline-arm-syscall | `port/z6s/autostart/libwolf_autostart.c:20` | `#define SYS_write     4` |
| critical | abi | inline-arm-syscall | `port/z6s/autostart/libwolf_autostart.c:21` | `#define SYS_open      5` |
| critical | abi | inline-arm-syscall | `port/z6s/autostart/libwolf_autostart.c:23` | `#define SYS_execve   11` |
| critical | abi | inline-arm-syscall | `port/z6s/autostart/libwolf_autostart.c:25` | `#define SYS_nanosleep 162` |
| critical | abi | inline-arm-syscall | `port/z6s/autostart/libwolf_autostart.c:46` | `register long r7 __asm__("r7") = number;` |
| critical | abi | inline-arm-syscall | `port/z6s/autostart/libwolf_autostart.c:47` | `register long r0 __asm__("r0");` |
| critical | abi | inline-arm-syscall | `port/z6s/autostart/libwolf_autostart.c:48` | `__asm__ volatile("svc 0" : "=r"(r0) : "r"(r7) : "memory");` |
| critical | abi | inline-arm-syscall | `port/z6s/autostart/libwolf_autostart.c:54` | `register long r7 __asm__("r7") = number;` |
| critical | abi | inline-arm-syscall | `port/z6s/autostart/libwolf_autostart.c:55` | `register long r0 __asm__("r0") = a0;` |
| critical | abi | inline-arm-syscall | `port/z6s/autostart/libwolf_autostart.c:56` | `__asm__ volatile("svc 0" : "+r"(r0) : "r"(r7) : "memory");` |
| critical | abi | inline-arm-syscall | `port/z6s/autostart/libwolf_autostart.c:62` | `register long r7 __asm__("r7") = number;` |
| critical | abi | inline-arm-syscall | `port/z6s/autostart/libwolf_autostart.c:63` | `register long r0 __asm__("r0") = a0;` |
| critical | abi | inline-arm-syscall | `port/z6s/autostart/libwolf_autostart.c:64` | `register long r1 __asm__("r1") = a1;` |
| critical | abi | inline-arm-syscall | `port/z6s/autostart/libwolf_autostart.c:65` | `__asm__ volatile("svc 0" : "+r"(r0) : "r"(r1), "r"(r7) : "memory");` |
| critical | abi | inline-arm-syscall | `port/z6s/autostart/libwolf_autostart.c:71` | `register long r7 __asm__("r7") = number;` |
| critical | abi | inline-arm-syscall | `port/z6s/autostart/libwolf_autostart.c:72` | `register long r0 __asm__("r0") = a0;` |
| critical | abi | inline-arm-syscall | `port/z6s/autostart/libwolf_autostart.c:73` | `register long r1 __asm__("r1") = a1;` |
| critical | abi | inline-arm-syscall | `port/z6s/autostart/libwolf_autostart.c:74` | `register long r2 __asm__("r2") = a2;` |
| critical | abi | inline-arm-syscall | `port/z6s/autostart/libwolf_autostart.c:75` | `__asm__ volatile("svc 0" : "+r"(r0)` |
| medium | filesystem | mount-layout | `port/z6s/autostart/libwolf_autostart.c:89` | `static const char path[] = "/mnt/extsd/wolf3d/autostart.log";` |
| critical | abi | inline-arm-syscall | `port/z6s/autostart/libwolf_autostart.c:90` | `long fd = z6s_syscall3(SYS_open, (long) path,` |
| critical | abi | inline-arm-syscall | `port/z6s/autostart/libwolf_autostart.c:93` | `z6s_syscall3(SYS_write, fd, (long) text, (long) z6s_strlen(text));` |
| critical | abi | inline-arm-syscall | `port/z6s/autostart/libwolf_autostart.c:94` | `z6s_syscall3(SYS_write, fd, (long) "\n", 1);` |
| medium | filesystem | mount-layout | `port/z6s/autostart/libwolf_autostart.c:114` | `static char busybox[] = "/mnt/extsd/busybox";` |
| medium | filesystem | mount-layout | `port/z6s/autostart/libwolf_autostart.c:117` | `static char script[] = "/mnt/extsd/wolf3d/run_wolf3d.sh";` |
| medium | filesystem | mount-layout | `port/z6s/autostart/libwolf_autostart.c:119` | `"PATH=/bin:/sbin:/usr/bin:/usr/sbin:/mnt/extsd";` |
| medium | filesystem | mount-layout | `port/z6s/autostart/libwolf_autostart.c:120` | `static char env_home[] = "HOME=/mnt/extsd/wolf3d";` |
| critical | abi | inline-arm-syscall | `port/z6s/autostart/libwolf_autostart.c:127` | `long pid = z6s_syscall0(SYS_fork);` |
| critical | abi | inline-arm-syscall | `port/z6s/autostart/libwolf_autostart.c:138` | `z6s_syscall2(SYS_nanosleep, (long) &delay, 0);` |
| critical | abi | inline-arm-syscall | `port/z6s/autostart/libwolf_autostart.c:139` | `z6s_syscall3(SYS_execve, (long) busybox, (long) argv, (long) envp);` |
| medium | filesystem | mount-layout | `port/z6s/src/sdl_mixer_z6s.cpp:63` | `FILE *fp = fopen("/mnt/extsd/wolf3d/audio.log", "a");` |
| high | audio | direct-alsa-pcm | `port/z6s/src/sdl_mixer_z6s.cpp:360` | `g_pcm = pcm_open(0, 0, PCM_OUT, &config);` |
| high | hardware-boundary | absolute-kernel-path | `port/z6s/src/sdl_mixer_z6s.cpp:363` | `set_error("cannot open /dev/snd/pcmC0D0p: %s",` |
| high | audio | direct-alsa-pcm | `port/z6s/src/sdl_mixer_z6s.cpp:363` | `set_error("cannot open /dev/snd/pcmC0D0p: %s",` |
| high | audio | direct-alsa-pcm | `port/z6s/src/sdl_mixer_z6s.cpp:364` | `g_pcm ? pcm_get_error(g_pcm) : "pcm_open returned null");` |
| high | audio | direct-alsa-pcm | `port/z6s/src/sdl_mixer_z6s.cpp:375` | `if(pcm_prepare(g_pcm) < 0)` |
| high | hardware-boundary | absolute-kernel-path | `port/z6s/src/sdl_mixer_z6s.cpp:394` | `audio_log("ALSA opened /dev/snd/pcmC0D0p rate=%d channels=2 period_frames=%d",` |
| high | audio | direct-alsa-pcm | `port/z6s/src/sdl_mixer_z6s.cpp:394` | `audio_log("ALSA opened /dev/snd/pcmC0D0p rate=%d channels=2 period_frames=%d",` |
| high | audio | direct-alsa-pcm | `port/z6s/src/sdl_mixer_z6s.cpp:466` | `int written = pcm_writei(g_pcm, g_period, (unsigned int) g_period_frames);` |
| high | audio | direct-alsa-pcm | `port/z6s/src/sdl_mixer_z6s.cpp:467` | `if(written < 0 && !g_write_error_logged && pcm_prepare(g_pcm) == 0)` |
| high | audio | direct-alsa-pcm | `port/z6s/src/sdl_mixer_z6s.cpp:468` | `written = pcm_writei(g_pcm, g_period,` |
| high | audio | direct-alsa-pcm | `port/z6s/src/sdl_mixer_z6s.cpp:475` | `audio_log("pcm_writei failed after recovery: %s; disabling audio",` |
| high | input | direct-linux-input | `port/z6s/src/z6s_input.cpp:4` | `#include <linux/input.h>` |
| medium | filesystem | mount-layout | `port/z6s/src/z6s_input.cpp:65` | `FILE *fp = fopen("/mnt/extsd/wolf3d/input.log", "a");` |
| high | hardware-boundary | absolute-kernel-path | `port/z6s/src/z6s_input.cpp:91` | `snprintf(path, sizeof(path), "/dev/input/event%d", n);` |
| high | input | direct-linux-input | `port/z6s/src/z6s_input.cpp:91` | `snprintf(path, sizeof(path), "/dev/input/event%d", n);` |
| high | input | direct-linux-input | `port/z6s/src/z6s_input.cpp:103` | `ioctl(fd, EVIOCGBIT(0, sizeof(evbits)), evbits);` |
| high | input | direct-linux-input | `port/z6s/src/z6s_input.cpp:105` | `ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(keybits)), keybits);` |
| high | input | direct-linux-input | `port/z6s/src/z6s_input.cpp:107` | `ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(absbits)), absbits);` |
| high | input | direct-linux-input | `port/z6s/src/z6s_input.cpp:114` | `(bit_is_set(ABS_MT_POSITION_X, absbits) \|\|` |
| low | configuration | environment-configuration | `port/z6s/src/z6s_input.cpp:118` | `const char *touchEnabled = getenv("Z6S_TOUCH_INPUT");` |
| high | input | direct-linux-input | `port/z6s/src/z6s_input.cpp:142` | `d.xcode = bit_is_set(ABS_MT_POSITION_X, absbits) ?` |
| high | input | direct-linux-input | `port/z6s/src/z6s_input.cpp:143` | `ABS_MT_POSITION_X : ABS_X;` |
| high | input | direct-linux-input | `port/z6s/src/z6s_input.cpp:144` | `d.ycode = bit_is_set(ABS_MT_POSITION_Y, absbits) ?` |
| high | input | direct-linux-input | `port/z6s/src/z6s_input.cpp:145` | `ABS_MT_POSITION_Y : ABS_Y;` |
| high | input | direct-linux-input | `port/z6s/src/z6s_input.cpp:146` | `if(ioctl(fd, EVIOCGABS(d.xcode), &d.xi) < 0)` |
| high | input | direct-linux-input | `port/z6s/src/z6s_input.cpp:150` | `if(ioctl(fd, EVIOCGABS(d.ycode), &d.yi) < 0)` |
| low | configuration | environment-configuration | `port/z6s/src/z6s_input.cpp:264` | `if(getenv("Z6S_TOUCH_SWAP_XY")) { int t = x; x = y; y = t; }` |
| low | configuration | environment-configuration | `port/z6s/src/z6s_input.cpp:265` | `if(getenv("Z6S_TOUCH_INVERT_X")) x = 319 - x;` |
| low | configuration | environment-configuration | `port/z6s/src/z6s_input.cpp:266` | `if(getenv("Z6S_TOUCH_INVERT_Y")) y = 199 - y;` |
| high | hardware-boundary | absolute-kernel-path | `scripts/build_z6s.sh:10` | `JOBS="${JOBS:-$(nproc 2>/dev/null \|\| echo 2)}"` |
| high | hardware-boundary | absolute-kernel-path | `scripts/build_z6s.sh:37` | `command -v "$tool" >/dev/null \|\| {` |
| medium | build | legacy-compiler-contract | `scripts/build_z6s.sh:43` | `ARCH_FLAGS="-march=armv5te -mtune=arm926ej-s -mfloat-abi=soft"` |
| high | hardware-boundary | absolute-kernel-path | `scripts/build_z6s.sh:68` | `[ "$(cat "$SDL_BUILD/.z6s-config-tag" 2>/dev/null \|\| true)" != "$SDL_CONFIG_TAG" ]; then` |
| medium | build | legacy-compiler-contract | `scripts/build_z6s.sh:140` | `"${CROSS}g++" $COMMON_FLAGS -std=gnu++98 -fno-exceptions \` |
| medium | build | legacy-compiler-contract | `scripts/build_z6s.sh:141` | `-fno-rtti "${INCLUDES[@]}" "${DEFINES[@]}" -c "$src" -o "$out"` |
| low | rendering | fixed-render-dimensions | `scripts/test_qemu.sh:39` | `--resf 320 200 --bits 32 --nodblbuf --joystick -1 \` |
| medium | filesystem | mount-layout | `sdcard/wolf3d/disable_autostart.sh:3` | `ROOT=/mnt/extsd/wolf3d` |
| medium | filesystem | mount-layout | `sdcard/wolf3d/disable_autostart.sh:4` | `BUSYBOX=/mnt/extsd/busybox` |
| medium | filesystem | mount-layout | `sdcard/wolf3d/disable_autostart.sh:7` | `"$BUSYBOX" cp "$ROOT/EasyUI.safe.cfg" /mnt/extsd/EasyUI.cfg` |
| high | hardware-boundary | absolute-kernel-path | `sdcard/wolf3d/disable_autostart.sh:8` | `"$BUSYBOX" killall wolf3d-z6s-wl1 2>/dev/null \|\| true` |
| high | hardware-boundary | absolute-kernel-path | `sdcard/wolf3d/disable_autostart.sh:9` | `"$BUSYBOX" killall wolf3d-z6s-wl6 2>/dev/null \|\| true` |
| high | hardware-boundary | absolute-kernel-path | `sdcard/wolf3d/disable_autostart.sh:10` | `for pid in $("$BUSYBOX" pidof zkgui 2>/dev/null); do` |
| high | hardware-boundary | absolute-kernel-path | `sdcard/wolf3d/disable_autostart.sh:11` | `"$BUSYBOX" kill -CONT "$pid" 2>/dev/null \|\| true` |
| high | runtime-integration | process-control | `sdcard/wolf3d/disable_autostart.sh:11` | `"$BUSYBOX" kill -CONT "$pid" 2>/dev/null \|\| true` |
| medium | filesystem | mount-layout | `sdcard/wolf3d/enable_autostart.sh:3` | `ROOT=/mnt/extsd/wolf3d` |
| medium | filesystem | mount-layout | `sdcard/wolf3d/enable_autostart.sh:4` | `BUSYBOX=/mnt/extsd/busybox` |
| medium | filesystem | mount-layout | `sdcard/wolf3d/enable_autostart.sh:7` | `"$BUSYBOX" cp "$ROOT/EasyUI.wolf3d_autostart.cfg" /mnt/extsd/EasyUI.cfg` |
| medium | filesystem | mount-layout | `sdcard/wolf3d/run_wolf3d.sh:4` | `ROOT=/mnt/extsd/wolf3d` |
| medium | filesystem | mount-layout | `sdcard/wolf3d/run_wolf3d.sh:5` | `BUSYBOX=/mnt/extsd/busybox` |
| high | hardware-boundary | absolute-kernel-path | `sdcard/wolf3d/run_wolf3d.sh:19` | `if ! "$BUSYBOX" pidof zkgui >/dev/null 2>&1; then` |
| high | hardware-boundary | absolute-kernel-path | `sdcard/wolf3d/run_wolf3d.sh:28` | `for pid in $("$BUSYBOX" pidof zkgui 2>/dev/null); do` |
| high | runtime-integration | process-control | `sdcard/wolf3d/run_wolf3d.sh:30` | `"$BUSYBOX" kill -STOP "$pid" >> "$LOG" 2>&1 \|\| true` |
| high | hardware-boundary | absolute-kernel-path | `sdcard/wolf3d/run_wolf3d.sh:36` | `for pid in $("$BUSYBOX" pidof zkgui 2>/dev/null); do` |
| high | runtime-integration | process-control | `sdcard/wolf3d/run_wolf3d.sh:38` | `"$BUSYBOX" kill -CONT "$pid" >> "$LOG" 2>&1 \|\| true` |
| high | hardware-boundary | absolute-kernel-path | `sdcard/wolf3d/run_wolf3d.sh:45` | `for gov in /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor; do` |
| high | hardware-boundary | absolute-kernel-path | `sdcard/wolf3d/run_wolf3d.sh:47` | `old=$("$BUSYBOX" cat "$gov" 2>/dev/null \|\| echo unknown)` |
| high | hardware-boundary | absolute-kernel-path | `sdcard/wolf3d/run_wolf3d.sh:74` | `date >> "$LOG" 2>/dev/null \|\| true` |
| high | hardware-boundary | absolute-kernel-path | `sdcard/wolf3d/run_wolf3d.sh:102` | `"$BUSYBOX" chmod 666 /dev/fb0 /dev/input/event* >> "$LOG" 2>&1 \|\| true` |
| high | input | direct-linux-input | `sdcard/wolf3d/run_wolf3d.sh:102` | `"$BUSYBOX" chmod 666 /dev/fb0 /dev/input/event* >> "$LOG" 2>&1 \|\| true` |
| high | hardware-boundary | absolute-kernel-path | `sdcard/wolf3d/run_wolf3d.sh:103` | `FB_SIZE=$("$BUSYBOX" cat /sys/class/graphics/fb0/virtual_size 2>/dev/null \|\| echo unknown)` |
| high | hardware-boundary | absolute-kernel-path | `sdcard/wolf3d/run_wolf3d.sh:104` | `FB_BPP=$("$BUSYBOX" cat /sys/class/graphics/fb0/bits_per_pixel 2>/dev/null \|\| echo unknown)` |
| medium | rendering | sdl12-framebuffer-api | `sdcard/wolf3d/run_wolf3d.sh:109` | `export SDL_VIDEODRIVER=fbcon` |
| high | hardware-boundary | absolute-kernel-path | `sdcard/wolf3d/run_wolf3d.sh:110` | `export SDL_FBDEV=/dev/fb0` |
| low | configuration | environment-configuration | `sdcard/wolf3d/run_wolf3d.sh:115` | `export Z6S_FORCE_OPAQUE=1` |
| low | configuration | environment-configuration | `sdcard/wolf3d/run_wolf3d.sh:116` | `export Z6S_NATIVE_WIDTH=480` |
| low | rendering | fixed-render-dimensions | `sdcard/wolf3d/run_wolf3d.sh:116` | `export Z6S_NATIVE_WIDTH=480` |
| low | configuration | environment-configuration | `sdcard/wolf3d/run_wolf3d.sh:117` | `export Z6S_NATIVE_HEIGHT=272` |
| low | rendering | fixed-render-dimensions | `sdcard/wolf3d/run_wolf3d.sh:117` | `export Z6S_NATIVE_HEIGHT=272` |
| low | configuration | environment-configuration | `sdcard/wolf3d/run_wolf3d.sh:118` | `export Z6S_METRICS_LOG=$ROOT/performance.log` |
| low | configuration | environment-configuration | `sdcard/wolf3d/run_wolf3d.sh:119` | `export Z6S_FORCE_AUDIO=1` |
| high | hardware-boundary | absolute-kernel-path | `sdcard/wolf3d/run_wolf3d.sh:120` | `export Z6S_ALSA_PCM=/dev/snd/pcmC0D0p` |
| high | audio | direct-alsa-pcm | `sdcard/wolf3d/run_wolf3d.sh:120` | `export Z6S_ALSA_PCM=/dev/snd/pcmC0D0p` |
| low | configuration | environment-configuration | `sdcard/wolf3d/run_wolf3d.sh:120` | `export Z6S_ALSA_PCM=/dev/snd/pcmC0D0p` |
| low | configuration | environment-configuration | `sdcard/wolf3d/run_wolf3d.sh:121` | `export Z6S_TOUCH_INPUT=${Z6S_TOUCH_INPUT:-1}` |
| high | hardware-boundary | absolute-kernel-path | `sdcard/wolf3d/run_wolf3d.sh:124` | `if "$BUSYBOX" grep -q 'Handlers=.*kbd' /proc/bus/input/devices 2>/dev/null; then` |
| low | configuration | environment-configuration | `sdcard/wolf3d/run_wolf3d.sh:125` | `export Z6S_DEMO_MODE=0` |
| low | configuration | environment-configuration | `sdcard/wolf3d/run_wolf3d.sh:128` | `export Z6S_DEMO_MODE=1` |
| low | rendering | fixed-render-dimensions | `sdcard/wolf3d/run_wolf3d.sh:131` | `log "Starting Wolf3D $EDITION: 320x200 logical -> 480x272 native, 32 bpp, 11025 Hz"` |
| low | rendering | fixed-render-dimensions | `sdcard/wolf3d/run_wolf3d.sh:132` | `"$BIN" --resf 320 200 --bits 32 --samplerate 11025 \` |
| medium | filesystem | mount-layout | `sdcard/wolf3d/test_once.sh:3` | `ROOT=/mnt/extsd/wolf3d` |
| medium | filesystem | mount-layout | `sdcard/wolf3d/test_once.sh:4` | `BUSYBOX=/mnt/extsd/busybox` |
| medium | filesystem | mount-layout | `sdcard/wolf3d/test_once.sh:9` | `echo "One-shot test complete. See /mnt/extsd/wolf3d/wolf3d.log."` |
