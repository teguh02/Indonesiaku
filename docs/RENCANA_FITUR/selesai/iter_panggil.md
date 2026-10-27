# Rencana Fitur: `_iter_` dan `_panggil_` (metode ajaib tahap 2d)

- **Status:** selesai
- **Target versi:** 1.1.0
- **Pemilik:** Teguh Rijanandi

## 1. Masalah

Dua metode ajaib terakhir di Fase 4 belum ada. Keduanya membuat objek bisa
dipakai di posisi sintaks yang selama ini hanya milik tipe bawaan.

```python
kelas Keranjang {
    init() { diri.isi = ["apel", "jeruk"] }
}

untuk x dalam Keranjang() { cetak(x) }
# 'untuk ... dalam' hanya dapat mengiterasi daftar, tupel, himpunan, ...

kelas Pengali {
    init(n) { diri.n = n }
}

kali2 = Pengali(2)
cetak(kali2(21))        # Hanya dapat memanggil fungsi dan kelas.
```

Padahal objek sudah bisa dibandingkan, di-`panjang()`, diindeks, dan di-hash.
Dua lubang ini yang tersisa.

## 2. Sintaks usulan

Tidak ada sintaks baru — dua nama metode baru:

```python
kelas Keranjang {
    init() { diri.isi = ["apel", "jeruk"] }
    _iter_() { kembalikan diri.isi }
}

untuk buah dalam Keranjang() {
    cetak(buah)                   # apel, jeruk
}

kelas Pengali {
    init(n) { diri.n = n }
    _panggil_(x) { kembalikan x * diri.n }
}

kali2 = Pengali(2)
cetak(kali2(21))                  # 42
```

## 3. Semantik

**`_iter_`** — dipanggil **sekali** di awal `untuk ... dalam`, dan nilai
kembaliannya yang diiterasi. Jadi ini bentuk "iterable", bukan protokol
iterator penuh.

- Kembaliannya harus sesuatu yang sudah bisa diiterasi (daftar, tupel,
  himpunan, teks, rentang, kamus). Kalau bukan → `GalatTipe` yang menyebut
  `_iter_`, bukan pesan generik.
- Objek tanpa `_iter_` tetap `GalatTipe`, tapi pesannya kini menyebutkan
  `_iter_` sebagai jalan keluarnya.
- Dipanggil sekali per `untuk`, jadi `untuk` bersarang atas objek yang sama
  aman.

**`_panggil_`** — membuat instance bisa dipanggil seperti fungsi.

- `obj(a, b)` → `obj._panggil_(a, b)`. Jumlah argumen bebas, termasuk nol.
- Argumen bernama dan nilai bawaan ikut jalan, karena pemanggilannya lewat
  `call()` yang sama dengan metode biasa.
- Objek tanpa `_panggil_` tetap `GalatTipe` "Hanya dapat memanggil ...",
  dengan pesan yang kini menyebut `_panggil_`.
- Galat dari dalam metode merambat normal.

**Yang sengaja TIDAK dikerjakan di sini:** `_berikutnya_` (protokol iterator
malas/lazy) — itu Fase 5 bersama `hasilkan`/generator, dan butuh keadaan
iterator per objek. `_iter_` di sini cukup untuk membuat kelas koleksi
sendiri bisa di-`untuk`-kan, yang merupakan kebutuhan nyata hari ini.

## 4. Rencana implementasi

**`_panggil_` — satu cabang di `callValue()`** (`src/vm.c`). Fungsi itu sudah
memutuskan apa yang bisa dipanggil (`OBJ_CLOSURE`, `OBJ_CLASS`,
`OBJ_BOUND_METHOD`, `OBJ_NATIVE`); tinggal tambah `OBJ_INSTANCE`: cari
`_panggil_`, ganti slot callee dengan bound method, lalu `call()` seperti
biasa. Karena semua pemanggilan (`OP_CALL`, `OP_INVOKE` lewat `callValue`)
melewati titik ini, satu cabang menutup semua jalur — termasuk objek yang
dioper sebagai argumen ke `peta()`/`saring()`.

