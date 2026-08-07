#!/bin/bash
# Simple performance benchmark for Indonesiaku.
# Times a set of compute-heavy example programs and prints per-run wall time.
# Not a pass/fail gate — a baseline to spot regressions across commits.

set -u
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO="$(dirname "$SCRIPT_DIR")"
cd "$REPO"

BIN="./indk"
[ -f "$BIN" ] || BIN="./indk.exe"
if [ ! -x "$BIN" ] && [ ! -f "$BIN" ]; then
    echo "Binary not found. Build first (make)."
    exit 1
fi

BENCHES=(
    "examples/fibonacci.idk"
    "examples/recursion_deep.idk"
    "examples/performance_stress.idk"
    "examples/loop_nested.idk"
    "examples/mutual_recursion.idk"
)

echo "=================================================="
echo " INDONESIAKU BENCHMARK  ($(date))"
echo "=================================================="
for b in "${BENCHES[@]}"; do
    [ -f "$b" ] || continue
    start=$(date +%s.%N)
    "$BIN" "$b" > /dev/null 2>&1
    end=$(date +%s.%N)
    printf "  %-38s %8.3f s\n" "$(basename "$b")" "$(echo "$end - $start" | bc)"
done
echo "=================================================="
