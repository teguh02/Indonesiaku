# Rencana Fitur: dekorator

- **Status:** dikerjakan
- **Target versi:** 1.1.0
- **Pemilik:** Teguh Rijanandi

## 1. Masalah

Membungkus fungsi atau metode sekarang harus ditulis tangan, dan namanya
disebut tiga kali:

```python
fungsi catat_waktu(f) {
    fungsi bungkus(x) {
        cetak("mulai")
        hasil = f(x)
        cetak("selesai")
        kembalikan hasil
    }
    kembalikan bungkus
}

fungsi hitung(x) { kembalikan x * 2 }
hitung = catat_waktu(hitung)        # mudah lupa, jauh dari definisinya
```

Selain itu, metode yang tidak butuh `diri` dan properti terhitung tidak punya
bentuk sama sekali.

## 2. Sintaks usulan

`@` di baris sebelum `fungsi`, `kelas`, atau metode:

```python
@catat_waktu
fungsi hitung(x) { kembalikan x * 2 }
```

Boleh bertumpuk, diterapkan dari bawah ke atas (seperti Python):

```python
@a
@b
fungsi f() { }        # f = a(b(f))
```

Boleh memakai argumen, karena yang ditulis sesudah `@` adalah ekspresi biasa:

```python
@ulangi(3)
fungsi sapa() { cetak("halo") }
```

Tiga dekorator bawaan untuk kelas:

```python
kelas Lingkaran {
    init(jari) { diri.jari = jari }

    @properti
    luas() { kembalikan 3.14159 * diri.jari * diri.jari }

    @metode_statis
    satuan() { kembalikan "cm" }

    @metode_kelas
    nama_kelas(kelas_ini) { kembalikan kelas_ini }
}

l = Lingkaran(2)
cetak(l.luas)              # 12.56636 — tanpa tanda kurung
cetak(Lingkaran.satuan())  # cm
```

## 3. Semantik

**Dekorator umum.** `@ekspr` sebelum definisi berarti: definisikan seperti
biasa, lalu ganti nilainya dengan `ekspr(nilai_itu)`.

- Berlaku untuk `fungsi`, `kelas`, dan metode di dalam kelas.
- Bertumpuk: yang paling dekat dengan definisi dijalankan lebih dulu.
- `@nama.atribut` dan `@nama(arg)` sah — yang diurai adalah ekspresi, dengan
  satu batasan: tidak boleh ada baris baru di dalamnya (menjaga pesan galat
  tetap menunjuk baris `@`).
- Dekorator dievaluasi **saat definisi dibuat**, sekali.
- Hasil dekorator boleh apa saja; kalau bukan fungsi, galatnya muncul saat
  dipanggil — sama seperti Python.

**`@metode_statis`** — metode tanpa `diri`. Dipanggil lewat kelas
(`Lingkaran.satuan()`) maupun instance (`l.satuan()`); keduanya tidak
menyisipkan penerima.

**`@metode_kelas`** — argumen pertamanya menerima **kelasnya**, bukan
instance. Dipanggil lewat kelas maupun instance.

**`@properti`** — metode tanpa argumen yang diakses **tanpa tanda kurung**:
`l.luas` menjalankan metodenya dan mengembalikan nilainya. Hanya untuk baca;
`l.luas = 5` menimpanya dengan field instance biasa (tidak ada `setter`).

`ponytail:` `@properti` hanya baca. Penyetel (`@luas.setter` di Python)
butuh pasangan metode per properti dan jalur tulis tersendiri di
`OP_SET_PROPERTY`; tambahkan kalau ada kebutuhan nyata, bukan sebelumnya.

## 4. Rencana implementasi

**Dipecah dua commit** karena dua bagian ini berdiri sendiri:

### Commit 1 — mekanisme `@` umum

1. `src/scanner.h`/`scanner.c` — `TOKEN_AT` untuk `'@'`. Karakter ini belum
   dipakai untuk apa pun, jadi tidak ada konflik.
2. `src/compiler.c`:
   - `declaration()`: saat `TOKEN_AT`, kumpulkan daftar dekorator dulu
     (ekspresi per baris), lalu kompilasi `fungsi`/`kelas` yang menyusul,
     lalu terapkan terbalik: untuk tiap dekorator, `OP_CALL 1` atas nilainya
     dan simpan kembali ke nama yang sama.
   - `method()`: dekorator pada metode diterapkan sebelum `OP_METHOD`.
