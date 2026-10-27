# Rencana Fitur: terurut/min/max atas objek (metode ajaib tahap 2C)

- **Status:** selesai
- **Target versi:** 1.1.0
- **Pemilik:** Teguh Rijanandi

## 1. Masalah

Kelas sudah bisa dibandingkan sejak K19 tahap 1 — `<`, `>`, `<=`, `>=` semua
jalan lewat `_kurang_dari_` dan kawan-kawan. Tapi builtin yang *membandingkan
untuk bekerja* masih menolak objek:

```python
kelas Siswa {
    init(nama, nilai) {
        diri.nama = nama
        diri.nilai = nilai
    }
    _kurang_dari_(lain) { kembalikan diri.nilai < lain.nilai }
}

kelas_a = [Siswa("Budi", 80), Siswa("Ani", 92)]

cetak(terurut(kelas_a))   # GalatTipe: terurut() hanya menerima angka atau teks
cetak(min(kelas_a))       # GalatTipe: min() tidak dapat membandingkan tipe yang berbeda
cetak(max(kelas_a))       # idem
```

Jadi `a < b` berhasil tetapi `terurut([a, b])` gagal — tidak konsisten, dan
mengurutkan daftar objek adalah kebutuhan sehari-hari (nilai siswa, harga
barang, tanggal).

## 2. Sintaks usulan

Tidak ada sintaks baru. Tiga builtin yang sudah ada mulai menerima objek yang
punya `_kurang_dari_`:

```python
siswa = [Siswa("Budi", 80), Siswa("Ani", 92), Siswa("Citra", 75)]

untuk s dalam terurut(siswa) {
    cetak(s.nama)                 # Citra, Budi, Ani
}
cetak(min(siswa).nama)            # Citra
cetak(max(siswa).nama)            # Ani
cetak(terurut(siswa, benar)[0].nama)   # Ani — argumen menurun tetap berlaku
```

## 3. Semantik

**Satu metode saja yang dibutuhkan: `_kurang_dari_`.** Sama seperti operator
`<` hari ini, dan sama seperti Python yang hanya menuntut `__lt__` untuk
`sorted`/`min`/`max`. `_lebih_dari_` dipakai sebagai cerminan bila hanya itu
yang tersedia, lewat jalur cerminan yang sudah ada.

- `min(d)` / `max(d)` — objek dibandingkan berpasangan dengan `_kurang_dari_`.
  Daftar kosong tetap `GalatNilai`, satu argumen bukan urutan tetap
  `GalatTipe` (pesan lama dipertahankan, golden `kesalahan_native`
  menegaskannya).
- `terurut(d)` / `terurut(d, benar)` — mengurutkan **stabil**: elemen yang
  tidak saling lebih kecil mempertahankan urutan masukan. Ini perilaku baru
  yang dijanjikan hanya untuk jalur objek; angka dan teks tetap lewat `qsort`
  yang sudah ada.
- Campuran objek dengan angka/teks dalam satu daftar tetap `GalatTipe`.
- Objek **tanpa** `_kurang_dari_` dan tanpa `_lebih_dari_` tetap `GalatTipe`,
  dengan pesan yang menyebut metode yang perlu ditambahkan.
- Galat yang dilempar dari dalam `_kurang_dari_` merambat keluar; hasil
  separuh-urut tidak pernah dikembalikan.

## 4. Rencana implementasi

**`min`/`max` murah.** Keduanya lewat satu fungsi `valueLess()` di
`native.c`; cukup satu cabang baru di sana. Pembanding objeknya sudah ada di
`vm.c` (`compareMagicNow`, dipakai `OP_CHAIN_COMPARE`) — tinggal diekspor
sebagai `instanceLessFor()` lewat `vm.h`, seperti `instanceLengthFor()` di
tahap 2A.

**`terurut` tidak boleh memakai `qsort`.** Pembanding yang menjalankan kode
Indonesiaku melanggar dua asumsi `qsort` sekaligus:

1. `_kurang_dari_` bisa **melempar galat**, dan `qsort` tidak punya cara
   berhenti di tengah — ia akan terus membandingkan dengan keadaan VM yang
   sudah rusak.
