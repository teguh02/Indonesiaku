# Rencana Fitur: Operator `dalam` / `tidak dalam` (in / not in)

- **Status:** selesai (2026-10-24)
- **Target versi:** 0.6.0 (Fase 1)
- **Pemilik:** teguh02

## 1. Masalah

Memeriksa keanggotaan harus lewat fungsi yang berbeda-beda per tipe
(`punya(kamus, k)`, `daftar_berisi(list, x)` dari pustaka, `teks_berisi`).
Ekspresi Python `x in daftar` gagal kompilasi:

```
cetak(2 dalam [1, 2, 3])   # Kesalahan di 'dalam': Harapkan ')' setelah argumen.
```

## 2. Sintaks usulan

```
jika "apel" dalam buah { ... }
jika kunci tidak dalam kamus_data { ... }
cetak("ana" dalam "pisang")      # benar (substring)
cetak(5 dalam rentang(0, 10, 5)) # benar
```

`dalam` sudah kata kunci (dipakai `untuk x dalam ...`); kini juga operator
biner. `tidak dalam` adalah satu operator dua kata, seperti `not in`.

## 3. Semantik

| Kanan | `x dalam kanan` benar bila | Kiri yang tidak cocok |
|---|---|---|
| list | ada elemen `== x` | — (selalu boleh) |
| teks | `x` adalah substring (`""` selalu benar) | bukan teks → galat tipe |
| kamus | `x` adalah kunci | bukan teks → `salah` (kunci kini hanya teks) |
| rentang | `x` bulat, di antara awal..akhir, dan sejajar langkah | bukan angka → `salah` |
| lainnya | — | galat: "Operand kanan 'dalam' harus list, teks, kamus, atau rentang." |

- Presedensi sama dengan perbandingan (`<`, `==`), seperti Python:
  `tidak x dalam d` = `tidak (x dalam d)`; `a + b dalam c` = `(a + b) dalam c`.
- Hasil selalu `benar`/`salah`. Galat dapat ditangkap `coba`.
- Kesetaraan list memakai `==` yang sekarang (identitas untuk list/kamus
  bersarang; berubah otomatis saat K7 diimplementasikan).
- **Belum seperti Python:** rantai perbandingan. Python membaca
  `2 in [1, 2] == True` sebagai `(2 in [1, 2]) and ([1, 2] == True)` → `False`;
  Indonesiaku kini membacanya `(2 dalam [1, 2]) == benar` → `benar`. Ditangani
  bersama butir "rantai perbandingan" di Fase 1.

**Catatan B16:** karena akhir baris belum mengakhiri pernyataan, baris yang
diawali `tidak` setelah sebuah ekspresi kini terbaca sebagai `tidak dalam`
yang belum lengkap (galat kompilasi yang jelas, bukan salah diam-diam).

## 4. Rencana implementasi

| Bagian | Perubahan |
|---|---|
| `src/chunk.h` / `debug.c` | `OP_IN` |
| `src/compiler.c` | aturan infix untuk `TOKEN_DALAM` (`PREC_COMPARISON`) dan infix `TOKEN_TIDAK` yang mewajibkan `dalam` (emit `OP_IN`, `OP_NOT`) |
| `src/vm.c` | handler `OP_IN` per tipe |
| `src/object.c` | `rangeContains` |

`untuk x dalam ...` tidak terpengaruh: nama variabel dibaca dengan
`consume(TOKEN_IDENTIFIER)`, bukan ekspresi.

## 5. Pengujian

- Golden `examples/operator_dalam.idk`: list, teks (termasuk Unicode dan
  `""`), kamus, rentang (langkah negatif), `tidak dalam`, presedensi dengan
  `dan`/`atau`/`tidak`/`+`, di dalam `jika`/`selagi`.
- Galat operand kanan salah (ditangkap `coba`).
- Regresi: semua contoh `untuk ... dalam` tetap jalan.

## 6. Dampak dokumentasi

- [x] `website/panduan/03_PANDUAN_SINTAKS.md` (operator)
- [x] `CHANGELOG.md`, `docs/ROADMAP.md`, `docs/ANALISIS/02` (status)
- [x] `AGENTS.md` (catatan B16 untuk `tidak` infix)
