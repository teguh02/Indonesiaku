# Rencana Fitur: Basis data SQLite

- **Status:** usulan
- **Target versi:** 0.9.0 (Fase 3) — **dimajukan dari Fase 8**
- **Pemilik:** Teguh
- **Keputusan terkait:** calon K18

## 1. Masalah

Indonesiaku tidak punya akses basis data sama sekali. Satu-satunya cara
menyimpan data terstruktur adalah berkas JSON lewat `pustaka/json.idk`, yang
dipakai `examples/todo_cli.idk`:

```indonesiaku
variabel db = json_urai(baca_semua(f))     # muat SELURUH berkas
tambah(db["todo"], item)
tulis_semua(f, json_teks(db))              # tulis ULANG seluruh berkas
```

Konsekuensinya: tidak ada kueri, tidak ada indeks, tidak ada transaksi, dan
setiap penulisan menulis ulang seluruh berkas. Kehilangan daya di tengah
`tulis_semua` merusak seluruh basis data. Untuk mata kuliah basis data —
konteks utama pemakaian bahasa ini — ini membuat Indonesiaku tidak terpakai.

`docs/ROADMAP.md` menaruh `sqlite` di **Fase 8**, satu paket dengan FFI.
Penempatan itu keliru: FFI tidak dibutuhkan (lihat §7).

## 2. Sintaks usulan

Tidak ada sintaks baru — fungsi biasa, mengikuti pola handle yang sudah
dipakai `buka_berkas`/`tutup` dan soket.

```indonesiaku
variabel db = db_buka("sekolah.db")

db_eksekusi(db, "CREATE TABLE IF NOT EXISTS siswa (id INTEGER PRIMARY KEY, nama TEXT, nilai REAL)")

# Parameter binding dengan '?' — WAJIB, bukan opsional (lihat §3)
db_eksekusi(db, "INSERT INTO siswa (nama, nilai) VALUES (?, ?)", ["Budi", 85.5])

variabel baris = db_ambil_semua(db, "SELECT nama, nilai FROM siswa WHERE nilai > ?", [80])
untuk b dalam baris {
    cetak(b["nama"], b["nilai"])        # tiap baris adalah kamus
}

variabel satu = db_ambil_satu(db, "SELECT * FROM siswa WHERE id = ?", [1])
cetak(db_baris_terpengaruh(db), db_id_terakhir(db))

# Transaksi
db_mulai(db)
coba {
    db_eksekusi(db, "UPDATE siswa SET nilai = nilai + 1")
    db_komit(db)
} kecuali e {
    db_batal(db)
    cetak("gagal:", e)
}

db_tutup(db)
```

`db_buka(":memory:")` membuat basis data di memori — ideal untuk pengujian
dan untuk latihan di playground yang filesystem-nya read-only.

## 3. Semantik

### Pemetaan tipe

| SQLite | Indonesiaku |
|---|---|
| `NULL` | `kosong` |
| `INTEGER` | angka |
| `REAL` | angka |
| `TEXT` | teks |
| `BLOB` | GalatTipe (belum ada tipe bita) |

Semua angka Indonesiaku adalah `double`: INTEGER di luar [-2^53, 2^53]
menimbulkan GalatNilai, termasuk db_id_terakhir, bukan konversi lossy.

### Fungsi

| Nama | Hasil |
|---|---|
| `db_buka(jalur)` | handle `OBJ_DB`; `":memory:"` didukung |
| `db_eksekusi(db, sql, params=[])` | `kosong`; untuk DDL/DML |
| `db_ambil_semua(db, sql, params=[])` | daftar kamus |
| `db_ambil_satu(db, sql, params=[])` | kamus, atau `kosong` bila tidak ada |
| `db_baris_terpengaruh(db)` | angka (`sqlite3_changes`) |
| `db_id_terakhir(db)` | angka (`sqlite3_last_insert_rowid`) |
| `db_mulai/db_komit/db_batal(db)` | transaksi |
| `db_tutup(db)` | menutup; aman dipanggil dua kali |

### Keamanan — tidak boleh disederhanakan

- **Parameter binding wajib ada sejak versi pertama.** Tanpa `?`, pengguna
  akan merangkai SQL dengan `+` dan setiap program yang diajarkan menjadi
  contoh SQL injection. Ini alasan `db_eksekusi` menerima `params` sejak awal,
  bukan "ditambahkan nanti".
