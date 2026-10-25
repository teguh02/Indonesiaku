# Rencana Fitur: Sisa Fase 2 — tupel tanpa kurung, himpunan_beku, dan 5 builtin

- **Status:** selesai
- **Target versi:** 0.8 (menutup Fase 2)
- **Keputusan terkait:** K2, K8, K17

## 1. Masalah

`docs/ROADMAP.md` menandai Fase 2 "baru sebagian selesai" dengan tujuh sisa
yang didaftar eksplisit. Semuanya kecil-kecil tetapi saling berkaitan, jadi
dikerjakan sebagai satu paket:

1. **tupel tanpa kurung** — `x = 1, 2` dan `kembalikan a, b` (K17, belum
   diputuskan sebelumnya).
2. **`himpunan_beku`** — himpunan yang dapat di-hash, jadi bisa jadi kunci
   kamus atau anggota himpunan lain.
3. **`hash(x)`** — mengekspos `hashValue()` yang sudah ada ke bahasa.
4. **`iter(x)` / `lanjutkan(it)`** — protokol iterator eksplisit.
5. **`turunan_dari(a, b)`** — `issubclass`, pasangan `adalah_instansi`.
6. **`ke_desimal(x)`** — `float()`; sejak K2 `ke_angka("3")` memberi bulat,
   sehingga tidak ada cara memaksa desimal.
7. **`daftar(x)`** — `list()`; `tupel()` dan `himpunan()` sudah ada, hanya
   konversi ke daftar yang hilang.

## 2. Sintaks

```indonesiaku
x = 1, 2                 # tupel tanpa kurung → (1, 2)
y = 1,                   # tupel satu elemen
fungsi f() { kembalikan 1, 2 }

variabel b = himpunan_beku([1, 2])
variabel d = {b: "bisa jadi kunci"}

cetak(hash("a") == hash("a"))
cetak(turunan_dari(GalatTipe, Galat))
cetak(ke_desimal("3"), ke_desimal(3))
cetak(daftar("abc"), daftar((1, 2)))

variabel it = iter([1, 2])
cetak(lanjutkan(it), lanjutkan(it))
cetak(lanjutkan(it, "habis"))
```

`next` dinamai **`lanjutkan`** — `lanjut` sudah menjadi kata kunci (continue),
dan builtin bernama `next` akan janggal di bahasa berkata kunci Indonesia.

## 3. Semantik

### tupel tanpa kurung

Koma setelah sebuah nilai membentuk tupel **hanya** pada dua posisi: sisi
kanan penugasan di tingkat pernyataan, dan `kembalikan`. Di tempat lain koma
sudah bermakna lain (argumen panggilan, elemen daftar, pasangan kamus) dan
tidak boleh berubah arti.

Dua penjaga dipakai bersamaan:

- Flag kompiler `allowTupleRHS`, dipasang hanya oleh `expressionStatement()`
  dan `returnStatement()`.
- `scannerGroupingDepth() == 0`, sehingga di dalam `(`/`[` koma tetap berarti
  pemisah. Ini yang menjaga `f(x = 1, y)` dan `[a, b]` tidak berubah.

`a, b = 1, 2` tetap ditangani `unpackStatement()` yang sudah ada — jalur itu
lebih dulu diperiksa, jadi perilakunya tidak berubah.

### himpunan_beku

`ObjSet` mendapat satu field `bool frozen`. **Tidak** ada tipe objek baru:
tata letaknya harus tetap kompatibel dengan `ObjDict` (dijaga
`_Static_assert`), dan menambah tipe berarti menyentuh setiap `switch`
`ObjType` tanpa manfaat nyata.

- `hashValue` menerima himpunan beku (hash gabungan anggota, bebas urutan
  karena himpunan tidak berurut secara semantik), menolak yang mutable —
  persis Python.
- Semua jalur tulis (`tambah`, `hapus`, `buang`) menolak himpunan beku.
- Operator `| & - ^` bekerja untuk keduanya; hasilnya beku bila **kedua**
  operand beku, meniru Python.
- `jenis()` memulangkan `"himpunan_beku"`.

### Builtin lain

- `hash(x)` → bulat; `GalatTipe` untuk nilai tak dapat di-hash.
- `turunan_dari(a, b)` → logika; keduanya wajib kelas (`GalatTipe`). Sebuah
  kelas adalah turunan dari dirinya sendiri, seperti Python.
- `ke_desimal(x)` → desimal dari bulat/desimal/teks/logika; `GalatNilai`
  untuk teks yang bukan angka.
- `daftar(x)` → daftar baru dari daftar/tupel/himpunan/teks/rentang/kamus
  (kuncinya). Selalu salinan baru, termasuk saat argumennya daftar.
- `iter(x)` → `OBJ_ITERATOR` (tipe objek baru, memang perlu karena harus
  menyimpan posisi). `lanjutkan(it)` menaikkan `HentiIterasi` saat habis;
  `lanjutkan(it, bawaan)` memulangkan `bawaan` alih-alih menaikkan galat.
  Kelas `HentiIterasi` sudah terdaftar tetapi belum pernah dipakai — baru
  sekarang ada yang menaikkannya, jadi perlu field di `vm`.

Iterator memotret sumbernya lewat `iterCount`/`iterAt` yang sudah ada.
Mengubah daftar saat iterasi berjalan **tidak** didefinisikan ketat: panjang
dibaca ulang setiap `lanjutkan`, jadi menyusutkan daftar mengakhiri iterasi
lebih awal, bukan membaca di luar batas.

