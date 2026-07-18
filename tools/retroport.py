#!/usr/bin/env python3
"""Evidence collector and release guard for AI-assisted legacy porting.

This program intentionally does not make AI calls. It produces deterministic
evidence that a GPT-5.6 Codex workflow can interpret without inventing facts.
"""

from __future__ import annotations

import argparse
import datetime as dt
import fnmatch
import hashlib
import json
import os
import platform
import re
import shutil
import subprocess
import sys
import tomllib
from collections import Counter
from pathlib import Path
from typing import Any, Iterable, Sequence


SCHEMA_VERSION = 1
ENTRY_PATTERNS = (
    ("c-main", re.compile(r"\b(?:int|void)\s+main\s*\(")),
    ("shared-library-hook", re.compile(r"\bonEasyUIInit\s*\(")),
    ("shell-entry", re.compile(r"^#!.*\b(?:sh|bash)\b")),
    ("frame-presenter", re.compile(r"\bVL_Z6SPresent\s*\(")),
    ("input-pump", re.compile(r"\bZ6S_InputPump\s*\(")),
    ("audio-pump", re.compile(r"\bMix_Z6S_Pump\s*\(")),
)


class RetroPortError(RuntimeError):
    """Actionable CLI error."""


def utc_now() -> str:
    epoch = os.environ.get("SOURCE_DATE_EPOCH")
    if epoch:
        value = dt.datetime.fromtimestamp(int(epoch), tz=dt.timezone.utc)
    else:
        value = dt.datetime.now(tz=dt.timezone.utc)
    return value.replace(microsecond=0).isoformat().replace("+00:00", "Z")


def load_config(root: Path, config_path: Path | None = None) -> dict[str, Any]:
    path = config_path or root / "retroport.toml"
    if not path.is_absolute():
        path = root / path
    try:
        with path.open("rb") as handle:
            config = tomllib.load(handle)
    except (OSError, tomllib.TOMLDecodeError) as exc:
        raise RetroPortError(f"cannot load target profile {path}: {exc}") from exc
    if config.get("schema_version") != SCHEMA_VERSION:
        raise RetroPortError(
            f"unsupported profile schema: {config.get('schema_version')!r}"
        )
    return config


def relative_path(path: Path, root: Path) -> str:
    return path.resolve().relative_to(root.resolve()).as_posix()


def is_excluded(relative: str, excluded: Sequence[str]) -> bool:
    parts = Path(relative).parts
    return any(name in parts for name in excluded)


def iter_source_files(root: Path, config: dict[str, Any]) -> Iterable[Path]:
    project = config["project"]
    extensions = {item.lower() for item in project["source_extensions"]}
    excluded = tuple(project.get("exclude_directories", ()))
    max_bytes = int(project.get("max_text_bytes", 1_048_576))
    seen: set[str] = set()
    for source_root in project["source_roots"]:
        base = root / source_root
        if not base.exists():
            continue
        paths = (base,) if base.is_file() else base.rglob("*")
        for path in paths:
            if not path.is_file() or path.suffix.lower() not in extensions:
                continue
            rel = relative_path(path, root)
            if rel in seen or is_excluded(rel, excluded):
                continue
            try:
                size = path.stat().st_size
            except OSError:
                continue
            if size <= max_bytes:
                seen.add(rel)
                yield path


def read_text(path: Path) -> str:
    return path.read_text(encoding="utf-8", errors="replace")


def source_metrics(files: Sequence[Path], root: Path) -> dict[str, Any]:
    by_extension: Counter[str] = Counter()
    total_lines = 0
    nonblank_lines = 0
    bytes_total = 0
    for path in files:
        text = read_text(path)
        lines = text.splitlines()
        total_lines += len(lines)
        nonblank_lines += sum(bool(line.strip()) for line in lines)
        bytes_total += path.stat().st_size
        by_extension[path.suffix.lower() or "[none]"] += 1
    return {
        "files": len(files),
        "bytes": bytes_total,
        "lines": total_lines,
        "nonblank_lines": nonblank_lines,
        "files_by_extension": dict(sorted(by_extension.items())),
        "roots": sorted(
            {relative_path(path, root).split("/", 1)[0] for path in files}
        ),
    }