**`_iter_` — satu opcode baru di `forStatement()`** (`src/compiler.c` +
`src/vm.c`). `untuk` sudah di-desugar jadi loop berbasis indeks: simpan
sequence ke slot tersembunyi, `OP_LIST_LEN`, lalu `OP_SEQ_GET` tiap putaran.
Sisipkan `OP_ITER_OBJEK` tepat setelah ekspresi sequence dievaluasi:

- bukan instance → tidak melakukan apa-apa (nol biaya untuk kode lama);
- instance dengan `_iter_` → panggil lewat `call()`, nilai kembaliannya
  mendarat di slot yang sama, dan loop berikutnya berjalan di atas hasil itu;
- instance tanpa `_iter_` → biarkan, `OP_LIST_LEN` yang melaporkan galat.

Opcode ini berjalan **sekali** sebelum loop, bukan tiap putaran — itu sebabnya
`_iter_` tidak boleh dipanggil dari `OP_LIST_LEN`/`OP_SEQ_GET`, yang keduanya
berjalan per putaran dan akan memanggil ulang metode pengguna n kali.

`ponytail:` `_iter_` sengaja hanya "kembalikan sesuatu yang iterable",
bukan protokol iterator penuh. Naikkan ke `_berikutnya_` + keadaan iterator
saat Fase 5 (`hasilkan`/generator) dikerjakan — saat itu barulah ada alasan
memikul biayanya.

## 5. Rencana uji

- Golden: `examples/oop/iter_panggil.idk` + `tests/expected/iter_panggil.out`.
- Galat: `tests/galat/iter_bukan_koleksi.idk`, `panggil_tanpa_metode.idk`
  (+ `.err`).
- Kasus batas wajib:
  - `_iter_` mengembalikan daftar, tupel, teks, rentang, dan kamus;
  - `untuk` bersarang atas objek yang sama;
  - `hentikan`/`lanjut` di dalam `untuk` atas objek;
  - `_iter_` mengembalikan daftar kosong;
  - `_iter_` melempar galat → merambat, loop tidak jalan;
  - `_panggil_` dengan 0, 1, dan banyak argumen; argumen bernama; nilai bawaan;
  - objek `_panggil_` dioper ke `peta()`;
  - `_panggil_` melempar galat → merambat;
  - objek tanpa metode untuk kedua kasus → `GalatTipe` dengan pesan yang
    menyebut nama metodenya.
- Wajib build sanitizer (`-DDEBUG_STRESS_GC -fsanitize=address,undefined`)
  atas seluruh golden + `tests/galat/`.

## 6. Dampak dokumentasi & tooling

- [x] `website/panduan/03_PANDUAN_SINTAKS.md`
- [x] `editors/vscode-indonesiaku/` — 2 nama metode baru (daftar magic,
      tmLanguage, snippet)
- [x] `README.md`, `CHANGELOG.md`, `docs/ROADMAP.md`,
      `docs/ANALISIS/02_PETA_FITUR_PYTHON.md`
- [x] `AGENTS.md` bila ada jebakan baru
- [x] Rencana ini → `selesai/`

## 7. Risiko & alternatif

**Kompatibilitas.** Keduanya hanya menambah cabang pada kasus yang sebelumnya
**selalu galat** (objek di `untuk`, objek dipanggil). Tidak ada program lama
yang berubah perilaku.

**Biaya pada kode lama.** `OP_ITER_OBJEK` adalah satu `IS_INSTANCE` sekali per
`untuk` — bukan per putaran. `callValue` menambah satu `case` pada `switch`
yang sudah ada.

**Alternatif yang ditolak:** memanggil `_iter_` dari dalam `OP_LIST_LEN`.
Terlihat lebih sedikit kode (tanpa opcode baru), tapi `OP_SEQ_GET` juga butuh
hasilnya tiap putaran, sehingga metode pengguna terpanggil berulang — salah
secara semantik dan boros.
