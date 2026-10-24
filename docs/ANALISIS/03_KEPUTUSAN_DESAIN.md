# 03 — Keputusan Desain yang Harus Dikunci Sebelum 1.0

Bahasa yang "stabil" berarti **program yang ditulis hari ini tetap berjalan
sama besok**. Karena itu, perubahan yang merusak kompatibilitas (breaking
change) harus dilakukan **sekarang**, di seri 0.x, selagi penggunanya masih
sedikit. Setelah 1.0, semantik di bawah ini dibekukan dan hanya boleh berubah
di versi mayor (2.0).

Setiap keputusan memiliki **rekomendasi**. Pemilik proyek menyetujui atau
mengganti rekomendasi; setelah disetujui, buat rencana di
`docs/RENCANA_FITUR/` dan tandai "disetujui" di sini.

| ID | Keputusan | Rekomendasi | Merusak kompatibilitas? | Status |
|---|---|---|---|---|
| K1 | Truthiness | Ikuti Python | Ya | usulan |
| K2 | Tipe angka | `bulat` + `desimal` | Ya (cetak `7/2`) | usulan |
| K3 | Scope assignment di fungsi | Lokal secara bawaan + `global`/`nonlokal` | Ya | usulan |
| K4 | Kurung kurawal vs indentasi | Pertahankan `{ }` untuk 1.x | Tidak | usulan |
| K5 | Semua error runtime bisa ditangkap | Ya, dengan kelas `Galat*` | Tidak | usulan |
| K6 | Kunci kamus hilang | `GalatKunci` + `.ambil()` | Ya | usulan |
| K7 | `==` pada list/kamus | Struktural | Ya | usulan |
| K8 | Kunci kamus | Semua nilai hashable | Tidak | usulan |
| K9 | Protokol iterasi | `untuk` atas list, teks, kamus, rentang, iterator | Tidak | usulan |
| K10 | Metode pada tipe bawaan | Ya (`a.tambah(x)`) + fungsi lama tetap ada | Tidak | usulan |
| K11 | `jenis()` | Kembalikan objek kelas | Ya | usulan |
| K12 | `cetak` | Fungsi biasa, argumen dipisah spasi | Ya (multi-arg) | usulan |
| K13 | String | UTF-8 di memori, indeks & panjang per karakter (code point) | Ya | usulan |
| K14 | Kebijakan stabilitas & versi | SemVer + periode deprecation 1 minor | — | usulan |

---

## K1 — Truthiness mengikuti Python

**Sekarang:** hanya `salah` dan `kosong` yang dianggap salah (warisan Lox).
**Usulan:** `salah`, `kosong`, `0`, `0.0`, `""`, `[]`, kamus kosong,
himpunan kosong, tupel kosong → salah. Objek dengan metode `_logika_` /
`_panjang_` mengikuti hasilnya.

**Alasan:** idiom Python `jika daftar { ... }` dan `selagi antrian { ... }`
adalah hal pertama yang disalin orang dari tutorial Python. Mengubahnya
setelah 1.0 tidak mungkin.

**Dampak:** satu fungsi `isFalsey()` di `vm.c`. Periksa `pustaka/*.idk` dan
`examples/` yang mungkin bergantung pada `0` bernilai benar.

## K2 — Dua tipe angka: `bulat` dan `desimal`

**Usulan:**

- `VAL_INT` (int64) ditambahkan di samping `VAL_NUMBER` (double).
- Literal tanpa titik → bulat; dengan titik/eksponen → desimal.
- `7 / 2 = 3.5` (selalu desimal), `7 // 2 = 3` (floor), `-7 // 2 = -4`,
  `-7 % 3 = 2` (mengikuti Python).
- Overflow int64 → fase 2b: naik otomatis ke bignum (`ObjBigInt`). Sebelum
  bignum ada, overflow menaikkan `GalatOverflow` — **jangan** diam-diam
  jadi double.
- Cetak desimal dengan algoritma *shortest round-trip* (`%.17g` lalu
  persingkat; atau port Ryu/Grisu) → `0.1 + 0.2` tercetak
  `0.30000000000000004`, `3.0` tercetak `3.0`.

**Catatan nama:** builtin `bulat` sekarang adalah pembulatan (round). Ganti
menjadi `bulatkan`, dan pertahankan `bulat` sebagai alias usang
(deprecated) satu versi.

## K3 — Assignment di dalam fungsi membuat variabel lokal

**Sekarang:** `x = 5` di dalam fungsi menulis ke **global**; kata kunci
`variabel` dibutuhkan untuk lokal. Ini kebalikan Python dan sumber bug
klasik ("kenapa variabel saya berubah di luar fungsi?").

**Usulan (sama dengan Python):**

- Di tingkat atas, `x = 5` → global (tetap).
- Di dalam fungsi, `x = 5` → lokal, **kecuali** dideklarasikan
  `global x` atau `nonlokal x`.
- `variabel x = 5` tetap diterima (sinonim eksplisit), tidak wajib.
- Membaca nama yang belum di-assign di fungsi tetap mencari global/upvalue.

**Implementasi:** compiler satu lintasan perlu tahu sebelum memancarkan
`OP_GET_*` apakah suatu nama akan di-assign di badan fungsi. Pilihan:
(a) pre-scan token badan fungsi untuk mengumpulkan nama yang di-assign
(murah, cocok dengan arsitektur sekarang), atau (b) pindah ke AST
(lihat `04_TEKNIK_LANJUTAN.md` §1). Rekomendasi: (a) untuk 1.0.

