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
| `min(a, b, ...)` / `min(daftar)` | Nilai terkecil. Dua bentuk: beberapa argumen, **atau** satu urutan (daftar/rentang/teks) |
| `max(a, b, ...)` / `max(daftar)` | Nilai terbesar. Dua bentuk, sama seperti `min` |
| `abs(x)` | Nilai absolut |
| `akar(x)` | Akar kuadrat (x ≥ 0) |
| `pangkat(x, y)` | x pangkat y. `bulat ** bulat` (pangkat ≥ 0) tetap `bulat`, seperti operator `**` |
| `bulat(x)` | Pembulatan ke bilangan terdekat, **menjauhi nol** pada nilai ½ (`bulat(2.5) == 3`). Mengembalikan `bulat` |
| `bulatkan(x, digit=0)` | Pembulatan ke `digit` angka desimal dengan **banker's rounding** (`bulatkan(2.5) == 2`). Tanpa `digit` hasilnya `bulat`; dengan `digit` hasilnya `desimal` — baca peringatan di bawah |
| `lantai(x)` | Pembulatan ke bawah (floor). Mengembalikan `bulat`, seperti `math.floor` |
| `atap(x)` | Pembulatan ke atas (ceil). Mengembalikan `bulat`, seperti `math.ceil` |
| `bagi_sisa(a, b)` | Daftar `[hasil_bagi, sisa]` sekaligus, konsisten dengan `//` dan `%` |
| `biner(n)` | Teks representasi biner: `"0b1010"` |
| `oktal(n)` | Teks representasi oktal: `"0o12"` |
| `heksa(n)` | Teks representasi heksadesimal: `"0xff"` |
| `acak()` | Angka acak `[0, 1)` |
| `acak_bulat(min, maks)` | Bilangan bulat acak `[min, maks]` |

#### `min` / `max` — dua bentuk pemanggilan

```indonesiaku
cetak(min([5, 2, 9]))        # 2
cetak(max([5, 2, 9]))        # 9
cetak(max(5, 2, 9))          # 9
coba {
    cetak(max([]))
} kecuali e {
    cetak(e.pesan)           # max() dari urutan kosong
}
```

Bentuk lama `max(a, b, c)` tetap berlaku — tidak ada program lama yang rusak.
Bentuk baru menerima daftar, rentang, atau teks. Urutan **kosong** menimbulkan
galat, jadi periksa `panjang()` dulu bila daftarnya mungkin kosong.

#### ⚠️ `bulat` vs `bulatkan` — jangan tertukar!

Ini jebakan paling sering. Kedua fungsi membulatkan, tetapi **aturan pada nilai
tepat ½ berbeda**:

