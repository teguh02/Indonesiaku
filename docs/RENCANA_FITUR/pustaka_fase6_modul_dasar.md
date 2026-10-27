# Rencana Fitur: Fase 6 — Pustaka standar, gelombang A (modul murni Indonesiaku)

- **Status:** disetujui
- **Target versi:** 1.2.0 (menutup bagian Fase 6)
- **Pemilik:** kontributor Fase 6
- **Butir ROADMAP:** Fase 6 — `koleksi`, `iterasi`, `fungsional`,
  `statistik`, `salin`.

## 1. Masalah

Fase 3 memberi kita namespace modul (`impor x`, `dari x impor y`), dan katalog
[`07_KATALOG_PUSTAKA_STANDAR.md`](../ANALISIS/07_KATALOG_PUSTAKA_STANDAR.md)
sudah menetapkan nama modul P2. Yang belum ada isinya. Hari ini
`impor koleksi` gagal (`Modul 'koleksi' tidak ditemukan.`), dan tidak ada
`Penghitung`, `Antrian`, statistik, `salin`, atau versi fungsional/iterasi.
Satu-satunya yang tersedia adalah `pustaka/*.idk` lama yang diberi awalan
(`mat_`, `teks_`) dan di-`impor "..."` dengan namespace bersama.

## 2. Sintaks usulan

Modul **bernamespace**, tanpa awalan pada nama:

```indonesiaku
impor koleksi
dari statistik impor rata_rata, median
dari koleksi impor Penghitung, KamusBawaan, Antrian

c = Penghitung("abracadabra")
cetak(c.paling_umum(2))          # [["a", 5], ["b", 2]]
cetak(rata_rata([1, 2, 3, 4]))   # 2.5
cetak(koleksi.KamusBerurut())
```

Tidak ada kata kunci, operator, atau sintaks bahasa baru. Semuanya berkas
`.idk` di `pustaka/`, ditemukan lewat jalur pencarian modul Fase 3
(`pustaka/` relatif direktori kerja). Berkas lama (`pustaka/teks.idk`,
`matematika.idk`, …) **tidak disentuh**: keduanya hidup berdampingan.

## 3. Semantik

- Nilai yang dikembalikan dan galat mengikuti Python 3:
  urutan kosong → `GalatNilai` pada `rata_rata`/`median`/`varians`;
  `paling_umum`/`modus` mengembalikan daftar/nilai yang wajar untuk kosong.
- Objek pembungkus memakai metode ajaib K19, bukan tipe baru:
  - `Penghitung` — `_panjang_`, `_iter_` (mengulang kunci), `_ambil_`
    (hitungan, `0` bila absen), `_tambah_`/`_kurangi_` (gabung/kurang),
    `_sama_`, `_logika_`, `_teks_`, `_repr_`.
  - `KamusBawaan` — `_ambil_` memanggil pabrik saat kunci absen, lalu
    menyimpannya; `_atur_`, `_iter_`, `_panjang_`, `_logika_`, `_teks_`,
    `_repr_`.
  - `Antrian` — dua daftar di dalam (kiri dibalik), `_panjang_`, `_iter_`,
    `_ambil_`, `_teks_`, `_repr_`.
- `KamusBerurut` adalah alias `kamus` (kamus sudah berurut penyisipan).
- **Keterbatasan bahasa yang memaksa penyesuaian** (diverifikasi langsung
  terhadap biner 1.1.0):
  1. **Tidak ada fungsi variadik** (`*args`) — parameter hanya
     `ident (= bawaan)?`. Karena itu `iterasi.rangkai`/`produk` menerima
     **daftar urutan** alih-alih argumen tak terbatas, dan `pungsional.parsial`
     menerima argumen depan sebagai **daftar** lalu mengembalikan fungsi satu
     argumen. Perbedaan ini ditulis di panduan.
  2. **Tidak ada penyuntikan metode ke kelas saat runtime** — `K.x = f` tidak
     menghasilkan metode yang bisa dipanggil lewat `objek.x()`. Karena itu
     `fungsional.urutan_lengkap` dan `koleksi.tupel_bernama` **ditunda**;
     keduanya butuh masing-masing variadik/runtime method injection.
  3. `_iter_` harus mengembalikan **koleksi**, bukan objek iterator.
