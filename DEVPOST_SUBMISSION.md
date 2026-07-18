# Devpost submission draft

Fields marked **TODO** require the author's final link, capture, or event-specific
identifier. They are intentionally not fabricated.

## Project title

RetroPort AI

## Tagline

Evidence-first GPT-5.6 modernization for legacy software on constrained
hardware.

## Track

Developer Tools

## Elevator pitch

RetroPort AI is a human-built developer workflow that uses AI as an engineering
ally. It helps modernize undocumented legacy ports without confusing model
confidence with engineering evidence. A deterministic scanner maps
the target ABI, dependencies, platform seams, patch surface, binary footprint,
and release risks. A repository-local Codex skill then guides GPT-5.6 through a
human-approved reasoning loop: facts, inferences, proposals, tests, and measured
results remain distinct. The real case study reconstructs Wolf4SDL for an
ARMv5 NOVA3D Z6S display, fixes its full-screen presentation path, supports
keyboard/touch/demo operation, and adds low-overhead device telemetry while
keeping commercial data and vendor artifacts private.

## Inspiration

The project began when I lost the original embedded Wolf3D project but still had
the SD card, my legally owned game data, and the small LCD appliance. I preserved
the media, inspected the hardware, installed each iteration, and reported what
the real panel did. I chose to build a focused Wolf3D showcase instead of a
general RetroPie image, and I defined the logo/menu/demo, keyboard, touch, audio,
and recovery requirements. Rebuilding it exposed a
broader developer problem: old ports rarely fail because of one missing line of
code. They fail at the boundaries between undocumented hardware, obsolete build
systems, licensing, fragile patches, and unverified assumptions. We wanted an AI
workflow that makes those boundaries explicit and teaches the developer while
helping them act.

## What it does

- scans a legacy tree with configurable, line-level portability rules;
- extracts target constraints, dependency pins, patch statistics, and entry
  points into JSON/Markdown evidence;
- checks the Git index for commercial data, vendor binaries, credentials, local
  paths, and other release risks;
- measures ARM ELF footprint and parses low-rate physical-device telemetry;
- provides a GPT-5.6-ready Codex skill that keeps observations, hypotheses,
  proposed changes, human approval, and verification separate;
- applies the method to a reproducible Wolf4SDL/Z6S port with fixed-resolution
  video, evdev input, minimal ALSA audio, and keyboard-free demo startup.

## How we built it

I led the product and hardware work; Codex acted as a technical collaborator.
Together we first audited all 2,184 recovered files before editing. That inventory found
four upstream projects, generated output, commercial WL6 data, proprietary UI
resources, a redistributable-binary risk, personal paths, and eight meaningful
Wolf4SDL modification points hidden inside copied source trees. We recorded a
binary baseline and pinned each upstream revision.

The deterministic layer is a dependency-free Python CLI configured by
`retroport.toml`. The AI layer is a repository-local Codex skill with a strict
evidence schema and human gate. The conventional engineering layer remains
ordinary shell builds, patch reconstruction, unit tests, `readelf`, QEMU, and
physical-device logs. This makes the model useful where reasoning is valuable
without asking it to impersonate a compiler or measurement instrument.

I tested the iterations on the physical LCD, identified the non-full-screen and
black-demo failures, approved the fixed-mode approach, and published the working
hardware video. For the case study, the engine renders at 320x200 and presents through one
480x272x32 opaque scaler. Keyboard/touch events use capability-based evdev
discovery. With no keyboard, the original logo and menu appear before demo mode
starts. A tiny main-loop mixer avoids unreliable target threads. The new
telemetry records first-present time, five-second presentation windows, and peak
resident memory using integer arithmetic and no per-frame file I/O.

## Meaningful use of Codex and GPT-5.6

Codex is a collaborator inside the developer's workflow, not the project owner.
The developer invokes the `retroport-ai` skill, reviews its reasoning, and makes
the final decision. GPT-5.6 receives deterministic evidence rather than an
unbounded request to “port this game.” It is asked to:

1. cite concrete file/line evidence;
2. label facts, inferences, and unknowns;
3. rank portability risks and explain trade-offs;
4. propose one bounded patch plus verification criteria;
5. wait for human approval;
6. interpret the resulting test and benchmark delta.

This human/AI loop helped produce and document the Build Week MVP: the developer
set scope and acceptance criteria, while the AI accelerated the public asset
audit, scanner configuration, review workflow, and bounded telemetry change. It
is neither a decorative chatbot nor a claim that AI built the project alone.
**TODO before submission:**
run the public workflow in the primary Codex task with GPT-5.6 selected and add
the `/feedback` Session ID below.

Primary Codex `/feedback` Session ID: **TODO**

## Challenges

- Reconstructing a target split across a vendor musl process and a standalone
  static glibc game without modifying internal firmware.
