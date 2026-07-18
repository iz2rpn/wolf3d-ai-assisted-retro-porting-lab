---
name: retroport-ai
description: Evidence-driven workflow for analyzing, porting, and modernizing legacy software with Codex and GPT-5.6. Use when Codex must map an unfamiliar C/C++ or embedded codebase, identify non-portable APIs and hardware boundaries, compare upstream code with a port, rank migration work, propose a reviewable patch, collect reproducible binary facts, or produce a cited modernization report without inventing measurements.
---

# RetroPort AI

## Overview

Combine deterministic repository evidence with GPT-5.6 synthesis. Keep facts,
inferences, proposals, human decisions, and measured results visibly separate.

## Preconditions

- Work from the repository root containing `retroport.toml` and
  `tools/retroport.py`.
- Use GPT-5.6 for the reasoning pass. If the active model label is not visible,
  mark model provenance as unverified and ask the user to confirm it before a
  submission claim. Never infer the model from prose in the repository.
- Read [report-schema.md](references/report-schema.md) before interpreting a
  generated report.
- Treat commercial data, firmware, credentials, recovered binaries, and local
  user paths as private even when they are useful for a hardware test.

## 1. Collect evidence

Run the scanner before proposing architectural changes:

```bash
python tools/retroport.py --root . analyze \
  --json reports/generated/retroport-report.json \
  --markdown reports/generated/retroport-report.md
```

Run the release guard separately. A failure is evidence to address, not a
reason to discard the analysis:

```bash
python tools/retroport.py --root . check-release \
  --json reports/generated/release-check.json
```

Read `retroport.toml`, the JSON report, the build entry point, and only the
source files needed to validate material findings. Do not scan ignored private
assets manually unless the user explicitly places them in scope.

## 2. Build the technical model

Produce these sections:

1. **Observed architecture** — entry points, data/control flow, dependencies,
   build, runtime boundaries, and target constraints.
2. **Evidence table** — fact, `path:line`, confidence, and why it matters.
3. **Inferences** — conclusions derived from multiple facts, explicitly
   labelled as inference.
4. **Unknowns** — missing hardware measurements, ambiguous licenses, or
   behavior that requires a target test.
5. **Ranked migration plan** — impact, effort, risk, demonstrability,
   dependency, and completion criterion.

Do not call every scanner match a bug. Hardware bindings and old APIs can be
intentional constraints. Deduplicate repeated evidence before prioritizing it.

## 3. Propose one bounded experiment

Choose the smallest change that can prove or disprove a material hypothesis.
Before editing, show:

- the problem and cited evidence;
- the proposed files and behavioral effect;
- the test command;
- the expected measurement, or `not measurable in this environment`;
- rollback/recovery considerations;
- license or asset implications.

Wait for human approval unless the user already explicitly asked to implement
that exact bounded change. Record accepted, modified, and rejected proposals.

## 4. Implement without hiding the boundary

- Edit project-owned adapters and tools directly.
- Represent upstream changes as small patch files. Update the source-fetch
  script and verify each patch against a clean pinned checkout.
- Avoid broad formatting, line-ending churn, or unrelated cleanup.
- Centralize target constants only when it reduces a demonstrated duplication
  or configuration defect.
- Add comments for hardware contracts and non-obvious recovery behavior, not
  line-by-line narration.
- Never add game data, firmware assets, recovered vendor libraries, BusyBox
  binaries, secrets, generated executables, or private absolute paths to Git.

## 5. Verify and measure

Run the fast verification commands declared in `retroport.toml`. Run target
builds and QEMU checks when the toolchain is available. Never translate a QEMU
smoke timeout into a physical FPS claim.

Capture binary facts with:

```bash
python tools/retroport.py --root . benchmark \
  --binary wl1=dist/z6s/wl1/wolf3d-z6s \
  --binary wl6=dist/z6s/wl6/wolf3d-z6s \
  --output benchmarks/runs/current.json
```

When physical metrics are unavailable, preserve `null` and its reason. After
the target is tested, record the command/method, run count, raw samples,
summary, and variability before claiming an improvement.

## 6. Report the AI/human boundary

End with:

- **GPT-5.6 analysis:** synthesis and proposals produced from cited evidence;
- **Codex actions:** files inspected/edited and commands executed;
- **Human decisions:** accepted, changed, or rejected proposals;
- **Verification:** exact commands, results, and untested areas;
- **Measured result:** before/after values or an explicit statement that no
  comparable measurement exists;
- **Next experiment:** the highest-value remaining uncertainty.

Use this sentence when it remains accurate:

> AI did not replace the developer. It accelerated analysis, experimentation,
> and learning while the developer retained control over architecture,
> validation, and final decisions.

