## What changed

<!-- Describe the bounded change. -->

## Why

<!-- Link the observed problem and evidence. -->

## Verification

- [ ] `python -m unittest -v`
- [ ] `python tools/retroport.py --root . check-release`
- [ ] Patch reconstruction, cross-build, and QEMU checks when applicable
- [ ] Physical-device evidence labeled separately from host/QEMU evidence

## Release safety

- [ ] No game data, vendor binaries, firmware, generated builds, credentials,
      device identifiers, or personal absolute paths are included.
- [ ] Public prose and code comments are in English.
- [ ] AI-assisted suggestions were reviewed and approved by a human.
