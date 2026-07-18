# RetroPort AI — Project Plan

Status date: 2026-07-18  
Target event: OpenAI Build Week, Developer Tools track  
Submission deadline: 2026-07-21 at 5:00 PM PT  
Product status: working prototype built around one real case study

## Executive decision

RetroPort AI will be presented as an evidence-driven, AI-assisted workflow for
understanding, porting, and modernizing legacy software. Wolfenstein 3D on the
NOVA3D Z6S is the first case study, not the product itself.

The competitive MVP is deliberately narrow:

1. a deterministic scanner collects repository, portability, dependency,
   hardware-boundary, patch, and release-safety evidence;
2. a repository-local Codex skill guides GPT-5.6 through that evidence;
3. GPT-5.6 produces a cited assessment and migration plan, and may propose a
   small patch;
4. a human reviews the design and authorizes changes;
5. deterministic build, smoke-test, and benchmark commands verify the result;
6. the Wolf3D case study shows the workflow on real legacy C/C++ and constrained
   ARM hardware.

This is not a generic chatbot. Deterministic tools own facts and measurements;
GPT-5.6 owns synthesis and proposed changes; the developer owns architecture,
approval, hardware validation, and release decisions.

## Repository audit

### Current state

- The working tree contains 2,184 files and is not yet a root Git repository.
- `third_party/` contains 1,682 files from four pinned upstream repositories.
- `sdcard/` contains 401 deployment files, including private game data,
  firmware resources, a vendor library, BusyBox, and generated binaries.
- `WOLF3D/` contains the owner's commercial WL6 data and DOS executable.
- `build/` and `dist/` contain generated objects and ARM binaries.
- Public-facing documentation is currently Italian-first, with at least 169
  Italian-language matches in the intended public source and documentation.
- The requested GitHub governance, architecture, benchmark, AI-development,
  roadmap, contribution, and Devpost documents do not yet exist.

The current local package works as a hardware deployment workspace, but it is
not safe to publish as-is.

### Entry points and runtime flow

| Layer | Entry point | Responsibility |
|---|---|---|
| Vendor integration | `onEasyUIInit()` in `port/z6s/autostart/libwolf_autostart.c` | Proxies the vendor EasyUI hook and starts the launcher without linking glibc into the musl host process. |
| Device launcher | `sdcard/wolf3d/run_wolf3d.sh` | Selects WL1/WL6, suspends the vendor HMI, fixes CPU policy, configures framebuffer/input/audio, starts the game, and restores device state. |
| Game | `main()` → `InitGame()` → `DemoLoop()` in Wolf4SDL `wl_main.cpp` | Initializes subsystems and runs the menu/game/demo loop. |
| Frame presentation | `ThreeDRefresh()` → `VL_Z6SPresent()` | Scales the 320×200 indexed frame into one opaque 480×272×32 framebuffer frame. |
| Input | `IN_ProcessEvents()` → `Z6S_InputPump()` | Bridges Linux evdev keyboard/touch events into SDL 1.2 events and supports hot-plug. |
| Audio | `IN_ProcessEvents()` → `Mix_Z6S_Pump()` | Mixes eight channels synchronously and writes 11,025 Hz stereo PCM through tinyalsa. |

### Build and dependency graph

`scripts/fetch_sources.sh` pins and patches:

| Dependency | Pinned commit | Role | Local change |
|---|---|---|---|
| Wolf4SDL | `3d41ccce8f8fecbed83aa9d8d42734c2c7e62374` | Modernized game engine | 8 files, 312 insertions and 1 deletion |
| SDL 1.2 | `457d4e55ffe1b6ad4c4fa4559dbda8360bf8253d` | Static fbcon video/timing/events | 10 inserted lines in fbcon console handling |
| tinyalsa | `e43025bbf702eb7dd8edd48c1eb50530c60f1de8` | Direct ALSA PCM access | 5 insertions and 1 deletion for the plugin-free build |
| id Wolf3D source | `05167784ef009d0d0daefe8d012b027f39dc8541` | Historical reference only | No semantic local change |

The cross-build uses GCC `arm-linux-gnueabi`, ARMv5TE, ARM926EJ-S tuning,
soft-float, `-Os`, section garbage collection, static SDL, and a static game
binary. The EasyUI proxy is a separate freestanding shared object with only
`dlopen` and `dlsym` as dynamic imports.

### Custom and modified behavior

- **Video:** fixed native 480×272×32 mode, precomputed nearest-neighbor maps,
  full-frame opaque conversion, and a single presentation path for logo, menu,
  fades, and 3D.
