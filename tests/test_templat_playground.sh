#!/usr/bin/env bash
# Memastikan setiap template playground benar-benar berjalan (exit 0, stderr kosong).
# Template yang membusuk = contoh rusak yang dipamerkan ke pengunjung situs.
set -u

AKAR="$(cd "$(dirname "$0")/.." && pwd)"
INDK="${INDK_BIN:-$AKAR/indk}"
JS="$AKAR/website/assets/templat-playground.js"

[ -x "$INDK" ] || { echo "LEWAT: indk belum dibangun ($INDK)"; exit 0; }

TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

# Pecah berkas JS menjadi satu .idk per template.
python3 - "$JS" "$TMP" <<'PY'
import re, sys, pathlib
js = pathlib.Path(sys.argv[1]).read_text()
out = pathlib.Path(sys.argv[2])
pola = r'id:\s*"([^"]+)",\s*\n\s*label:\s*"[^"]+",\s*\n\s*kode:\s*\n`(.*?)`\s*\n\s*\}'
n = 0
for tid, kode in re.findall(pola, js, re.S):
    (out / f"{tid}.idk").write_text(kode)
    n += 1
print(n)
PY

JUMLAH=$(ls -1 "$TMP"/*.idk 2>/dev/null | wc -l)
if [ "$JUMLAH" -eq 0 ]; then
  echo "GAGAL: tidak ada template terbaca dari $JS"
  exit 1
fi

LOLOS=0
GAGAL=0
for f in "$TMP"/*.idk; do
  nama="$(basename "$f" .idk)"
  # Template "masukan" membaca stdin; yang lain tidak terpengaruh umpan ini.
  keluaran="$(printf 'Teguh\n21\n' | "$INDK" "$f" 2>"$TMP/err.$nama")"
  kode=$?
  galat="$(cat "$TMP/err.$nama")"
  if [ $kode -eq 0 ] && [ -z "$galat" ] && [ -n "$keluaran" ]; then
    echo "  ok   $nama"
    LOLOS=$((LOLOS + 1))
  else
    echo "  GAGAL $nama (exit=$kode)"
    [ -n "$galat" ] && echo "        $galat"
    GAGAL=$((GAGAL + 1))
  fi
done

echo "TEMPLAT PLAYGROUND: $LOLOS/$JUMLAH berjalan"
[ $GAGAL -eq 0 ]
