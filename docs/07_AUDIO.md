# 7. Audio and speaker safety

## Runtime architecture

Full SDL_mixer is unnecessary for this target, and the legacy SDL audio thread
was unreliable on the recovered firmware. The Z6S adapter implements only the
API used by `id_sd.cpp`:

- PCM S16 WAV loading;
- eight effect channels with panning;
- the AdLib/OPL music callback;
- groups and reserved channels;
- saturated 16-bit mixing.

`Mix_Z6S_Pump()` generates one period from the main loop. tinyalsa opens:

```text
/dev/snd/pcmC0D0p
11,025 Hz
stereo signed 16-bit little-endian
128 frames per period
4 periods
```

The runtime buffer is 512 bytes. This keeps latency, memory, and CPU demand low.
If the PCM is absent, gameplay continues silently.

The first device test showed a PCM left in `SETUP`: open succeeded, but the
first `WRITEI` returned `EBADFD`. The backend now calls `pcm_prepare()` after
open and allows one preparation retry after a write failure. A second failure
disables audio for the session, preventing repeated log I/O from harming video.

Log: `/mnt/extsd/wolf3d/audio.log`.

## Physical speaker safety

Use a passive speaker between `SPK+` and `SPK-` only after confirming the load
specified for the board or amplifier. Do not connect either pin to ground. Do
not attach headphones, AUX input, or a ground-referenced external amplifier;
`SPK+`/`SPK-` often indicates a bridged output that can be damaged by grounding
one side.

Because impedance and power are not established by current evidence, start with
low software volume and a brief test. If reliable markings are unavailable,
identify or measure the amplifier first.

## Test sequence

1. Boot without a speaker and inspect `audio.log`.
2. Confirm an `ALSA opened` line with 11,025 Hz and 128 period frames.
3. Power off completely.
4. Connect the verified passive speaker across `SPK+`/`SPK-`.
5. Power on and test a short effect before continuous music.

Persistent `pcm_writei` failures are logged once. Clicking or broken music may
indicate underruns; change buffer size or sample rate only after recording CPU
load because those changes trade latency and work.