- Diagnosing a black physical panel even when framebuffer memory contained
  correct RGB; the high byte needed to be explicitly opaque.
- Keeping every screen in one physical mode so logo-to-demo transitions did not
  expose 320x200 scanout or mode switches.
- Supporting ARMv5 soft-float with legacy SDL/autoconf under modern toolchains.
- Proving a public repository is safe when the local working tree must retain
  legally owned commercial data for testing.
- Reporting performance honestly when QEMU cannot model the target display.

## Accomplishments

- Human hardware work and AI-assisted software analysis became one reproducible,
  reviewable developer tool and case study.
- Both WL1 and WL6 profiles build as static ARM EABI5 soft-float executables.
- Patch reconstruction and five unit tests pass.
- Both profiles survive 20-second QEMU no-keyboard demo smoke tests.
- The device owner physically validated correct, smooth LCD operation.
- Instrumentation costs about 1.2 KiB of loaded code/read-only data and 16 bytes
  of BSS while leaving stripped file sizes unchanged.
- The public Git history excludes commercial game data, vendor assets, firmware,
  generated builds, and device logs.

## What we learned

AI-assisted legacy work becomes more reliable when the model reasons over a
versioned evidence artifact, proposes a small change, and is forced to state how
the claim will be disproved. Legal boundaries are also part of architecture:
they should be executable release rules, not a note added after the code works.
Finally, a qualitative physical success and a numerical benchmark are different
claims; both can be valuable when labeled correctly.

## Success metrics

| Metric | Result |
|---|---|
| deterministic tests | 5/5 pass |
| patch-stack reproduction | pass |
| cross-build profiles | 2/2 pass |
| QEMU no-keyboard smoke tests | 2/2 at 20 seconds |
| tracked commercial/vendor artifacts | 0 after release gate |
| telemetry footprint | +1,008 to +1,024 B `.text`, +184 B `.rodata`, +16 B BSS |
| physical LCD operation | owner-reported pass |
| physical presentation rate/RAM | **TODO: parse fresh device log** |

## Potential impact

Small industrial panels, kiosks, educational devices, abandoned appliances, and
archival software often remain useful long after their original toolchains
disappear. RetroPort AI offers maintainers a repeatable path from “mystery tree”
to a target contract, legal release boundary, bounded patch, and auditable
result. The same workflow can teach newcomers why a port works instead of only
handing them generated code.

## Limitations

- The current case study targets one Linux framebuffer appliance.
- The full cross-build requires a Linux/WSL toolchain and user-supplied legal
  game data.
- USB host/OTG and speaker electrical limits remain hardware-specific.
- Presentation-call rate is not equivalent to display refresh or GPU FPS.
- The model still requires human judgment, source-license review, tests, and
  physical validation.

## What's next

We will add a copyright-free fixture case, explicit late-frame measurement,
SPDX verification, more target-contract adapters, and an optional review UI.
The immediate next step is to capture the instrumented LCD log and publish a
second fully libre case study.

## Credits and acknowledgments

- id Software for releasing the historical Wolfenstein 3D source.
- The Wolf4SDL maintainers for the portable engine base.
- SDL 1.2 and tinyalsa contributors.
- [@iz2rpn](https://github.com/iz2rpn), project author and device owner, for the
  idea, requirements, legal media, hands-on debugging, physical validation, and
  final decisions.
- OpenAI Codex and GPT-5.6 as engineering allies for the evidence-guided audit,
  alternative analysis, automation, and documentation workflow.

All trademarks and game assets belong to their respective owners. No game data
or vendor firmware is included.

## Links

- Source repository: **TODO: public GitHub URL**
- Physical hardware proof (YouTube Short):
  [Wolf3D on the NOVA3D Z6S](https://www.youtube.com/shorts/TgmA7cbyw-s)
- Final narrated submission demo (public YouTube, no more than three minutes):
  **TODO**
- Project page: **TODO: Devpost URL**
- License: `GPL-2.0-only`; see `LICENSE` and `THIRD_PARTY_NOTICES.md`

## Final event checklist

- [ ] Public repository or approved private access configured.
- [ ] Primary Codex task uses GPT-5.6 and `/feedback` ID is recorded.
- [ ] Narrated public YouTube demo is no longer than three minutes.
- [ ] Video explicitly explains both Codex and GPT-5.6.
- [ ] Device log and benchmark table are refreshed.
- [ ] Release guard passes on the exact commit being submitted.
- [ ] Devpost links and credits contain no placeholders.
- [ ] Submission is completed before the official deadline.

Official references: [OpenAI Build Week](https://openai.com/build-week/),
[Devpost event page](https://openai.devpost.com/), and
[event FAQ](https://openai.devpost.com/details/faqs).
