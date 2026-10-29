#!/usr/bin/env bash
# Membuktikan paket .deb benar-benar dapat dipasang DAN di-upgrade di atas
# versi lama tanpa menghapusnya lebih dulu.
#
# Tanpa uji ini, klaim "tidak perlu uninstall versi lama" hanyalah teks di
# dalam pesan --periksa-pembaruan.
#
# Dilewati bila dpkg-deb/dpkg tidak ada (mis. runner macOS/Windows), atau
# bila tidak dijalankan sebagai root -- dpkg -i butuh root.
set -u

AKAR="$(cd "$(dirname "$0")/.." && pwd)"
cd "$AKAR" || exit 1

if ! command -v dpkg-deb >/dev/null 2>&1 || ! command -v dpkg >/dev/null 2>&1; then
    echo "ok   lewat: dpkg tidak tersedia di sistem ini"
    exit 0
fi
if [ "$(id -u)" -ne 0 ]; then
    echo "ok   lewat: butuh root untuk dpkg -i"
    exit 0
fi
if dpkg -l indonesiaku 2>/dev/null | grep -q '^ii'; then
    echo "ok   lewat: paket indonesiaku sudah terpasang di mesin ini"
    exit 0
fi

VERSI="$(./indk -v | tr -d '[:space:]')"
TMP="$(mktemp -d)"
GAGAL=0

bersihkan() {
    dpkg -r indonesiaku >/dev/null 2>&1
    rm -rf "$TMP"
}
trap bersihkan EXIT

periksa() {
    local nama="$1" harap="$2" dapat="$3"
    if [ "$harap" = "$dapat" ]; then
        echo "  ok   $nama"
    else
        echo "  GAGAL $nama: harap '$harap', dapat '$dapat'"
        GAGAL=1
    fi
}

# Dua paket dengan versi berbeda dari biner yang sama. Versi "lama" memakai
# nomor yang pasti lebih kecil; isinya identik, yang diuji adalah mekanisme
# upgrade dpkg, bukan perbedaan fiturnya.
if ! bash packaging/deb/bangun.sh "$VERSI" amd64 indk "$TMP/baru.deb" >/dev/null 2>&1; then
    echo "GAGAL: tidak dapat membangun paket .deb"
    exit 1
fi

# Paket "lama": versi dipalsukan lewat salinan biner agar guard versi lolos.
# Guard membandingkan `indk -v`, jadi paket lama memakai versi biner yang sama
# tetapi nomor paket 0.0.1 akan ditolak guard -- karena itu paket lama dibuat
# dengan melewati skrip dan menyusun control sendiri.
mkdir -p "$TMP/lama/DEBIAN" "$TMP/lama/usr/bin" "$TMP/lama/usr/share/indonesiaku"
install -m 0755 indk "$TMP/lama/usr/bin/indk"
cp -r pustaka "$TMP/lama/usr/share/indonesiaku/"
cat > "$TMP/lama/DEBIAN/control" <<EOF
Package: indonesiaku
Version: 0.0.1
Section: devel
Priority: optional
Architecture: amd64
Maintainer: Teguh Rijanandi <teguhrijanandi02@gmail.com>
Description: Bahasa pemrograman Indonesiaku (paket uji versi lama)
EOF
if command -v fakeroot >/dev/null 2>&1; then
    fakeroot dpkg-deb --build "$TMP/lama" "$TMP/lama.deb" >/dev/null 2>&1
else
    dpkg-deb --build "$TMP/lama" "$TMP/lama.deb" >/dev/null 2>&1
fi

# 1. Pasang versi lama.
dpkg -i "$TMP/lama.deb" >/dev/null 2>&1
periksa "pasang versi lama" "0.0.1" "$(dpkg-query -W -f='${Version}' indonesiaku 2>/dev/null)"

# 2. Stdlib harus terjangkau dari direktori mana pun, bukan hanya dari cwd
#    repo -- ini yang gagal bila pustaka/ tidak ikut terpasang.
printf 'impor json\ncetak("stdlib ok")\n' > "$TMP/u.idk"
periksa "stdlib terpasang" "stdlib ok" "$(cd / && indk "$TMP/u.idk" 2>&1)"

# 3. INTI: pasang versi baru DI ATAS yang lama, tanpa dpkg -r lebih dulu.
KELUARAN_UPGRADE="$(dpkg -i "$TMP/baru.deb" 2>&1)"
periksa "upgrade tanpa uninstall" "0" "$?"
if ! printf '%s' "$KELUARAN_UPGRADE" | grep -q 'over (0.0.1)'; then
    echo "  GAGAL upgrade bukan in-place: dpkg tidak melaporkan 'over (0.0.1)'"
    GAGAL=1
else
    echo "  ok   dpkg melaporkan upgrade in-place"
fi

periksa "versi berubah" "$VERSI" "$(dpkg-query -W -f='${Version}' indonesiaku 2>/dev/null)"
periksa "biner cocok paket" "$VERSI" "$(indk -v | tr -d '[:space:]')"
periksa "stdlib tetap jalan" "stdlib ok" "$(cd / && indk "$TMP/u.idk" 2>&1)"

# 4. Tidak ada berkas yatim dari versi lama.
TERDAFTAR="$(dpkg -L indonesiaku | grep -c '/usr/share/indonesiaku/pustaka/.*\.idk$')"
NYATA="$(find /usr/share/indonesiaku/pustaka -name '*.idk' | wc -l)"
periksa "tanpa berkas yatim" "$TERDAFTAR" "$NYATA"

dpkg -r indonesiaku >/dev/null 2>&1
trap - EXIT
rm -rf "$TMP"

if [ "$GAGAL" -eq 0 ]; then
    echo "ok   paket .deb dapat dipasang dan di-upgrade tanpa uninstall"
    exit 0
fi
echo "GAGAL: mekanisme paket/upgrade tidak bekerja seperti seharusnya"
exit 1
