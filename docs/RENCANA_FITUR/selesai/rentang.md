# Rencana Fitur: `rentang()` (range)

- **Status:** selesai (2026-10-24)
- **Target versi:** 0.6.0 (Fase 1)
- **Pemilik:** teguh02
- **Keputusan terkait:** K9 (protokol iterasi), disetujui

## 1. Masalah

Perulangan angka — hal pertama di setiap tutorial Python — harus ditulis
manual:

```
i = 0
selagi i < 10 {
    cetak(i)
    i += 1
}
```

`untuk i dalam rentang(10)` menghasilkan `Variabel tidak terdefinisi 'rentang'`.

## 2. Sintaks usulan

```
untuk i dalam rentang(5) { cetak(i) }          # 0 1 2 3 4
untuk i dalam rentang(2, 6) { cetak(i) }       # 2 3 4 5
untuk i dalam rentang(10, 0, -3) { cetak(i) }  # 10 7 4 1
r = rentang(0, 10, 2)
cetak(r, panjang(r), r[1])                     # rentang(0, 10, 2) 5 2
cetak(4 dalam r)                               # benar (lihat operator_dalam.md)
untuk huruf dalam "café" { cetak(huruf) }      # c a f é (ikut dikerjakan)
```

## 3. Semantik

- `rentang(akhir)`, `rentang(awal, akhir)`, `rentang(awal, akhir, langkah)`;
  `awal` bawaan 0, `langkah` bawaan 1.
- Hasilnya **objek rentang yang malas** (tidak membuat list), seperti
  Python 3: `rentang(10000000)` tidak memakan memori sebanding jumlahnya.
- Panjang: `max(0, ceil((akhir - awal) / langkah))`.
- `r[i]` = `awal + i * langkah`; indeks di luar jangkauan → galat (dapat
  ditangkap). Indeks negatif menyusul bersama indeks negatif list (Fase 1).
- Dapat diiterasi `untuk`, diukur `panjang()`, diuji `dalam`, dan bernilai
  salah bila kosong (K1).
- Dicetak `rentang(0, 10, 2)`; langkah 1 dicetak `rentang(0, 5)` (seperti
  Python).
- `jenis(r)` → `"rentang"`.
- Galat (dapat ditangkap): argumen bukan bilangan bulat, jumlah argumen bukan
  1–3, `langkah` = 0.
- Sampai tipe `bulat` ada (K2), argumen berupa angka desimal yang bernilai
  bulat (`3.0`) diterima.

## 4. Rencana implementasi

| Bagian | Perubahan |
|---|---|
| `src/object.h` / `object.c` | `OBJ_RANGE`, `ObjRange {start, stop, step}`, `newRange`, `rangeLength`, cetak |
| `src/memory.c` | bebaskan objek (tidak ada referensi untuk ditandai) |
| `src/native.c` | `rentang()`; `panjang()` dan `jenis()` mengenali rentang |
| `src/vm.c` | `OP_LIST_LEN` dan `OP_INDEX_GET` menerima rentang (dipakai desugar `untuk`); `isFalsey` |

`untuk` sudah di-desugar menjadi perulangan indeks (`OP_LIST_LEN` +
`OP_INDEX_GET`), jadi compiler tidak berubah. Sekalian, `OP_LIST_LEN`
menerima teks sehingga `untuk huruf dalam "abc"` berfungsi (bagian dari K9).

## 5. Pengujian

- Golden `examples/rentang.idk`: tiga bentuk argumen, langkah negatif,
  rentang kosong, `hentikan`/`lanjut`, rentang bersarang, cetak, panjang,
  indeks, iterasi teks, galat argumen (ditangkap `coba`).
- `rentang(1000000)` di dalam `untuk` (memori tetap kecil).
- `-DDEBUG_STRESS_GC`.

## 6. Dampak dokumentasi

- [x] `website/panduan/03_PANDUAN_SINTAKS.md` (perulangan `untuk`)
- [x] `website/panduan/07_PUSTAKA_STANDAR.md`
- [x] `CHANGELOG.md`, `docs/ROADMAP.md`, `docs/ANALISIS/02` (status)
