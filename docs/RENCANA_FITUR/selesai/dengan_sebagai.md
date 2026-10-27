# Rencana Fitur: `dengan ... sebagai` (manajer konteks)

- **Status:** selesai
- **Target versi:** 1.1.0
- **Pemilik:** Teguh Rijanandi

## 1. Masalah

Sumber daya yang harus dilepas (berkas, koneksi basis data, kunci) sekarang
menuntut `coba ... akhirnya` ditulis tangan setiap kali:

```python
f = buka("data.txt")
coba {
    cetak(f.baca())
} akhirnya {
    f.tutup()
}
```

Mudah lupa, dan berulang. `TOKEN_DENGAN` sudah dikenali scanner sejak lama
tetapi **tidak pernah dipakai compiler** — kata kunci yang dipesan tanpa
implementasi. Ini item Fase 4 yang paling terasa manfaatnya sehari-hari.

## 2. Sintaks usulan

```python
dengan buka("data.txt") sebagai f {
    cetak(f.baca())
}
```

`sebagai <nama>` boleh dihilangkan ketika nilainya tidak dipakai:

```python
dengan kunci_global() {
    kerjakan()
}
```

## 3. Semantik

Mengikuti Python, disederhanakan di satu titik (lihat di bawah):

- Ekspresi dievaluasi, lalu metode **`_masuk_()`** dipanggil pada hasilnya.
  Nilai kembalian `_masuk_()` yang diikat ke `sebagai <nama>` — bukan objek
  manajernya. Ini membuat pola seperti `_masuk_()` mengembalikan berkas yang
  sudah terbuka bisa ditulis.
- Blok dijalankan.
- **`_keluar_()` dijamin dipanggil** saat blok selesai: baik selesai normal,
  galat, `kembalikan`, `hentikan`, maupun `lanjut`.
- Galat di dalam blok **tetap merambat** setelah `_keluar_()` dijalankan.
- Objek tanpa `_masuk_`/`_keluar_` → `GalatTipe` yang menyebut metode yang
  kurang.
- `<nama>` adalah variabel lokal biasa yang hidup sampai akhir blok.

**Penyederhanaan yang disengaja:** `_keluar_()` dipanggil **tanpa argumen**
dan nilai kembaliannya diabaikan. Python mengoper `(tipe, nilai, jejak)` dan
memperlakukan kembalian benar sebagai "galat ditelan". Menelan galat diam-diam
adalah sumber bug yang sulit dilacak, dan tiga argumen itu menuntut objek
jejak tumpukan yang belum ada di bahasa ini.

`ponytail:` `_keluar_()` tanpa argumen dan tanpa penelanan galat. Kalau
suatu saat benar-benar ada kebutuhan manajer yang menelan galat (mis.
`kecualikan` seperti `contextlib.suppress`), naikkan dengan menambah argumen
galat dan memeriksa kembaliannya — jalur `_keluar_` sudah terpusat di satu
tempat, jadi perubahannya lokal.

## 4. Rencana implementasi

Jaminan "selalu dipanggil walau galat, `kembalikan`, `hentikan`, atau
`lanjut`" **sudah ada dan sudah teruji** di `akhirnya`:

- `FinallyRegion` mencatat tiap `coba ... akhirnya` yang aktif;
- `exitTryBlocks()` menjalankan blok-blok itu sebelum melompat keluar;
- `OP_TRY_BEGIN`/`OP_TRY_END` mengurus jalur galat.

Jadi `dengan` **tidak membangun mesin baru**. `FinallyRegion` diberi penanda
jenis:

- `FINALLY_SOURCE` — perilaku sekarang: blok `akhirnya` di-scan ulang dari
  sumber oleh `inlineFinallyBlock()`;
- `FINALLY_EXIT_CALL` — milik `dengan`: alih-alih me-rescan teks, emit tiga
  instruksi `OP_GET_LOCAL <slot manajer>`, `OP_INVOKE _keluar_ 0`, `OP_POP`.