**Migrasi:** tambahkan peringatan kompilasi di 0.5 ("assignment ke global
`x` dari dalam fungsi; tambahkan `global x`"), ubah perilaku di 0.6.

## K4 — Kurung kurawal atau indentasi?

Python identik dengan indentasi. Namun Indonesiaku sudah memakai `{ }` di
seluruh contoh, panduan, pustaka, dan REPL.

| | `{ }` (sekarang) | Indentasi (Python) |
|---|---|---|
| Kemiripan dengan Python | Sedang | Tinggi |
| Pesan error untuk pemula | Jelas ("harapkan '}'") | Sering membingungkan (tab vs spasi) |
| Salin-tempel dari WhatsApp/web | Aman | Indentasi sering rusak |
| REPL multi-baris | Sudah ada | Perlu ditulis ulang |
| Biaya implementasi | 0 | Scanner INDENT/DEDENT + ubah semua contoh |

**Rekomendasi:** pertahankan `{ }` untuk seri 1.x — ini juga menjadi identitas
Indonesiaku ("mirip Python, lebih ramah salin-tempel"). Sisa token
`INDENT/DEDENT/COLON` di `scanner.h` dihapus agar tidak membingungkan.
Mode indentasi bisa dievaluasi ulang untuk 2.0 lewat rencana fitur
tersendiri.

## K5 — Semua error runtime menjadi pengecualian yang bisa ditangkap

**Sekarang:** `runtimeError()` langsung mencetak dan keluar (exit 70) untuk
indeks di luar jangkauan, variabel tak terdefinisi, tipe salah, dll.

**Usulan:** `runtimeError` diganti `throwGalat(KELAS, pesan)` yang membuat
instansi kelas pengecualian (hierarki di `02_PETA_FITUR_PYTHON.md` §F) dan
melakukan unwinding seperti `naikkan`. Hanya bila tidak ada handler, VM
mencetak traceback dan keluar 70. Pengecualian hanya untuk kegagalan internal
VM yang tidak boleh ditangkap (stack C rusak, kehabisan memori).

`kecuali` diperluas:

```
kecuali GalatIndeks sebagai e { ... }
kecuali (GalatTipe, GalatNilai) sebagai e { ... }
kecuali e { ... }          # tangkap semua (bentuk lama tetap sah)
```

## K6 — Kunci kamus yang tidak ada

**Usulan:** `d["x"]` untuk kunci tak ada → `GalatKunci`. Untuk perilaku lama
pakai `d.ambil("x")` (→ `kosong`) atau `d.ambil("x", bawaan)`.

## K7 — Kesetaraan struktural

`[1, 2] == [1, 2]` → `benar`; kamus dibandingkan per pasangan; instansi
memakai `_sama_` bila didefinisikan, selain itu identitas. Operator
`adalah` untuk identitas.

## K8 — Kunci kamus bisa berupa angka, bool, tupel, teks

Butuh fungsi hash per tipe dan `Table` yang berkunci `Value`, bukan
`ObjString*`. Tabel global VM tetap berkunci string untuk kecepatan.

## K9 — Protokol iterasi

`untuk x dalam <apa saja yang bisa diiterasi>`. Tahap 1: special-case di VM
untuk list, teks (per karakter), kamus (per kunci), rentang. Tahap 2 (fase 5):
objek dengan `_iter_`/`_berikutnya_` dan generator `hasilkan`.

## K10 — Metode pada tipe bawaan

`"abc".huruf_besar()`, `daftar.tambah(4)`, `d.kunci()`. Implementasi: tabel
metode native per tipe (`vm.listMethods`, `vm.stringMethods`, ...) yang dicari
di `OP_INVOKE`/`OP_GET_PROPERTY` saat penerima bukan instansi. Fungsi lama
(`tambah(a, x)`) tetap ada agar kode lama tidak rusak.

## K11 — `jenis()` mengembalikan kelas

`jenis(5)` → kelas `bulat`; `cetak(jenis(5))` → `<kelas 'bulat'>`;
`jenis(x) == bulat` dan `adalah_instansi(x, bulat)` berfungsi. Tipe bawaan
menjadi objek kelas sungguhan (lihat 04 §4).

## K12 — `cetak` menjadi fungsi biasa

`cetak(1, 2)` → `1 2`; `cetak()` → baris kosong;
`cetak("a", "b", pisah="-", akhir="")`. Butuh argumen bernama (fase 1).
`cetak` menjadi nilai biasa (bisa dioper ke fungsi lain).

Perbedaan cetak di dalam koleksi (seperti Python): `cetak("a")` → `a`,
tetapi `cetak(["a"])` → `["a"]` (pakai `repr` untuk elemen). Sekarang
tercetak `[a]`.

## K13 — String berbasis karakter

Simpan UTF-8, tetapi `panjang`, indeks, irisan, dan iterasi bekerja per code
point. Optimasi: tandai string "hanya ASCII" saat dibuat → indeks O(1);
selain itu indeks O(n) atau tabel offset tersimpan.

## K14 — Kebijakan stabilitas

- **SemVer.** 0.x: boleh breaking change, wajib dicatat di `CHANGELOG.md`
  bagian "Perubahan yang merusak". 1.x: tidak ada breaking change pada
  sintaks/semantik/builtin yang terdokumentasi.
- **Deprecation:** fitur yang akan dihapus mencetak peringatan ke stderr
  minimal satu versi minor sebelum dihapus.
- **Spesifikasi:** setelah K1–K13 diputuskan, tulis
  `docs/SPESIFIKASI.md` (tata bahasa EBNF + semantik). Tes golden menjadi
  *conformance suite* untuk spesifikasi itu.
