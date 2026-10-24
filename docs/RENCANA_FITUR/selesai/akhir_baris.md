# Rencana Fitur: Akhir baris sebagai pemisah pernyataan

- **Status:** selesai
- **Target versi:** 0.6.0 (Fase 1)
- **Keputusan terkait:** B16

## 1. Masalah

Scanner saat ini membuang karakter baris baru. Akibatnya ekspresi pada baris berikutnya dapat menjadi kelanjutan ekspresi sebelumnya, misalnya `nilai` diikuti `(argumen)` terbaca sebagai pemanggilan.

## 2. Sintaks usulan

```idk
variabel nilai = 1
cetak(nilai)

cetak(
    nilai,
    2
)
```

Baris baru mengakhiri pernyataan di luar `()`, `[]`, dan `{}`. Titik koma tetap memisahkan beberapa pernyataan pada satu baris.

## 3. Semantik

- Baris kosong dan komentar tidak menghasilkan pernyataan.
- Parser mengabaikan `TOKEN_NEWLINE` di dalam ekspresi berkurung, daftar, parameter, dan argumen.
- Setelah sebuah pernyataan, satu atau lebih newline/`;` diterima sebagai pemisah.
- Blok tetap memakai `{}`; newline tidak menggantikan penutup blok.

## 4. Rencana implementasi

| Bagian | Perubahan |
|---|---|
| `src/scanner.[ch]` | Emit `TOKEN_NEWLINE` alih-alih membuang `\n` |
| `src/compiler.c` | Lewati newline pada posisi ekspresi yang legal dan konsumsi pemisah antarpernyataan |
| `examples/`, `tests/expected/` | Golden untuk pemisahan dan ekspresi multiline |

Tidak ada alokasi objek atau opcode baru, sehingga tidak ada risiko GC tambahan.

## 5. Pengujian

- Golden: dua ekspresi bertetangga, pemanggilan/list/parameter multiline, komentar dan `;`.
- Galat: ekspresi tidak lengkap sebelum newline tetap gagal.
- Jalankan golden penuh, `DEBUG_STRESS_GC`, dan ASan/UBSan.

## 6. Dampak dokumentasi

- [x] `website/panduan/03_PANDUAN_SINTAKS.md`
- [ ] `website/panduan/07_PUSTAKA_STANDAR.md` (tidak ada builtin)
- [x] `README.md`
- [x] `CHANGELOG.md` + `docs/ROADMAP.md`
- [x] `AGENTS.md`

## 7. Risiko & alternatif

Menghapus newline hanya di scanner lebih kecil, tetapi mempertahankan bug B16. Indentasi/DEDENT tidak ditambahkan: blok tetap eksplisit dengan `{}`.