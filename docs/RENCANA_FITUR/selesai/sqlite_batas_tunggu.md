# Rencana Fitur: batas tunggu (busy timeout) SQLite

- **Status:** selesai
- **Target versi:** 1.4.0
- **Pemilik:** Teguh Rijanandi

## 1. Masalah

Dua proses Indonesiaku yang menulis ke berkas basis data yang sama langsung
gagal, bukan menunggu:

```indonesiaku
variabel db = db_buka("data.db")
db_eksekusi(db, "INSERT INTO catatan (isi) VALUES (?)", ["halo"])
# GalatNilai: SQLite: database is locked
```

SQLite mengunci seluruh berkas saat menulis. Tanpa batas tunggu, kunci yang
dipegang proses lain **selama satu milidetik pun** membuat proses kedua
langsung menyerah. Default SQLite memang 0 — tidak menunggu sama sekali.

Ini bukan masalah teoretis: `examples/todo_server.idk` menyimpan data lewat
berkas, dan begitu ada dua permintaan yang menulis bersamaan, salah satunya
gagal dengan galat yang membingungkan pengguna.

Roadmap Fase 10 langkah 2 menyebut "busy timeout" sebagai bagian melengkapi
SQLite sebelum driver lain dikerjakan.

## 2. Sintaks usulan

Tidak ada kata kunci baru. Satu builtin baru:

```indonesiaku
variabel db = db_buka("data.db")

db_batas_tunggu(db, 5000)    # tunggu sampai 5000 ms sebelum menyerah
db_batas_tunggu(db, 0)       # kembali ke perilaku lama: langsung gagal
```

Builtin, bukan argumen tambahan `db_buka()`, karena `db_buka` sudah
mensyaratkan tepat satu argumen dan menambah parameter opsional di sana
mengubah pesan galat yang sudah diuji.

## 3. Semantik

- `db_batas_tunggu(db, milidetik)` → `kosong`.
- `milidetik` harus `bulat` ≥ 0. `desimal`, negatif, atau tipe lain →
  `GalatTipe`/`GalatNilai` yang **dapat ditangkap** `coba/kecuali`.
- Nilai di atas `INT_MAX` ditolak: `sqlite3_busy_timeout` menerima `int` C,
  dan menyempitkan diam-diam akan memberi batas tunggu yang salah.
- Handle yang sudah ditutup → `GalatNilai`, sama seperti builtin `db_*` lain.
- Batas berlaku per koneksi, sampai diubah lagi. Tidak ada status global.
- Tidak mengubah perilaku program lama: tanpa memanggilnya, batas tetap 0.

## 4. Rencana implementasi

| Bagian | Perubahan |
|---|---|
| `src/db.c` | `nativeFnDbBatasTunggu` — validasi lalu `sqlite3_busy_timeout` |
| `src/native.h` | deklarasi |
| `src/vm.c` | `defineNative("db_batas_tunggu", ...)` |

Tidak ada opcode, tipe objek, atau aturan parsing baru.

**GC-safety:** tidak ada alokasi sama sekali — hanya membaca dua argumen yang
sudah ada di slot `args[]` yang ter-root. Tidak ada jebakan GC.

## 5. Pengujian

- Golden: `examples/db_batas_tunggu.idk` + `tests/expected/db_batas_tunggu.out`.
  Menguji penyetelan batas, nilai 0, dan **pembuktian nyata**: dua koneksi ke
  berkas yang sama, satu memegang transaksi tulis, koneksi kedua dengan batas
  tunggu harus mengembalikan galat `database is locked` setelah benar-benar
  menunggu — diukur dengan `jam()`, bukan diasumsikan.
- Tes galat: `tests/galat/db_batas_tunggu_tipe.idk` + `.err` untuk argumen
  bukan bulat.
- Kasus batas wajib: nol, negatif, desimal, teks, handle tertutup, argumen
  kurang/lebih, nilai melampaui `INT_MAX`.
- Jalankan dengan `-DDEBUG_STRESS_GC` — keluaran harus identik.

Sanitizer tidak wajib di sini karena tidak ada alokasi, tetapi suite penuh
tetap dijalankan.

## 6. Dampak dokumentasi & tooling

- [x] `website/panduan/07_PUSTAKA_STANDAR.md` — tabel builtin `db_*`
- [x] `editors/vscode-indonesiaku/extension.js` — BUILTINS
- [x] `editors/vscode-indonesiaku/syntaxes/indonesiaku.tmLanguage.json`
- [ ] `website/panduan/03_PANDUAN_SINTAKS.md` — tidak perlu, bukan sintaks
- [ ] snippet — tidak perlu, pemanggilan satu baris
- [x] `CHANGELOG.md` + `docs/ROADMAP.md`
- [ ] `README.md` — tidak perlu, bukan sintaks terlihat
- [ ] `AGENTS.md` — tidak ada jebakan baru
- [x] Berkas rencana ini → `docs/RENCANA_FITUR/selesai/`

## 7. Risiko & alternatif

**Risiko:** batas tunggu yang panjang membuat program tampak menggantung.
Dikurangi dengan dokumentasi yang menyebut batas wajar (beberapa detik) dan
mempertahankan default 0 — pengguna harus memilih secara sadar.

**Alternatif yang ditolak:**

- *Parameter opsional pada `db_buka()`* — mengubah pesan galat yang sudah
  punya tes, dan menyembunyikan setelan koneksi di tempat yang jarang dibaca.
- *`sqlite3_busy_handler` dengan callback Indonesiaku* — menjalankan kode
  pengguna dari dalam callback C saat kunci sedang ditahan berisiko
  reentrancy dan GC di jalur yang sangat dalam (lihat K19 di `AGENTS.md`).
  Tidak sepadan untuk kebutuhan yang diselesaikan satu angka.
- *WAL mode otomatis* — mengubah format berkas basis data pengguna tanpa
  diminta. Itu keputusan mereka, bukan keputusan kita.

## 8. Yang TIDAK dikerjakan di sini

Langkah 2 Fase 10 juga menyebut **BLOB masuk-keluar**. Itu **terblokir**:
Indonesiaku belum punya tipe byte (`OBJ_BYTES` tidak ada di `src/object.h`),
dan tipe byte dijadwalkan di **Fase 23**. Memetakan BLOB ke `teks` akan rusak
untuk data biner yang mengandung NUL atau bukan UTF-8 yang sah.

`src/db.c` sudah menolaknya secara eksplisit dan jujur
("db: BLOB belum didukung"), jadi tidak ada bug yang perlu ditutup — hanya
fitur yang menunggu fondasinya. Rencana BLOB dibuat setelah Fase 23.