| x | `bulat(x)` (menjauhi nol) | `bulatkan(x)` (banker's rounding) |
|---|---|---|
| `0.5` | `1` | `0` |
| `1.5` | `2` | `2` |
| `2.5` | `3` | `2` |
| `3.5` | `4` | `4` |
| `-0.5` | `-1` | `0` |

```indonesiaku
cetak(bulat(0.5), bulat(1.5), bulat(2.5), bulat(3.5), bulat(-0.5))
# 1 2 3 4 -1

cetak(bulatkan(0.5), bulatkan(1.5), bulatkan(2.5), bulatkan(3.5), bulatkan(-0.5))
# 0 2 2 4 0
```

`bulatkan` memakai *banker's rounding* (pembulatan ke genap terdekat), persis
seperti `round()` di Python: nilai tepat ½ dibulatkan ke bilangan **genap**
terdekat, bukan selalu ke atas. Aturan ini sengaja dipakai di dunia keuangan
dan statistik karena tidak menggeser rata-rata ke atas saat banyak angka
dibulatkan.

**Pilih yang mana?**
- Butuh aturan sekolah ("½ dibulatkan ke atas")? Pakai **`bulat`**.
- Butuh hasil yang sama persis dengan Python, atau butuh argumen `digit`? Pakai **`bulatkan`**.

Hanya `bulatkan` yang menerima argumen kedua, yaitu jumlah angka di belakang
koma (boleh negatif untuk membulatkan ke puluhan/ratusan):

```indonesiaku
cetak(bulatkan(3.14159, 2))    # 3.14
cetak(bulatkan(1234.5678, -2)) # 1200.0  (ada 'digit' -> hasilnya desimal)
```

#### `bagi_sisa` — hasil bagi dan sisa sekaligus

```indonesiaku
hb = bagi_sisa(17, 5)
cetak(hb)                     # [3, 2]
cetak(hb[0], hb[1])           # 3 2
cetak(17 // 5, 17 % 5)        # 3 2  -- hasil yang sama
cetak(bagi_sisa(-7, 2))       # [-4, 1]
coba {
    bagi_sisa(5, 0)
} kecuali e {
    cetak(e.pesan)            # Pembagian dengan nol.
}
```

Hasilnya selalu konsisten dengan operator `//` (pembagian bulat ke bawah) dan
`%`, termasuk untuk bilangan negatif. Pembagi `0` menimbulkan galat.

#### `biner`, `oktal`, `heksa` — angka ke teks berbasis lain

```indonesiaku
cetak(biner(10))      # 0b1010
cetak(oktal(10))      # 0o12
cetak(heksa(255))     # 0xff
cetak(biner(-255))    # -0b11111111
coba {
    biner(1.5)
} kecuali e {
    cetak(e.pesan)    # biner() memerlukan bilangan bulat
}
```

Ketiganya mengembalikan **teks** (bukan angka) lengkap dengan awalan `0b`,
`0o`, atau `0x`; tanda minus diletakkan di depan awalan. Argumen harus
bilangan bulat — pecahan menimbulkan `GalatTipe`. `bulat` sampai 64 bit
dikonversi dengan tepat; hanya `desimal` di luar jangkauan int64 yang
menimbulkan `GalatNilai`.

### 1.2 String / Teks

| Fungsi | Deskripsi |
|--------|-----------|
| `panjang(x)` | Panjang teks (karakter), list, kamus, atau rentang. Mengembalikan `bulat` |
| `rentang([awal,] akhir[, langkah])` | Deret bilangan bulat malas: `rentang(5)` → 0, 1, 2, 3, 4. Dipakai di `untuk`, bisa `panjang()`, `r[i]`, `x dalam r` |
| `huruf_besar(s)` | Ubah ke huruf kapital |
| `huruf_kecil(s)` | Ubah ke huruf kecil |
| `ganti(teks, cari, ganti)` | Ganti semua kemunculan substring |
| `karakter(teks, i)` | Karakter (string 1 huruf) pada indeks `i` |
| `potong(teks, mulai, jumlah)` | Substring mulai dari `mulai` sepanjang `jumlah` |
| `ord(karakter)` | Kode Unicode dari karakter sebagai `bulat` (`ord("é") == 233`) |
| `chr(kode)` | Karakter dari kode Unicode 0–1114111 (`chr(233) == "é"`) |

Panjang, indeks, `karakter`, dan `potong` menghitung **karakter**, bukan byte,
sehingga teks beraksen dan emoji aman: `panjang("héllo") == 5`,
`"café"[3] == "é"`.

Pengindeksan string juga tersedia: `teks[i]` mengembalikan karakter ke-`i`
(string bersifat immutable — tidak dapat diubah lewat indeks).

### 1.3 List

| Fungsi | Deskripsi |
|--------|-----------|
| `tambah(list, nilai)` | Tambah elemen ke akhir list |
| `hapus(list)` | Hapus & kembalikan elemen terakhir |
| `hapus_pada(list, i)` | Hapus & kembalikan elemen pada indeks `i` (geser sisanya) |
| `sisip(list, i, nilai)` | Sisipkan `nilai` pada indeks `i` |
| `panjang(list)` | Jumlah elemen |
| `jumlah(d, awal=0)` | Total semua elemen angka, dimulai dari `awal`. Semua `bulat` → hasil `bulat`; satu `desimal` → hasil `desimal` |
| `semua(d)` | `benar` jika **semua** elemen bernilai benar |
| `ada_yang(d)` | `benar` jika **ada minimal satu** elemen bernilai benar |
| `terurut(d, menurun=salah)` | **Daftar baru** yang terurut; argumen aslinya tidak berubah |
| `terbalik(d)` | **Daftar baru** dengan urutan dibalik |
| `pasangkan(a, b, ...)` | Gabungkan beberapa urutan jadi daftar pasangan (zip) |
| `peta(f, d)` | Daftar hasil penerapan fungsi `f` ke tiap elemen (map) |
| `saring(f, d)` | Daftar elemen yang membuat `f` bernilai benar (filter) |

Literal: `[1, 2, 3]`. Indeks baca/tulis: `list[i]`, `list[i] = x`.

> Kedelapan fungsi baru di atas (`jumlah`, `semua`, `ada_yang`, `terurut`,
> `terbalik`, `pasangkan`, `peta`, `saring`) tidak hanya menerima daftar —
> **rentang** dan **teks** juga boleh, karena ketiganya sama-sama urutan.
> Semuanya mengembalikan **daftar baru** dan tidak pernah mengubah argumennya.

#### `jumlah` — menjumlahkan isi urutan

```indonesiaku
cetak(jumlah([1, 2, 3, 4]))     # 10
cetak(jumlah([]))               # 0
cetak(jumlah([1, 2], 10))       # 13
cetak(jumlah(rentang(1, 101)))  # 5050
coba {
    jumlah([1, "dua"])
} kecuali e {
    cetak(e.pesan)              # jumlah() hanya menerima angka
}
```

Urutan kosong menghasilkan `0` (bukan galat). Argumen kedua `awal` adalah nilai
awal penjumlahan, berguna untuk menambahkan ke total yang sudah ada. Semua
elemen harus angka — elemen lain menimbulkan `GalatTipe`.

#### `semua` dan `ada_yang` — memeriksa syarat sekaligus

```indonesiaku
cetak(semua([benar, benar]))    # benar
cetak(semua([1, 0, 3]))         # salah
cetak(semua([]))                # benar
cetak(ada_yang([salah, benar])) # benar
cetak(ada_yang([]))             # salah
```

Keduanya memakai aturan *truthiness* Indonesiaku (lihat `ke_logika`), jadi `0`
dan `""` dianggap salah. Perhatikan dua kasus kosong yang terasa aneh tetapi
memang benar secara logika (dan sama dengan Python): `semua([])` adalah
`benar` ("tidak ada elemen yang melanggar"), sedangkan `ada_yang([])` adalah
`salah` ("tidak ada satu pun elemen yang memenuhi").

#### `terurut` dan `terbalik` — selalu daftar baru

```indonesiaku
d = [3, 1, 2]
cetak(terurut(d))               # [1, 2, 3]
cetak(terurut(d, benar))        # [3, 2, 1]
cetak(d)                        # [3, 1, 2]  -- argumen asli tetap utuh
cetak(terurut(["pisang", "apel"]))  # ["apel", "pisang"]
cetak(terbalik([1, 2, 3]))      # [3, 2, 1]
cetak(terbalik("halo"))         # ["o", "l", "a", "h"]
coba {
    terurut([1, "dua"])
} kecuali e {
    cetak(e.pesan)              # terurut() tidak dapat membandingkan angka dengan teks
}
```

Argumen kedua `terurut` adalah `menurun`: isi `benar` untuk urutan dari besar
ke kecil. Kirim secara **posisional** (`terurut(d, benar)`) — fungsi bawaan
belum menerima argumen bernama, jadi `terurut(d, menurun=benar)` ditolak.
Isi daftar harus **seragam** — campuran angka dan teks tidak bisa
dibandingkan dan menimbulkan `GalatTipe`. Perhatikan bahwa `terbalik("halo")`
mengembalikan daftar karakter, bukan teks; gunakan `teks_balik` dari
`pustaka/teks.idk` bila Anda butuh teks.

#### `pasangkan` — menyandingkan beberapa urutan (zip)

```indonesiaku
cetak(pasangkan([1, 2, 3], ["a", "b"]))
# [[1, "a"], [2, "b"]]
cetak(pasangkan([1, 2], ["a", "b"], [benar, salah]))
# [[1, "a", benar], [2, "b", salah]]

untuk p dalam pasangkan(["Ani", "Budi"], [80, 95]) {
    cetak(p[0], "mendapat nilai", p[1])
}
# Ani mendapat nilai 80
# Budi mendapat nilai 95
```

Panjang hasilnya mengikuti argumen **terpendek** — kelebihan elemen dibuang
tanpa peringatan (lihat contoh pertama: `3` tidak muncul karena daftar kedua
hanya berisi dua elemen).

#### `peta` dan `saring` — mengolah tiap elemen

```indonesiaku
fungsi kuadrat(x) { kembalikan x * x }
fungsi ganjil(x) { kembalikan x % 2 == 1 }

cetak(peta(kuadrat, [1, 2, 3, 4]))   # [1, 4, 9, 16]
cetak(saring(ganjil, rentang(10)))   # [1, 3, 5, 7, 9]
cetak(jumlah(peta(kuadrat, saring(ganjil, rentang(10)))))  # 165
```

Perhatikan urutan argumen: **fungsi dulu, baru urutan** (kebalikan dari
`daftar_peta`/`daftar_saring` di `pustaka/daftar.idk`). Keduanya langsung
mengembalikan daftar jadi (*eager*, bukan malas seperti `rentang`), sehingga
hasilnya bisa langsung di-`panjang()`, diindeks, atau dirangkai seperti pada
baris terakhir.

### 1.4 Kamus (Dictionary)

| Fungsi | Deskripsi |
|--------|-----------|
| `kamus()` | Buat kamus kosong |
| `punya(kamus, kunci)` | Apakah kunci ada → boolean. Kunci boleh nilai apa pun yang hashable |
| `kunci(kamus)` | List semua kunci, urutan penyisipan |
| `hapus_kunci(kamus, kunci)` | Hapus pasangan kunci-nilai |
| `enumerasi(urutan, awal=0)` | List pasangan `[indeks, nilai]` untuk list, teks, rentang, atau kamus |

Akses: `k[kunci]`, `k[kunci] = nilai`. Kunci boleh teks, angka (`bulat`/
`desimal`), `benar`/`salah`, atau `kosong`; `1` dan `1.0` adalah kunci yang
sama. Daftar/kamus/rentang tidak bisa menjadi kunci (`GalatTipe`).

### 1.5 Konversi & Format

| Fungsi | Deskripsi |
|--------|-----------|
| `ke_angka(teks)` | String → angka (gagal dapat ditangkap) |
| `ke_teks(nilai)` | Nilai apa pun → string |
| `ke_bulat(x)` | Angka/teks/boolean → `bulat`, **memotong ke arah nol** |
| `ke_logika(x)` | Nilai apa pun → `benar`/`salah` menurut aturan *truthiness* |
| `repr(x)` | Representasi untuk **debug**: teks ditampilkan **dengan tanda kutip** |
| `format(pola, ...)` | Ganti setiap `{}` di pola dengan argumen |
| `jenis(x)` | Nama tipe: `"angka"`, `"string"`, `"boolean"`, `"kosong"`, `"list"`, `"kamus"`, `"objek"`, `"kelas"`, `"berkas"` |

#### `ke_bulat` — memotong, bukan membulatkan

```indonesiaku
cetak(ke_bulat(3.9))     # 3
cetak(ke_bulat(-3.9))    # -3  <- memotong ke arah nol
cetak(lantai(-3.9))      # -4  <- bandingkan: floor ke bawah
cetak(ke_bulat("12"))    # 12
cetak(ke_bulat(benar))   # 1
coba {
    ke_bulat("dua belas")
} kecuali e {
    cetak(e.pesan)       # ke_bulat() gagal: 'dua belas' bukan bilangan bulat
}
```

Hati-hati pada bilangan negatif: `ke_bulat` membuang bagian desimal (menuju
nol), sedangkan `lantai` selalu turun. Untuk angka positif keduanya sama.
Teks yang tidak bisa dibaca sebagai bilangan bulat menimbulkan `GalatNilai`
yang bisa ditangkap `coba/kecuali`.

#### `ke_logika` — aturan benar/salah yang dipakai seluruh bahasa

```indonesiaku
cetak(ke_logika(0), ke_logika(""), ke_logika([]), ke_logika(kosong))
# salah salah salah salah
cetak(ke_logika(5), ke_logika("hai"), ke_logika([0]))
# benar benar benar
```

Yang dianggap **salah**: `salah`, `kosong`, angka `0`, teks kosong `""`, serta
daftar, kamus, dan rentang yang kosong. Semua nilai lain `benar` — termasuk
`[0]`, karena daftarnya sendiri tidak kosong. Aturan ini sama dengan yang
dipakai `jika`, `selama`, `semua`, dan `ada_yang`.

#### `repr` — teks dengan tanda kutip untuk debug

```indonesiaku
cetak(ke_teks("halo"))   # halo
cetak(repr("halo"))      # "halo"
cetak(repr(5))           # 5
cetak(repr(benar), repr(kosong))   # benar kosong
cetak(repr([1, "a"]))    # [1, "a"]
```

Pakai `ke_teks` untuk ditampilkan ke pengguna, dan `repr` saat men-*debug* —
tanda kutip membuat Anda bisa membedakan angka `5` dari teks `"5"`, dan
melihat spasi tersembunyi di ujung teks.


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
| `adalah_instansi(objek, kelas)` | Apakah `objek` instansi `kelas` (termasuk lewat pewarisan) → boolean |
| `bisa_dipanggil(x)` | Apakah `x` bisa dipanggil seperti fungsi → boolean |
| `id(x)` | Identitas objek berupa angka, konsisten dengan operator `adalah` |
| `argumen` | Variabel global: list argumen baris perintah |

#### `adalah_instansi` dan `bisa_dipanggil` — memeriksa tipe dengan aman

```indonesiaku
kelas Hewan {
    init(nama) { diri.nama = nama }
}
kelas Anjing < Hewan {
    init(nama) { super.init(nama) }
}
a = Anjing("Rex")
cetak(adalah_instansi(a, Anjing))   # benar
cetak(adalah_instansi(a, Hewan))    # benar  <- lewat pewarisan
cetak(adalah_instansi(5, Hewan))    # salah

fungsi sapa(n) { kembalikan "Halo " + n }
cetak(bisa_dipanggil(sapa))         # benar
cetak(bisa_dipanggil(Anjing))       # benar  <- kelas dipanggil untuk membuat objek
cetak(bisa_dipanggil(5))            # salah
```

`adalah_instansi` ikut memperhitungkan **pewarisan**, jadi lebih tepat daripada
membandingkan `jenis(x)` yang hanya mengembalikan `"objek"` untuk semua
instansi. `bisa_dipanggil` berguna sebelum memanggil nilai yang Anda terima
sebagai parameter, misalnya di `peta`/`saring`.

#### `id` — identitas objek

```indonesiaku
a = [1, 2]
b = a
c = [1, 2]
cetak(id(a) == id(b))   # benar  <- a dan b objek yang sama
cetak(id(a) == id(c))   # salah  <- isinya sama, objeknya beda
cetak(a adalah b)       # benar
cetak(a adalah c)       # salah
```

`id(x) == id(y)` selalu memberi jawaban yang sama dengan `x adalah y`. Pakai
`id` saat Anda ingin **mencatat atau mencetak** identitas (misalnya untuk
men-*debug* daftar yang tidak sengaja dibagi-pakai); pakai `adalah` saat cukup
membandingkan. Nilainya tidak punya arti di luar jalannya program — jangan
disimpan ke berkas.


### 1.9 Soket TCP

Server mendengarkan di `127.0.0.1` saja (localhost). Semua kesalahan dapat
ditangkap `coba/kecuali`. Untuk server HTTP, gunakan `pustaka/http.idk`.

| Fungsi | Deskripsi |
|--------|-----------|
| `soket_dengar(port)` | Buat soket server yang mendengarkan di `127.0.0.1:port` |
| `soket_terima(server)` | Blocking: terima koneksi → soket klien |
| `soket_baca(klien)` | Baca data yang diterima (satu recv) → string |
| `soket_tulis(klien, teks)` | Kirim teks → jumlah byte |
| `soket_tutup(soket)` | Tutup soket (server atau klien) |

### 1.10 Basis Data (SQLite)

SQLite tertanam langsung di dalam `indk` — tidak ada yang perlu dipasang.
Semua kesalahan dapat ditangkap `coba/kecuali`.

| Fungsi | Deskripsi |
|--------|-----------|
| `db_buka(jalur)` | Buka/buat basis data → handle. `":memory:"` = di memori saja |
| `db_eksekusi(db, sql, params)` | Jalankan satu pernyataan (DDL/DML) → `kosong` |
| `db_ambil_semua(db, sql, params)` | Jalankan kueri → daftar kamus (satu kamus per baris) |
| `db_ambil_satu(db, sql, params)` | Baris pertama saja → kamus, atau `kosong` bila tidak ada |
| `db_baris_terpengaruh(db)` | Jumlah baris yang diubah pernyataan terakhir |
| `db_id_terakhir(db)` | `id` baris hasil INSERT terakhir |
| `db_mulai(db)` | Mulai transaksi |
| `db_komit(db)` | Simpan transaksi |
| `db_batal(db)` | Batalkan transaksi |
| `db_tutup(db)` | Tutup handle; aman dipanggil dua kali |

`params` boleh dikosongkan bila SQL tidak memakai `?`.

**Selalu pakai `?`, jangan pernah merangkai SQL dengan `+`.** Nilai yang
dikirim lewat `params` disimpan sebagai data, tidak pernah dieksekusi:

```indonesiaku
variabel db = db_buka(":memory:")
db_eksekusi(db, "CREATE TABLE siswa (id INTEGER PRIMARY KEY, nama TEXT, nilai REAL)")
db_eksekusi(db, "INSERT INTO siswa (nama, nilai) VALUES (?, ?)", ["Budi", 85.5])

untuk b dalam db_ambil_semua(db, "SELECT nama, nilai FROM siswa WHERE nilai > ?", [80]) {
    cetak(b["nama"], b["nilai"])        # Budi 85.5
}
db_tutup(db)
```

Pemetaan tipe: `NULL` ↔ `kosong`, `INTEGER`/`REAL` ↔ angka, `TEXT` ↔ teks.

Batasan yang disengaja — semuanya bergalat, bukan diam-diam salah:

- **Satu pernyataan per panggilan.** `"INSERT ...; DROP TABLE ..."` ditolak
  sebelum apa pun dijalankan.
- **Jumlah `?` harus sama** dengan panjang `params`.
- **Hanya `?` anonim**, bukan `:nama` atau `?1`.
- **`BLOB` belum didukung** (menunggu tipe `bita`), dan `INTEGER` di luar
  jangkauan tepat angka desimal (±2⁵³) ditolak daripada dibulatkan diam-diam.
- Handle yang sudah ditutup → galat, bukan crash.

Contoh lengkap, termasuk transaksi dan uji anti-injeksi:

{{embed:examples/db_sqlite.idk}}

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

### 2.5 `pustaka/json.idk`

```python
impor "pustaka/json.idk"
```

| Fungsi | Deskripsi |
|--------|-----------|
| `json_teks(nilai)` | Serialisasi kamus/list/angka/string/boolean/kosong → string JSON |
| `json_urai(teks)` | Parsing string JSON → nilai Indonesiaku (kesalahan dapat ditangkap) |

Contoh:
```python
db = kamus()
db["todos"] = [kamus()]
teks = json_teks(db)          # {"todos":[{}]}
kembali = json_urai(teks)     # kamus dengan list
```

Digunakan oleh aplikasi contoh `examples/todo_cli.idk` (CRUD Todo-list
berbasis berkas JSON) dan `examples/todo_demo.idk`.

### 2.6 `pustaka/http.idk`

```python
impor "pustaka/http.idk"
```

| Fungsi | Deskripsi |
|--------|-----------|
| `http_urai_request(teks)` | Parse request HTTP mentah → kamus `{metode, path, versi, header, badan}` |
| `http_response(kode, tipe_konten, badan)` | Bangun string response HTTP |
| `http_response_json(kode, badan)` | Pintasan response JSON |

Digunakan oleh `examples/todo_server.idk` — REST API Todo-list berbasis HTTP
di atas soket TCP.

---

Lihat contoh lengkap di folder `examples/` (mis. `pustaka_teks.idk`,
`pustaka_daftar.idk`, `berkas.idk`, `konversi.idk`, `json_pakai.idk`,
`todo_demo.idk`, `todo_server.idk`, `http_pakai.idk`).

---

## 3. Aplikasi Contoh: Todo-list JSON DB

Dua aplikasi contoh lengkap menunjukkan penggunaan pustaka standar untuk
membangun aplikasi nyata dengan penyimpanan JSON.

### 3.1 CRUD berbasis command-line

`examples/todo_cli.idk` — memetakan operasi REST ke perintah CLI
(`tambah`=POST, `daftar`/`lihat`=GET, `selesai`/`ubah`=PUT, `hapus`=DELETE):

{{embed:examples/todo_cli.idk}}

### 3.2 Server REST API (HTTP)

`examples/todo_server.idk` — server HTTP sungguhan (soket TCP) dengan endpoint
`GET/POST/PUT/DELETE /todos`:

{{embed:examples/todo_server.idk}}