3. Tidak ada opcode baru: penerapannya hanyalah pemanggilan biasa.

### Commit 2 — `@properti`, `@metode_statis`, `@metode_kelas`

Ketiganya bukan fungsi biasa — maknanya harus diketahui saat metode
didaftarkan. Karena itu dikenali **berdasarkan nama** di `method()` dan
disimpan sebagai jenis metode, bukan sebagai pembungkus runtime:

1. `src/object.h` — `ObjClass` mendapat dua tabel kecil: `properties` dan
   `statics`. `metode_kelas` ikut `statics` dengan satu penanda, karena
   keduanya sama-sama tidak menerima instance.
   **Tiga edit GC wajib** untuk tiap tabel baru (`initTable`, `freeTable`,
   `markTable` di `blackenObject`) — jebakan yang sudah tercatat di AGENTS.md.
2. `src/vm.c`:
   - `OP_GET_PROPERTY` pada instance: setelah field instance, **sebelum**
     atribut kelas, periksa `properties` → panggil metodenya sekarang.
   - `invoke()`/`callValue` untuk `statics`: jangan sisipkan penerima;
     untuk `metode_kelas`, sisipkan kelasnya.
3. `src/compiler.c` — `method()` mengenali ketiga nama dan memancarkan
   `OP_METHOD` varian (`OP_PROPERTY`, `OP_STATIC`, `OP_CLASSMETHOD`).

## 5. Rencana uji

Commit 1:
- Golden `examples/dekorator.idk` + expected.
- Kasus: dekorator tunggal; bertumpuk (urutan dibuktikan lewat cetak);
  dekorator berargumen; dekorator atas `kelas`; dekorator atas metode;
  dekorator yang mengembalikan nilai bukan fungsi lalu dipanggil → galat;
  dekorator dijalankan sekali saja.
- Galat: `@` tanpa definisi sesudahnya; nama dekorator tidak dikenal.

Commit 2:
- Golden `examples/oop/properti_statis.idk` + expected.
- Kasus: `@properti` dibaca tanpa kurung; dipakai di dalam metode lain;
  diwariskan ke subkelas; galat dari dalam properti merambat;
  `@metode_statis` lewat kelas dan lewat instance; `@metode_kelas` menerima
  kelas yang benar termasuk dari subkelas; `@properti` ditimpa dengan
  `obj.x = ...`.
- Galat: `@properti` pada metode yang menerima argumen.

Keduanya wajib lewat build sanitizer (`-DDEBUG_STRESS_GC
-fsanitize=address,undefined`) atas seluruh golden + `tests/galat/` —
commit 2 menambah tabel pada objek heap.

## 6. Dampak dokumentasi & tooling

- [ ] `website/panduan/03_PANDUAN_SINTAKS.md`
- [ ] `editors/vscode-indonesiaku/` — `@` dalam grammar, tiga nama bawaan,
      snippet
- [ ] `README.md`, `CHANGELOG.md`, `docs/ROADMAP.md`,
      `docs/ANALISIS/02_PETA_FITUR_PYTHON.md`
- [ ] `AGENTS.md` bila ada jebakan baru
- [ ] Rencana ini → `selesai/`

## 7. Risiko & alternatif

**Kompatibilitas.** `@` belum berarti apa pun — sekarang galat "Karakter tidak
dikenal". Tidak ada program lama yang berubah.

**Risiko: `@properti` menambah satu lookup di jalur panas.** `OP_GET_PROPERTY`
dipakai setiap akses atribut. Mitigasi: tabel `properties` hanya diperiksa
setelah field instance gagal, dan kelas tanpa properti berarti tabel kosong
(satu perbandingan kapasitas). Pola yang sama dipakai atribut kelas dan
terbukti tidak mengganggu.

**Alternatif yang ditolak:** `@properti` sebagai fungsi bawaan yang
mengembalikan objek pembungkus, seperti `property()` di Python. Lebih
"murni", tapi menuntut tipe objek baru plus pemeriksaan di setiap akses
atribut — lebih banyak kode untuk hasil yang sama.

**Alternatif yang ditolak:** menjalankan ketiga dekorator bawaan sebagai
pemanggilan runtime biasa. Tidak bisa: `metode_statis` harus mengubah cara
metode **dipanggil**, keputusan yang diambil saat pendaftaran metode, bukan
saat nilainya dihitung.
