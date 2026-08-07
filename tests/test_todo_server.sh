#!/bin/bash
# Integration test for the Todo-list REST HTTP server (examples/todo_server.idk).
# Starts the server, drives it with curl over real HTTP, asserts JSON responses,
# then shuts it down via the /berhenti endpoint. Exits non-zero on any mismatch.

set -u
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO="$(dirname "$SCRIPT_DIR")"
cd "$REPO"

BIN="./indk"
[ -f "$BIN" ] || BIN="./indk.exe"
if [ ! -f "$BIN" ]; then echo "Binary not found; build first (make)."; exit 1; fi

PORT=8137
BASE="http://127.0.0.1:$PORT"
DB="data_todo_server.json"
rm -f "$DB"

# Start server in the background.
"$BIN" examples/todo_server.idk "$PORT" &
SERVER_PID=$!

cleanup() {
    curl -s "$BASE/berhenti" > /dev/null 2>&1 || true
    sleep 0.3
    kill "$SERVER_PID" 2>/dev/null || true
    rm -f "$DB"
}
trap cleanup EXIT

# Wait for the server to accept connections (up to ~5s).
ready=0
for _ in $(seq 1 50); do
    if curl -s "$BASE/todos" > /dev/null 2>&1; then ready=1; break; fi
    sleep 0.1
done
if [ "$ready" -ne 1 ]; then echo "Server tidak siap"; exit 1; fi

fail=0
check() {
    local desc="$1"; local expected="$2"; local actual="$3"
    if [ "$actual" = "$expected" ]; then
        echo "  OK  $desc"
    else
        echo "  FAIL $desc"
        echo "       expected: [$expected]"
        echo "       actual:   [$actual]"
        fail=1
    fi
}

echo "== Todo REST server integration test =="

check "GET /todos (kosong)" "[]" "$(curl -s "$BASE/todos")"
check "POST #1" '{"id":1,"teks":"Beli susu","selesai":false}' \
    "$(curl -s -X POST -d '{"teks":"Beli susu"}' "$BASE/todos")"
check "POST #2" '{"id":2,"teks":"Cuci baju","selesai":false}' \
    "$(curl -s -X POST -d '{"teks":"Cuci baju"}' "$BASE/todos")"
check "GET /todos (2 item)" \
    '[{"id":1,"teks":"Beli susu","selesai":false},{"id":2,"teks":"Cuci baju","selesai":false}]' \
    "$(curl -s "$BASE/todos")"
check "GET /todos/1" '{"id":1,"teks":"Beli susu","selesai":false}' \
    "$(curl -s "$BASE/todos/1")"
check "PUT /todos/1 (selesai)" '{"id":1,"teks":"Beli susu","selesai":true}' \
    "$(curl -s -X PUT -d '{"selesai":true}' "$BASE/todos/1")"
check "DELETE /todos/2" '{"pesan":"dihapus"}' \
    "$(curl -s -X DELETE "$BASE/todos/2")"
check "GET /todos (final)" '[{"id":1,"teks":"Beli susu","selesai":true}]' \
    "$(curl -s "$BASE/todos")"
check "GET /todos/99 (404)" '{"kesalahan":"tidak ditemukan"}' \
    "$(curl -s "$BASE/todos/99")"

if [ "$fail" -eq 0 ]; then
    echo "== ALL TODO SERVER TESTS PASSED =="
    exit 0
else
    echo "== TODO SERVER TESTS FAILED =="
    exit 1
fi
