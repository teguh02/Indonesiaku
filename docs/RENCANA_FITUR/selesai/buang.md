# Rencana Fitur: `buang` (del)

- **Status:** selesai
- **Target versi:** 0.8.0
- **Pemilik:** —
- **Ref:** Fase 2 di `docs/ROADMAP.md`

## 1. Masalah

Tidak ada cara menghapus sesuatu. `hapus_kunci(k, "x")` hanya untuk kamus;
menghapus elemen daftar harus `hapus_pada(d, i)` (yang menggeser), dan tidak ada
cara menghapus atribut objek atau variabel sama sekali. Program yang menyalin
`del` dari Python gagal di baris pertama.

## 2. Sintaks usulan

```indonesiaku
buang angka[0]        # hapus elemen daftar
buang kamus["a"]      # hapus kunci kamus
buang obj.atribut     # hapus atribut objek
buang x               # hapus nama
```

`buang` adalah pernyataan (bukan ekspresi) dan tidak memakai `{ }`.

## 3. Semantik

- **Elemen daftar:** indeks negatif dihitung dari belakang; di luar jangkauan →
  `GalatIndeks`. Elemen di kanan bergeser (ukuran berkurang satu).
- **Kunci kamus:** kunci wajib teks dan harus ada; tidak ada → `GalatKunci`.
  Urutan penyisipan kunci lain dipertahankan.
- **Atribut objek:** hanya untuk instans; atribut yang tidak ada →
  `GalatAtribut`; bukan objek → `GalatTipe`.
- **Nama:** global dihapus dari tabel (membacanya lagi = `GalatNama`). Lokal
  dan upvalue diisi penanda internal `VAL_TAKTERISI`, sehingga membacanya
  menimbulkan galat (padanan `UnboundLocalError`), bukan `kosong`.
- Semua galat di atas bisa ditangkap `coba/kecuali`.

## 4. Rencana implementasi

| Bagian | Perubahan |
|---|---|
| `src/scanner.c` | kata kunci `buang` (5 huruf, sebaris dengan `benar`/`bukan`) |
| `src/compiler.c` | `buangStatement` + `deleteVariable` + `buangFromBase`; satu aturan parse kosong |
| `src/vm.c` | `OP_DELETE_GLOBAL/LOCAL/UPVALUE`, `OP_INDEX_DELETE`, `OP_DELETE_PROPERTY`; cek `VAL_TAKTERISI` di `OP_GET_LOCAL`/`OP_GET_UPVALUE`; simpan `galatKunci`/`galatAtribut` di VM |
| `src/chunk.h`, `src/debug.c` | nama opcode |

**GC-safety:** tidak ada alokasi di jalur ini; daftar dan kamus sudah rooted di
stack saat diubah.

## 5. Pengujian

- Golden: `examples/buang.idk` (daftar, kamus, nama global, atribut, galat yang
  ditangkap).
- Tes galat: `tests/galat/buang_kunci_tidak_ada.idk` dan
  `tests/galat/buang_target_salah.idk` (galat kompilasi).
- Regresi: `examples/lingkup_assignment.idk` (pembacaan lokal), seluruh suite.
- `-DDEBUG_STRESS_GC`: keluaran identik.

## 6. Dampak dokumentasi

- [x] `website/panduan/03_PANDUAN_SINTAKS.md`
- [x] `README.md`, `CHANGELOG.md`, `docs/ROADMAP.md`, `AGENTS.md`
- [x] `editors/vscode-indonesiaku/` (kata kunci `buang`)

## 7. Risiko & alternatif

- **Alternatif ditolak:** `buang` sebagai fungsi `buang(wadah, kunci)` — tidak
  bisa menghapus nama, dan tidak sesuai `del`.
- **Batas yang diterima:** belum ada `buang a[1:3]` (irisan) dan belum ada
  penghapusan bersyarat.
