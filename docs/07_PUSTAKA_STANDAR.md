# 📚 Referensi Pustaka Standar — Indonesiaku

Dokumen ini mendaftar semua **fungsi bawaan** (ditulis dalam C, selalu tersedia)
dan **pustaka standar** (ditulis dalam Indonesiaku, di folder `pustaka/`,
di-`impor` sesuai kebutuhan).

Kesalahan pada fungsi bawaan (argumen salah, berkas gagal dibuka, dll.) dapat
ditangkap dengan `coba/kecuali`, kecuali kesalahan internal seperti indeks di
luar jangkauan.

---

## 1. Fungsi Bawaan (Built-in)

### 1.1 Matematika

| Fungsi | Deskripsi |
|--------|-----------|
| `min(a, b, ...)` | Nilai terkecil (minimal 2 argumen) |
| `max(a, b, ...)` | Nilai terbesar (minimal 2 argumen) |
| `abs(x)` | Nilai absolut |
| `akar(x)` | Akar kuadrat (x ≥ 0) |
| `pangkat(x, y)` | x pangkat y |
| `bulat(x)` | Pembulatan ke bilangan terdekat |
| `lantai(x)` | Pembulatan ke bawah (floor) |
| `atap(x)` | Pembulatan ke atas (ceil) |
| `acak()` | Angka acak `[0, 1)` |
| `acak_bulat(min, maks)` | Bilangan bulat acak `[min, maks]` |

### 1.2 String / Teks

| Fungsi | Deskripsi |
|--------|-----------|
| `panjang(x)` | Panjang string, list, atau kamus |
| `huruf_besar(s)` | Ubah ke huruf kapital |
| `huruf_kecil(s)` | Ubah ke huruf kecil |
| `ganti(teks, cari, ganti)` | Ganti semua kemunculan substring |
| `karakter(teks, i)` | Karakter (string 1 huruf) pada indeks `i` |
| `potong(teks, mulai, jumlah)` | Substring mulai dari `mulai` sepanjang `jumlah` |

Pengindeksan string juga tersedia: `teks[i]` mengembalikan karakter ke-`i`
(string bersifat immutable — tidak dapat diubah lewat indeks).

### 1.3 List

| Fungsi | Deskripsi |
|--------|-----------|
| `tambah(list, nilai)` | Tambah elemen ke akhir list |
| `hapus(list)` | Hapus & kembalikan elemen terakhir |
| `panjang(list)` | Jumlah elemen |

Literal: `[1, 2, 3]`. Indeks baca/tulis: `list[i]`, `list[i] = x`.

### 1.4 Kamus (Dictionary)

| Fungsi | Deskripsi |
|--------|-----------|
| `kamus()` | Buat kamus kosong |
| `punya(kamus, kunci)` | Apakah kunci ada → boolean |
| `kunci(kamus)` | List semua kunci |
| `hapus_kunci(kamus, kunci)` | Hapus pasangan kunci-nilai |

Akses: `k["kunci"]`, `k["kunci"] = nilai`. Kunci berupa string.

### 1.5 Konversi & Format

| Fungsi | Deskripsi |
|--------|-----------|
| `ke_angka(teks)` | String → angka (gagal dapat ditangkap) |
| `ke_teks(nilai)` | Nilai apa pun → string |
| `format(pola, ...)` | Ganti setiap `{}` di pola dengan argumen |
| `jenis(x)` | Nama tipe: `"angka"`, `"string"`, `"boolean"`, `"kosong"`, `"list"`, `"kamus"`, `"objek"`, `"kelas"`, `"berkas"` |

### 1.6 Berkas (File I/O)

Semua kesalahan dapat ditangkap `coba/kecuali`.

| Fungsi | Deskripsi |
|--------|-----------|
| `buka_berkas(path, mode)` | Buka berkas → handle. Mode: `"b"` baca, `"t"` tulis, `"s"` sambung |
| `tutup(berkas)` | Tutup handle |
| `baca(berkas)` | Baca seluruh sisa isi → string |
| `baca_baris(berkas)` | Baca satu baris → string, atau `kosong` di akhir |
| `tulis(berkas, teks)` | Tulis ke handle → jumlah byte |
| `baca_semua(path)` | Baca seluruh berkas dari path → string |
| `tulis_semua(path, teks)` | Tulis (timpa) ke path |
| `tambah_berkas(path, teks)` | Tambahkan ke akhir berkas |
| `ada_berkas(path)` | Apakah berkas ada → boolean |
| `hapus_berkas(path)` | Hapus berkas |

