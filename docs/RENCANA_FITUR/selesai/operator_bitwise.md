# Rencana Fitur: Operator bitwise `& | ^ ~ << >>`

- **Status:** selesai (2026-10-25)
- **Target versi:** Fase 2
- **Pemilik:** teguh02
- **Keputusan terkait:** K2 (bulat int64) — bitwise baru bermakna setelah ada bilangan bulat sungguhan

## 1. Masalah

Operator bit Python belum ada. Karakternya bahkan belum dikenali scanner:

```
cetak(5 & 3)    # [baris 1] Kesalahan: Karakter tidak dikenal.
cetak(1 << 3)   # Kesalahan di '<': Harapkan ekspresi.
```

Dibutuhkan untuk flag/izin, mask warna, hash, protokol jaringan biner, dan
latihan logika di kelas pemrograman.

## 2. Sintaks usulan

```indonesiaku
izin = BACA | TULIS            # 1 | 2 -> 3
jika izin & TULIS { cetak("boleh tulis") }
cetak(5 ^ 3, ~5, 1 << 4, -8 >> 1)   # 6 -6 16 -4
x = 0b1010
x |= 0b0101                    # 15
x <<= 2                        # 60
```

Tidak ada kata kunci baru. Simbol sama persis dengan Python.

## 3. Semantik (mengikuti Python 3)

| Operator | Arti | Catatan |
|---|---|---|
| `a & b` | AND bit | |
| `a \| b` | OR bit | |
| `a ^ b` | XOR bit | `^` **bukan** pangkat (pangkat tetap `**`) |
| `~a` | NOT bit | `~a == -a - 1` |
| `a << n` | geser kiri | `n` negatif → `GalatNilai`; hasil di luar int64 → `GalatLuapan` (Python punya bignum; kita belum) |
| `a >> n` | geser kanan aritmetika | dibulatkan ke bawah: `-8 >> 1 == -4`, `-1 >> 100 == -1` |

- **Presedensi** (dari lemah ke kuat), sama dengan Python:
  perbandingan < `|` < `^` < `&` < `<<` `>>` < `+` `-` < `*` `/` … < unary (`-`, `~`) < `**`.
  Jadi `1 + 2 << 1 == 6` dan `5 & 3 == 1` (perbandingan paling lemah).
- **Tipe:** hanya `bulat`. `desimal` → `GalatTipe` (Python: TypeError).
  `logika` dengan `logika` → `logika` (`benar & salah == salah`), seperti
  `bool` Python. `logika` dengan `bulat` → `bulat` (`benar | 4 == 5`).
- `~benar`/`~salah` ditolak (`GalatTipe`, pakai `tidak`): Python 3.12
  menandainya usang dan hasilnya (`-2`) membingungkan pemula.
- Penugasan majemuk: `&= |= ^= <<= >>=` (sisi kanan ekspresi utuh, seperti
  operator majemuk lain).
- Semua galat dapat ditangkap `coba/kecuali`.
- **Disiapkan untuk himpunan:** handler VM memilih jalur menurut tipe operand,
  sehingga `himpunan | himpunan` (gabungan) dan `&` (irisan) nanti cukup
  menambah cabang, tanpa opcode baru. Begitu juga `kamus | kamus`.

## 4. Rencana implementasi

| Bagian | Perubahan |
|---|---|
| `src/scanner.h` / `scanner.c` | token `& \| ^ ~ << >>` dan versi `=`-nya |
| `src/compiler.c` | presedensi `PREC_BIT_OR`, `PREC_BIT_XOR`, `PREC_BIT_AND`, `PREC_SHIFT` di antara `PREC_COMPARISON` dan `PREC_TERM`; `binary()` dan `unary()`; tabel `matchCompoundAssign` |
| `src/chunk.h` / `debug.c` | `OP_BIT_AND`, `OP_BIT_OR`, `OP_BIT_XOR`, `OP_BIT_NOT`, `OP_SHIFT_LEFT`, `OP_SHIFT_RIGHT` |
| `src/value.c` / `value.h` | `intShiftLeft` (deteksi luapan tanpa UB), `intShiftRight` (lantai, `n >= 64` aman) |
| `src/vm.c` | handler per opcode; pemeriksaan tipe; galat bertipe |
| `editors/vscode-indonesiaku/` | operator baru di tmLanguage |

Tidak ada alokasi di jalur ini, jadi tidak ada risiko GC baru.

## 5. Pengujian

- Golden `examples/operator_bitwise.idk`: setiap operator, presedensi
  campuran, bilangan negatif, geser ≥ 64, `logika` & `bulat`, penugasan
  majemuk, literal `0b`/`0x`, dan contoh flag izin. Setiap baris dibandingkan
  dengan `python3` sebelum golden ditulis.
- Galat yang ditangkap di golden: `desimal`, geser negatif, luapan, `~benar`.
- `tests/galat/bitwise_desimal.idk`: galat tak tertangani (format pesan).
- `-DDEBUG_STRESS_GC`, ASan/UBSan (geser adalah sumber UB klasik di C), WSL.

## 6. Dampak dokumentasi

- [x] `website/panduan/03_PANDUAN_SINTAKS.md` (operator + tabel presedensi)
- [x] `editors/vscode-indonesiaku/` (pewarnaan operator)
- [x] `README.md`, `CHANGELOG.md`
- [x] `docs/ROADMAP.md`, `docs/ANALISIS/02_PETA_FITUR_PYTHON.md`
- [x] `AGENTS.md`