2. `_kurang_dari_` bisa **mengubah daftar yang sedang diurutkan** (menambah
   elemen → `realloc` → `qsort` memegang pointer basi) atau memicu GC.

Karena itu jalur objek memakai **insertion sort** yang ditulis sendiri:
berhenti pada galat pertama, dan membaca ulang pointer array di setiap
langkah sehingga `realloc` di tengah jalan tidak menghasilkan pointer basi.
Stabil pula, sesuai janji di atas.

`ponytail:` insertion sort O(n²) — dipilih karena benar di hadapan kode
pengguna yang re-entrant, bukan karena paling cepat. Daftar objek yang
diurutkan di dunia nyata berukuran puluhan–ratusan. Kalau suatu saat ada
kasus dengan puluhan ribu objek, naikkan ke merge sort atas salinan array
(tetap stabil, tetap tahan realloc) — bukan kembali ke `qsort`.

Angka dan teks **tidak** melewati jalur ini: tetap `qsort` seperti sekarang,
jadi tidak ada regresi kinerja untuk pemakaian yang ada.

## 5. Rencana uji

- Golden: `examples/oop/urut_objek.idk` + `tests/expected/urut_objek.out`.
- Galat: `tests/galat/urut_objek_tanpa_metode.idk` + `.err`.
- Kasus batas wajib:
  - `terurut` **stabil** — dua objek dengan nilai sama mempertahankan urutan
    masukan (diuji dengan nama berbeda pada nilai yang sama).
  - `terurut(d, benar)` membalik urutan.
  - `min`/`max` atas daftar objek, dan `min`/`max` dengan argumen berganda.
  - Hanya `_lebih_dari_` yang didefinisikan → cerminan dipakai.
  - Galat dari dalam `_kurang_dari_` merambat, bukan hasil separuh-urut.
  - Daftar campuran objek + angka → `GalatTipe`.
  - Daftar objek satu elemen dan kosong.
- Wajib build sanitizer (`-DDEBUG_STRESS_GC -fsanitize=address,undefined`)
  terhadap seluruh golden dan `tests/galat/`: jalur ini mengalokasi,
  menjalankan kode pengguna, dan menyentuh array yang bisa di-`realloc`.
  `tests/test_all.sh` membangun ulang secara normal sehingga tidak memakai
  biner instrumentasi.

## 6. Dampak dokumentasi & tooling

- [x] `website/panduan/03_PANDUAN_SINTAKS.md`
- [x] `website/panduan/07_PUSTAKA_STANDAR.md` (`terurut`, `min`, `max`)
- [x] `editors/vscode-indonesiaku/` — tidak ada nama baru, tidak ada perubahan
- [x] `README.md`
- [x] `CHANGELOG.md` + `docs/ROADMAP.md`
      + `docs/ANALISIS/02_PETA_FITUR_PYTHON.md`
- [x] `AGENTS.md` (jebakan: jangan `qsort` dengan pembanding yang menjalankan
      kode pengguna)
- [x] Berkas rencana ini: status → `selesai`, lalu pindah ke
      `docs/RENCANA_FITUR/selesai/`

## 7. Risiko & alternatif

**Kompatibilitas.** Daftar objek sebelumnya selalu `GalatTipe` pada ketiga
builtin ini, jadi tidak ada program lama yang bisa berubah perilakunya. Jalur
angka/teks tidak disentuh sama sekali.

**Kode pengguna di tengah algoritma.** Risiko utamanya persis yang membuat
`qsort` ditolak: galat di tengah, realloc, dan GC. Ditangani dengan berhenti
pada galat pertama dan membaca ulang pointer tiap langkah; dibuktikan lewat
uji sanitizer, bukan diasumsikan.

**Perbandingan yang tidak konsisten.** Pengguna bisa menulis `_kurang_dari_`
yang melanggar urutan total (mis. selalu `benar`). Hasilnya urutan yang aneh
tapi tidak crash — sama seperti Python. Tidak dijaga runtime.

**Alternatif yang ditolak:** menambah parameter `kunci=` seperti
`sorted(key=...)` di Python. Berguna, tetapi masalah yang berbeda dan bisa
ditambahkan belakangan tanpa mengubah apa pun di rencana ini. YAGNI.
