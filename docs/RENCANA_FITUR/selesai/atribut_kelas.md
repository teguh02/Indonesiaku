# Rencana Fitur: atribut kelas

- **Status:** selesai
- **Target versi:** 1.1.0
- **Pemilik:** Teguh Rijanandi

## 1. Masalah

Badan kelas hanya menerima metode. Nilai yang dimiliki bersama oleh semua
instance harus ditaruh di `init()` dan disalin ke tiap objek, atau dijadikan
variabel global di luar kelas:

```python
kelas Lingkaran {
    init(r) {
        diri.PHI = 3.14159      # disalin ulang untuk SETIAP objek
        diri.r = r
    }
}
```

Padahal `PHI` sama untuk semua lingkaran. Menulisnya sebagai variabel global
membuatnya lepas dari kelas; menulisnya di `init` memboroskan satu entri per
instance dan menyiratkan nilainya bisa berbeda-beda.

Konstanta kelas, penghitung instance, dan nilai bawaan bersama adalah pola
sehari-hari — ini item kedua Fase 4.

## 2. Sintaks usulan

```python
kelas Lingkaran {
    PHI = 3.14159
    jumlah_dibuat = 0

    init(r) { diri.r = r }
    luas() { kembalikan Lingkaran.PHI * diri.r * diri.r }
}

cetak(Lingkaran.PHI)          # 3.14159     — lewat kelas
cetak(Lingkaran(2).PHI)       # 3.14159     — lewat instance
Lingkaran.jumlah_dibuat = 5   # ditulis lewat kelas
```

Bentuknya `NAMA = ekspresi` di badan kelas, sejajar dengan metode. Pemisah
baris baru atau `;` sama seperti anggota lain.

## 3. Semantik

Mengikuti Python:

- **Dibaca lewat kelas** (`Lingkaran.PHI`) maupun **lewat instance**
  (`obj.PHI`).
- **Urutan pencarian pada instance:** field instance → atribut kelas →
  metode. Jadi `diri.PHI = 1` membuat salinan milik objek itu saja dan
  menutupi atribut kelas, persis seperti Python.
- **Penulisan `obj.PHI = x` selalu membuat field instance**, tidak pernah
  mengubah atribut kelas. Mengubah yang dipakai bersama harus eksplisit lewat
  kelas: `Lingkaran.PHI = x`.
- **Diwariskan**: subkelas melihat atribut induknya. Menulis di subkelas
  membuat miliknya sendiri, tidak menyentuh induk.
- Ekspresinya dievaluasi **sekali**, saat kelas didefinisikan — bukan tiap
  instance dibuat. Boleh memakai nilai apa pun (angka, teks, daftar, bahkan
  pemanggilan fungsi).
- Nama atribut dan nama metode berbagi satu ruang nama: yang ditulis
  belakangan menang, seperti Python.

## 4. Rencana implementasi

**`ObjClass` dapat satu `Table fields`.** `methods` sengaja tidak dipakai
ulang: isinya wajib `ObjClosure` di banyak tempat (`bindMethod`,
`instanceMagic`, `OP_INHERIT`), dan menyelipkan nilai biasa ke sana akan
membuat semuanya harus memeriksa tipe lebih dulu.

Sentuhan yang diperlukan:

1. `src/object.h`/`object.c` — field `fields` pada `ObjClass`, di-`initTable`
   saat `newClass`, `freeTable` saat `freeObject`.
2. `src/memory.c` — `blackenObject` harus **menandai `fields`** selain
   `methods`. Tanpa ini nilainya dibebaskan GC padahal masih terpakai; wajib
   dibuktikan dengan build stress-GC.
3. `src/compiler.c` — di dalam badan kelas, bedakan `NAMA = ...` dari
   metode dengan satu token lihat-depan (`checkNext(TOKEN_EQUAL)`): kompilasi
   ekspresinya lalu `OP_CLASS_FIELD nameConstant`. Kelas sudah ada di atas
   stack selama badan dikompilasi, jadi tidak perlu slot tambahan.
