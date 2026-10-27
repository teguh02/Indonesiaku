# Rencana Fitur: kelas abstrak

- **Status:** selesai
- **Target versi:** 1.1.0
- **Pemilik:** Teguh Rijanandi

## 1. Masalah

Kelas dasar yang menuntut subkelasnya menyediakan metode tertentu tidak punya
cara menyatakan itu. Yang bisa ditulis sekarang hanya penjaga manual, dan
galatnya baru muncul saat metodenya dipanggil — bisa jauh setelah objeknya
dibuat:

```python
kelas Bentuk {
    luas() { naikkan GalatTipe }     # berharap subkelas menimpanya
}

b = Bentuk()        # lolos
# ... jauh kemudian ...
b.luas()            # baru di sini ketahuan
```

Yang diinginkan: salah ditangkap **saat objeknya dibuat**, dan kontraknya
terbaca di definisi kelas.

## 2. Sintaks usulan

Dekorator `@abstrak` pada metode, sejalan dengan tiga dekorator bawaan yang
sudah ada:

```python
kelas Bentuk {
    @abstrak
    luas() { }

    @abstrak
    keliling() { }

    # metode biasa boleh memakai yang abstrak
    ringkas() { kembalikan f"luas {diri.luas()}" }
}

kelas Persegi < Bentuk {
    init(sisi) { diri.sisi = sisi }
    luas() { kembalikan diri.sisi * diri.sisi }
    keliling() { kembalikan 4 * diri.sisi }
}

cetak(Persegi(3).ringkas())   # luas 9
Bentuk()                      # galat: masih punya metode abstrak
```

## 3. Semantik

- Kelas yang **punya metode abstrak belum terisi tidak bisa dibuat objeknya**.
  Galatnya muncul di `Bentuk()`, menyebut nama metode yang belum diisi.
- Subkelas yang menimpa **semua** metode abstrak menjadi kelas biasa.
- Subkelas yang menimpa **sebagian** tetap abstrak — dan itu sah, selama tidak
  dibuat objeknya.
- Subkelas boleh menambah metode abstrak baru.
- Badan metode abstrak diabaikan (konvensi: kosong). Memanggilnya lewat
  `super` tetap mungkin; yang dilarang hanya membuat objek kelas abstraknya.
- Bekerja dengan pewarisan berganda: metode abstrak dari induk mana pun ikut
  terhitung, dan terisi oleh implementasi dari induk mana pun sesuai MRO.

## 4. Rencana implementasi

Kemalasan utama: **tidak ada mekanisme baru.** `@abstrak` adalah nilai
`SPECIAL_*` keempat, menumpang seluruh jalur yang sudah dibangun untuk
`@properti`/`@metode_statis`/`@metode_kelas`.

1. `src/object.h` — `#define SPECIAL_ABSTRAK 4`.
2. `src/compiler.c` — `method()` mengenali nama `abstrak` seperti tiga
   lainnya; `FunctionType` tetap `TYPE_METHOD`.
3. `src/vm.c` — saat memanggil kelas (`callValue`, `case OBJ_CLASS`),
   periksa apakah ada metode abstrak yang belum terisi; kalau ada, galat
   sebelum instance dibuat.

**Cara memeriksa, dan kenapa bukan saat pendaftaran.** Sebuah metode abstrak
dianggap terisi kalau kelas yang lebih awal di MRO mendefinisikannya sebagai
metode biasa. Karena `defineMethod` sudah menulis penanda 0 saat subkelas
menimpa nama ber-penanda, `specialKind()` yang sudah ada menjawab persis
pertanyaan itu — penimpaan oleh kelas mana pun di MRO sudah tercermin.

Jadi pemeriksaannya: telusuri MRO, kumpulkan nama ber-`SPECIAL_ABSTRAK`,
lalu untuk tiap nama tanyakan `specialKind(klass, nama)`. Kalau masih
`SPECIAL_ABSTRAK`, berarti belum ada yang mengisinya.

`ponytail:` pemeriksaan ini jalan **tiap kali kelas dipanggil**, biayanya
sepanjang MRO × jumlah metode abstrak. Kelas nyata punya nol metode abstrak,
jadi jalur normal hanya satu pemeriksaan kapasitas tabel per leluhur. Kalau
nanti terukur mengganggu, simpan satu bendera `isAbstract` yang dihitung
sekali saat kelasnya selesai dibuat — tapi itu menuntut hati-hati terhadap
metode yang ditambahkan setelahnya.

## 5. Rencana uji

- Golden `examples/oop/kelas_abstrak.idk` + expected.
- Kasus: kelas abstrak tidak bisa dibuat; subkelas lengkap bisa; subkelas
  sebagian tetap abstrak tetapi boleh diwarisi lagi sampai lengkap; metode
  biasa di kelas abstrak memanggil metode abstrak lewat subkelas; metode
  abstrak lewat pewarisan berganda (satu induk menuntut, induk lain mengisi);
  subkelas menambah metode abstrak baru; `adalah_instansi` tetap benar.
- Galat: membuat objek kelas abstrak; membuat objek subkelas yang baru
  mengisi sebagian.
- Wajib lewat build sanitizer atas seluruh golden + `tests/galat/`.

## 6. Dampak dokumentasi & tooling

- [x] `website/panduan/03_PANDUAN_SINTAKS.md`
- [x] `editors/vscode-indonesiaku/` — `@abstrak` + snippet
- [x] `README.md`, `CHANGELOG.md`, `docs/ROADMAP.md`,
      `docs/ANALISIS/02_PETA_FITUR_PYTHON.md`
- [x] `AGENTS.md` bila ada jebakan baru
- [x] Rencana ini → `selesai/`
- [x] Fase 4 ditandai **selesai** di ROADMAP

## 7. Risiko & alternatif

**Alternatif yang ditolak: `kelas abstrak Bentuk { ... }`** — kata kunci di
tingkat kelas. Butuh token baru dan bendera tersendiri, lalu tetap harus
memeriksa metode mana yang belum terisi. Dekorator per metode menyatakan
kontraknya lebih tepat dan menumpang jalur yang sudah ada.

**Alternatif yang ditolak: meniru `abc.ABCMeta` Python** — metaclass belum
ada di Indonesiaku, dan menambahkannya demi satu fitur jelas tidak sebanding.

**Risiko: `@abstrak` bersama `@properti` pada metode yang sama.** Satu metode
hanya menyimpan satu penanda, jadi kombinasinya tidak didukung. Ditolak
compiler dengan pesan jelas, bukan dibiarkan menang diam-diam.


## 8. Catatan hasil

- **Rencana meleset satu langkah: metode abstrak tidak boleh ikut
  diratakan.** `OP_INHERIT` menyalin `own` tiap leluhur ke `methods`, jadi
  badan kosong `@abstrak` milik induk pertama menang atas implementasi nyata
  dari induk kedua. Pola mixin (`Gabung < Kontrak, Pekerja`) menghasilkan
  `kosong` sebelum perataan dibuat melewati nama ber-`SPECIAL_ABSTRAK`.
- **`specialKind()` ternyata tidak cukup** untuk memutuskan "sudah terisi":
  ia mengembalikan penanda PERTAMA di MRO, yaitu penanda abstrak itu sendiri.
  Yang benar: cari kelas mana pun di MRO yang punya nama itu di `own` tanpa
  penanda abstrak.
