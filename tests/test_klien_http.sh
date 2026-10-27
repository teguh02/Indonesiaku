#!/usr/bin/env bash
# Klien HTTP Indonesiaku (http_ambil) melawan server HTTP Indonesiaku
# (examples/todo_server.idk) lewat jaringan sungguhan. Server dijalankan di
# port sendiri supaya tidak bertabrakan dengan tests/test_todo_server.sh.

set -u
cd "$(dirname "$0")/.." || exit 1

BIN=./indk
[ -x "$BIN" ] || BIN=./indk.exe
if [ ! -x "$BIN" ]; then echo "indk belum dibangun." >&2; exit 1; fi

PORT=8151
DB="data_todo_server.json"
rm -f "$DB"
"$BIN" examples/todo_server.idk "$PORT" > /dev/null 2>&1 &
SERVER_PID=$!
cleanup() {
    curl -s "http://127.0.0.1:$PORT/berhenti" > /dev/null 2>&1 || true
    sleep 0.3
    kill "$SERVER_PID" 2>/dev/null || true
    rm -f "$DB"
}
trap cleanup EXIT

siap=0
for _ in $(seq 1 50); do
    if curl -s "http://127.0.0.1:$PORT/todos" > /dev/null 2>&1; then siap=1; break; fi
    sleep 0.1
done
if [ "$siap" -ne 1 ]; then echo "Server tidak siap"; exit 1; fi

HARAP="GET /todos: 200 []
POST /todos: 201 benar {\"id\":1,\"teks\":\"Beli kopi ☕\",\"selesai\":false}
tipe konten: application/json
GET /todos/1: 200 {\"id\":1,\"teks\":\"Beli kopi ☕\",\"selesai\":false}
GET /todos/99: 404 salah
GalatJaringan: 404 Not Found untuk http://127.0.0.1:$PORT/todos/99"

DAPAT=$("$BIN" examples/klien_todo.idk "$PORT" 2>&1 | tr -d '\r')
echo "Klien HTTP melawan server todo:"
if [ "$DAPAT" = "$HARAP" ]; then
    echo "  ok   http_ambil GET/POST/404 (termasuk badan UTF-8)"
    exit 0
fi
echo "  GAGAL"
diff <(echo "$HARAP") <(echo "$DAPAT") | sed 's/^/        /'
exit 1