def collect_findings(
    files: Sequence[Path], root: Path, config: dict[str, Any]
) -> list[dict[str, Any]]:
    compiled = [
        (rule, re.compile(rule["regex"])) for rule in config.get("rules", ())
    ]
    findings: list[dict[str, Any]] = []
    for path in files:
        rel = relative_path(path, root)
        for number, line in enumerate(read_text(path).splitlines(), start=1):
            for rule, pattern in compiled:
                match = pattern.search(line)
                if not match:
                    continue
                findings.append(
                    {
                        "rule_id": rule["id"],
                        "title": rule["title"],
                        "category": rule["category"],
                        "severity": rule["severity"],
                        "path": rel,
                        "line": number,
                        "match": match.group(0)[:160],
                        "excerpt": line.strip()[:240],
                        "guidance": rule["message"],
                    }
                )
    return sorted(findings, key=lambda item: (item["path"], item["line"], item["rule_id"]))


def collect_entry_points(files: Sequence[Path], root: Path) -> list[dict[str, Any]]:
    entries: list[dict[str, Any]] = []
    for path in files:
        rel = relative_path(path, root)
        for number, line in enumerate(read_text(path).splitlines(), start=1):
            for kind, pattern in ENTRY_PATTERNS:
                if pattern.search(line):
                    entries.append(
                        {
                            "kind": kind,
                            "path": rel,
                            "line": number,
                            "excerpt": line.strip()[:200],
                        }
                    )
    return sorted(entries, key=lambda item: (item["kind"], item["path"], item["line"]))


def collect_patch_stats(root: Path) -> list[dict[str, Any]]:
    results: list[dict[str, Any]] = []
    patch_root = root / "patches"
    if not patch_root.exists():
        return results
    for path in sorted(patch_root.glob("*.patch")):
        files: set[str] = set()
        additions = deletions = hunks = 0
        for line in read_text(path).splitlines():
            if line.startswith("diff --git a/"):
                match = re.match(r"diff --git a/(.+?) b/(.+)$", line)
                if match:
                    files.add(match.group(2))
            elif line.startswith("@@"):
                hunks += 1
            elif line.startswith("+") and not line.startswith("+++"):
                additions += 1
            elif line.startswith("-") and not line.startswith("---"):
                deletions += 1
        results.append(
            {
                "path": relative_path(path, root),
                "files": sorted(files),
                "file_count": len(files),
                "hunks": hunks,
                "additions": additions,
                "deletions": deletions,
                "sha256": sha256_file(path),
            }
        )
    return results


def collect_dependencies(root: Path) -> list[dict[str, str]]:
    fetch = root / "scripts" / "fetch_sources.sh"
    if not fetch.exists():
        return []
    pattern = re.compile(
        r"clone_at\s+(?P<name>\S+)\s+(?P<url>\S+)\s+\\?\s*\n\s*(?P<commit>[0-9a-f]{40})"
    )
    return [match.groupdict() for match in pattern.finditer(read_text(fetch))]


def git_tracked_files(root: Path) -> list[Path] | None:
    if not (root / ".git").exists():
        return None
    command = [
        "git",
        "-c",
        f"safe.directory={root.resolve().as_posix()}",
        "-C",
        str(root),
        "ls-files",
        "-z",
    ]
    try:
        result = subprocess.run(command, check=True, capture_output=True)
    except (OSError, subprocess.CalledProcessError):
        return None
    return [root / item.decode("utf-8") for item in result.stdout.split(b"\0") if item]


def walk_release_files(root: Path, config: dict[str, Any]) -> list[Path]:
    tracked = git_tracked_files(root)
    if tracked is not None:
        return sorted(path for path in tracked if path.is_file())
    excluded = tuple(config["project"].get("exclude_directories", ()))
    files = []
    for path in root.rglob("*"):
        if path.is_file() and not is_excluded(relative_path(path, root), excluded):
            files.append(path)
    return sorted(files)


def glob_matches(path: str, pattern: str) -> bool:
    lower_path = path.lower()
    lower_pattern = pattern.lower()
    return fnmatch.fnmatch(lower_path, lower_pattern) or fnmatch.fnmatch(
        Path(lower_path).name, lower_pattern
    )


