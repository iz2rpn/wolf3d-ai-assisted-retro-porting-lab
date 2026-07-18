# AI development log

This log separates pre-existing work from the new Build Week contribution and
records a human-led collaboration. [@iz2rpn](https://github.com/iz2rpn), the
device owner and project author, originated the project,
preserved and inspected the hardware, supplied lawful test inputs, set product
requirements, made final technical decisions, performed the physical tests, and
approved or rejected changes. Codex helped as an engineering ally with audit,
hypothesis generation, bounded implementation, verification automation, and
documentation. This is not a transcript and does not claim an unverified model
label.

## Provenance rule

Codex was used throughout the reconstruction. The Build Week MVP adds a
repository-local workflow explicitly designed to run with GPT-5.6. For the
submission, the author must select GPT-5.6 in the primary Codex task, run the
skill on the public repository, and record the `/feedback` session ID. That
required human action is not inferred or fabricated here.

## Case study 1: reconstructing the target contract

**Human contribution.** The owner recognized the value of the abandoned LCD,
preserved the SD, provided access to the device, supplied the legally owned game
copy, and defined the goal of rebuilding the complete game as a learnable port.

**Problem.** The original project was lost; only an SD image, device evidence,
and a running embedded display remained.

**Initial state.** Source provenance, ABI, boot chain, and the boundary between
engine code and commercial data were unclear.

**AI-assisted analysis.** Codex organized the recovered evidence, identified
ARM926EJ-S/ARMv5TE and EABI5 soft-float constraints, separated the vendor proxy
ABI from the static game ABI, and mapped the boot, video, input, and audio seams.

**Human decision.** The owner chose a focused Wolf3D port instead of a general
RetroPie image because the device resources are minimal and the result is more
distinctive.

**Applied change.** Exact upstream revisions, reproducible patch layers, build
scripts, and reversible SD launch behavior were created.

**Verification.** Cross-build ABI checks and QEMU startup tests were added. The
device owner retained commercial game data outside the public source boundary.

**Lesson.** An explicit hardware contract and legal asset boundary must precede
optimization.

## Case study 2: fixed-resolution black-screen diagnosis

**Problem.** The first reconstructed build opened as a 320x200 window and the
demo became black after startup.

**Initial state.** Logo/menu and 3D rendering did not share a reliable physical
presentation contract. Framebuffer dumps contained plausible RGB values.

**AI-assisted analysis.** Codex compared logical and physical modes and
hypothesized that the vendor's high pixel byte acts as alpha. It proposed one
fixed 480x272x32 scanout, a 320x200 logical renderer, precomputed scaling maps,
and an opaque alpha byte on every presented pixel.

**Human review and debugging.** The owner installed iterations on the SD,
observed the incorrect 320x200 presentation and black demo, reported those
specific failures, then tested the correction on the physical LCD and confirmed
correct, smooth operation. This is qualitative validation, not a numerical FPS
measurement.

**Applied change.** Logo, menu, fades, demo, and gameplay now use the same
`VL_Z6SPresent` path; SDL mode probing and fbcon console ownership are disabled
for this firmware.

**Verification.** Patch reconstruction passes, both profiles build, both remain
alive for 20 seconds under QEMU, and the owner validated the LCD result.

**Lesson.** A framebuffer can contain valid RGB while a vendor scanout remains
black; pixel-format and presentation-path evidence matter more than a screenshot
of memory.

## Case study 3: input, demo fallback, and audio

**Problem.** Touch alone is awkward for Wolf3D, USB keyboard host capability is
uncertain, and the recovered PCM device initially entered an unusable state.

**AI-assisted analysis.** Codex proposed capability-based evdev discovery,
hot-plug scanning, a simple touch grid, and a no-keyboard demo policy. For audio
it proposed a bounded main-loop mixer and explicit `pcm_prepare` recovery.

**Human decision.** Keyboard remains the preferred control, touch remains
optional, and no-keyboard startup must show the original logo and menu before
starting a demo.

**Applied change.** The launcher selects interactive or demo mode from keyboard
evidence. The engine rescans input devices. Audio runs without a background
thread and disables itself after a controlled retry instead of stalling video.

**Verification.** QEMU exercises the no-keyboard path. USB host behavior,
speaker limits, and quantitative audio stability remain device-specific checks.

**Lesson.** Graceful degradation is a feature on constrained hardware, but it
must remain observable in logs.

## Case study 4: Build Week productization

**Problem.** The recovered port worked, but it was not yet a reusable developer
tool, a safe public repository, or an auditable AI collaboration story.

**Initial state.** The tree contained 2,184 files, including upstream checkouts,
generated output, private WL6 data, vendor UI resources, a BusyBox binary, local
paths, and Italian-only public documentation. Physical performance claims were
not instrumented.

**AI-assisted analysis.** Codex performed a full inventory before refactoring,
measured the existing binaries, calculated upstream patch deltas, identified
license risks, and ranked candidate MVPs. The highest-value bounded slice was an
evidence-first scanner plus a GPT-5.6 Codex workflow applied to the real Z6S
video path.

**Human decision.** The owner defined the Build Week direction, requested an
educational and competitive public project, approved English-only public
documentation, insisted that only legal material be released, produced the
physical YouTube proof, and corrected the narrative to reflect human ownership.

**Applied change.** The project gained `retroport.toml`, the deterministic CLI,
five tests, a release guard, a repository-local Codex skill, fixed legal
boundaries, English documentation, device telemetry, and submission materials.

**Verification.** Unit tests, Python compilation, patch reconstruction,
cross-builds, QEMU smoke tests, and a tracked-file release scan are required
before publication.

**Measured result.** Telemetry adds 1,024 bytes of `.text` to WL1 and 1,008
bytes to WL6, 184 bytes of `.rodata`, and 16 bytes of BSS per profile. The
stripped file sizes remain unchanged. Device FPS and memory remain unknown until
the new log is captured.

**Lesson.** The strongest AI story is a capable human using AI as a traceable
engineering partner around a real artifact—not a prompt replacing ownership,
judgment, or hands-on work.

## Division of work

| Human developer / device owner | Codex and GPT-5.6 as an ally |
|---|---|
| Chose the product and rejected the over-broad RetroPie direction | Helped compare scope, resource constraints, and demo value |
| Preserved the SD and supplied the lawful original data | Helped inventory the recovered tree and define the public boundary |
| Set requirements for logo, menu, demo, keyboard, touch, audio, and full-screen behavior | Helped map those requirements to bounded platform seams and patches |
| Performed every physical LCD iteration and reported concrete failures | Helped form hypotheses and automate build/QEMU verification |
| Accepted/rejected suggestions and owned the final trade-offs | Structured evidence, alternatives, tests, and measurement semantics |
| Recorded and published the real hardware video | Helped prepare the concise README, demo plan, and submission draft |
| Owns licensing decisions, publication, and final submission | Provides a release guard and review checklist; does not make legal decisions |

## Suggestion register

| Suggestion | Decision | Reason |
|---|---|---|
| Rebuild as a general RetroPie appliance | Rejected | Too broad for target resources and schedule |
| Use Wolf4SDL as the portable base | Accepted | Isolates platform seams while preserving gameplay |
| Use one fixed physical video mode | Accepted | Removes mode transitions and black-screen ambiguity |
| Keep keyboard and add touch fallback | Accepted | Supports real play and kiosk/demo use |
| Start a demo when no keyboard is present | Accepted | Produces a self-running device demo |
| Add continuous per-frame logging | Rejected | SD I/O and formatting would distort the hot path |
| Add five-second integer telemetry windows | Accepted | Low overhead and reviewable semantics |
| Auto-apply AI patches without review | Rejected | Conflicts with evidence-first, human-gated design |
| Claim physical FPS from QEMU | Rejected | QEMU does not model the target framebuffer timing |
| Publish recovered commercial/vendor files | Rejected | Copyright and redistribution risk |
| Build a hosted dashboard | Deferred | Not required for the high-value MVP |

## Reproducing the GPT-5.6 interaction

1. Open the repository in Codex and select GPT-5.6.
2. Run `python3 tools/retroport.py analyze` and keep the generated report.
3. Invoke `$retroport-ai` with one bounded subsystem question.
4. Require facts, inferences, proposals, tests, and unknowns to remain labeled.
5. Approve or reject the proposed change explicitly.
6. Run the conventional verification commands.
7. Capture the Codex task, commit, outputs, and `/feedback` session ID for the
   submission evidence.