- **Input:** direct non-blocking evdev enumeration, keyboard translation,
  touch calibration, a 4×3 gameplay touch grid, direct menu selection, and
  two-second hot-plug scans.
- **Audio:** a minimal SDL_mixer-compatible layer, eight effect channels,
  main-loop pumping, tinyalsa PCM output, explicit `pcm_prepare()`, and one
  controlled recovery attempt.
- **Startup:** original logo, a five-second menu preview, automatic demo when
  no keyboard is present, keyboard hot-plug, and reversible vendor-HMI control.
- **Build:** pinned upstreams, four reproducible patches, two game-data
  profiles, static ARM verification, and QEMU smoke tests.

### Verified baseline facts

No unmeasured result is treated as a benchmark.

| Metric | WL1 | WL6 | Evidence status |
|---|---:|---:|---|
| File size | 1,000,384 bytes | 1,000,632 bytes | Measured locally |
| `.text` section | 734,604 bytes | 735,148 bytes | Measured with `arm-linux-gnueabi-size` |
| `.rodata` section | 161,466 bytes | 161,482 bytes | Measured with `arm-linux-gnueabi-size` |
| `.bss` section | 275,952 bytes | 275,856 bytes | Measured with `arm-linux-gnueabi-size` |
| ABI | ARM EABI5 soft-float, static | ARM EABI5 soft-float, static | Verified with `readelf` |
| QEMU no-keyboard smoke test | 20 seconds | 20 seconds | Passed by intentional timeout |
| Physical FPS/frame time | Not measured | Not measured | Requires device instrumentation |
| Physical RSS/startup/touch latency | Not measured | Not measured | Requires device instrumentation |

The first physical test confirmed launcher startup, Goodix touch discovery,
and ALSA PCM opening. It exposed partial-screen video, a black 3D demo, and an
unprepared PCM. The current build contains fixes for all three; final physical
performance remains to be measured.

### Technical debt and release blockers

1. **Public/private boundary — critical.** Commercial WL6 data, shareware data,
   firmware UI files, a vendor shared library, BusyBox, and generated binaries
   coexist with publishable sources.
2. **No root history — critical.** There is no root Git history to distinguish
   pre-existing work from Build Week work. The first public commit must be
   marked as a baseline import, followed by focused commits.
3. **No integrated GPT-5.6 feature — critical.** Codex accelerated the port,
   but the repository does not yet expose a judge-runnable GPT-5.6 workflow.
4. **Generated and mutated source trees — high.** The build normalizes CRLF and
   runs Autotools inside the SDL checkout. This makes hundreds of files appear
   modified even though only one SDL source file changes semantically.
5. **Missing automated baseline — high.** Existing measurements live in prose;
   there is no machine-readable report or regression comparison.
6. **Hard-coded target settings — medium.** Device paths, dimensions, PCM
   card/device, and log locations are spread between C/C++ and shell code.
7. **Release duplication — medium.** The SD workspace has a WL1 compatibility
   alias, duplicate proxy binaries, stale build objects, and 242 identical
   animation frames. These are deployment/recovery artifacts, not public
   source assets.
8. **Small lifetime leak — low.** Z6S scaler maps and its frame buffer are not
   released by `VL_Shutdown()`. The process is short-lived, so this is not a
   current runtime failure, but it is visible modernization debt.
9. **Limited test surface — medium.** QEMU validates parser/data-loop survival,
   not framebuffer scanout, real audio, USB host mode, FPS, or touch latency.

### License and asset assessment

This section is an engineering release gate, not legal advice.

| Component | Local evidence | Public-release decision |
|---|---|---|
| Wolf4SDL / Wolf3D-derived engine | Wolf4SDL offers the original source under its included GPL v2 text or the historical id limited-use license | Use and document the GPL path for the port; preserve notices and publish corresponding source/patches with any binary release. |
| SDL 1.2 | Included LGPL 2.1 license | Preserve license and source offer; static-link obligations require special care. |
| tinyalsa | BSD-style notice | Preserve copyright and disclaimer. |
| OPL emulator | Wolf4SDL includes separate MAME/GPL alternatives | Document the selected implementation and its notice; do not imply one license covers every third-party file. |
| BusyBox 1.21.1 binary | Reports GPLv2 at runtime; no matching source bundle is present locally | Exclude the binary from Git. Require users to recover it from their own device or provide a compliant source/build route before redistributing it. |
| `libzkgui.so` and firmware UI | Vendor binary/resources; no redistributable license was found | Exclude from Git and all public releases. |
| WL6 data and `WOLF3D.EXE` | Owner-provided commercial copy | Exclude from Git, screenshots that expose files, archives, and releases. Explain how users provide their own legally obtained data. |
| WL1 data | Copyrighted game data even when distributed as shareware | Exclude by default; link to a lawful acquisition path rather than bundling it. |
| RetroPort AI tooling and original Z6S glue | Original project work, subject to linked/derived-code obligations | Add an explicit root license and third-party notices before publication. |