def check_release(root: Path, config: dict[str, Any]) -> dict[str, Any]:
    release = config["release"]
    violations: list[dict[str, Any]] = []
    files = walk_release_files(root, config)
    deny_globs = release.get("deny_globs", ())
    content_rules = [
        (item, re.compile(item["regex"])) for item in release.get("deny_content", ())
    ]
    max_bytes = int(config["project"].get("max_text_bytes", 1_048_576))

    for required in release.get("required_files", ()):
        if not (root / required).is_file():
            violations.append(
                {
                    "kind": "missing-required-file",
                    "path": required,
                    "message": "Required public repository file is missing.",
                }
            )

    for path in files:
        rel = relative_path(path, root)
        for pattern in deny_globs:
            if glob_matches(rel, pattern):
                violations.append(
                    {
                        "kind": "denied-path",
                        "path": rel,
                        "pattern": pattern,
                        "message": "Tracked path matches the private/generated asset policy.",
                    }
                )
        try:
            if path.stat().st_size > max_bytes:
                continue
            text = path.read_text(encoding="utf-8")
        except (OSError, UnicodeError):
            continue
        for rule, pattern in content_rules:
            for number, line in enumerate(text.splitlines(), start=1):
                if pattern.search(line):
                    violations.append(
                        {
                            "kind": "denied-content",
                            "rule_id": rule["id"],
                            "path": rel,
                            "line": number,
                            "message": rule["message"],
                        }
                    )
    violations.sort(key=lambda item: (item["path"], item.get("line", 0), item["kind"]))
    return {
        "mode": "git-tracked" if git_tracked_files(root) is not None else "filesystem",
        "files_checked": len(files),
        "passed": not violations,
        "violations": violations,
    }


def analyze(root: Path, config: dict[str, Any]) -> dict[str, Any]:
    files = list(iter_source_files(root, config))
    findings = collect_findings(files, root, config)
    severity_counts = Counter(item["severity"] for item in findings)
    category_counts = Counter(item["category"] for item in findings)
    return {
        "schema_version": SCHEMA_VERSION,
        "generated_at": utc_now(),
        "generator": "tools/retroport.py",
        "project": {
            "name": config["project"]["name"],
            "case_study": config["project"].get("case_study"),
            "target": config["project"].get("target"),
        },
        "source_metrics": source_metrics(files, root),
        "dependencies": collect_dependencies(root),
        "patches": collect_patch_stats(root),
        "entry_points": collect_entry_points(files, root),
        "finding_summary": {
            "total": len(findings),
            "by_severity": dict(sorted(severity_counts.items())),
            "by_category": dict(sorted(category_counts.items())),
        },
        "findings": findings,
        "release_check": check_release(root, config),
        "interpretation_contract": {
            "facts": "Only fields in this report are deterministic evidence.",
            "ai_role": "Explain, prioritize, and propose changes with citations.",
            "human_role": "Approve architecture, patches, hardware tests, and release.",
        },
    }


def markdown_report(report: dict[str, Any]) -> str:
    metrics = report["source_metrics"]
    lines = [
        "# RetroPort AI evidence report",
        "",
        f"Generated: `{report['generated_at']}`  ",
        f"Target: {report['project'].get('target') or 'not specified'}",
        "",
        "## Deterministic scope",
        "",
        f"- {metrics['files']} files, {metrics['lines']} lines, {metrics['bytes']} bytes scanned.",
        f"- {report['finding_summary']['total']} portability evidence items.",
        f"- {len(report['patches'])} patch files and {len(report['dependencies'])} pinned dependencies.",
        f"- Release guard: {'PASS' if report['release_check']['passed'] else 'FAIL'}.",
        "",
        "This report contains evidence, not an autonomous refactoring decision.",
        "GPT-5.6 may explain and prioritize it, but every proposed patch requires",
        "human review and deterministic verification.",
        "",
        "## Pinned dependencies",
        "",
        "| Name | Commit | Upstream |",
        "|---|---|---|",
    ]
    for item in report["dependencies"]:
        lines.append(f"| {item['name']} | `{item['commit']}` | `{item['url']}` |")
    lines.extend(["", "## Patch surface", "", "| Patch | Files | Hunks | + | - |", "|---|---:|---:|---:|---:|"])
    for item in report["patches"]:
        lines.append(
            f"| `{item['path']}` | {item['file_count']} | {item['hunks']} | "
            f"{item['additions']} | {item['deletions']} |"
        )
    lines.extend(["", "## Entry points", "", "| Kind | Location | Evidence |", "|---|---|---|"])
    for item in report["entry_points"]:
        excerpt = item["excerpt"].replace("|", "\\|")
        lines.append(f"| {item['kind']} | `{item['path']}:{item['line']}` | `{excerpt}` |")
    lines.extend(["", "## Portability evidence", "", "| Severity | Category | Rule | Location | Evidence |", "|---|---|---|---|---|"])
    for item in report["findings"]:
        excerpt = item["excerpt"].replace("|", "\\|").replace("`", "'")
        lines.append(
            f"| {item['severity']} | {item['category']} | {item['rule_id']} | "
            f"`{item['path']}:{item['line']}` | `{excerpt}` |"
        )
    if report["release_check"]["violations"]:
        lines.extend(["", "## Release blockers", ""])
        for item in report["release_check"]["violations"]:
            location = item["path"] + (f":{item['line']}" if item.get("line") else "")
            lines.append(f"- `{location}` — {item['message']}")
    lines.append("")
    return "\n".join(lines)


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for block in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def git_revision(root: Path) -> str | None:
    tracked = git_tracked_files(root)
    if tracked is None:
        return None
    command = [
        "git",
        "-c",
        f"safe.directory={root.resolve().as_posix()}",
        "-C",
        str(root),
        "rev-parse",
        "HEAD",
    ]
    try:
        return subprocess.run(command, check=True, capture_output=True, text=True).stdout.strip()
    except (OSError, subprocess.CalledProcessError):
        return None


