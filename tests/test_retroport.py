from __future__ import annotations

import json
import tempfile
import textwrap
import unittest
from pathlib import Path

from tools import retroport


PROFILE = r"""
schema_version = 1

[project]
name = "fixture"
source_roots = ["src", "patches"]
source_extensions = [".c", ".patch"]
exclude_directories = [".git"]
max_text_bytes = 100000

[release]
required_files = ["README.md"]
deny_globs = ["*.WL6"]

[[release.deny_content]]
id = "private-path"
regex = '(?i)[A-Z]:[\\/]Users[\\/][^\\/\s]+'
message = "private path"

[[rules]]
id = "device"
title = "device"
category = "hardware"
severity = "high"
regex = '/dev/[A-Za-z0-9]+'
message = "isolate it"

[benchmark]
size_tools = []
physical_metrics = ["fps"]
"""


class RetroPortTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        (self.root / "src").mkdir()
        (self.root / "patches").mkdir()
        (self.root / "README.md").write_text("fixture\n", encoding="utf-8")
        (self.root / "retroport.toml").write_text(textwrap.dedent(PROFILE), encoding="utf-8")

    def tearDown(self) -> None:
        self.temp.cleanup()

    def test_analysis_cites_portability_evidence_and_patch_surface(self) -> None:
        (self.root / "src" / "main.c").write_text(
            'int main(void) { return open("/dev/fb0", 0); }\n', encoding="utf-8"
        )
        (self.root / "patches" / "fix.patch").write_text(
            "diff --git a/a.c b/a.c\n--- a/a.c\n+++ b/a.c\n@@ -1 +1 @@\n-old\n+new\n",
            encoding="utf-8",
        )
        config = retroport.load_config(self.root)
        report = retroport.analyze(self.root, config)
        self.assertEqual(report["finding_summary"]["total"], 1)
        self.assertEqual(report["findings"][0]["path"], "src/main.c")
        self.assertEqual(report["findings"][0]["line"], 1)
        self.assertTrue(any(item["kind"] == "c-main" for item in report["entry_points"]))
        self.assertEqual(report["patches"][0]["additions"], 1)
        self.assertEqual(report["patches"][0]["deletions"], 1)

    def test_release_guard_rejects_assets_and_private_paths(self) -> None:
        (self.root / "secret.WL6").write_bytes(b"commercial")
        (self.root / "src" / "path.c").write_text(
            'const char *p = "C:\\Users\\alice\\game";\n', encoding="utf-8"
        )
        result = retroport.check_release(self.root, retroport.load_config(self.root))
        self.assertFalse(result["passed"])
        kinds = {item["kind"] for item in result["violations"]}
        self.assertEqual(kinds, {"denied-content", "denied-path"})

    def test_benchmark_records_facts_and_unknown_physical_metrics(self) -> None:
        binary = self.root / "game.bin"
        binary.write_bytes(b"retroport")
        report = retroport.capture_benchmark(
            self.root, retroport.load_config(self.root), ["demo=game.bin"]
        )
        self.assertEqual(report["profiles"]["demo"]["file_bytes"], 9)
        self.assertEqual(report["profiles"]["demo"]["sha256"], retroport.sha256_file(binary))
        self.assertIsNone(report["physical_metrics"]["fps"])

    def test_device_metrics_are_parsed_without_relabeling_present_fps(self) -> None:
        metrics = self.root / "performance.log"
        metrics.write_text(
            "metric=sdl_first_present elapsed_ms=240 logical=320x200 native=480x272\n"
            "metric=present_window window_ms=5000 frames=250 fps_milli=50000 "
            "avg_present_interval_us=20000 maxrss_kb=4096\n"
            "metric=present_window window_ms=5000 frames=240 fps_milli=48000 "
            "avg_present_interval_us=20833 maxrss_kb=4200\n",
            encoding="utf-8",
        )
        result = retroport.parse_device_metrics(metrics)
        self.assertEqual(result["sdl_first_present_ms"], 240)
        self.assertEqual(result["present_fps"]["mean"], 49.0)
        self.assertEqual(result["peak_resident_memory_bytes"], 4200 * 1024)
        self.assertIn("VL_Z6SPresent", result["interpretation"])

    def test_numeric_comparison_never_fabricates_missing_values(self) -> None:
        delta = retroport.numeric_deltas(
            {"size": 10, "fps": None}, {"size": 8, "fps": None}
        )
        self.assertEqual(delta, {"size": {"before": 10, "after": 8, "delta": -2}})


if __name__ == "__main__":
    unittest.main()
