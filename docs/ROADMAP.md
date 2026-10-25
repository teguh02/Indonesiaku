# Roadmap — Bahasa Indonesiaku

**Satu-satunya sumber kebenaran untuk roadmap.** Versi saat ini: `0.4.0`
(`INDK_VERSION` di `src/common.h`). Riwayat detail per rilis ada di
[`CHANGELOG.md`](../CHANGELOG.md).

**Visi:** bahasa bergaya Python dengan kata kunci Bahasa Indonesia —
semantik selaras dengan Python 3 sehingga apa yang dipelajari di
Indonesiaku langsung berlaku di Python, dan sebaliknya. Dasar analisis dan
alasan setiap fase ada di [`ANALISIS/`](ANALISIS/README.md).

## Sudah selesai

| Versi | Isi | Bukti di repo |
|---|---|---|
| 0.1.0 | Scanner, compiler Pratt, VM stack, REPL, fungsi, `jika`/`selagi`, `cetak` | `src/` |
| 0.2.0 | Closure/upvalue, list, kamus, `untuk...dalam`, `hentikan`/`lanjut`, kelas + warisan, `naikkan`/`coba`/`kecuali`, `impor`, GC mark-sweep | `examples/closure.idk`, `examples/oop/` |
| 0.3.0 | Pustaka standar `pustaka/*.idk`, file I/O, waktu/tanggal, konversi, escape string, `argumen`, REPL multi-baris | `pustaka/`, `examples/berkas.idk` |
| 0.4.0 | JSON, HTTP, soket TCP, server REST contoh, `ord`/`chr`/`sisip`/`hapus_pada`; pesan error dengan nomor baris + jejak panggilan | `examples/todo_server.idk`, `src/vm.c` `runtimeError` |

## Fase pengembangan

Status: ⬜ belum · 🟨 dikerjakan · ✅ selesai. Setiap butir besar mendapat
berkas `RENCANA_FITUR/<nama>.md` saat mulai dikerjakan. Kode B*/K* merujuk
ke [`ANALISIS/01_KONDISI_SAAT_INI.md`](ANALISIS/01_KONDISI_SAAT_INI.md) (bug)
dan [`ANALISIS/03_KEPUTUSAN_DESAIN.md`](ANALISIS/03_KEPUTUSAN_DESAIN.md)
(keputusan desain).

### Fase 0 — Stabilisasi (target 0.5.0) — *"tidak ada jawaban salah"*

Tujuan: program pemula (halo dunia, percabangan, perulangan, list, kamus,
fungsi) selalu benar atau gagal dengan pesan jelas. Tanpa fitur besar baru.

| Butir | Ref | Status |
|---|---|---|
| Perbaiki `//` (tidak memancarkan opcode) + tambah `//=` | B1, B13 | ✅ |
| Tolak `f(x=1)` dengan error kompilasi sampai argumen bernama ada | B2 | ✅ |
| Batas rekursi 1000 (`FRAMES_MAX`); jejak error merangkum frame berulang | B3 | ✅ |
| `**` asosiatif kanan; `-2 ** 2 == -4` | B4, B5 | ✅ |
| `%` dan `//` mengikuti tanda pembagi (Python) | B6 | ✅ |
| Penugasan majemuk memakai seluruh ekspresi kanan (`x *= 2 + 3`) | B14 | ✅ |
| `tidak` lebih lemah dari perbandingan | B15 | ✅ |
| Harness tes galat `tests/galat/` (program yang harus gagal) | — | ✅ |
| `lewati` dan `akhirnya` diimplementasikan | B7 | ✅ |
| Semua galat runtime bisa ditangkap `kecuali`; handler `coba` dilepas saat `kembalikan`/`hentikan`/`lanjut` | B8 | ✅ |
| Bagi nol menjadi error | B9 | ✅ |
| Cetak desimal *shortest round-trip* | B10 | ✅ |
| `panjang`/indeks/`potong`/`ord`/`chr` per karakter Unicode | B11 | ✅ |
| `cetak(a, b)` satu baris dipisah spasi; `cetak()` baris kosong | B12 | ✅ |
| `kembalikan`/`hentikan`/`lanjut` keluar dari `coba` ber-`akhirnya` menjalankan blok akhirnya | B7 lanjutan | ✅ |
| Hapus token mati (`INDENT`/`DEDENT`/`NEWLINE`); `;` jadi pemisah pernyataan; `:` dicadangkan untuk Fase 1 | 01 §6 | ✅ |
| Selaraskan README/panduan dengan kenyataan (semua blok kode di dokumentasi diuji dengan `indk`) | 01 §6 | ✅ |
| Tes golden untuk setiap bug di atas | — | ✅ |
| Putuskan K1–K14 (keputusan desain) — semua rekomendasi disetujui | 03 | ✅ |