## Competitive MVP

### User journey

1. A developer opens an unfamiliar legacy C/C++ repository in Codex with
   GPT-5.6 selected.
2. They invoke the repository-local `retroport-ai` skill.
3. The skill runs a deterministic scanner and emits JSON plus Markdown evidence
   with file/line references.
4. GPT-5.6 classifies portability risks, explains architecture, ranks work, and
   writes a migration plan constrained by the target profile.
5. The developer approves one small patch.
6. Codex applies it and runs the declared verification commands.
7. A benchmark command records only measured values and compares them with the
   baseline.
8. The Wolf3D case study demonstrates the same workflow producing a real ARM
   binary and a working embedded result.

### MVP components

- `retroport.toml`: machine-readable target and policy profile.
- `tools/retroport.py`: standard-library CLI with `analyze`, `check-release`,
  and `benchmark` commands.
- `.codex/skills/retroport-ai/`: distributable Codex skill that tells GPT-5.6
  how to use evidence, cite findings, request human approval, and verify patches.
- `reports/`: generated baseline examples with no private paths or assets.
- `case-studies/wolf3d-z6s.md`: before/problem/analysis/decision/patch/test/result.
- GitHub and Devpost documentation, a three-minute demo plan, and an explicit
  AI-development record.

### Explicit non-goals for Build Week

- No hosted SaaS or generic chat UI.
- No autonomous patch application without review.
- No claim of compiler-grade C++ semantic analysis.
- No invented physical benchmark numbers.
- No redistribution of game, firmware, or recovered binaries.
- No attempt to support every operating system or build system in the MVP.

## Prioritized activities

Difficulty and risk use 1 = low and 10 = high. All other scores use 10 = best.

| ID | Activity | Priority | Visual impact | Innovation | Real utility | AI use | Technical quality | Ease | Demo ease | Originality | Risk | Submission ROI |
|---|---|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| A1 | Establish public/private boundary, `.gitignore`, release audit, and root history | P0 | 3 | 3 | 10 | 3 | 10 | 8 | 7 | 4 | 2 | 10 |
| A2 | Build deterministic RetroPort evidence scanner | P0 | 7 | 8 | 10 | 7 | 9 | 7 | 10 | 8 | 4 | 10 |
| A3 | Build repository-local Codex/GPT-5.6 `retroport-ai` skill | P0 | 8 | 9 | 10 | 10 | 9 | 7 | 10 | 9 | 4 | 10 |
| A4 | Generate machine-readable baseline and regression comparison | P0 | 8 | 7 | 9 | 6 | 10 | 7 | 9 | 7 | 3 | 10 |
| A5 | Create an evidence-backed Wolf3D case study and architecture diagrams | P0 | 10 | 8 | 9 | 9 | 9 | 8 | 10 | 9 | 2 | 10 |
| A6 | English-first GitHub documentation and licensing package | P0 | 8 | 4 | 10 | 6 | 10 | 6 | 9 | 5 | 3 | 10 |
| A7 | Add device-side metrics for frame count, startup, memory, and touch events | P1 | 9 | 7 | 10 | 5 | 9 | 5 | 8 | 8 | 6 | 8 |
| A8 | Centralize Z6S constants and fix bounded cleanup/configuration debt | P1 | 4 | 4 | 8 | 5 | 9 | 6 | 5 | 4 | 5 | 6 |
| A9 | Produce three-minute video script, shot list, captions, and live-demo fallback | P0 | 10 | 6 | 9 | 9 | 8 | 8 | 10 | 7 | 2 | 10 |
| A10 | Add hosted web UI or autonomous multi-repository service | P3 | 10 | 8 | 8 | 10 | 5 | 2 | 6 | 7 | 9 | 3 |

## Dependencies, completion criteria, and demo value