def parse_size_output(output: str) -> dict[str, int]:
    sections: dict[str, int] = {}
    for line in output.splitlines():
        match = re.match(r"^\s*(\.[A-Za-z0-9_.]+)\s+(\d+)\s+", line)
        if match:
            sections[match.group(1)] = int(match.group(2))
    return sections


def parse_device_metrics(path: Path) -> dict[str, Any]:
    """Parse key=value samples emitted by the Z6S frame presenter."""
    records: list[dict[str, Any]] = []
    for number, line in enumerate(read_text(path).splitlines(), start=1):
        values: dict[str, Any] = {"line": number}
        for token in line.split():
            if "=" not in token:
                continue
            key, value = token.split("=", 1)
            values[key] = int(value) if re.fullmatch(r"-?\d+", value) else value
        if values.get("metric"):
            records.append(values)

    first = next(
        (item for item in records if item.get("metric") == "sdl_first_present"),
        None,
    )
    windows = [item for item in records if item.get("metric") == "present_window"]
    fps_values = [item["fps_milli"] / 1000.0 for item in windows if "fps_milli" in item]
    interval_values = [
        item["avg_present_interval_us"] / 1000.0
        for item in windows
        if "avg_present_interval_us" in item
    ]
    rss_values = [item["maxrss_kb"] * 1024 for item in windows if item.get("maxrss_kb", -1) >= 0]

    def summary(values: Sequence[float]) -> dict[str, float | int] | None:
        if not values:
            return None
        return {
            "samples": len(values),
            "mean": sum(values) / len(values),
            "min": min(values),
            "max": max(values),
        }

    return {
        "source": str(path),
        "raw_samples": records,
        "sdl_first_present_ms": first.get("elapsed_ms") if first else None,
        "present_fps": summary(fps_values),
        "average_present_interval_ms": summary(interval_values),
        "peak_resident_memory_bytes": max(rss_values) if rss_values else None,
        "window_samples": len(windows),
        "interpretation": (
            "Presentation frequency measures completed VL_Z6SPresent calls. "
            "It is not a GPU counter and should be reported with the target configuration."
        ),
    }


def capture_benchmark(
    root: Path,
    config: dict[str, Any],
    binaries: Sequence[str],
    device_log: Path | None = None,
) -> dict[str, Any]:
    profiles: dict[str, Any] = {}
    size_tool = next(
        (tool for tool in config.get("benchmark", {}).get("size_tools", ()) if shutil.which(tool)),
        None,
    )
    for specification in binaries:
        if "=" not in specification:
            raise RetroPortError(f"binary must use LABEL=PATH: {specification}")
        label, raw_path = specification.split("=", 1)
        path = Path(raw_path)
        if not path.is_absolute():
            path = root / path
        if not path.is_file():
            raise RetroPortError(f"binary does not exist: {path}")
        item: dict[str, Any] = {
            "path": relative_path(path, root),
            "file_bytes": path.stat().st_size,
            "sha256": sha256_file(path),
            "sections": None,
        }
        if size_tool:
            result = subprocess.run([size_tool, "-A", str(path)], capture_output=True, text=True)
            if result.returncode == 0:
                item["sections"] = parse_size_output(result.stdout)
            else:
                item["section_reason"] = result.stderr.strip() or "size tool failed"
        else:
            item["section_reason"] = "No configured GNU-compatible size tool was found."
        profiles[label] = item
    physical = {
        name: None for name in config.get("benchmark", {}).get("physical_metrics", ())
    }
    physical["reason"] = "Physical metrics require an instrumented target run."
    device_metrics = None
    if device_log is not None:
        resolved_log = device_log if device_log.is_absolute() else root / device_log
        if not resolved_log.is_file():
            raise RetroPortError(f"device metrics log does not exist: {resolved_log}")
        device_metrics = parse_device_metrics(resolved_log)
    return {
        "schema_version": SCHEMA_VERSION,
        "captured_at": utc_now(),
        "git_revision": git_revision(root),
        "host": {"platform": platform.platform(), "python": platform.python_version()},
        "size_tool": size_tool,
        "profiles": profiles,
        "physical_metrics": physical,
        "device_metrics": device_metrics,
    }


