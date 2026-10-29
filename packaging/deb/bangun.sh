#!/usr/bin/env bash
# Membangun paket .deb dari biner indk yang sudah dibangun.
#
# Layout sengaja mengikuti findInInterpreterRoots() di src/sistem.c:
#   /usr/bin/indk                       <- biner, langsung di PATH
#   /usr/share/indonesiaku/pustaka      <- <exe>/../share/indonesiaku/pustaka
# Tanpa pustaka/ yang ikut terpasang, `impor web` gagal dengan
# "Modul 'web' tidak ditemukan" — biner telanjang TIDAK cukup.
#
# Pemakaian: packaging/deb/bangun.sh <versi> <arsitektur> <biner> <keluaran.deb>
set -eu

VERSI="${1:?versi, contoh 1.4.0}"
ARCH="${2:-amd64}"
BINER="${3:-indk}"
KELUARAN="${4:-indonesiaku_${VERSI}_${ARCH}.deb}"

AKAR="$(cd "$(dirname "$0")/../.." && pwd)"
cd "$AKAR"

if [ ! -x "$BINER" ]; then
    echo "GAGAL: biner '$BINER' tidak ada atau tidak dapat dieksekusi." >&2
    exit 1
fi

# Versi paket HARUS sama dengan INDK_VERSION di dalam biner. Kalau tidak,
# `dpkg -l` dan `indk -v` melaporkan angka berbeda, dan --periksa-pembaruan
# (yang membaca INDK_VERSION) menyimpulkan hal yang salah.
VERSI_BINER="$("./$BINER" -v 2>/dev/null | tr -d '[:space:]')" || VERSI_BINER=""
if [ -z "$VERSI_BINER" ]; then
    VERSI_BINER="$("$BINER" -v 2>/dev/null | tr -d '[:space:]')" || VERSI_BINER=""
fi
if [ -n "$VERSI_BINER" ] && [ "$VERSI_BINER" != "$VERSI" ]; then
    echo "GAGAL: versi paket '$VERSI' != INDK_VERSION biner '$VERSI_BINER'." >&2
    echo "Perbarui INDK_VERSION di src/common.h atau perbaiki tag rilis." >&2
    exit 1
fi

STAGING="$(mktemp -d)"
bersihkan() { rm -rf "$STAGING"; }
trap bersihkan EXIT

mkdir -p "$STAGING/DEBIAN"
mkdir -p "$STAGING/usr/bin"
mkdir -p "$STAGING/usr/share/indonesiaku"
mkdir -p "$STAGING/usr/share/doc/indonesiaku"

install -m 0755 "$BINER" "$STAGING/usr/bin/indk"
cp -r pustaka "$STAGING/usr/share/indonesiaku/"
cp -r examples "$STAGING/usr/share/indonesiaku/"
for f in README.md CHANGELOG.md LICENSE; do
    [ -f "$f" ] && cp "$f" "$STAGING/usr/share/doc/indonesiaku/"
done

# Ukuran terpasang dalam KB, dibulatkan ke atas (dipakai apt untuk estimasi).
UKURAN="$(du -sk "$STAGING" | cut -f1)"

cat > "$STAGING/DEBIAN/control" <<EOF
Package: indonesiaku
Version: $VERSI
Section: devel
Priority: optional
Architecture: $ARCH
Maintainer: Teguh Rijanandi <teguhrijanandi02@gmail.com>
Installed-Size: $UKURAN
Homepage: https://github.com/teguh02/Indonesiaku-Lang
Description: Bahasa pemrograman Indonesiaku
 Interpreter bytecode VM untuk bahasa pemrograman Indonesiaku:
 kata kunci berbahasa Indonesia, semantik mengacu Python, blok
 memakai kurung kurawal tanpa aturan indentasi.
 .
 Memasang biner indk beserta pustaka standarnya. Memasang paket
 versi baru di atas yang lama akan menimpanya; versi lama tidak
 perlu dihapus lebih dulu.
EOF

# dpkg memerlukan kepemilikan root:root di dalam paket. fakeroot
# menyediakannya tanpa menjalankan build sebagai root sungguhan.
if command -v fakeroot >/dev/null 2>&1; then
    fakeroot dpkg-deb --build "$STAGING" "$KELUARAN"
else
    dpkg-deb --build "$STAGING" "$KELUARAN"
fi

echo "OK: $KELUARAN"
dpkg-deb --info "$KELUARAN" | sed -n '1,12p'