- `db_eksekusi` hanya menjalankan **satu** pernyataan. `sqlite3_prepare_v2`
  mengembalikan sisa string; sisa hanya boleh whitespace/komentar; pernyataan kedua → `GalatNilai`
  sebelum prepare pertama (preflight sqlite3_complete memahami trigger/quote;
  tanpa menyiapkan SQL kedua). Pengecekan tail tetap dilakukan setelah prepare.
  Preflight prefix dapat kuadratik pada body trigger sangat besar.
  Ini mencegah `"...; DROP TABLE ..."` lolos lewat satu panggilan.
- Jumlah `?` harus sama dengan panjang `params`; kalau tidak → `GalatNilai`
  sebelum eksekusi.

### Error & siklus hidup

- Semua galat lewat `nativeRaise` → bisa ditangkap `coba/kecuali`. Pesan
  memuat teks dari `sqlite3_errmsg`.
- `OBJ_DB` **auto-close saat GC**, persis seperti `OBJ_FILE` dan `OBJ_SOCKET`
  yang sudah ada. Pola `freeObject` diikuti apa adanya.
- Memakai handle yang sudah ditutup → `GalatNilai`, bukan crash.

### Keputusan implementasi awal (belum selesai)

- Sepuluh fungsi: buka, tutup, eksekusi, ambil_semua, ambil_satu,
  baris_terpengaruh, id_terakhir, mulai, komit, batal.
- Argumen opsional hanya posisional. Parameter hanya anonymous `?`;
  named/numbered ditolak. Validasi seluruh tipe/nilai parameter sebelum step.
- SQL/jalur dengan embedded NUL ditolak; teks parameter/hasil memakai panjang
  byte eksplisit (NUL pada data tetap didukung). Logika di-bind INTEGER 0/1.
- Angka parameter non-finite ditolak; bilangan bulat di luar rentang tepat
  double ditolak. BLOB hasil menimbulkan GalatTipe; INTEGER tidak tepat
  menimbulkan GalatNilai. SQL DQS=0: petik ganda identifier, tunggal string.
- Daftar, kamus baris, kunci dan nilai di-root sepanjang alokasi; semua
  statement difinalisasi pada setiap jalur. Handle NULL berarti tertutup;
  close/GC memakai sqlite3_close_v2, transaksi terbuka di-rollback saat close.
- ambil_satu membaca baris pertama saja; kolom bernama sama ditimpa terakhir.
  Error konversi hasil bukan rollback otomatis atas efek SQL RETURNING.
- SQLite ATTACH tetap tersedia: mengikuti kebijakan filesystem yang sudah
  ada, bukan penguatan sandbox. Jangan mengakses DB produksi untuk uji.
- Vendor resmi dipin dengan URL/versi/SHA256 dan provenance public domain.
  CI raw source lists, fuzz build, Docker dan clean Windows diaudit.
  Uji Windows/Alpine tidak boleh diklaim tanpa tool yang benar-benar berjalan.

## 4. Rencana implementasi

| Bagian | Perubahan |
|---|---|
| `src/sqlite3.c`, `src/sqlite3.h` | amalgamation resmi, **vendored apa adanya** |
| `Makefile` | tambah ke `SOURCES`; `-DSQLITE_OMIT_LOAD_EXTENSION` dll. |
| `src/object.h` / `object.c` | `OBJ_DB` + `ObjDb { sqlite3* db; }` (NULL = tertutup) |
| `src/memory.c` | `freeObject` → `sqlite3_close_v2` bila belum ditutup |
| `src/db.c`, `src/native.h` | 10 fungsi di atas |
| `src/vm.c` | `defineNative` di `initVM` |
| `.gitignore`, CI | amalgamation ikut di-commit (lihat §7) |

Flag kompilasi (mengecilkan biner, mematikan yang tidak dipakai):

```
-DSQLITE_OMIT_LOAD_EXTENSION   # tidak ada .so pihak ketiga
-DSQLITE_DQS=0                 # "x" identifier; string SQL memakai petik tunggal
-DSQLITE_THREADSAFE=0          # VM kita single-thread
-DSQLITE_DEFAULT_MEMSTATUS=0
```

`sqlite3.c` **tidak boleh disunting** dan dikecualikan dari gaya kode kita —
ia dikompilasi tanpa `-Wall -Wextra` (amalgamation memicu banyak warning yang
bukan urusan kita) lewat aturan Makefile tersendiri.