| ID | Depends on | Difficulty | Demo/submission value | Completion criterion |
|---|---|---:|---|---|
| A1 | None | 4 | Makes the repository legally reviewable and gives judges a clean test path. | A clean Git status contains no denied assets; release audit fails on a planted forbidden file; baseline import is explicitly labelled pre-existing. |
| A2 | A1 | 6 | Turns “AI helped me” into a visible product input with cited evidence. | CLI runs on this repository and a small fixture; JSON is schema-stable; Markdown findings cite real files/lines; tests pass without network access. |
| A3 | A2 | 6 | Satisfies the meaningful GPT-5.6/Codex requirement and is directly judge-runnable. | Skill passes `quick_validate.py`; a GPT-5.6 Codex run consumes the generated report and produces the required cited assessment, plan, and review gate. |
| A4 | A1, A2 | 5 | Provides a clear before/after screen and prevents invented metrics. | Baseline command records compiler/ABI/section/file/test facts; unavailable device metrics are `null` with reasons; comparison reports deltas. |
| A5 | A2, A4 | 4 | Supplies the emotionally strong “black screen to working hardware” story. | Case study links problem → evidence → decision → patch → verification and distinguishes measured from pending outcomes. |
| A6 | A1–A5 | 6 | Directly addresses README, licensing, testing, and judging criteria. | English is the primary public language; required files exist; links resolve; asset acquisition is lawful; claims match evidence. |
| A7 | A4 | 7 | Enables real performance numbers if the device is available before recording. | Device log produces parseable startup/frame/memory/input measurements; at least three runs are recorded with method and variability. |
| A8 | A4 | 6 | Shows disciplined modernization rather than broad cosmetic refactoring. | Each change has a concrete finding, patch, focused test, and no regression in both QEMU profiles. |
| A9 | A3–A6, optionally A7 | 4 | Converts technical depth into a coherent three-minute product story. | Full and short English scripts, timed scenes, on-screen text, commands, captions, and prerecorded fallback are complete; total is ≤180 seconds. |
| A10 | All P0 work | 9 | Visually attractive but unnecessary and risky before the deadline. | Deferred unless every P0 item is complete with time remaining. |

## Work sequence

### Phase 1 — Audit and plan

Complete when this document is checked against repository evidence and no code
refactor has occurred before it.

### Phase 2 — Release-safe baseline

Create the public/private boundary, root license/notices, `.gitignore`, Git
baseline, deterministic release audit, and machine-readable pre-change facts.

### Phase 3 — RetroPort AI MVP

Implement the scanner, target profile, fixtures, tests, and Codex skill. Keep
the model output downstream of deterministic evidence. Never let a model claim
a benchmark value that the collector did not measure.

### Phase 4 — Small high-ROI modernization

Use the new workflow on the Wolf3D case study. Apply only bounded changes that
have a documented reason and verification path. Candidate changes are scaler
resource cleanup, configuration parsing, clearer target constants, and
structured device metrics.

### Phase 5 — Verification

Run unit tests, release audit, source fetch/patch checks, both cross-builds,
ELF checks, and both QEMU smoke profiles. Run physical tests only when the
device/SD is available. Record missing measurements as missing.

### Phase 6 — Submission package

Rewrite public documentation in English, preserve optional Italian history
under `docs/it/`, prepare Devpost copy, produce the timed demo plan, and perform
an audit for private files, unsupported claims, broken links, and Italian text.

## Human decisions and external requirements

The developer must still:

- confirm the exact author name and copyright line for the root license;
- run the primary Codex thread with GPT-5.6 selected and retain evidence;
- run `/feedback` in the primary thread and copy the Session ID into Devpost;
- record/upload the public YouTube demo with required English audio;
- provide final physical-device footage and measurements;
- choose public GitHub or a private repository shared with the required judging
  addresses;
- review the official rules and submit before the deadline.

## Definition of done

RetroPort AI is ready for submission when:

- the public tree contains no commercial data, firmware assets, recovered
  vendor binaries, BusyBox binary, generated binaries, or private paths;
- a judge can run the deterministic scanner and the Codex skill without
  rebuilding Wolf3D;
- the GPT-5.6 result cites scanner evidence and clearly separates proposals
  from facts;
- the Wolf3D case study still builds and passes both QEMU smoke profiles;
- every benchmark value has a command, environment, and source artifact;
- English README, architecture, build, benchmark, AI log, roadmap, change log,
  contributing guide, license notices, Devpost copy, and demo script exist;
- the repository identifies pre-existing code and work completed during the
  Build Week submission period;
- the three-minute video shows a working project and specifically explains how
  Codex and GPT-5.6 were used;
- final release, language, link, and claim audits pass.
