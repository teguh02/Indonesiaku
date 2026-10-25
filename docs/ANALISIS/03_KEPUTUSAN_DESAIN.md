# 03 — Keputusan Desain yang Harus Dikunci Sebelum 1.0

Bahasa yang "stabil" berarti **program yang ditulis hari ini tetap berjalan
sama besok**. Karena itu, perubahan yang merusak kompatibilitas (breaking
change) harus dilakukan **sekarang**, di seri 0.x, selagi penggunanya masih
sedikit. Setelah 1.0, semantik di bawah ini dibekukan dan hanya boleh berubah
di versi mayor (2.0).

Setiap keputusan memiliki **rekomendasi**. Pemilik proyek menyetujui atau
mengganti rekomendasi; setelah disetujui, buat rencana di
`docs/RENCANA_FITUR/` dan tandai "disetujui" di sini.

**2026-10-24:** pemilik proyek menyetujui semua rekomendasi K1–K14 (blok
tetap `{ }` untuk seri 1.x). Perubahan yang merusak kompatibilitas mengikuti
kebijakan K14: dicatat di `CHANGELOG.md` dan didahului peringatan bila perlu
(mis. K3 diberi peringatan kompilasi di 0.5, berlaku di 0.6).

| ID | Keputusan | Rekomendasi | Merusak kompatibilitas? | Status |
|---|---|---|---|---|
| K1 | Truthiness | Ikuti Python | Ya | disetujui |
| K2 | Tipe angka | `bulat` + `desimal` | Ya (cetak `7/2`) | disetujui |
| K3 | Scope assignment di fungsi | Lokal secara bawaan + `global`/`nonlokal` | Ya | disetujui |
| K4 | Kurung kurawal vs indentasi | Pertahankan `{ }` untuk 1.x | Tidak | disetujui |
| K5 | Semua error runtime bisa ditangkap | Ya, dengan kelas `Galat*` | Tidak | disetujui — galat bisa ditangkap ✅ (Fase 0); kelas `Galat*` di Fase 1 |
| K6 | Kunci kamus hilang | `GalatKunci` + `.ambil()` | Ya | disetujui |
| K7 | `==` pada list/kamus | Struktural | Ya | disetujui |
| K8 | Kunci kamus | Semua nilai hashable | Tidak | disetujui |
| K9 | Protokol iterasi | `untuk` atas list, teks, kamus, rentang, iterator | Tidak | disetujui |
| K10 | Metode pada tipe bawaan | Ya (`a.tambah(x)`) + fungsi lama tetap ada | Tidak | disetujui |
| K11 | `jenis()` | Kembalikan objek kelas | Ya | disetujui |
| K12 | `cetak` | Fungsi biasa, argumen dipisah spasi | Ya (multi-arg) | disetujui — multi-argumen ✅ (Fase 0); `pisah=`/`akhir=` di Fase 1 |
| K13 | String | UTF-8 di memori, indeks & panjang per karakter (code point) | Ya | selesai ✅ (Fase 0) |
| K14 | Kebijakan stabilitas & versi | SemVer + periode deprecation 1 minor | — | disetujui |
| K15 | Nilai bawaan parameter | Dievaluasi **tiap panggilan**, bukan sekali saat definisi | Ya (beda dari Python) | disetujui — selesai ✅ (Fase 1) |
| K16 | Argumen bernama | `f(b=2)`, boleh melewati parameter di tengah | Ya | disetujui — selesai ✅ (Fase 1) |

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

## K15 — Nilai bawaan parameter dievaluasi tiap panggilan

Python mengevaluasi nilai bawaan **sekali**, saat `def` dijalankan. Akibatnya
`def f(a=[])` berbagi satu list untuk semua panggilan — jebakan *mutable
default argument* yang menghabiskan waktu pemula setiap tahun.

Indonesiaku mengevaluasi nilai bawaan **setiap kali fungsi dipanggil**:

```indonesiaku
fungsi kumpul(x, daftar=[]) {
    tambah(daftar, x)
    kembalikan daftar
}
cetak(kumpul(1))     # [1]
cetak(kumpul(2))     # [2]   — di Python [1, 2]
```

Konsekuensi yang diterima sadar:

- Nilai bawaan yang mahal dihitung ulang tiap panggilan.
- `fungsi f(x=waktu())` memberi waktu **saat panggil**, bukan saat definisi —
  justru yang biasanya diinginkan orang.
- Nilai bawaan boleh merujuk parameter di kirinya (`fungsi f(a, b=a)`), karena
  ia dijalankan di dalam frame yang slot sebelumnya sudah terisi.

Implementasi: prolog bersyarat. `call()` mengisi slot parameter yang tidak
diberikan pemanggil dengan penanda internal `VAL_TAKTERISI`, lalu tiap nilai
bawaan diawali `OP_ISI_BAWAAN slot, lompat` yang melewati ekspresinya bila
slot sudah terisi.

Versi pertama memakai **prolog bertingkat** (`ObjFunction.entryPoints`, lompat
langsung ke titik masuk sesuai jumlah argumen). Itu diganti saat argumen
bernama (K16) masuk: titik masuk tunggal hanya bisa mengisi **ekor**
parameter, sedangkan `f(1, c=9)` meninggalkan lubang di tengah.

**Jangan "perbaiki" ini menjadi perilaku Python.**

## K16 — Argumen bernama

`f(b=2)` mengirim nilai berdasarkan nama parameter. Argumen posisional wajib
mendahului argumen bernama (seperti Python).

Pencocokan nama terjadi **saat runtime**, bukan saat kompilasi, karena callee
adalah nilai runtime: `ObjFunction.paramNames` menyimpan nama parameter dan
`OP_CALL_BERNAMA` membawa daftar nama sebagai konstanta chunk. Mencocokkan
saat kompilasi untuk fungsi global yang kebetulan sudah dikenal ditolak —
hasilnya akan bergantung pada urutan definisi, dan menghasilkan dua jalur kode
untuk satu fitur.

Galat yang dihasilkan menyebut nama parameternya, bukan sekadar jumlah
argumen: `Parameter wajib 'a' tidak diisi.`, `Fungsi tidak punya parameter
bernama 'warna'.`, `Parameter 'a' diisi lebih dari sekali.`

Fungsi bawaan (native) belum menerima argumen bernama dan menolaknya dengan
pesan jelas. `cetak(pisah=, akhir=)` menunggu K12.
