# Rencana Fitur: Iterator malas `_berikutnya_`

- **Status:** dikerjakan
- **Target versi:** 1.3.0
- **Pemilik:** Tim Indonesiaku

## 1. Masalah

Fase 4 sudah menyediakan `_iter_()` untuk mengganti objek yang digunakan
oleh `untuk`, tetapi iterasi saat ini masih bergantung pada panjang dan indeks
sekuens (`OP_LIST_LEN` + `OP_SEQ_GET`). Objek pengguna belum dapat menyediakan
iterator malas yang menghasilkan satu nilai setiap kali diminta.

Contoh yang belum didukung:

```indonesiaku
kelas Penghitung {
    fungsi _iter_() {
        kembalikan diri
    }

    fungsi _berikutnya_() {
        jika diri.posisi >= diri.batas {
            naikkan BerhentiIterasi("selesai")
        }
        variabel nilai = diri.posisi
        diri.posisi = diri.posisi + 1
        kembalikan nilai
    }
}
```

## 2. Sintaks usulan

Tidak ada kata kunci baru untuk subfitur ini. Protokol memakai metode ajaib:

- `_iter_()` mengembalikan objek iterator.
- `_berikutnya_()` mengembalikan satu nilai berikutnya.
- Iterator selesai dengan `BerhentiIterasi`.

`untuk item dalam sumber { ... }` memakai `_iter_()` sekali, lalu meminta
nilai melalui `_berikutnya_()` sampai selesai.

## 3. Semantik

- Semua sekuens lama tetap bekerja tanpa perubahan hasil: list, rentang, dan
  teks tetap memakai jalur internal yang sudah ada.
- Jika nilai hasil `_iter_()` menyediakan `_berikutnya_()`, VM memilih protokol
  iterator malas dan tidak memerlukan panjang atau indeks.
- `_berikutnya_()` dipanggil tepat satu kali untuk setiap nilai yang dikonsumsi.
- `BerhentiIterasi` mengakhiri loop secara normal; error lain tetap menjadi
  error runtime biasa dan tidak ditelan.
- `_iter_()` yang menghasilkan nilai tanpa `_berikutnya_()` menghasilkan error
  tipe yang jelas saat loop dimulai.
- Pemanggilan metode dapat menjalankan kode pengguna, mengalokasikan objek,
  dan memicu GC; receiver serta iterator harus tetap menjadi root selama call.
- `hentikan` dan `lanjut` mempertahankan perilaku yang sudah ada.

## 4. Rencana implementasi

| Bagian | Perubahan |
|---|---|
| `src/compiler.c` | Memancarkan jalur iterator malas pada `untuk` tanpa merusak jalur sekuens lama. |
| `src/vm.c` | Dispatch `_berikutnya_`, deteksi `BerhentiIterasi`, dan unwinding loop normal. |
| `src/object.c` / `src/native.c` | Reuse lookup magic method dan error bawaan yang sudah ada; tidak menambah tipe objek pada tahap ini. |
| `src/chunk.h` / `src/debug.c` | Opcode baru hanya jika jalur yang ada tidak cukup; pilih reuse terlebih dahulu. |
| `src/vm.h` | State sementara hanya jika diperlukan oleh unwinding. |

GC-safety wajib diperiksa pada receiver iterator, hasil `_berikutnya_()`, dan
jalur error. Jangan `pop` iterator sebelum pemanggilan yang dapat menjalankan
kode pengguna atau memicu GC.

## 5. Pengujian

- Contoh golden: `examples/oop/iterator_malas.idk` +
  `tests/expected/iterator_malas.out`.
- Tes error: iterator tanpa `_berikutnya_`, `_berikutnya_()` mengembalikan
  `BerhentiIterasi`, dan error lain dari `_berikutnya_`.
- Kasus wajib: iterator kosong, satu nilai, banyak nilai, nested loop,
  `hentikan`, `lanjut`, `_iter_()` dipanggil sekali, `_berikutnya_()` dipanggil
  sekali per konsumsi, serta iterator di bawah `DEBUG_STRESS_GC` + ASan/UBSan.
- Jalankan `tests/test_all.sh` dan `tests/test_panduan.sh`.
- Jangan mengubah golden lama tanpa membaca setiap diff.

## Status implementasi sementara

Loop `untuk` sudah dapat memakai `_berikutnya_()` dan menangkap `HentiIterasi`, tetapi dukungan `iter()`/`lanjutkan()` serta desain `ObjIterator` kustom belum selesai. Rencana ini tetap terbuka sampai seluruh jalur iterator terverifikasi di bawah stress GC.

## 6. Dampak dokumentasi & tooling

- [ ] `website/panduan/03_PANDUAN_SINTAKS.md`
- [ ] `website/panduan/07_PUSTAKA_STANDAR.md` (jika tipe/error baru menjadi builtin)
- [ ] `editors/vscode-indonesiaku/extension.js`
- [ ] `editors/vscode-indonesiaku/syntaxes/indonesiaku.tmLanguage.json`
- [ ] `editors/vscode-indonesiaku/snippets/indonesiaku.code-snippets`
- [ ] `README.md`
- [ ] `CHANGELOG.md` + `docs/ROADMAP.md` + `docs/ANALISIS/02_PETA_FITUR_PYTHON.md`
- [ ] `AGENTS.md`
- [ ] Berkas ini: status → `selesai`, lalu pindahkan ke `docs/RENCANA_FITUR/selesai/`

## 7. Risiko & alternatif

- Risiko utama adalah mengubah jalur `untuk` yang sudah stabil dan merusak
  list/rentang/teks; jalur internal lama dipertahankan sebagai fallback.
- Risiko berikutnya adalah salah menangkap `BerhentiIterasi` atau error akibat
  kode pengguna; hanya sentinel selesai yang boleh mengakhiri loop normal.
- Generator (`hasilkan`), generator expression, comprehension, dan pattern
  matching sengaja tidak dikerjakan dalam rencana ini; masing-masing memerlukan
  desain dan pengujian lanjutan setelah protokol iterator stabil.