**GC-safety:** `db_ambil_semua` membangun daftar berisi kamus sambil
mengalokasi string untuk setiap kolom. Daftar hasil **dan** kamus baris yang
sedang diisi harus keduanya ada di stack VM saat `dictSet` dipanggil —
`dictSet` bisa menumbuhkan tabel dan memicu GC. Ini pola yang sama dengan
`OP_BUILD_DICT`, tapi bersarang dua tingkat; titik paling rawan di paket ini.

## 5. Pengujian

- Golden `examples/db_sqlite.idk` + `tests/expected/db_sqlite.out`, seluruhnya
  memakai `":memory:"` supaya tidak ada berkas tersisa dan hasilnya
  deterministik.
- Kasus batas wajib:
  - CREATE → INSERT → SELECT → UPDATE → DELETE lengkap
  - `db_ambil_satu` tanpa hasil → `kosong`
  - `NULL` ↔ `kosong` bolak-balik
  - transaksi: `db_batal` benar-benar membatalkan
  - jumlah `?` tidak cocok → galat
  - dua pernyataan dalam satu `db_eksekusi` → galat
  - SQL injection klasik `"'; DROP TABLE siswa; --"` sebagai **parameter**
    harus tersimpan sebagai teks biasa, tabel tetap utuh (ini tes keamanan,
    wajib ada)
  - handle ditutup lalu dipakai → galat, bukan crash
- Tes galat: `db_sql_salah`, `db_params_tidak_cocok`, `db_ditutup`.
- `-DDEBUG_STRESS_GC` + ASan **wajib** — lihat catatan GC di §4.
- CI: amalgamation menambah waktu kompilasi; kompilasi `sqlite3.o` sekali dan
  biarkan cache Makefile bekerja (objek hanya bergantung pada header).

## 6. Dampak dokumentasi

- [ ] `website/panduan/07_PUSTAKA_STANDAR.md` — seksi basis data
- [ ] `website/panduan/` — pertimbangkan panduan baru "Basis Data"
- [ ] `README.md` + `CHANGELOG.md`
- [ ] `docs/ROADMAP.md` — pindahkan `sqlite` dari Fase 8 ke Fase 3
- [ ] `docs/ANALISIS/02_PETA_FITUR_PYTHON.md` — `sqlite3` ❌ → ✅
- [ ] `docs/ANALISIS/03_KEPUTUSAN_DESAIN.md` — K18: vendoring amalgamation
- [ ] `editors/vscode-indonesiaku/extension.js` — 10 builtin baru
- [ ] `AGENTS.md` — `sqlite3.c` jangan disunting; jebakan GC bersarang
- [ ] `docker/images/Dockerfile` — pastikan biner tetap jalan di Alpine/musl

## 7. Risiko & alternatif

**Kenapa tidak menunggu FFI (Fase 8)?** Karena tidak perlu. SQLite
didistribusikan sebagai *amalgamation*: satu `sqlite3.c` + satu header,
public domain, tanpa dependensi di luar libc. Menambahkannya identik dengan
menambahkan satu berkas `.c` ke `SOURCES`. FFI adalah proyek besar tersendiri
dan menahan basis data di belakangnya tidak dibenarkan secara teknis.

**Vendoring vs `-lsqlite3` sistem.** Vendoring dipilih: build tetap satu
`make` tanpa dependensi di Windows/macOS/Alpine, versi SQLite terkunci
sehingga hasil uji deterministik, dan tidak ada langkah `apt install` untuk
mahasiswa. Harganya: repo bertambah ~8 MB dan amalgamation harus diperbarui
manual saat ada CVE. Mitigasi: catat versi persisnya di `CHANGELOG.md` dan
berkas provenance terpisah (jangan edit vendor), dan perlakukan pembaruannya seperti dependensi biasa.

**Ukuran biner** naik dari ~140 KB ke sekitar 1–1,5 MB. Untuk sebuah bahasa
pemrograman ini wajar; flag `SQLITE_OMIT_*` di atas menekannya.

**Playground.** `db_buka` ke berkas akan gagal di container read-only —
itu benar dan diinginkan. `":memory:"` tetap jalan, jadi contoh di panduan
harus memakainya.

**MySQL/PostgreSQL tidak termasuk** dan tetap di Fase 8: keduanya butuh
client library eksternal plus protokol jaringan, bukan satu berkas `.c`.
Untuk saat ini `soket_*` + rencana modul `pola` sudah cukup bagi yang nekat.
