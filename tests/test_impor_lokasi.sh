#!/usr/bin/env bash
# Modul pustaka/ harus ditemukan dari LOKASI INTERPRETER, bukan dari
# direktori kerja (rencana: docs/RENCANA_FITUR/pustaka_lokasi_interpreter.md).
#
# `indk` disalin ke direktori sementara dalam tiga tata letak instalasi,
# lalu sebuah program dijalankan dari direktori kerja LAIN tanpa INDK_PATH.
# Program itu memakai impor bernama, `dari ... impor`, bentuk teks lama
# `impor "pustaka/teks.idk"`, dan modul lokal yang harus tetap menang atas
# pustaka. Kontrol negatif: tanpa pustaka/ di dekat interpreter, impor gagal.

set -u
cd "$(dirname "$0")/.." || exit 1
REPO=$(pwd)

BIN=./indk
[ -x "$BIN" ] || BIN=./indk.exe
if [ ! -x "$BIN" ]; then
    echo "indk belum dibangun. Jalankan 'make' dulu." >&2
    exit 1
fi
EXE=$(basename "$BIN")

T=$(mktemp -d 2>/dev/null || mktemp -d -t indk-lokasi)
trap 'rm -rf "$T"' EXIT

mkdir -p "$T/kerja"
cat > "$T/kerja/program.idk" <<'EOF'
impor pola
dari koleksi impor Penghitung
impor "pustaka/teks.idk"
impor jalur
cetak(pola.cari_semua("\\d+", "a1b22"))
cetak(Penghitung("aab")["a"])
cetak(teks_balik("abc"))
cetak(jalur.ASAL)
EOF
# Modul lokal bernama sama dengan modul pustaka: harus tetap yang dipakai.
echo 'ASAL = "lokal"' > "$T/kerja/jalur.idk"

HARAP='["1", "22"]
2
cba
lokal'

gagal=0
uji_tata_letak() {    # nama, direktori binari, direktori pustaka
    local nama=$1 bin=$2 pus=$3
    mkdir -p "$bin" "$(dirname "$pus")"
    cp "$BIN" "$bin/$EXE"
    cp -r "$REPO/pustaka" "$pus"
    local keluar
    keluar=$(cd "$T/kerja" && env -u INDK_PATH "$bin/$EXE" program.idk 2>&1 | tr -d '\r')
    if [ "$keluar" = "$HARAP" ]; then
        echo "  ok   $nama"
    else
        echo "  GAGAL $nama"
        echo "$keluar" | sed 's/^/        /'
        gagal=1
    fi
}

echo "Impor pustaka/ dari lokasi interpreter:"
uji_tata_letak "di sebelah binari (build repo)"  "$T/repo"         "$T/repo/pustaka"
uji_tata_letak "bin/ + pustaka/ (installer)"      "$T/app/bin"      "$T/app/pustaka"
uji_tata_letak "bin/ + share/indonesiaku/pustaka" "$T/prefix/bin"   "$T/prefix/share/indonesiaku/pustaka"

# Kontrol negatif: tidak ada pustaka/ di mana pun -> impor tetap gagal.
mkdir -p "$T/tanpa"
cp "$BIN" "$T/tanpa/$EXE"
echo 'impor pola' > "$T/kerja/tanpa.idk"
keluar=$(cd "$T/kerja" && env -u INDK_PATH "$T/tanpa/$EXE" tanpa.idk 2>&1 | tr -d '\r')
if echo "$keluar" | grep -q "Modul 'pola' tidak ditemukan"; then
    echo "  ok   tanpa pustaka/: impor gagal seperti seharusnya"
else
    echo "  GAGAL kontrol negatif: $keluar"
    gagal=1
fi

exit $gagal
