# Benchmarks

## Measurement policy

RetroPort AI records only reproducible observations. A missing physical result
is `null`, not an estimate. Every tracked benchmark names its capture date,
revision, tool, and interpretation.

The baseline is [baseline-2026-07-18.json](benchmarks/baseline-2026-07-18.json).
The instrumented build is
[post-telemetry-2026-07-18.json](benchmarks/post-telemetry-2026-07-18.json).

## Binary comparison

| Profile | Metric | Baseline | Instrumented | Delta |
|---|---:|---:|---:|---:|
| WL1 | file size | 1,000,384 B | 1,000,384 B | 0 B |
| WL1 | `.text` | 734,604 B | 735,628 B | +1,024 B |
| WL1 | `.rodata` | 161,466 B | 161,650 B | +184 B |
| WL1 | `.bss` | 275,952 B | 275,968 B | +16 B |
| WL6 | file size | 1,000,632 B | 1,000,632 B | 0 B |
| WL6 | `.text` | 735,148 B | 736,156 B | +1,008 B |
| WL6 | `.rodata` | 161,482 B | 161,666 B | +184 B |
| WL6 | `.bss` | 275,856 B | 275,872 B | +16 B |

The filesystem size remains unchanged because the additional sections fit the
existing stripped ELF layout. The meaningful cost is roughly 1.2 KiB of loaded
code/read-only data plus 16 bytes of BSS per profile.

## Verification evidence

| Check | Result | What it proves |
|---|---|---|
| WL1 cross-build | Pass | ARMv5 soft-float profile links and passes ABI checks |
| WL6 cross-build | Pass | Registered-data profile links and passes ABI checks |
| Patch reconstruction | Pass | Four public patch layers reproduce local modifications |
| Unit tests | 5/5 pass | Scanner, release checks, comparison, and log parsing |
| QEMU WL1 no-keyboard | 20 s pass | Startup, data load, and demo path remain alive |
| QEMU WL6 no-keyboard | 20 s pass | Startup, data load, and demo path remain alive |

QEMU reports the intended logical/native path:

```text
Z6S video scaler: 320x200 logical -> 480x272x32 native
```

Audio failure under QEMU is expected because the test host does not provide the
target `/dev/snd` node; the game degrades to silent operation and continues.

## Physical metrics still required

| Metric | Status | Capture method |
|---|---|---|
| first present | Not yet captured after instrumentation | first `sdl_first_present` record |
| presentation rate | Not yet captured | at least three five-second windows |
| resident memory | Not yet captured | `maxrss_kb` from the same windows |
| dropped/late frames | Not implemented | external video or a defined deadline counter |
| touch latency | Not measured | synchronized input/video capture |
| audio stability | Not measured | device log plus audible test |

`present_fps` in the generated report means completed SDL presentation calls
per elapsed second. It is a useful regression signal, but it must not be labeled
GPU FPS or end-to-end display refresh.

## Reproducing the table

Build both profiles, run `tools/retroport.py benchmark`, and compare the JSON.
Do not replace tracked benchmark files with a different toolchain result unless
the host, revision, and reason are recorded in the commit.
