# Rencana Fitur: Kolom pada galat runtime

- **Status:** selesai
- **Target versi:** 0.7.0 (sisa Fase 1)
- **Keputusan terkait:** 04 §5

## 1. Masalah

Galat kompilasi sudah menunjuk kolom dan mencetak potongan baris. Galat
**runtime** belum:

```
Operand '+' harus dua angka, dua teks, atau dua daftar.
[baris 2] dalam skrip
```

Pada baris dengan beberapa operasi sejenis, baris saja tidak cukup untuk tahu
yang mana yang gagal.

## 2. Dua penghalang nyata

Ini sebabnya butir ini sengaja ditunda saat mengerjakan traceback kompilasi:

1. **`chunk->lines` hanya menyimpan baris.** Perlu kolom per-bytecode.
2. **Teks sumber sudah mati saat program berjalan.** `main.c` mem-`free`
   sumber setelah `interpret()`, dan `importModule()` mem-`free` sumber modul
   tepat setelah `compile()`. Tanpa sumber, potongan baris tidak bisa
   dicetak.

## 3. Desain

| Berkas | Perubahan |
|---|---|
| `src/chunk.h/.c` | `int* columns` sejajar `lines`; `ObjString* source` |
| `src/chunk.c` | `writeChunk(chunk, byte, line, column)` |
| `src/compiler.c` | `emitByte` mengirim kolom; `compile()` menyimpan sumber |
| `src/memory.c` | tandai `chunk.source` saat menandai `OBJ_FUNCTION` |
| `src/vm.c` | `reportError` mencetak kolom + potongan baris |

**Sumber disimpan sebagai `ObjString` di dalam `Chunk`.** Semua fungsi hasil
satu kali `compile()` menunjuk `ObjString` yang sama, jadi tidak ada duplikasi
isi — hanya satu pointer per fungsi. Karena ia objek GC biasa, sumber modul
otomatis hidup selama fungsinya hidup, dan ikut bebas saat tidak terpakai.
Ini sekaligus menjawab penghalang (2) tanpa manajemen memori manual.

Biaya memori: **4 byte per bytecode** untuk `columns`, plus satu salinan teks
sumber per modul. Dapat diterima untuk pesan galat yang jauh lebih berguna;
dicatat sebagai `ponytail:` di `chunk.h`.

Penyajian mengikuti format galat kompilasi yang sudah ada supaya konsisten:

```
Operand '+' harus dua angka, dua teks, atau dua daftar.
[baris 2, kolom 11] dalam skrip
    cetak(x + "a")
              ^
```

Potongan baris hanya dicetak untuk **bingkai terdalam** (tempat galat
terjadi). Mencetaknya untuk setiap bingkai membuat traceback dalam jadi
berisik; Python pun hanya menandai barisnya.

## 4. Lingkup

- Kolom menunjuk token yang **terakhir diparsing** saat bytecode dipancarkan
  (`parser.previous`). Dugaan awal saya "untuk operator biner ia menunjuk
  operatornya" **salah** — diuji, ternyata menunjuk ujung operand kanan:

  ```
  cetak(x + "a")        caret di bawah "a", bukan di bawah +
  variabel d = {1: ...} caret di bawah }
  ```

  Tetap menunjuk ekspresi yang benar dan jauh lebih berguna daripada tanpa
  kolom, jadi diterima apa adanya. Menunjuk operator berarti setiap aturan
  parse menyimpan token operatornya sendiri — perubahan luas demi presisi
  beberapa karakter. Ditandai `ponytail:` di `emitByte`.
- Bingkai yang diulang (rekursi) tetap diringkas seperti sekarang.
- Potongan tidak dicetak bila sumbernya tidak tersedia (mis. REPL yang
  barisnya sudah berlalu) — harus degradasi mulus, bukan crash.

## 5. Pengujian

- Galat di awal/tengah/akhir baris; di dalam fungsi; di dalam modul `impor`
- Rekursi dalam (peringkasan tetap jalan)
- REPL / sumber tidak tersedia → tetap mencetak baris tanpa potongan
- Semua `.err` yang ada akan berubah → dibaca satu per satu
- Sanitizer: `source` harus ter-mark; jalankan dengan `DEBUG_STRESS_GC`

## 6. Dampak dokumentasi

- [x] `website/panduan/03_PANDUAN_SINTAKS.md` — contoh galat runtime diperbarui dan diverifikasi
- [x] `README.md` + `CHANGELOG.md`
- [x] `docs/ROADMAP.md` — butir ⬜ → ✅
- [ ] `AGENTS.md` — tertunda: berkas diproteksi Hermes, perlu persetujuan interaktif pemilik repo

## 7. Hasil (2026-10-25)

```text
tests/test_all.sh                  126/126 PASSED
ASan+UBSan+DEBUG_STRESS_GC         bersih (f-string bersarang 3 lapis, 300x)
```

32 golden `.err` berubah. Diverifikasi secara mekanis bahwa **setiap**
perubahan hanya menambah kolom dan potongan baris: pesan dinormalisasi dan
dibandingkan dengan versi lama, hasilnya identik untuk ke-32 berkas. Posisi
caret juga diperiksa tidak pernah melewati akhir baris.

### Dua bug yang hanya tertangkap sanitizer

Keduanya **sudah ada sebelumnya** tetapi tidak terlihat karena `parser.previous.start`
tidak pernah dibaca setelah kompilasi f-string — fitur ini yang pertama kali
membacanya.

1. **use-after-free di `fstring()`** — setelah sub-scanner selesai,
   `parser.previous` masih menunjuk buffer salinan ekspresi yang langsung
   dibebaskan. Diperbaiki dengan mengembalikan `parser.previous` ke token
   f-string sebelum `FREE_ARRAY`.
2. **`restoreScanner` tidak mengembalikan `scanner.source`** — jadi penunjuk
   sumber tertinggal menunjuk buffer f-string yang sudah bebas. Akar yang
   sebenarnya; `ScannerState` kini menyimpan `source` dan `sourceEnd`.

Selain itu `lineStartOf()` hanya memeriksa batas bawah buffer, sehingga token
dari buffer lain yang beralamat lebih tinggi lolos dan dibaca mundur keluar
batas. Kini memeriksa kedua batas.

`scannerSourceEnd` awalnya memindai sampai NUL setiap dipanggil — itu O(n)
per bytecode alias kompilasi O(n²). Diganti field `sourceEnd` yang di-cache.

### Kolom di dalam f-string

Ekspresi f-string dikompilasi dari salinan sementara, jadi kolom yang dihitung
darinya menunjuk posisi yang salah di baris asli. Kolom f-string kini
dihitung **sebelum** sub-scanner aktif dan dipakai untuk seluruh bytecode di
dalamnya, sehingga caret menunjuk f-string-nya — kurang presisi, tetapi tidak
pernah menyesatkan.
