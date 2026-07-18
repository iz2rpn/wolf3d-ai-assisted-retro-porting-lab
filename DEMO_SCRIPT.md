# Demo script

Target length: **2 minutes 50 seconds**. The final video must include audible
English narration and remain below the event's three-minute limit. Record the
physical LCD and Codex UI horizontally at 1080p when possible. Add burned-in
captions because the device audio may be quiet.

## Full submission cut

| Time | Visual | Voiceover | On-screen text |
|---:|---|---|---|
| 0:00-0:12 | Working Z6S LCD; close-up of SD and board | “This tiny ARMv5 display outlived its original project. WOLF3D – AI-Assisted Retro Porting Lab turns a recovered legacy port into reproducible, measurable engineering.” | `WOLF3D – AI-Assisted Retro Porting Lab` |
| 0:12-0:28 | Repository tree; briefly show ignored private directories | “The local tree had more than two thousand files, including commercial game data and vendor binaries. The public history contains only legal source, pins, patches, tests, and documentation.” | `Legal boundary enforced before publish` |
| 0:28-0:43 | Terminal: run `retroport.py analyze`; open the report | “A deterministic scanner extracts the target contract, dependency pins, platform seams, patch footprint, and exact line evidence. Unknowns stay unknown.” | `Facts first • JSON + Markdown` |
| 0:43-1:08 | Codex with GPT-5.6 selected; invoke `$retroport-ai`; scroll facts/inferences/proposal | “In Codex, GPT-5.6 reasons over that evidence. The repository skill separates facts, inferences, proposals, and tests, then waits for human approval. This is the core product—not a chatbot pasted onto the demo.” | `GPT-5.6 + Codex • human-gated patch` |
| 1:08-1:25 | Show the compact video telemetry diff and tests | “For this run it bounded the video change: one fixed native mode and low-rate integer telemetry. Five tests, patch reconstruction, both ARM builds, and both QEMU profiles pass.” | `5/5 tests • 2/2 builds • 2/2 QEMU` |
| 1:25-1:52 | Power-on LCD with no keyboard; hold on original logo, menu, then demo | “With no keyboard, the original logo appears, the menu remains visible, and demo mode starts automatically. Logo, menu, and gameplay use the same 480 by 272 presentation path—no mode switch and no black demo.” | `320x200 logical → 480x272x32 native` |
| 1:52-2:08 | Show keyboard or touch control; optionally show audio log | “A keyboard can hot-plug through evdev, while touch remains available. The minimal audio path degrades safely if hardware is missing.” | `Keyboard + touch + demo fallback` |
| 2:08-2:27 | Remove SD after power-off; show normal UI or recovery diagram | “The installation is reversible: no internal firmware replacement. Remove the SD after power-off and the original appliance remains intact.” | `Reversible SD deployment` |
| 2:27-2:42 | Benchmark JSON/table with captured device values | “The WOLF3D lab parses first-present time, five-second presentation windows, and peak memory. Every number keeps its source and definition.” | `Measured, never guessed` |
| 2:42-2:50 | Title card and repository URL | “WOLF3D – AI-Assisted Retro Porting Lab helps developers port, verify, and learn from legacy software—one evidence-backed change at a time.” | `GitHub URL • Developer Tools` |

## Exact terminal actions

Prepare reports before recording so the video does not depend on network access.
Use a clean terminal with a large font:

```bash
python3 tools/retroport.py analyze \
  --json reports/wolf3d-evidence.json \
  --markdown reports/wolf3d-evidence.md
python3 -m unittest -v
bash scripts/verify_patches.sh
python3 tools/retroport.py check-release
```

In Codex, use this prompt with GPT-5.6 selected:

```text
Use $retroport-ai to review the Z6S presentation path from the generated
evidence. Separate facts, inferences, proposals, tests, and unknowns. Explain
the smallest measurable change and stop for human approval before editing.
```

Do not expose absolute personal paths, commercial data filenames beyond generic
profile names, API keys, private repository pages, or the contents of ignored
directories while recording.

## Capture checklist

- Film the physical screen in focus and lock camera exposure to avoid flicker.
- Show the full startup sequence in one continuous shot: logo, menu, demo.
- Keep the keyboard visibly disconnected for the demo-fallback shot.
- Record a separate short keyboard/touch shot; do not restart the whole demo.
- Capture Codex with the GPT-5.6 model label visible.
- Include narration explaining what Codex does and what GPT-5.6 contributes.
- Show the release check result and the public repository URL.
- Add captions and trim dead build time.
- Use only legally captured visuals and audio.

## Fallback plan

If the LCD cannot be filmed reliably, use a previously recorded continuous
physical-device clip and overlay the raw `performance.log` plus parsed report.
Label QEMU footage explicitly as a host smoke test; never present it as the
physical device. If device audio is noisy, mute it and retain voiceover/captions.

## 60-second backup cut

- 0:00-0:08 — hardware problem and result;
- 0:08-0:20 — deterministic evidence scan;
- 0:20-0:35 — GPT-5.6/Codex human-gated reasoning;
- 0:35-0:50 — physical logo/menu/demo full-screen sequence;
- 0:50-1:00 — tests, legal boundary, repository URL.
