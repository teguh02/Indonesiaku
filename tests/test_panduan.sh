#!/usr/bin/env bash
# Menjalankan SETIAP blok ```indonesiaku di website/panduan/*.md.
#
# Syarat lulus: blok dijalankan oleh indk dengan exit 0 dan stderr kosong.
# Blok yang memang bukan program utuh (potongan tanpa konteks, transkrip REPL,
# atau demo galat yang sengaja gagal) ditandai di markdown-nya sebagai
# ```indonesiaku-takjalan dan dilewati.
#
# ponytail: yang diperiksa adalah "contohnya jalan", bukan "outputnya benar".
# Komentar hasil di panduan ditulis bebas ("# 7.0   (selalu ada .0)"), jadi
# mencocokkannya butuh menulis ulang 133 komentar jadi asersi. Kebenaran output
# sudah dijaga tests/expected/. Naikkan ke pencocokan output bila komentar
# hasil sudah diseragamkan.

set -u
cd "$(dirname "$0")/.." || exit 1

RED='\033[0;31m'; GREEN='\033[0;32m'; BLUE='\033[0;34m'; YELLOW='\033[0;33m'; NC='\033[0m'

BIN=./indk
[ -x "$BIN" ] || BIN=./indk.exe
if [ ! -x "$BIN" ]; then
    echo "indk belum dibangun. Jalankan 'make' dulu." >&2
    exit 1
fi

# `timeout` adalah GNU coreutils: ada di Linux, TIDAK ada di macOS bawaan
# (gtimeout bila coreutils dipasang lewat brew). Tanpa penjaga ini setiap blok
# keluar 127 dan seluruh suite merah di macOS.
if command -v timeout >/dev/null 2>&1; then
    TIMEOUT="timeout 60"
elif command -v gtimeout >/dev/null 2>&1; then
    TIMEOUT="gtimeout 60"
else
    TIMEOUT=""
fi

WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

TOTAL=0; PASSED=0; FAILED=0; SKIPPED=0; FAILURES=""

echo -e "${BLUE}Menguji contoh kode di website/panduan/${NC}"

for md in website/panduan/*.md; do
    # Pecah berkas jadi blok: setiap blok ditulis ke $WORK/blok_N.idk.
    # Hanya pagar ```indonesiaku (bukan indonesiaku-takjalan) yang diambil.
    # Pagar boleh didahului spasi (daftar bernomor) atau '> ' (blockquote);
    # prefiks itu dibuang dari tiap baris kode. Aman karena Indonesiaku tidak
    # punya aturan indentasi sama sekali.
    awk -v dir="$WORK" '
        /^[[:space:]>]*```indonesiaku-takjalan[[:space:]]*$/ { lewati = 1; next }
        /^[[:space:]>]*```indonesiaku[[:space:]]*$/ { n++; ambil = 1; out = dir "/blok_" n ".idk"; next }
        /^[[:space:]>]*```/ {
            if (lewati) { lewati = 0 }
            else if (ambil) { ambil = 0; close(out) }
            next
        }
        ambil {
            baris = $0
            sub(/^[[:space:]]*>[[:space:]]?/, "", baris)
            print baris > out
        }
    ' "$md"

    for blok in "$WORK"/blok_*.idk; do
        [ -e "$blok" ] || continue
        TOTAL=$((TOTAL+1))
        nomor=$(basename "$blok" .idk | sed 's/blok_//')
        label="$(basename "$md")#$nomor"

        stderr_file="$WORK/err.txt"
        $TIMEOUT "$BIN" "$blok" >/dev/null 2>"$stderr_file"
        code=$?
        stderr=$(cat "$stderr_file")

        if [ "$code" -eq 0 ] && [ -z "$stderr" ]; then
            PASSED=$((PASSED+1))
        else
            if [ "$code" -ne 0 ]; then reason="exit=$code"
            else reason="stderr: $(echo "$stderr" | head -1)"; fi
            echo -e "  ${RED}x${NC} $label  ($reason)"
            FAILED=$((FAILED+1)); FAILURES="$FAILURES $label"
        fi
    done
    rm -f "$WORK"/blok_*.idk

    # Hitung yang sengaja dilewati, untuk dilaporkan.
    n_lewati=$(grep -c '^[[:space:]>]*```indonesiaku-takjalan[[:space:]]*$' "$md")
    SKIPPED=$((SKIPPED + n_lewati))
done

echo -e "${BLUE}=================================================${NC}"
echo -e "panduan: ${GREEN}$PASSED/$TOTAL dijalankan bersih${NC}, ${YELLOW}$SKIPPED ditandai takjalan${NC}"
if [ "$FAILED" -eq 0 ]; then
    exit 0
else
    echo -e "${RED}GAGAL: $FAILED contoh di panduan tidak jalan:${NC}"
    for f in $FAILURES; do echo -e "  ${RED}- $f${NC}"; done
    exit 1
fi