- Galat dari dalam modul bisa ditangkap `coba/kecuali` seperti biasa
  (`naikkan`), dan kelas galatnya memakai hierarki yang sudah ada.

## 4. Rencana implementasi

Tidak ada perubahan `src/`. Semua di `pustaka/`:

| Berkas | Isi |
|---|---|
| `pustaka/koleksi.idk` | `Penghitung`, `KamusBawaan`, `KamusBerurut`, `Antrian` |
| `pustaka/statistik.idk` | `rata_rata`, `median`, `modus`, `varians`, `varians_populasi`, `simpangan_baku`, `simpangan_baku_populasi`, `kuantil` |
| `pustaka/iterasi.idk` | `hitung_naik`, `ulangi`, `putar_ulang`, `akumulasi`, `rangkai`, `saring_dengan`, `ambil_selama`, `lewati_selama`, `saring_salah`, `kelompokkan`, `iris`, `berpasangan`, `kelompok_per`, `peta_bintang`, `pasangkan_terpanjang`, `produk`, `permutasi`, `kombinasi`, `kombinasi_berulang` (semuanya **eager**: mengembalikan daftar sampai Fase 5 memberi generator) |
| `pustaka/fungsional.idk` | `lipat`, `parsial`, `urutkan_dengan`, `urutkan_dengan_banding`, `banding_ke_kunci` |
| `pustaka/salin.idk` | `salin` (dangkal), `salin_dalam` (rekursif untuk daftar/kamus/himpunan/tupel; nilai lain dikembalikan apa adanya) |

Tidak menyentuh alokasi/stack/objek C, jadi tidak ada risiko GC baru — tetapi
golden tetap dijalankan di bawah `-DDEBUG_STRESS_GC` sebagai pengaman.

## 5. Pengujian

- Golden per modul: `examples/pustaka_koleksi.idk`,
  `pustaka_statistik.idk`, `pustaka_iterasi.idk`, `pustaka_fungsional.idk`,
  `pustaka_salin.idk` + `tests/expected/*.out` (stdout saja, stderr kosong).
- Tes galat: `tests/galat/statistik_kosong.idk/.err` (rata-rata urutan kosong).
- Kasus batas: kunci absen `KamusBawaan`, `Antrian` kosong, `salin_dalam`
  daftar bersarang, `kuantil` satu elemen, `modus` semua unik.
- `tests/test_all.sh`/`.ps1` hijau; blok `indonesiaku` baru di
  `website/panduan/07_PUSTAKA_STANDAR.md` ikut dijalankan `test_panduan.sh`.
- Golden tidak diregenerasi buta.

## 6. Dampak dokumentasi & tooling

- [ ] `website/panduan/07_PUSTAKA_STANDAR.md` — bagian 2.x per modul
- [ ] `editors/vscode-indonesiaku/` — **tidak berubah**: tidak ada kata kunci
      atau builtin baru, hanya fungsi di modul `pustaka/`
- [ ] `README.md` — daftar modul standar
- [ ] `CHANGELOG.md` + `docs/ROADMAP.md` +
      `docs/ANALISIS/02_PETA_FITUR_PYTHON.md`
- [ ] `AGENTS.md` — catat jebakan `_iter_` mengembalikan koleksi & batas
      variadik
- [ ] Berkas rencana ini → `docs/RENCANA_FITUR/selesai/`

## 7. Risiko & alternatif

- **Risiko: modul baru bentrok nama dengan `pustaka/*.idk` lama.** Tidak:
  modul punya namespace sendiri, dan `impor "..."` lama tetap memakai namespace
  pemanggil.
- **Alternatif ditolak:** memindahkan `pustaka/teks.idk` dst. ke bentuk
  bernamespace sekaligus. Itu breaking change untuk seluruh contoh lama (K14);
  di luar lingkup gelombang ini.
- **Alternatif ditolak:** menunggu Fase 5 (generator) sebelum menulis
  `iterasi`. Katalog sendiri mengizinkan versi eager; menulisnya sekarang
  memberi nilai dan jadi jaring regresi saat versi malas datang.
- **Alternatif ditolak:** mengimplementasikan variadik (`*args`) di Fase 6 agar
  API persis Python. Itu fitur bahasa (Fase 7), bukan pustaka; dicatat sebagai
  ketergantungan, bukan dikerjakan diam-diam.