## 4. Berkas tersentuh

- `src/object.h` / `src/object.c` — `frozen` di `ObjSet`, `OBJ_ITERATOR`,
  `newIterator`, `newFrozenSet`.
- `src/memory.c` — tandai dan bebaskan iterator.
- `src/value.c` — `hashValue` untuk himpunan beku; `valuesEqual` membandingkan
  himpunan beku dengan himpunan biasa secara struktural (seperti Python).
- `src/vm.c` — daftarkan 6 native + `vm.hentiIterasi`; hasil operator himpunan
  ikut beku; tolak tulis ke himpunan beku; `jenis()`.
- `src/native.c` / `src/native.h` — enam native baru.
- `src/scanner.c` / `src/scanner.h` — `scannerGroupingDepth()`.
- `src/compiler.c` — `allowTupleRHS` + koma pembentuk tupel.
- `examples/` + `tests/expected/` + `tests/galat/` — golden dan tes galat.
- `website/panduan/03_PANDUAN_SINTAKS.md`, `07_PUSTAKA_STANDAR.md`.
- `editors/vscode-indonesiaku/` — BUILTINS + tmLanguage + snippet.
- `CHANGELOG.md`, `README.md`, `docs/ROADMAP.md`,
  `docs/ANALISIS/02_PETA_FITUR_PYTHON.md`, `AGENTS.md`.

## 5. Rencana uji

- Golden `examples/sisa_fase2.idk` — ketujuh fitur.
- Tes galat: himpunan beku ditulis, `hash` pada daftar, `turunan_dari` bukan
  kelas, `ke_desimal` teks bukan angka, `lanjutkan` setelah habis.
- Kasus batas wajib: `x = 1,` (satu elemen); `f(x = 1, y)` **tidak** berubah;
  `[a, b]` dan `{"a": 1, "b": 2}` tidak berubah; `a, b = 1, 2` tetap unpack;
  himpunan beku sebagai kunci kamus dan anggota himpunan lain; `himpunan ==
  himpunan_beku` dengan isi sama.
- Sanitizer wajib: menyentuh alokasi dan tipe objek baru — build
  `-DDEBUG_STRESS_GC -fsanitize=address,undefined`, jalankan golden **dan**
  `tests/galat/` terhadap biner itu.
- Golden yang berubah dibaca satu per satu.

## 6. Dampak dokumentasi & tooling

- [x] `website/panduan/03_PANDUAN_SINTAKS.md`
- [x] `website/panduan/07_PUSTAKA_STANDAR.md`
- [x] `editors/vscode-indonesiaku/extension.js` BUILTINS
- [x] `editors/vscode-indonesiaku/syntaxes/indonesiaku.tmLanguage.json`
- [x] `editors/vscode-indonesiaku/snippets/indonesiaku.code-snippets`
- [x] `README.md`
- [x] `CHANGELOG.md` + `docs/ROADMAP.md` + `ANALISIS/02_PETA_FITUR_PYTHON.md`
- [x] `AGENTS.md`
- [x] Berkas rencana ini → `docs/RENCANA_FITUR/selesai/`

## 6b. Hasil nyata (diisi setelah selesai)

- Suite 150/150 (sebelumnya 144); 1 golden + 5 tes galat baru.
- Sanitizer ASan+UBSan+`DEBUG_STRESS_GC` atas 144 fixture: **nol temuan**.
  Stress-GC terpisah (150 himpunan beku sebagai kunci kamus, 100 iterator,
  60 penggabungan beku, 200 tupel-kembalian) juga bersih.
- **Temuan di luar rencana:** 35 builtin lama tidak pernah ter-highlight di
  `tmLanguage.json` — termasuk `rentang`, `terurut`, `himpunan`, `tupel`,
  `repr`, dan seluruh `db_*`. Daftar builtin editor kini **dibangkitkan** dari
  `defineNative()` di `src/vm.c`, diurutkan nama terpanjang dulu agar
  `himpunan_beku` tidak kalah oleh `himpunan` dalam alternasi regex.
- Dua golden lama berubah (`kamus_kunci`, `galat/kamus_kunci_tidak_hashable`):
  murni karena pesan "dapat di-hash" kini menyebut `himpunan_beku`. Diperiksa
  satu per satu, nol perubahan perilaku.
- `variabel a, b = ...` tetap tidak didukung (di luar lingkup); `a, b = ...`
  adalah bentuk yang benar dan sudah ada sejak `penugasan_berganda`.

## 7. Risiko & alternatif

- **Risiko terbesar: koma pembentuk tupel mengubah arti kode lama.** Karena
  itu dua penjaga sekaligus, dan kasus batas di atas diuji eksplisit.
- `himpunan_beku` sebagai field, bukan tipe objek baru: menambah `OBJ_FROZENSET`
  berarti menyentuh setiap `switch` `ObjType` (GC, cetak, `jenis`, `==`, hash)
  hanya untuk membedakan satu bit perilaku.
- Iterator **memang** tipe objek baru: ia menyimpan posisi, tidak ada struktur
  lama yang bisa dipakai ulang tanpa memalsukannya.
- Dipertimbangkan lalu ditolak: koma pembentuk tupel di sembarang ekspresi
  (seperti Python penuh). Itu membuat `f(a, b)` ambigu dan menuntut penulisan
  ulang `argumentList`, `dictLiteral`, dan literal daftar — tidak sepadan.