4. `src/chunk.h`/`debug.c` — opcode `OP_CLASS_FIELD`.
5. `src/vm.c`:
   - `OP_CLASS_FIELD`: `tableSet(&klass->fields, nama, pop())`.
   - `OP_GET_PROPERTY` dengan **kelas** di puncak stack: cari `fields`, lalu
     `methods` (metode diambil lewat kelas = fungsi biasa, belum terikat).
   - `OP_GET_PROPERTY` dengan **instance**: setelah `instance->fields` gagal,
     telusuri `klass->fields` naik ke rantai superclass, **sebelum**
     `bindMethod`.
   - `OP_SET_PROPERTY` dengan **kelas** di `peek(1)`: tulis ke `klass->fields`.
   - `OP_INHERIT`: **tidak** menyalin tabel induk. Pencarian menelusuri rantai
     `superclass` saat dibaca, supaya mengubah atribut induk langsung terlihat
     oleh subkelas — seperti Python, dan tanpa menggandakan memori.

`ponytail:` pencarian atribut menelusuri rantai superclass secara linier tiap
akses, tanpa cache. Kedalaman warisan di dunia nyata 2–4 tingkat, jadi ini
lebih murah daripada invalidasi cache. Kalau MRO C3 nanti masuk, titik
telusurnya satu ini saja yang berubah.

## 5. Rencana uji

- Golden: `examples/oop/atribut_kelas.idk` + `tests/expected/atribut_kelas.out`.
- Galat: `tests/galat/atribut_kelas_tidak_ada.idk` (+ `.err`).
- Kasus batas wajib:
  - baca lewat kelas dan lewat instance;
  - `diri.X = ...` menutupi atribut kelas, dan instance lain tidak terpengaruh;
  - menulis lewat kelas terlihat oleh semua instance yang belum menutupinya;
  - diwariskan ke subkelas; menulis di subkelas tidak menyentuh induk;
  - mengubah atribut induk terlihat oleh subkelas (bukti rantai, bukan salinan);
  - atribut berisi daftar/kamus (objek yang dialokasi) — penanda GC;
  - atribut dievaluasi sekali, dibuktikan dengan pemanggilan fungsi bernilai
    samping;
  - nama atribut sama dengan nama metode (yang belakangan menang);
  - atribut dipakai di dalam metode lewat nama kelas;
  - membaca atribut yang tidak ada → `GalatAtribut`.
- Wajib build sanitizer (`-DDEBUG_STRESS_GC -fsanitize=address,undefined`)
  atas seluruh golden + `tests/galat/`: ada tabel baru yang harus ditandai GC.

## 6. Dampak dokumentasi & tooling

- [x] `website/panduan/03_PANDUAN_SINTAKS.md`
- [x] `editors/vscode-indonesiaku/` — snippet atribut kelas (tak ada kata
      kunci baru)
- [x] `README.md`, `CHANGELOG.md`, `docs/ROADMAP.md`,
      `docs/ANALISIS/02_PETA_FITUR_PYTHON.md`
- [x] `AGENTS.md` — jebakan: tabel baru pada objek wajib ditandai di
      `blackenObject`
- [x] Rencana ini → `selesai/`

## 7. Risiko & alternatif

**Kompatibilitas.** `NAMA = ...` di badan kelas sekarang adalah galat
kompilasi, jadi tidak ada program lama yang berubah arti. Urutan pencarian
baru hanya menambah satu langkah di antara field instance dan metode; kelas
tanpa atribut tidak terpengaruh.

**GC.** Ini risiko utamanya: tabel baru yang lupa ditandai akan membebaskan
nilai yang masih hidup, dan gejalanya hanya muncul di bawah tekanan alokasi.
Karena itu tes wajib memuat atribut berisi daftar/kamus dan dijalankan di
bawah `-DDEBUG_STRESS_GC`.

**Alternatif yang ditolak:** menyimpan atribut di `methods` yang sudah ada.
Lebih sedikit baris, tapi setiap pembaca `methods` (termasuk `instanceMagic`
dan `bindMethod`) harus mulai memeriksa apakah isinya closure — menyebarkan
biaya ke jalur panas demi menghemat satu tabel.

**Alternatif yang ditolak:** menyalin tabel induk saat `OP_INHERIT`. Pencarian
jadi O(1), tapi mengubah atribut induk setelah subkelas dibuat tidak akan
terlihat — berbeda dari Python, dan menggandakan memori untuk tiap subkelas.
