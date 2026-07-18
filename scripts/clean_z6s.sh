#!/usr/bin/env bash
set -euo pipefail
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
rm -rf "$ROOT/build/z6s" "$ROOT/dist/z6s"
echo "Removed generated Z6S build artifacts."