### Fase 1 — Inti mirip Python (0.6–0.7) — *"tutorial Python bab 1–5 bisa ditiru"*

| Butir | Ref | Status |
|---|---|---|
| Truthiness Python ([rencana](RENCANA_FITUR/selesai/kebenaran_python.md)) | K1 | ✅ |
| `rentang()` ([rencana](RENCANA_FITUR/selesai/rentang.md)); `untuk` atas teks dan rentang | K9 | ✅ |
| Operator `dalam` / `tidak dalam` ([rencana](RENCANA_FITUR/selesai/operator_dalam.md)) | 02 §A | ✅ |
| `enumerasi()`, `untuk` atas kamus | K9 | ✅ |
| `adalah` / `bukan`, rantai perbandingan (`1 < x < 3`, `a dalam b == c`) | 02 §A–B | ✅ |
| Hierarki kelas `Galat*` dan `kecuali Tipe sebagai e` (semua galat runtime sudah bisa ditangkap sejak Fase 0) | K5, 06 §9 | ✅ |
| Akhir baris mengakhiri pernyataan (di luar kurung), seperti Python | B16 | ✅ |
| Assignment di fungsi = lokal; `global`, `nonlokal` | K3 | ✅ |
| Literal kamus `{ }`, indeks negatif, irisan `[a:b:c]` | 02 §B | ✅ |
| Argumen bawaan (nilai default parameter) | K15 | ✅ |
| Argumen bernama `f(b=2)` | K16 | ✅ |
| `cetak` jadi fungsi biasa (`pisah=`, `akhir=`) | K12 | ⬜ |
| Fungsi anonim (ekspresi `fungsi(x) { ... }`), ekspresi kondisional | 02 §B | ⬜ |
| f-string, string `"""` multi-baris, literal `1e3`, `0x`, `0b`, `1_000` | 02 §B | ⬜ |
| Metode pada `teks`/`daftar`/`kamus` (P1 dari katalog 06) | K10, 06 §1–3 | ⬜ |
| `+`/`*` untuk teks & daftar, `==` struktural | K7 | ✅ |
| `pastikan` (assert) ([rencana](RENCANA_FITUR/selesai/pastikan.md)) | 02 §A | ✅ |
| Traceback dengan kolom + potongan baris; saran "mungkin maksudmu…" | 04 §5, 05 §5 | ⬜ |

### Fase 2 — Tipe data lengkap (0.8)

`bulat` int64 (+ bignum), `tupel`, `himpunan`, penugasan berganda & unpacking,
`*args`/`**kwargs`, list comprehension, operator bitwise, `buang` (del),
kunci kamus non-teks, string per karakter (Unicode), `jenis()` mengembalikan
kelas, builtin P1–P2 dari 02 §D — lihat
[`RENCANA_FITUR/builtin_inti.md`](RENCANA_FITUR/builtin_inti.md)
(`jumlah`, `terurut`, `semua`, `pasangkan`, `bulatkan`, …; sebagian bisa
masuk lebih awal di Fase 1 karena tidak butuh tipe baru).
✅ **Sebagian besar builtin inti sudah selesai** — 19 fungsi sudah masuk di
Fase 1 (`jumlah`, `semua`, `ada_yang`, `terurut`, `terbalik`, `pasangkan`,
`peta`, `saring`, `bulatkan`, `bagi_sisa`, `biner`, `oktal`, `heksa`,
`ke_bulat`, `ke_logika`, `adalah_instansi`, `bisa_dipanggil`, `repr`, `id`),
plus `max`/`min` dengan satu argumen daftar. Sisanya menunggu tipe baru:
`himpunan()`, `tupel()`, `daftar()`, `hash`, `iter`/`next`, `turunan_dari`,
`ke_desimal`, dan `biner` untuk bigint.
Ref: K2, K8, K11, K13, 06 §4–6.

