# Rencana Fitur: `bulat` (bilangan bulat int64)

- **Status:** selesai
- **Target versi:** 0.8.0
- **Pemilik:** —
- **Ref:** K2, K11 (nama tipe), Fase 2 di `docs/ROADMAP.md`

## 1. Masalah

Sekarang semua angka adalah `double`. Akibatnya:

```indonesiaku
cetak(2 ** 53 + 1)        # 9007199254740992 — salah satu hilang
cetak(7 / 2)              # 3.5        (benar)
cetak(6 / 2)              # 3          (di Python 3.0)
cetak(10 ** 18 + 1)       # presisi hilang di atas 2^53
```

Python membedakan `int` (presisi tak terbatas) dan `float`. Program yang
disalin dari tutorial Python karena itu memberi hasil berbeda, dan operasi
bitwise (Fase 2) tidak mungkin benar tanpa angka bulat.

## 2. Sintaks usulan

Tidak ada sintaks baru. Literal tanpa titik/eksponen menjadi `bulat`, dengan
titik atau eksponen menjadi `desimal`:

```indonesiaku
cetak(7)         # 7        (bulat)
cetak(7.0)       # 7.0      (desimal)
cetak(1e3)       # 1000.0   (desimal) — literal eksponen menyusul di butir lain
```

## 3. Semantik

Mengikuti Python 3:

| Ekspresi | Hasil | Tipe |
|---|---|---|
| `7 + 2`, `7 - 2`, `7 * 2` | `9`, `5`, `14` | bulat |
| `7 / 2` | `3.5` | desimal — `/` **selalu** desimal |
| `6 / 2` | `3.0` | desimal |
| `7 // 2`, `-7 // 2` | `3`, `-4` | bulat bila kedua operand bulat (floor) |
| `7 % 3`, `-7 % 3` | `1`, `2` | bulat, tanda mengikuti pembagi |
| `2 ** 10` | `1024` | bulat bila pangkat ≥ 0 dan kedua operand bulat |
| `2 ** -1` | `0.5` | desimal |
| `1 + 2.0` | `3.0` | desimal (salah satu desimal → desimal) |
| `1 == 1.0` | `benar` | perbandingan numerik lintas tipe |
| `1 adalah 1.0` | `salah` | `adalah` = identitas, tipe berbeda |
| `cetak(3.0)` | `3.0` | desimal selalu menampilkan `.0` |

- **Overflow:** `int64` yang meluap menaikkan `GalatLuapan` (subkelas
  `GalatAritmetika`, jadi bisa ditangkap `kecuali GalatAritmetika sebagai e`).
  Tidak pernah diam-diam menjadi `desimal` atau `nan`. Bignum menyusul di
  Fase 2b (`ObjBigInt`).
- **Literal terlalu besar** untuk int64 → galat kompilasi yang menyebut
  batasnya, bukan pembulatan diam-diam.
- **Pembagian dengan nol** tetap `GalatBagiNol` untuk `/`, `//`, `%`.
- **Indeks/irisan/`rentang()`** menerima `bulat` dan `desimal` utuh seperti
  sebelumnya; nilai pecahan tetap dipotong (perilaku lama, tidak diubah di
  sini).
- **Truthiness:** `0` dan `0.0` salah (sudah berlaku sejak K1).
- **`panjang`, `ord`, `ke_bulat`, `bulatkan`, `biner`/`oktal`/`heksa`**,
  `jumlah` atas bulat, `rentang()` dan `enumerasi()` menghasilkan/menerima
  `bulat` seperti Python.
- **SQLite:** kolom `INTEGER` menjadi `bulat` (sebelumnya ditolak di luar
  ±2^53 karena hanya ada double).

## 4. Rencana implementasi

| Bagian | Perubahan |
|---|---|
| `src/value.h` | `VAL_INT` + `int64_t integer` di union; `IS_INT`, `IS_FLOAT`, `isNumberValue`, `asNumber`; `INT_VAL` |
| `src/value.c` | cetak bulat; `numberToString` selalu memberi `.0` pada desimal utuh; `valuesEqual` numerik lintas tipe; `valuesIdentical` tetap per tipe; helper overflow + `intPow` |
| `src/vm.c` | `OP_ADD/SUBTRACT/MULTIPLY/DIVIDE/FLOOR_DIVIDE/MODULO/POWER/NEGATE`; `chainCompare`; `isFalsey`; `galatLuapan`; `OP_LIST_LEN`/`enumerasi` push bulat |
| `src/compiler.c` | literal angka: bulat vs desimal, literal di luar int64 = galat kompilasi |
| `src/native.c` | tipe kembalian yang di Python `int`; `ke_angka`/`ke_bulat`; `rentang`/`jumlah`/`terurut` |
| `src/db.c` | `SQLITE_INTEGER` ↔ `bulat` |
| `src/debug.c` | tidak berubah (konstanta dicetak lewat `printValue`) |

**GC-safety:** tidak ada objek baru yang dialokasikan di jalur aritmetika,
jadi tidak ada perubahan pola rooting. Yang harus dijaga: `BINARY_OP` dan
handler angka tetap `peek` sebelum memutuskan, karena `pop()` dua kali lalu
`runtimeError` akan merusak stack.

## 5. Pengujian

- Golden baru: `examples/tipe_bulat.idk` (aritmetika, pembagian, floor, modulo,
  pangkat, campuran, perbandingan, `panjang`/`ord`/`ke_bulat`, `rentang`).
- Tes galat baru: `tests/galat/bulat_luapan.idk` (penjumlahan/perkalian
  meluap) dan `tests/galat/literal_bulat_terlalu_besar.idk`.
- Semua golden lama dijalankan ulang; setiap perbedaan `.out` diperiksa satu
  per satu (perubahan yang diharapkan: `3.0` sekarang punya `.0`, `6 / 2`
  menjadi `3.0`, hasil aritmetika bulat tidak lagi `.0`).
- `-DDEBUG_STRESS_GC` + seluruh contoh: keluaran harus identik.

## 6. Dampak dokumentasi

- [x] `website/panduan/03_PANDUAN_SINTAKS.md` — bagian angka
- [x] `website/panduan/07_PUSTAKA_STANDAR.md` — tipe kembalian builtin
- [x] `README.md`
- [x] `CHANGELOG.md` (+ tanda ⚠️ karena mengubah keluaran `/` dan cetak desimal)
- [x] `docs/ROADMAP.md`, `docs/ANALISIS/03_KEPUTUSAN_DESAIN.md` (K2 sebagian)
- [x] `AGENTS.md`
- [x] `editors/vscode-indonesiaku/` (tidak ada kata kunci baru; `GalatLuapan`
      masuk daftar builtin bila ada)

## 7. Risiko & alternatif

- **Merusak keluaran program lama:** `6 / 2` dulu `3`, sekarang `3.0`; `3.0`
  dulu `3`, sekarang `3.0`. Ini konsekuensi K2 yang sudah disetujui; dicatat
  di `CHANGELOG.md` sebagai perubahan yang merusak.
- **Alternatif ditolak:** tetap double + `ObjBigInt` terpisah (dua jalur
  aritmetika, tidak menyelesaikan presisi 2^53); atau int64 dengan overflow
  otomatis ke double (melanggar K2 dan menghasilkan jawaban salah tanpa
  pesan).
- **Risiko implementasi:** `AS_NUMBER` yang dulu makro murni berubah menjadi
  fungsi `asNumber()`; semua pemakaian harus dicek agar tidak mengevaluasi
  argumen dua kali (mis. `IS_NUMBER(pop())`).