### 1.7 Waktu & Tanggal

| Fungsi | Deskripsi |
|--------|-----------|
| `waktu()` | Detik sejak epoch (Unix time) |
| `tanggal()` | Kamus `{tahun, bulan, hari, jam, menit, detik, hari_minggu, hari_tahun}` |
| `format_tanggal(pola)` | Format waktu sekarang dengan pola `strftime` |
| `jam()` | Waktu CPU berjalan (detik) |
| `henti(detik)` | Jeda eksekusi |

### 1.8 Lain-lain

| Fungsi / Variabel | Deskripsi |
|-------------------|-----------|
| `cetak(...)` | Cetak nilai ke output |
| `input(prompt)` | Baca satu baris dari stdin |
| `argumen` | Variabel global: list argumen baris perintah |

---

## 2. Pustaka Standar (`pustaka/`)

Di-`impor` sesuai kebutuhan. Fungsi diberi awalan agar tidak bentrok dengan
namespace global.

### 2.1 `pustaka/teks.idk`

```python
impor "pustaka/teks.idk"
```

| Fungsi | Deskripsi |
|--------|-----------|
| `teks_cari(teks, cari)` | Indeks kemunculan pertama, atau `-1` |
| `teks_berisi(teks, cari)` | Apakah `cari` ada di `teks` |
| `teks_mulai_dengan(teks, awalan)` | Boolean |
| `teks_akhiri_dengan(teks, akhiran)` | Boolean |
| `teks_pisah(teks, pemisah)` | List bagian |
| `teks_gabung(daftar, perekat)` | Gabungkan list jadi string |
| `teks_ulang(teks, n)` | Ulang `teks` sebanyak `n` |
| `teks_balik(teks)` | Teks terbalik |
| `teks_pangkas(teks)` | Hilangkan spasi di awal/akhir |

### 2.2 `pustaka/daftar.idk`

```python
impor "pustaka/daftar.idk"
```

| Fungsi | Deskripsi |
|--------|-----------|
| `daftar_peta(daftar, fn)` | Terapkan `fn` ke tiap elemen (map) |
| `daftar_saring(daftar, fn)` | Elemen yang `fn`-nya benar (filter) |
| `daftar_lipat(daftar, awal, fn)` | Reduksi (reduce) |
| `daftar_berisi(daftar, nilai)` | Boolean |
| `daftar_indeks_dari(daftar, nilai)` | Indeks pertama, atau `-1` |
| `daftar_iris(daftar, mulai, akhir)` | Sublist `[mulai, akhir)` |
| `daftar_balik(daftar)` | Urutan terbalik |
| `daftar_salin(daftar)` | Salinan dangkal |
| `daftar_urutkan(daftar)` | Terurut menaik (angka atau string) |

### 2.3 `pustaka/matematika.idk`

```python
impor "pustaka/matematika.idk"
```

| Fungsi / Konstanta | Deskripsi |
|--------------------|-----------|
| `MAT_PI`, `MAT_E` | Konstanta π dan e |
| `mat_genap(n)`, `mat_ganjil(n)` | Boolean |
| `mat_faktorial(n)` | n! |
| `mat_fpb(a, b)` | Faktor persekutuan terbesar |
| `mat_kpk(a, b)` | Kelipatan persekutuan terkecil |
| `mat_jumlah(daftar)` | Jumlah elemen |
| `mat_rata_rata(daftar)` | Rata-rata |
| `mat_maks_list(daftar)`, `mat_min_list(daftar)` | Nilai maks/min |
| `mat_adalah_prima(n)` | Boolean |

### 2.4 `pustaka/kamus.idk`

```python
impor "pustaka/kamus.idk"
```

| Fungsi | Deskripsi |
|--------|-----------|
| `kamus_nilai(k)` | List semua nilai |
| `kamus_gabung(a, b)` | Kamus baru: `a` ditimpa `b` |
| `kamus_dari_pasangan(pasangan)` | Kamus dari list `[[kunci, nilai], ...]` |
| `kamus_salin(k)` | Salinan dangkal |
| `kamus_ambil(k, kunci, bawaan)` | Nilai jika ada, jika tidak `bawaan` |

---

Lihat contoh lengkap di folder `examples/` (mis. `pustaka_teks.idk`,
`pustaka_daftar.idk`, `berkas.idk`, `konversi.idk`).
