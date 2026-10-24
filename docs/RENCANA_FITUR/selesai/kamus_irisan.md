# Rencana Fitur: Literal kamus, indeks negatif, dan irisan

- **Status:** selesai
- **Target versi:** 0.6.0 (Fase 1)
- **Keputusan terkait:** K4 (kurung kurawal), 02 §B

## 1. Masalah

Tiga kekurangan yang membuat kode terasa jauh dari Python:

1. Kamus hanya bisa dibuat lewat `kamus()` lalu diisi satu per satu:
   ```idk
   variabel d = kamus()
   d["nama"] = "Budi"
   d["umur"] = 25
   ```
2. Elemen terakhir harus dihitung manual: `a[panjang(a) - 1]`.
3. Mengambil sebagian urutan butuh builtin berbeda per tipe
   (`potong()` untuk teks, `daftar_iris()` dari pustaka untuk daftar).

## 2. Sintaks

```idk
# Literal kamus
variabel orang = {"nama": "Budi", "umur": 25}
variabel kosong_dict = {}
variabel bersarang = {
    "a": [1, 2],
    "b": {"c": 3},
}

# Indeks negatif
cetak(a[-1])        # elemen terakhir
cetak("halo"[-2])   # 'l'

# Irisan
cetak(a[1:3])       # elemen 1..2
cetak(a[:2])        # dari awal
cetak(a[2:])        # sampai akhir
cetak(a[::2])       # setiap 2 elemen
cetak(a[::-1])      # dibalik
cetak("halo"[1:4])  # irisan teks
```

### Disambiguasi `{`

Indonesiaku memakai `{ }` sebagai blok gaya PHP (K4), jadi `{` ambigu.
Aturan yang dipakai, mengikuti posisi parser:

- **Posisi statement** → `{` selalu membuka **blok**.
- **Posisi ekspresi** (aturan prefix Pratt) → `{` selalu membuka **literal kamus**.

Konsekuensi: `{"a": 1}` sebagai pernyataan berdiri sendiri bukan kamus (akan
dibaca sebagai blok dan gagal). Tulis `x = {"a": 1}` atau `cetak({"a": 1})`.
Ini batasan yang diterima — kamus yang dibuang hasilnya tidak berguna.

Karena `{` tidak menaikkan `groupingDepth` (blok butuh token newline),
literal kamus melewati `TOKEN_NEWLINE` secara eksplisit saat mengurai
pasangan, sehingga literal multi-baris tetap bisa ditulis.

## 3. Semantik

### Literal kamus
- `{}` menghasilkan kamus kosong; sama dengan `kamus()`.
- Kunci harus bernilai **teks** saat dievaluasi (batasan kamus saat ini);
  kunci bukan teks → `GalatTipe`.
- Urutan penyisipan dipertahankan (`ObjDict.keys`), sama seperti `dictSet`.
- Kunci duplikat: yang terakhir menang, posisi urutan mengikuti penyisipan pertama.
- Koma di akhir diperbolehkan.

### Indeks negatif
- Berlaku untuk **daftar**, **teks**, dan **rentang**: indeks `i < 0` menjadi
  `i + panjang`. Setelah penyesuaian, di luar jangkauan tetap `GalatIndeks`.
- Berlaku juga pada `a[-1] = x` (`OP_INDEX_SET`) untuk daftar.
- **Tidak** berlaku untuk `kamus[angka]` (itu akses kunci ke-n menurut urutan
  penyisipan, bukan urutan yang punya arti negatif) — biarkan apa adanya.
- `untuk` memakai pencacah 0..n-1, jadi tidak terpengaruh.

### Irisan
- Bentuk: `a[awal:akhir]`, `a[awal:akhir:langkah]`; tiap bagian boleh kosong.
- Mengikuti Python: indeks negatif dihitung dari belakang, batas di-*clamp*,
  irisan di luar jangkauan menghasilkan hasil kosong (bukan galat).