Dengan begitu `kembalikan`/`hentikan`/`lanjut` di dalam `dengan` langsung ikut
benar lewat `exitTryBlocks()` yang sudah ada — tanpa menyentuh satu pun dari
ketiga pernyataan itu. Batas `FINALLY_INLINE_MAX` juga otomatis berlaku, dan
jauh lebih murah: tiga instruksi per salinan, bukan seluruh blok.

Sentuhan:

1. `src/compiler.c` — `withStatement()`, dipasang di `statement()` pada
   `TOKEN_DENGAN`; `FinallyRegion.kind` + slot manajer; `inlineFinallyBlock()`
   bercabang pada `kind`.
2. Tidak ada opcode baru. Tidak ada perubahan `vm.c`, `object.h`, atau GC.
3. `src/scanner.c` — tidak berubah, `TOKEN_DENGAN` sudah ada.

Struktur kode yang dihasilkan sengaja dibuat sama persis dengan
`coba ... akhirnya` supaya jalur galatnya terbukti sama.

## 5. Rencana uji

- Golden: `examples/oop/dengan_sebagai.idk` +
  `tests/expected/dengan_sebagai.out`.
- Galat: `tests/galat/dengan_tanpa_masuk.idk`,
  `dengan_galat_merambat.idk` (+ `.err`).
- Kasus batas wajib:
  - selesai normal → `_keluar_()` jalan;
  - galat di dalam blok → `_keluar_()` jalan **lalu** galat merambat;
  - `kembalikan` di dalam `dengan` (di dalam fungsi) → `_keluar_()` jalan;
  - `hentikan` dan `lanjut` di dalam `dengan` di dalam perulangan;
  - `dengan` bersarang → urutan keluar terbalik (dalam dulu);
  - `dengan` tanpa `sebagai`;
  - `_masuk_()` mengembalikan objek lain (yang terikat adalah kembaliannya);
  - manajer tanpa `_masuk_` → `GalatTipe`; tanpa `_keluar_` → `GalatTipe`;
  - galat **di dalam** `_keluar_` sendiri;
  - `dengan` di dalam `coba`, dan `coba` di dalam `dengan`.
- Wajib build sanitizer atas seluruh golden + `tests/galat/`.

## 6. Dampak dokumentasi & tooling

- [x] `website/panduan/03_PANDUAN_SINTAKS.md`
- [x] `editors/vscode-indonesiaku/` — `dengan` sudah jadi kata kunci; tambah
      snippet dan dua nama metode ajaib ke daftar
- [x] `README.md`, `CHANGELOG.md`, `docs/ROADMAP.md`,
      `docs/ANALISIS/02_PETA_FITUR_PYTHON.md`
- [x] `AGENTS.md` bila ada jebakan baru
- [x] Rencana ini → `selesai/`

## 7. Risiko & alternatif

**Kompatibilitas.** `dengan` sudah menjadi kata kunci, jadi tidak ada program
lama yang memakainya sebagai nama variabel. Yang berubah hanyalah: dulu galat
kompilasi, sekarang bermakna.

**Risiko utama: `kembalikan` dari dalam `dengan`.** Inilah yang paling mudah
salah, dan justru alasan memakai ulang `FinallyRegion` — mesinnya sudah
menangani kasus itu untuk `akhirnya` dan sudah punya tes. Tetap diuji
eksplisit.

**Alternatif yang ditolak:** men-desugar `dengan` menjadi token
`coba ... akhirnya` lalu menyuruh compiler mengurainya sebagai teks. Terlihat
paling sedikit kode, tetapi menuntut membangun buffer sumber palsu dan
merusak nomor baris pada pesan galat.

**Alternatif yang ditolak:** opcode `OP_WITH_BEGIN`/`OP_WITH_END` tersendiri
di VM. Menduplikasi logika pelepasan handler yang sudah ada di `OP_TRY_*`,
dan harus menangani ulang jalur `kembalikan`/`hentikan`/`lanjut` yang sudah
benar di sisi compiler.
