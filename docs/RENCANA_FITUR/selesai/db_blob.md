# Rencana Fitur: BLOB (data biner) untuk SQLite

- **Status:** selesai
- **Target versi:** 1.4.0
- **Pemilik:** Teguh Rijanandi

## 1. Masalah

Kolom BLOB tidak bisa ditulis maupun dibaca:

```indonesiaku
variabel db = db_buka(":memory:")
db_eksekusi(db, "CREATE TABLE berkas (isi BLOB)")
db_eksekusi(db, "INSERT INTO berkas VALUES (?)", [[137, 80, 78, 71]])
# GalatTipe: db: tipe parameter tidak didukung
```

Membaca baris yang sudah berisi BLOB (dibuat alat lain, atau lewat
`x'...'` di SQL) juga gagal: `GalatTipe: db: BLOB belum didukung`.

Akibatnya basis data berisi PNG, PDF, atau hash tidak dapat dibaca sama
sekali oleh Indonesiaku — bahkan untuk kolom lain pada baris yang sama,
karena galat terjadi saat baris dibaca.

Roadmap Fase 10 langkah 2 menyebut "dukungan data biner/BLOB masuk-keluar".

## 2. Keputusan representasi

**BLOB = daftar bulat 0..255**, mengikuti konvensi yang sudah ditetapkan
`pustaka/bita.idk`:

> *"Bahasa ini belum punya tipe byte tersendiri, jadi bita direpresentasikan
> sebagai daftar bulat 0..255."*

Ini **membatalkan** alasan penundaan yang saya tulis di PR #16 ("terblokir
sampai tipe byte Fase 23"). Fase 23 akan memberi tipe byte yang lebih
efisien, tetapi konvensi yang dapat dipakai **sudah ada hari ini**, dan
`bita.idk` sudah menyediakan `ke_bita()`/`ke_teks()` untuk menjembatani teks
dan biner. Menunggu Fase 23 berarti membiarkan basis data biner tidak
terbaca tanpa alasan nyata.

Saat tipe byte asli hadir, `bita.idk` sudah dirancang menjadi pembungkus,
dan jalur BLOB ini dapat menerima tipe baru itu sebagai tambahan — bukan
pengganti yang merusak.

## 3. Semantik

**Menulis (parameter):**

- Daftar yang **seluruh** elemennya bulat 0..255 → di-bind sebagai BLOB.
- Daftar kosong `[]` → BLOB berukuran nol (bukan NULL). SQLite membedakan
  keduanya, jadi kita juga.
- Elemen di luar 0..255, pecahan, atau bukan angka → `GalatNilai` yang
  menyebut **posisi** elemennya; tanpa itu, mencari satu byte salah di
  daftar panjang menjadi tebak-tebakan.
- Daftar bersarang atau berisi teks → galat yang sama.

**Membaca:**

- Kolom BLOB → daftar bulat 0..255.
- BLOB berukuran nol → `[]`.

**Kompatibilitas:** tidak ada perilaku lama yang berubah. Sebelumnya daftar
selalu ditolak, jadi tidak ada program yang bergantung padanya.

## 4. Rencana implementasi

Semua di `src/db.c`:

| Baris | Perubahan |
|---|---|
| ~129–141 | validasi: terima daftar yang sah sebagai BLOB |
| ~191–205 | bind: `sqlite3_bind_blob` / `sqlite3_bind_zeroblob` |
| ~223–238 | baca: `SQLITE_BLOB` → daftar bulat |

**GC-safety (kritis).** Membaca BLOB **mengalokasikan**: satu `ObjList` plus
elemennya. `dbColumn()` dipanggil di dalam gelung yang sedang membangun
baris, jadi daftar yang baru dibuat harus di-`push` ke stack VM sebelum
alokasi berikutnya, kalau tidak GC bisa mengumpulkannya di tengah jalan.
Ini jebakan K19 di `AGENTS.md` dan wajib diuji dengan `-DDEBUG_STRESS_GC`.

Menulis tidak mengalokasikan objek Indonesiaku — hanya menyalin byte ke
SQLite — tetapi `SQLITE_TRANSIENT` tetap dipakai supaya SQLite menyalin
sendiri, karena daftar sumber bisa dikumpulkan GC setelah bind.

## 5. Pengujian

- Golden `examples/db_blob.idk` + `.out`: tulis-baca pulang-pergi, BLOB nol
  byte, byte NUL di tengah, byte 0 dan 255, data non-UTF-8 yang sah sebagai
  biner, dan `bita.ke_bita()`/`bita.ke_teks()` sebagai jembatan teks.
- **Bukti nyata**: tulis PNG 8 byte pertama (mengandung NUL dan byte > 127),
  baca kembali, bandingkan elemen per elemen.
- Tes galat untuk elemen di luar jangkauan.
- Kasus batas: `[]`, `[0]`, `[255]`, `[256]`, `[-1]`, `[1.5]`, `["a"]`,
  `[[1]]`.
- Wajib `-DDEBUG_STRESS_GC` dan ASan/UBSan — ada alokasi di jalur baca.

## 6. Dampak dokumentasi & tooling

- [x] `website/panduan/07_PUSTAKA_STANDAR.md` — bagian `db_*`
- [x] `CHANGELOG.md` + `docs/ROADMAP.md` (langkah 2 Fase 10 **dicentang**)
- [ ] editor — tidak ada builtin baru
- [x] Berkas rencana ini → `selesai/`

## 7. Risiko

Daftar bulat boros memori: satu byte menjadi satu `Value`. Untuk BLOB
berukuran megabita ini mahal. Diterima sadar — alternatifnya menunda fitur
sampai Fase 23, dan basis data biner tetap tidak terbaca sampai saat itu.
Ditandai `ponytail:` di kode dengan jalur peningkatannya.
