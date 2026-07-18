# Contributing

Contributions are welcome when they preserve the project's evidence-first and
legal-release boundaries.

Unless a file is necessarily derived from an upstream component, contributions
to original WOLF3D lab material are accepted under CC0-1.0. By contributing,
you confirm that you have the right to make that dedication. Upstream-derived
changes must retain the upstream license identified in `LICENSE`.

## Before opening a change

- Do not add game data, ROMs, WADs, recovered firmware, vendor assets, compiled
  binaries, credentials, device identifiers, or personal absolute paths.
- Keep upstream revisions pinned and adaptations as reviewable patches.
- Label physical observations separately from QEMU or host results.
- Do not convert unknown metrics into estimates.
- Keep public documentation and code comments in English.

## Development loop

```bash
python3 -m unittest -v
python3 -m py_compile tools/retroport.py
bash scripts/verify_patches.sh
python3 tools/retroport.py check-release
```

Run both cross-build profiles and `scripts/test_qemu.sh` when changing patches,
platform adapters, launcher behavior, or build scripts.

## Patch style

Keep device-specific code behind a narrow target boundary. Explain the observed
failure, the target contract, the proposed mechanism, and the verification
method. Prefer integer arithmetic and bounded logging on the frame path. Avoid
new threads or dependencies unless measurements justify them.

## AI-assisted contributions

AI output must be reviewable like any other contribution. Record:

- the evidence given to the model;
- the model and interface actually used;
- accepted, rejected, and modified suggestions;
- human approval before code changes;
- tests and measurements after the change.

Never paste confidential or copyrighted inputs into a public issue or report.

## Commit and review expectations

Use small imperative commits and keep generated/private files unstaged. A pull
request should state what changed, why, user impact, checks run, and remaining
hardware unknowns. Run the release guard against the Git index immediately
before pushing.