### Fase 3 — Modul & pustaka inti (0.9)

`ObjModule` dengan namespace, `impor x`, `dari x impor y sebagai z`, paket,
jalur pencarian (`INDK_PATH`), `__utama__`; modul P1 dari katalog 07:
`matematika`, `acak`, `teks`, `sistem`, `waktu`, `tanggal_waktu`, `json`
(native). Ditambah **basis data SQLite** — ✅ **selesai**, lihat
[`RENCANA_FITUR/selesai/basis_data_sqlite.md`](RENCANA_FITUR/selesai/basis_data_sqlite.md)
(dimajukan dari Fase 8: amalgamation adalah satu berkas `.c`, tidak butuh FFI).
SQLite 3.50.4 di-*vendor*; 10 fungsi `db_*` dengan parameter `?` wajib dan
satu pernyataan per panggilan. Sisa Fase 3 (`ObjModule`, `impor x`, paket)
belum dikerjakan.
Ref: 04 §6, 07 §1–5.

### ★ Rilis 1.0 — "Stabil untuk umum"

Syarat: Fase 0–3 selesai; `docs/SPESIFIKASI.md` ditulis; semua contoh di
panduan diuji otomatis; kebijakan stabilitas K14 berlaku. Setelah 1.0, tidak
ada breaking change sampai 2.0.

### Fase 4 — OOP lengkap (1.1–1.2)

Metode ajaib (06 §10), atribut kelas, `@properti`/`@metode_statis`/
`@metode_kelas`, dekorator, pewarisan berganda + MRO C3, `dengan ... sebagai`
(manajer konteks), `adalah_instansi`, kelas abstrak. Ref: 04 §4.

### Fase 5 — Iterator & generator (1.3)

Protokol `_iter_`/`_berikutnya_`, `hasilkan` (yield), generator expression,
dict/set comprehension, `cocokkan`/`kasus` (pattern matching).
Ref: 04 §7.

### Fase 6 — Pustaka standar luas (1.x berkelanjutan)

Modul P2 dari katalog 07: `os`/`jalur`, `koleksi`, `iterasi`,
`fungsional`, `pola` (regex), `csv`, `hash`, `base64`, `log`, `opsi`,
`proses`, `salin`, `statistik`, `kura` (turtle), klien HTTP, `bita`.
Sebagian besar cocok untuk kontributor baru.

### Fase 7 — Perkakas & kinerja

`indk uji`, `indk rapikan` (formatter), `indk periksa` (linter — lihat
[`RENCANA_FITUR/deteksi_tidak_terpakai.md`](RENCANA_FITUR/deteksi_tidak_terpakai.md)
untuk deteksi variabel/fungsi tidak terpakai), LSP +
ekstensi VS Code, debugger, `indk --dis`, playground via **WebAssembly**,
cache `.idkc`, computed goto, superinstruksi, inline cache, NaN-boxing, GC
generasional, utas/asinkron. Ref: 04 §1, §8–10, §12.

### Fase 8 — Ekosistem

Manajer paket + registri, API embedding C, FFI, MySQL/PostgreSQL (klien
jaringan — `sqlite` sendiri sudah dimajukan ke Fase 3), TLS/HTTPS, GUI,
AOT/bundel executable, anotasi tipe opsional, mode sandbox untuk sekolah.
Ref: 04 §11–13, 05.

## Cara memakai berkas ini

1. Untuk mengusulkan fitur: salin `RENCANA_FITUR/TEMPLATE.md`, isi, lalu
   tautkan dari butir fase yang relevan.
2. Saat fitur selesai: ubah status di sini + catat di `CHANGELOG.md`, lalu
   hapus/arsipkan berkas rencananya.
3. Jangan menyalin roadmap ini ke `README.md` atau berkas panduan.
