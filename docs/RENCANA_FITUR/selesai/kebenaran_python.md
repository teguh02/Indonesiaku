# Rencana Fitur: Truthiness ala Python (K1)

- **Status:** selesai (2026-10-24)
- **Target versi:** 0.6.0 (Fase 1)
- **Pemilik:** teguh02
- **Keputusan terkait:** K1 di [`ANALISIS/03_KEPUTUSAN_DESAIN.md`](../../ANALISIS/03_KEPUTUSAN_DESAIN.md) (disetujui)

## 1. Masalah

Hanya `salah` dan `kosong` yang dianggap salah. Idiom Python yang paling umum
disalin pemula tidak berjalan seperti yang diharapkan:

```
antrian = []
jika antrian { cetak("ada isi") }   # tercetak, padahal list kosong
nama = input("Nama: ")
jika tidak nama { cetak("nama wajib diisi") }   # tidak pernah tercetak
```

Tidak ada pesan error — programnya hanya salah diam-diam.

## 2. Sintaks usulan

Tidak ada sintaks baru. Hanya makna kondisi yang berubah:

```
jika daftar { ... }          # benar bila daftar tidak kosong
selagi antrian { ... }       # berhenti saat antrian kosong
nilai = masukan atau "bawaan"
```

## 3. Semantik

Nilai berikut dianggap **salah**; semua yang lain **benar**:

| Tipe | Nilai salah |
|---|---|
| logika | `salah` |
| kosong | `kosong` |
| angka | `0` (termasuk `-0`) — `nan` tetap benar, seperti Python |
| teks | `""` |
| list | `[]` |
| kamus | kamus tanpa kunci |
| rentang | rentang tanpa elemen (`rentang(0)`) |

Instansi kelas selalu benar (metode ajaib `_logika_`/`_panjang_` menyusul di
Fase 4). Berlaku di `jika`, `jika_lain`, `selagi`, `dan`, `atau`, `tidak`, `!`.
`dan`/`atau` tetap mengembalikan salah satu operand (bukan selalu
`benar`/`salah`), sama dengan Python: `"" atau "x"` → `"x"`.

Tidak ada galat baru.

## 4. Rencana implementasi

| Bagian | Perubahan |
|---|---|
| `src/vm.c` | `isFalsey()` memeriksa angka nol, teks kosong, list/kamus/rentang kosong |
| `src/table.c` / `src/vm.c` | hitung kunci hidup kamus (tabel menyimpan tombstone) |

Tidak ada opcode baru; semua jalur kondisi sudah lewat `isFalsey()`
(`OP_JUMP_IF_FALSE`, `OP_NOT`).

## 5. Pengujian

- Golden `examples/kebenaran.idk`: tabel nilai × `jika`/`tidak`/`dan`/`atau`,
  `selagi antrian`, pola `nilai atau bawaan`.
- Jalankan seluruh tes: `pustaka/*.idk` dan contoh lama mungkin bergantung
  pada `0` bernilai benar.
- `-DDEBUG_STRESS_GC`.

## 6. Dampak dokumentasi

- [x] `website/panduan/03_PANDUAN_SINTAKS.md` (bagian percabangan)
- [x] `CHANGELOG.md` — perubahan yang merusak kompatibilitas
- [x] `docs/ROADMAP.md`, `docs/ANALISIS/01` §5
- [x] `AGENTS.md`
