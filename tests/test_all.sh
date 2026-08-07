#!/bin/bash
# =================================================
#  INDONESIAKU - GOLDEN OUTPUT TEST SUITE (Linux/macOS)
#  Builds the project, runs every example, and asserts
#  stdout matches tests/expected/<name>.out.
#  Exits non-zero on any failure so CI can gate on it.
# =================================================

set -u

RED='\033[0;31m'; GREEN='\033[0;32m'; BLUE='\033[0;34m'; NC='\033[0m'

# Resolve repo root (parent of this script's dir)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO="$(dirname "$SCRIPT_DIR")"
cd "$REPO"

echo -e "${BLUE}Building Indonesiaku...${NC}"
if [ ! -f Makefile ]; then
    echo -e "${RED}Error: Makefile not found!${NC}"; exit 1
fi
make clean
make
if [ ! -f ./indk ]; then
    echo -e "${RED}Error: Binary 'indk' not found after build!${NC}"; exit 1
fi
echo -e "${GREEN}Build successful!${NC}"
echo ""

EXPECTED_DIR="$REPO/tests/expected"
if [ ! -d "$EXPECTED_DIR" ]; then
    echo -e "${RED}Error: tests/expected not found!${NC}"; exit 1
fi

TOTAL=0; PASSED=0; FAILED=0
FAILURES=""

echo -e "${BLUE}=================================================${NC}"
echo -e "${BLUE}  INDONESIAKU - GOLDEN OUTPUT TESTS${NC}"
echo -e "${BLUE}=================================================${NC}"

for exp in "$EXPECTED_DIR"/*.out; do
    name="$(basename "$exp" .out)"
    TOTAL=$((TOTAL+1))

    # Find the matching source under examples/ (any subdir)
    src="$(find examples -type f -name "$name.idk" | head -n1)"
    if [ -z "$src" ]; then
        echo -e "  ${RED}x${NC} $name  (no matching .idk source)"
        FAILED=$((FAILED+1)); FAILURES="$FAILURES $name"; continue
    fi

    errfile="$(mktemp)"
    actual="$(./indk "$src" 2>"$errfile")"; code=$?
    stderr="$(cat "$errfile")"; rm -f "$errfile"
    expected="$(cat "$exp")"

    if [ "$code" -eq 0 ] && [ "$actual" = "$expected" ] && [ -z "$stderr" ]; then
        echo -e "  ${GREEN}+${NC} $name"
        PASSED=$((PASSED+1))
    else
        if [ "$code" -ne 0 ]; then reason="exit=$code"
        elif [ -n "$stderr" ]; then reason="stderr: $stderr"
        else reason="output mismatch"; fi
        echo -e "  ${RED}x${NC} $name  ($reason)"
        FAILED=$((FAILED+1)); FAILURES="$FAILURES $name"
    fi
done

echo -e "${BLUE}=================================================${NC}"
echo -e "TOTAL: ${GREEN}$PASSED/$TOTAL PASSED${NC}"
if [ "$FAILED" -eq 0 ]; then
    echo -e "${GREEN}SUCCESS: ALL TESTS PASSING!${NC}"
    exit 0
else
    echo -e "${RED}FAILURE: $FAILED test(s) failed:${NC}"
    for f in $FAILURES; do echo -e "  ${RED}- $f${NC}"; done
    exit 1
fi