- `langkah` negatif membalik arah; `langkah == 0` → `GalatNilai`.
- Berlaku untuk **daftar** (menghasilkan daftar baru, salinan dangkal) dan
  **teks** (menghasilkan teks baru, per karakter UTF-8).
- Rentang/kamus yang diiris → `GalatTipe`.
- Irisan hanya untuk dibaca; `a[1:3] = ...` ditolak saat kompilasi.

## 4. Rencana implementasi

| Bagian | Perubahan |
|---|---|
| `src/scanner.c` | tidak ada token baru; `TOKEN_COLON` sudah ada |
| `src/compiler.c` | `dictLiteral()` sebagai aturan prefix `TOKEN_LEFT_BRACE`; `subscript()` mengenali `:` dan memancarkan `OP_SLICE` |
| `src/chunk.h`, `src/debug.c` | `OP_BUILD_DICT`, `OP_SLICE` |
| `src/vm.c` | handler kedua opcode; penyesuaian indeks negatif di `OP_INDEX_GET`/`OP_INDEX_SET` |
| `src/object.c` | tidak ada tipe baru (pakai `ObjDict`, `ObjList`, `ObjString`) |

**GC-safety:** `OP_BUILD_DICT` menyimpan kamus di stack selama `dictSet`
(dapat menumbuhkan tabel + array kunci); `OP_SLICE` menyimpan daftar hasil di
stack selama `writeValueArray`, dan menjaga sumber tetap di stack selama
`copyString` untuk irisan teks.

Bagian yang kosong pada irisan dipancarkan sebagai `OP_KOSONG` sehingga VM
membedakan "tidak diisi" dari nilai nyata; `kosong` eksplisit juga sah dan
berarti bawaan, sama seperti `None` di Python.

## 5. Pengujian

Golden `examples/kamus_irisan.idk` + `tests/expected/kamus_irisan.out`:
- literal kamus satu baris, multi-baris, kosong, koma akhir, kunci duplikat;
- urutan penyisipan dipertahankan saat diiterasi `untuk`;
- `a[-1]`, `a[-panjang(a)]`, penugasan `a[-1] = x`, teks negatif;
- irisan daftar & teks: penuh, tanpa awal, tanpa akhir, langkah, langkah
  negatif, batas melewati ujung, hasil kosong;
- irisan menghasilkan salinan (mengubah hasil tidak mengubah sumber).

Tes galat:
- `tests/galat/irisan_langkah_nol.idk` — `a[::0]`;
- `tests/galat/kamus_kunci_bukan_teks.idk` — `{1: "x"}`;
- `tests/galat/indeks_negatif_jauh.idk` — `a[-99]`.

Plus: suite penuh, `-DDEBUG_STRESS_GC`, ASan+UBSan, build C11.

## 6. Dampak dokumentasi

- [x] `website/panduan/03_PANDUAN_SINTAKS.md`
- [x] `README.md`
- [x] `CHANGELOG.md` + `docs/ROADMAP.md` + `docs/ANALISIS/02_PETA_FITUR_PYTHON.md`
- [x] `AGENTS.md` (jebakan `{` ambigu + prescan)
- [x] `editors/vscode-indonesiaku/` (grammar/snippet literal kamus)

## 7. Risiko & alternatif

**Risiko utama:** `prescanFunctionBody()` dan `skipBalancedBraces()` menghitung
`{`/`}` mentah dari aliran token. Literal kamus menambah pasangan brace
seimbang, sehingga keduanya tetap benar secara hitungan — tetapi prescan juga
memakai `depth == 1` untuk mengenali `variabel` tingkat badan fungsi, dan
sebuah literal kamus menaikkan depth. Itu aman (hanya membuat prescan
melewatkan `variabel` di dalam literal, yang memang mustahil), namun harus
diuji dengan fungsi yang berisi literal kamus multi-baris.

**Alternatif yang ditolak:** memakai kata kunci pembuka (`kamus {`) untuk
menghindari ambiguitas — ditolak karena menjauh dari Python tanpa imbalan,
sementara aturan posisi statement/ekspresi sudah cukup dan tidak ambigu bagi
parser satu lintasan.