def numeric_deltas(before: Any, after: Any) -> Any:
    if isinstance(before, dict) and isinstance(after, dict):
        result = {}
        for key in sorted(before.keys() & after.keys()):
            value = numeric_deltas(before[key], after[key])
            if value is not None:
                result[key] = value
        return result or None
    if isinstance(before, (int, float)) and not isinstance(before, bool) and isinstance(after, (int, float)) and not isinstance(after, bool):
        return {"before": before, "after": after, "delta": after - before}
    return None


def write_json(path: Path, value: Any) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, indent=2, sort_keys=True) + "\n", encoding="utf-8")


def write_text(path: Path, value: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(value, encoding="utf-8")


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=Path.cwd(), help="repository root")
    parser.add_argument("--config", type=Path, help="target profile (default: retroport.toml)")
    subparsers = parser.add_subparsers(dest="command", required=True)

    analyze_parser = subparsers.add_parser("analyze", help="collect deterministic porting evidence")
    analyze_parser.add_argument("--json", type=Path, help="write the full JSON report")
    analyze_parser.add_argument("--markdown", type=Path, help="write a human-readable report")

    release_parser = subparsers.add_parser("check-release", help="reject private/generated tracked files")
    release_parser.add_argument("--json", type=Path, help="write release-check JSON")

    benchmark_parser = subparsers.add_parser("benchmark", help="capture reproducible binary facts")
    benchmark_parser.add_argument("--binary", action="append", default=[], metavar="LABEL=PATH")
    benchmark_parser.add_argument("--device-log", type=Path, help="Z6S performance.log to import")
    benchmark_parser.add_argument("--output", type=Path, required=True)

    compare_parser = subparsers.add_parser("compare", help="compare numeric fields in two benchmark files")
    compare_parser.add_argument("--baseline", type=Path, required=True)
    compare_parser.add_argument("--current", type=Path, required=True)
    compare_parser.add_argument("--output", type=Path)
    return parser


def resolve_output(root: Path, path: Path) -> Path:
    return path if path.is_absolute() else root / path


def main(argv: Sequence[str] | None = None) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)
    root = args.root.resolve()
    try:
        config = load_config(root, args.config)
        if args.command == "analyze":
            report = analyze(root, config)
            if args.json:
                write_json(resolve_output(root, args.json), report)
            if args.markdown:
                write_text(resolve_output(root, args.markdown), markdown_report(report))
            print(json.dumps(report["finding_summary"], sort_keys=True))
            return 0
        if args.command == "check-release":
            result = check_release(root, config)
            if args.json:
                write_json(resolve_output(root, args.json), result)
            for item in result["violations"]:
                location = item["path"] + (f":{item['line']}" if item.get("line") else "")
                print(f"FAIL {location}: {item['message']}")
            print(
                f"Release check {'passed' if result['passed'] else 'failed'}: "
                f"{result['files_checked']} files, {len(result['violations'])} violations."
            )
            return 0 if result["passed"] else 1
        if args.command == "benchmark":
            report = capture_benchmark(root, config, args.binary, args.device_log)
            write_json(resolve_output(root, args.output), report)
            print(f"Wrote benchmark: {args.output}")
            return 0
        if args.command == "compare":
            before = json.loads(resolve_output(root, args.baseline).read_text(encoding="utf-8"))
            after = json.loads(resolve_output(root, args.current).read_text(encoding="utf-8"))
            comparison = {
                "schema_version": SCHEMA_VERSION,
                "baseline": str(args.baseline),
                "current": str(args.current),
                "numeric_deltas": numeric_deltas(before, after) or {},
            }
            if args.output:
                write_json(resolve_output(root, args.output), comparison)
            print(json.dumps(comparison, indent=2, sort_keys=True))
            return 0
    except (RetroPortError, OSError, json.JSONDecodeError) as exc:
        print(f"retroport: {exc}", file=sys.stderr)
        return 2
    parser.error("unknown command")
    return 2


if __name__ == "__main__":
    raise SystemExit(main())
