# Build report — 2026-07-17

This historical report describes the pre-telemetry build. See
[../BENCHMARKS.md](../BENCHMARKS.md) for the current instrumented comparison.

## Outputs

| Profile | File size | SHA-256 |
|---|---:|---|
| WL1 game | 1,000,384 B | `d2131777479cd41dd9eee3daafabe7b5d47c63282c1ba00938fa4e198932ae03` |
| WL6 game | 1,000,632 B | `1488ccd1cd423fbe9359ab81630e52efd4fbc9f7b7af644d3f282fbb7d9d3931` |
| autostart proxy | 67,856 B | `0df2c594009e7f4be43cefc874a88325c68a5adc077dad716f340507dda1d5f3` |

## Completed verification

- WL1 and WL6 built with cross GCC 15.2.0.
- Both games were static ELF32 ARMv5 EABI5 soft-float executables.
- The proxy exported both EasyUI hooks and imported only `dlopen`/`dlsym`.
- Both `qemu-arm --help` paths passed.
- Both data profiles remained alive for 20 seconds under the intentional smoke
  test timeout.
- The fixed presenter reported `320x200 logical -> 480x272x32 native`.
- No-keyboard startup retained the original logo, displayed the menu, and then
  entered demo mode.
- The launcher selected WL6 only with a complete compatible eight-file set.
- Shell syntax and the four upstream patch layers were checked.

## Private deployment status

The private deployment card was FAT32 and did not require formatting. At that
time, 401 local package files totaling 26,457,710 bytes were copied and re-read
with zero missing files, size mismatches, or hash mismatches. Those counts
included user-owned game data and vendor/runtime material that is deliberately
excluded from the public repository.

The first physical test confirmed the launcher, Goodix touch, and PCM open. It
also revealed the non-full-screen image, black 3D path, and PCM preparation
issue that drove the corrected build. The device owner later confirmed that the
corrected version works smoothly on the LCD.

## Pending at the time of this report

- numerical physical presentation and resident-memory measurements;
- powered USB host/OTG keyboard enumeration;
- verified speaker electrical limits and audible test;
- explicit HMI recovery observation after `disable_autostart.sh`.
