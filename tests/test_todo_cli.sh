#!/bin/bash
# Integration test for the Todo-list JSON DB CLI (examples/todo_cli.idk).
# Runs a full CRUD sequence across separate process invocations (real
# persistence) and asserts the observable output. Exits non-zero on any
# mismatch so CI can gate on it.

set -u
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO="$(dirname "$SCRIPT_DIR")"
cd "$REPO"

BIN="./indk"
[ -f "$BIN" ] || BIN="./indk.exe"
if [ ! -f "$BIN" ]; then echo "Binary not found; build first (make)."; exit 1; fi

APP="examples/todo_cli.idk"
DB="data_todo.json"
rm -f "$DB"

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

echo "== Todo CLI CRUD integration test =="

# CREATE
check "tambah 1" "Ditambahkan." "$("$BIN" "$APP" tambah 'Beli susu' 2>&1)"
check "tambah 2" "Ditambahkan." "$("$BIN" "$APP" tambah 'Cuci baju' 2>&1)"
check "tambah 3" "Ditambahkan." "$("$BIN" "$APP" tambah 'Belajar' 2>&1)"

# READ all
out="$("$BIN" "$APP" daftar 2>&1)"
expected="[ ] #1 Beli susu
[ ] #2 Cuci baju
[ ] #3 Belajar"
check "daftar (3 item)" "$expected" "$out"

# READ one
check "lihat 2" "[ ] #2 Cuci baju" "$("$BIN" "$APP" lihat 2 2>&1)"

# UPDATE - selesai
check "selesai 2" "Ditandai selesai." "$("$BIN" "$APP" selesai 2 2>&1)"
check "lihat 2 setelah selesai" "[x] #2 Cuci baju" "$("$BIN" "$APP" lihat 2 2>&1)"

# UPDATE - ubah teks
check "ubah 1" "Diubah." "$("$BIN" "$APP" ubah 1 'Beli roti' 2>&1)"
check "lihat 1 setelah ubah" "[ ] #1 Beli roti" "$("$BIN" "$APP" lihat 1 2>&1)"

# DELETE
check "hapus 3" "Dihapus." "$("$BIN" "$APP" hapus 3 2>&1)"

# READ all - final state persisted across all invocations
out="$("$BIN" "$APP" daftar 2>&1)"
expected="[ ] #1 Beli roti
[x] #2 Cuci baju"
check "daftar akhir (2 item)" "$expected" "$out"

# Error handling: delete non-existent id is reported, not crash
# (cetak prints each argument on its own line)
check "hapus id tak ada" "Kesalahan:
todo dengan id 99 tidak ditemukan" "$("$BIN" "$APP" hapus 99 2>&1)"

rm -f "$DB"

if [ "$fail" -eq 0 ]; then
    echo "== ALL TODO CLI TESTS PASSED =="
    exit 0
else
    echo "== TODO CLI TESTS FAILED =="
    exit 1
fi
