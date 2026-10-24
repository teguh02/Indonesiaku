# Changelog - Bahasa Pemrograman Indonesiaku

Semua perubahan penting pada proyek ini akan didokumentasikan dalam file ini.

Format berdasarkan [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
dan proyek ini mengikuti [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Fixed — Fase 0 (stabilisasi)
- 🐛 **`//` (pembagian bulat) rusak**: compiler tidak menghasilkan opcode,
  sehingga `1 + 10 // 3` menjadi `13`. Kini `OP_FLOOR_DIVIDE` membulatkan ke
  bawah seperti Python (`-7 // 2 == -4`). `//=` juga ditambahkan.
- 🐛 **Argumen bernama diam-diam salah**: `f(b=1, a=5)` dulu dikompilasi sebagai
  assignment global dan nilainya dikirim berurutan (`-4`). Kini ditolak saat
  kompilasi dengan pesan "Argumen bernama (nama=nilai) belum didukung".
- 🐛 **Batas rekursi** naik dari 64 menjadi 1000 panggilan (sama dengan Python).
  Jejak error merangkum frame yang berulang
  (`[baris sebelumnya diulang N kali lagi]`).
- 🐛 `**` kini asosiatif kanan (`2 ** 3 ** 2 == 512`) dan lebih kuat dari minus
  unary (`-2 ** 2 == -4`).
- 🐛 `%` mengikuti tanda pembagi seperti Python (`-7 % 3 == 2`).
- 🐛 Penugasan majemuk memakai seluruh ekspresi di sisi kanan: `x *= 2 + 3`
  dulu hanya mengalikan dengan `2`.
- 🐛 `tidak` kini lebih lemah dari perbandingan: `tidak 1 == 2` bernilai `benar`
  (dulu `(tidak 1) == 2`).
- 🐛 **Semua galat runtime bisa ditangkap** `coba/kecuali`: indeks di luar
  jangkauan, variabel tak terdefinisi, tipe operand salah, memanggil
  bukan-fungsi, rekursi terlalu dalam, dll. Nilai `e` adalah pesan galatnya.
  Hanya kerusakan internal VM yang tetap fatal.
- 🐛 `kembalikan`/`hentikan`/`lanjut` dari dalam `coba` kini melepas handler-nya
  (dulu handler basi bisa "menangkap" galat setelah fungsinya selesai).
- 🐛 **Bagi nol** (`/`, `//`, `%`, `0 ** -n`) kini galat "Pembagian dengan nol."
  (dulu menghasilkan `takhingga`/`nan`).
- 🐛 **Presisi cetak angka**: representasi terpendek yang tepat, seperti Python
  (`0.1 + 0.2` → `0.30000000000000004`, `2 ** 0.5` → `1.4142135623730951`).
- 🐛 **Teks Unicode**: `panjang`, `teks[i]`, `karakter`, dan `potong` menghitung
  karakter, bukan byte (`panjang("héllo") == 5`). `ord`/`chr` memakai kode
  Unicode (`ord("é") == 233`, `chr(128512)` → 😀).
- 🐛 Kata kunci `akhirnya` tidak pernah dikenali scanner (salah hitung panjang),
  sehingga terbaca sebagai nama variabel.

### Added — Fase 0
- ✨ `lewati` (pass): pernyataan kosong, termasuk `kelas Kosong { lewati }`.
- ✨ `akhirnya` (finally): `coba { } kecuali e { } akhirnya { }` dan
  `coba { } akhirnya { }`; blok akhirnya selalu jalan, lalu galat yang belum
  ditangkap diteruskan. Batasan: `kembalikan`/`hentikan`/`lanjut` yang keluar
  dari `coba` ber-`akhirnya` ditolak saat kompilasi (belum didukung).

### Changed — Fase 0 (merusak kompatibilitas)
- ⚠️ `cetak(a, b, c)` kini mencetak semua argumen dalam **satu baris** dipisah
  spasi (seperti Python `print`); dulu tiap argumen di baris sendiri.
  `cetak()` mencetak baris kosong.
- ⚠️ `chr()` menerima kode Unicode hingga `0x10FFFF` dan menghasilkan UTF-8;
  `chr(200)` kini `È`, bukan satu byte mentah.

### Changed
- 📁 **Pemisahan dokumentasi**: isi `docs/` dipindah ke `website/` — situs statis
  di `website/`, panduan pengguna di `website/panduan/`. Folder `docs/` kini
  khusus **perencanaan bahasa**.
- 🌐 Situs tidak lagi memakai Netlify (`netlify.toml` + `.netlify/` dihapus);
  akan di-host sendiri sebagai satu bundel Docker bersama playground.

### Added
- 🧪 **Tes galat** `tests/galat/*.idk` + `*.err`: menguji program yang *harus*
  gagal (exit ≠ 0 dan stderr persis sama). Dijalankan oleh kedua runner.
- 🧪 Tes golden `operator_aritmetika`, `rekursi_dalam`, `coba_akhirnya`,
  `galat_tertangkap`, `teks_unicode`, `cetak_angka`, dan tes galat
  `handler_basi`, `bagi_nol`, `akhirnya_teruskan`, `kembalikan_dalam_akhirnya`.
- 📚 `docs/ANALISIS/`: audit v0.4.0, peta fitur Python → Indonesiaku, keputusan
  desain, teknik lanjutan, dan katalog fungsi/kelas yang akan diduplikasi.
  `docs/ROADMAP.md` disusun ulang menjadi Fase 0–8 menuju 1.0.
- ⌨️ **stdin di playground** — panel Input terpisah di halaman playground;
  `POST /api/jalankan` menerima `{kode, input}` dan menyalurkan `input` sebagai
  stdin, sehingga `input("Nama: ")` membaca satu baris per pemanggilan.
- ⚙️ **API playground** — `docker/playground/server.py` menjalankan `indk` untuk
  `POST /api/jalankan` (`{kode, input}` → `{stdout, stderr, kode_keluar, ms}`)
  dengan batas 3 detik, 128 MB, 64 KB input/output, dan rate limit 60/menit per
  IP.
- 🐳 **Bundel Docker situs + playground** — satu image (nginx + `indk` + python),
  dijalankan sebagai dua layanan: `web` (publik) dan `runner` (jaringan
  internal, tanpa akses internet, read-only, non-root).
- 🎨 **Desain ulang situs** — Tailwind CSS (Play CDN) + token di
  `website/assets/tailwind-config.js`, Alpine.js untuk tema/bahasa, tipografi
  Plus Jakarta Sans + JetBrains Mono, mode terang & gelap berbasis kelas.
- 🌐 **Dua bahasa (ID/EN)** — semua teks UI di `website/assets/i18n.json`;
  pemilih bahasa tersimpan di `localStorage`, halaman Tentang & Penulis punya
  artikel penuh dalam dua bahasa.
- ⚡ **Senna.js (SPA)** — navigasi antar halaman tanpa reload lewat surface
  `#konten`, sementara setiap halaman tetap HTML utuh dengan URL sendiri
  (ramah mesin pencari). Tombol back/forward tetap bekerja.
- 🧪 **Halaman playground** — `website/playground/index.html`: editor kode +
  panel keluaran, `POST` AJAX ke `/api/jalankan` (`{ kode }` →
  `{ stdout, stderr, kode_keluar, ms }`), pintasan Ctrl+Enter. Backend API
  menyusul (halaman menampilkan status "backend tidak tersedia" bila belum ada).
- 🗺️ `docs/ROADMAP.md` sebagai satu-satunya sumber kebenaran roadmap, plus
  `docs/README.md` dan `docs/RENCANA_FITUR/TEMPLATE.md` untuk rencana per fitur.

### Fixed
- 🔗 Tautan antar-dokumen yang masih menunjuk `docs/…` (termasuk nama berkas
  yang salah sejak lama, mis. `02_PANDUAN_SINTAKS.md` → `03_PANDUAN_SINTAKS.md`).

## [0.4.0] - 2026-08-07

Rilis JSON + REST: aplikasi contoh Todo-list dalam dua bentuk — CRUD CLI
dan REST API HTTP (soket TCP), keduanya diuji di CI.

### Added
- 🌐 **Soket TCP** — tipe objek `OBJ_SOCKET` + native `soket_dengar(port)`,
  `soket_terima`, `soket_baca`, `soket_tulis`, `soket_tutup` (Winsock di
  Windows, BSD di POSIX; listen di 127.0.0.1). Auto-close saat GC.
- 🔌 **`pustaka/http.idk`** — `http_urai_request` (parse request HTTP),
  `http_response`/`http_response_json` (bangun response).
- 🚀 **Server REST HTTP** — `examples/todo_server.idk`: REST API Todo-list
  (`GET/POST/PUT/DELETE /todos`) dengan penyimpanan JSON. Diuji CI (job
  `todo-http` + `tests/test_todo_server.{sh,ps1}`).
- 📇 **JSON** — `pustaka/json.idk` (`json_teks`, `json_urai`), ditulis dalam
  Indonesiaku sendiri.
- ✅ **Todo-list JSON DB (CLI)** — `examples/todo_cli.idk` (CRUD command-line).
  Diuji CI (job `todo-crud`).
- 🔡 **Native tambahan** — `ord`, `chr`, `hapus_pada(list, i)`,
  `sisip(list, i, nilai)`.
- 🧪 **Uji sintaks dasar** — `examples/sintaks_dasar.idk` (smoke test golden).

### Fixed
- 🐛 **Regresi assignment implisit di dalam blok** — `x = 10` di dalam
  `jika`/`selagi`/`untuk` (setelah scoping v0.3.0). Kini memakai semantik
  definisi di kedalaman scope mana pun.
- 🐛 **Korupsi nilai kamus saat GC** — `OP_INDEX_SET` dict mem-pop operand
  sebelum `tableSet`; kini ditahan di stack. Ditemukan lewat parser JSON di
  bawah stress-GC.

### Changed
- `jenis()` mengembalikan `"fungsi"` untuk fungsi/closure/native/metode
  terikat, dan `"soket"` untuk soket.

## [0.3.0] - 2026-08-07

Rilis pustaka standar: fungsi bawaan untuk berkas, waktu, konversi, plus
pustaka teks/daftar/matematika/kamus yang ditulis dalam bahasa Indonesiaku
sendiri (dogfooding).

### Added
- 📚 **Pustaka standar dalam bahasa sendiri** (`pustaka/`, di-`impor`):
  `teks.idk` (pisah/gabung/cari/pangkas/balik/...), `daftar.idk`
  (peta/saring/lipat/urutkan/iris/...), `matematika.idk`
  (faktorial/fpb/kpk/prima/rata_rata/...), `kamus.idk`
  (nilai/gabung/dari_pasangan/...).
- 🔡 **Primitif & pengindeksan teks** — `karakter(teks, i)`,
  `potong(teks, mulai, jumlah)`, dan pengindeksan `teks[i]` (baca).
- 🧯 **Error native yang dapat ditangkap** — fungsi bawaan memicu kesalahan
  via `nativeRaise` yang bisa ditangkap `coba/kecuali` (dulu semua fatal).
- 🔤 **Konversi & format** — `ke_angka`, `ke_teks`, `format("... {} ...", ...)`,
  `acak_bulat(min, maks)`. `acak()` kini di-seed.
- 📄 **Berkas (File I/O)** — `buka_berkas`, `tutup`, `baca`, `baca_baris`,
  `tulis`, `baca_semua`, `tulis_semua`, `tambah_berkas`, `ada_berkas`,
  `hapus_berkas`. Tipe objek baru `OBJ_FILE` (auto-close saat GC).
- 🕐 **Waktu & Tanggal** — `waktu()`, `tanggal()` (kamus komponen),
  `format_tanggal(pola)`.
- 🧵 **Urutan escape string** — `\n \t \r \" \\ \'` kini diproses di literal
  string (scanner + compiler).
- ⌨️ **Argumen baris perintah** — variabel global `argumen` (list string);
  `indk skrip.idk arg1 arg2 ...`.
- 🖥️ **REPL multi-baris** — blok (`fungsi`/`kelas`/`jika`/...) dapat ditulis
  lintas baris; prompt lanjutan `...` sampai kurung kurawal seimbang.
- 📖 **Referensi pustaka standar** terstruktur di `docs/07_PUSTAKA_STANDAR.md`.

### Fixed
- 🐛 **Variabel bocor dari badan blok**: `variabel` yang dideklarasikan di
  dalam `jika`/`selagi`/`untuk` tidak di-scope, sehingga bocor ke variabel
  berikutnya (terutama saat `hentikan`). Kini setiap badan blok memakai
  scope tersendiri. Regressi dijaga oleh `examples/lingkup_variabel.idk`.

### Changed
- `<` dan `>` kini juga membandingkan string (via `strcmp`), memungkinkan
  pengurutan string.
- Pencetakan angka lebih presisi (15 digit, tanpa notasi ilmiah tak sengaja,
  integer tanpa `.0`).

## [0.2.0] - 2026-08-07

Rilis fitur besar: bahasa kini mencakup closure, struktur data, OOP penuh,
penanganan kesalahan, dan sistem modul — semua dengan garbage collector
mark-sweep dan cakupan pengujian golden-output.

### Added
- 🧠 **Closures & upvalues** — fungsi bersarang menangkap variabel lokal
  (gunakan `variabel` untuk lokal yang di-capture); upvalue tertutup persist
  setelah fungsi induk kembali.
- 📚 **List** — literal `[..]`, pengindeksan baca/tulis, list bersarang;
  builtin `panjang`, `tambah`, `hapus`.
- 🔑 **Kamus (dictionary)** — `kamus()`, pengindeksan kunci string; builtin
  `punya`, `kunci`, `hapus_kunci`.
- 🔁 **`untuk ... dalam`** — iterasi list (for-in).
- ⏭️ **`hentikan` / `lanjut`** — break & continue di `selagi` dan `untuk`.
- 🏛️ **Kelas & OOP** — `kelas`, `init` (konstruktor), `diri` (this), metode,
  field, pewarisan (`kelas Anak < Induk`), `super`, polimorfisme.
- 🧯 **Penanganan kesalahan** — `naikkan` (raise) dan `coba { } kecuali e { }`
  (try/catch) dengan unwinding call-stack dan nesting.
- 📦 **Sistem modul** — `impor "file.idk"` memuat file lain ke namespace global
  dengan cache anti-duplikasi.
- ♻️ **Garbage Collector mark-sweep** — koleksi otomatis berbasis ambang
  alokasi; flag `DEBUG_STRESS_GC` / `DEBUG_LOG_GC`.
- 🧰 Builtin baru: `abs`, `pangkat`, `bulat`, `lantai`, `atap`, `acak`,
  `huruf_besar`, `huruf_kecil`, `ganti`, `jenis`, `input`, `henti`, dan builtin
  list/kamus di atas.

### Fixed
- 🐛 **Build link error**: `src/native.c` tidak ada di `Makefile SOURCES`
  (dan one-liner gcc README) — menyebabkan `undefined reference` saat link.
- 🐛 **Stack underflow** pada setiap `jika`/`selagi`: `OP_JUMP_IF_FALSE`
  melakukan pop ganda. Kini tidak pop (sesuai desain clox).
- 🐛 **Operator `atau` (OR) rusak total** — salah kelas huruf di scanner
  (`case 'o'` alih-alih `'a'`) → "Variabel tidak terdefinisi 'atau'".
- 🐛 **Propagasi error**: `push`/`pop` yang gagal kini menghentikan eksekusi
  (dulu lanjut dengan stack korup).
- 🐛 **`OP_DEFINE_GLOBAL` tidak pop** → deklarasi meninggalkan nilai di stack,
  merusak slot lokal `untuk...dalam`. Deklarasi kini stack-neutral.
- 🐛 **`make clean` gagal di Windows** (`del /Q src/main.o` — `/` diartikan
  switch). Diperbaiki dengan path backslash.
- 🐛 **Makefile tanpa dependensi header** → perubahan `.h` tidak memicu
  rekompilasi (miscompile senyap). Kini `%.o: %.c $(HEADERS)`.
- 🐛 Hardening fungsi native: validasi tipe semua argumen sebelum konversi.

### Changed
- 🔒 CI (`test.yml`) tidak lagi `continue-on-error` pada langkah build & test;
  ditambah job **ASan/UBSan** (stress GC), **Valgrind**, dan **coverage (lcov)**.
- ✅ Suite pengujian beralih ke **golden-output** (bandingkan stdout + cek
  stderr/exit-code), bukan hanya exit-code. Ditambah benchmark performa.
- 📖 Dokumentasi sintaks diselaraskan ke sintaks kurung kurawal `{ }` yang
  sebenarnya di-parse (README sebelumnya salah mengajarkan gaya Python).
- `cetak` bekerja untuk semua tipe baru (list, kamus, objek).

## [0.1.1] - 2025-10-28

### Changed

- Bumped version to `0.1.1`.
- Added CLI flags `-v/--version` and `-h/--help`.
- Disabled compiler debug bytecode printing by default for release builds.
- Updated installer metadata and documentation to reference `v0.1.1`.

## [0.1.0] - 2025-10-27

### Added
- ✨ **Fitur Inti**
  - Scanner/Lexer dengan kata kunci Bahasa Indonesia
  - Parser dengan Pratt parsing untuk ekspresi
  - Compiler bytecode dari AST
  - Virtual Machine stack-based untuk eksekusi
  - REPL (Read-Eval-Print Loop) interaktif

- 📊 **Tipe Data**
  - Number (integer dan float)
  - Boolean (`benar`, `salah`)
  - Kosong (None/null)
  - String (basic, dengan quote single/double)
  - Object (internal: String, Function)

- 🔤 **Kata Kunci**
  - `benar`, `salah`, `kosong` - Literal values
  - `dan`, `atau`, `tidak` - Operator logika
  - `jika`, `jika_lain`, `selain` - Percabangan
  - `selagi` - Perulangan while
  - `fungsi` - Definisi fungsi
  - `kembalikan` - Return statement
  - `cetak` - Print/output

- ➕ **Operator**
  - Aritmatika: `+`, `-`, `*`, `/`, `%`, `**`
  - Perbandingan: `==`, `!=`, `<`, `<=`, `>`, `>=`
  - Logika: `dan`, `atau`, `tidak`
  - Unary: `-` (negasi), `tidak` (NOT)

- 🔧 **Fitur Bahasa**
  - Variabel dinamis (tanpa deklarasi tipe)
  - Fungsi dengan parameter
  - Return values dari fungsi
  - Rekursi (tail recursion dan nested recursion)
  - Scope lokal dan global
  - First-class functions

- 🏗️ **Infrastruktur**
  - Hash table untuk globals dan string interning
  - Memory management (allocator)
  - Value system dengan tagged union
  - Call frames untuk function calls
  - Stack-based VM dengan 64 frame depth

- 📝 **Dokumentasi**
  - README.md lengkap dengan tutorial
  - QUICKSTART.md untuk memulai cepat
  - SYNTAX_REFERENCE.md untuk referensi sintaks
  - Contoh program di folder examples/

- 🔨 **Build System**
  - Makefile untuk Linux/Mac/Windows
  - VS Code tasks.json untuk integrasi editor
  - .gitignore untuk version control

- 📦 **Contoh Program**
  - hello.idk - Hello World
  - variabel.idk - Variabel dan operasi aritmatika
  - percabangan.idk - If-elif-else statements
  - perulangan_selagi.idk - While loops
  - fungsi.idk - Function definitions dan calls
  - kalkulator.idk - Program kalkulator sederhana
  - fibonacci.idk - Rekursi Fibonacci
  - prima.idk - Cek bilangan prima

- 🐛 **Debug Tools**
  - Disassembler bytecode
  - Trace execution (dengan flag DEBUG_TRACE_EXECUTION)
  - Print bytecode (dengan flag DEBUG_PRINT_CODE)

### Technical Details
- **Architecture**: Bytecode VM dengan stack-based execution
- **Language**: C11 standard
- **Compiler**: GCC/Clang compatible
- **VM Opcodes**: 28 opcodes dasar
- **Max Frames**: 64 nested function calls
- **Max Stack**: 16384 values (FRAMES_MAX * 256)
- **Max Constants**: 256 per chunk
- **Max Locals**: 256 per function

### Known Limitations
- ❌ Belum ada for...in loop (gunakan while)
- ❌ Belum ada list/array operations
- ❌ Belum ada dictionary/object literals
- ❌ Belum ada string concatenation dengan +
- ❌ Belum ada break/continue statements
- ❌ Belum ada class/OOP
- ❌ Belum ada module system
- ❌ Belum ada exception handling
- ❌ Belum ada file I/O
- ❌ Error messages masih basic

### Performance Notes
- Function calls: Stack-based, relatif cepat
- Recursion: Mendukung hingga 64 level
- String interning: Otomatis untuk efisiensi memori
- Hash table: FNV-1a hash untuk globals lookup

## Development Roadmap

### v0.2 (Next)
- [ ] For...in loops dengan iterables
- [ ] List operations (create, index, append)
- [ ] Break dan continue statements
- [ ] String concatenation dengan operator +
- [ ] Compound assignment operators (+=, -=, dll)
- [ ] Better error messages dengan line numbers

### v0.3 (Future)
- [ ] Classes dan objects
- [ ] Methods dan properties
- [ ] `diri` (self) keyword
- [ ] Inheritance
- [ ] Garbage Collection (mark-sweep)
- [ ] String methods (upper, lower, split, dll)

### v0.4 (Future)
- [ ] Module system (`impor`, `dari`, `sebagai`)
- [ ] Standard library (math, string, file)
- [ ] File I/O operations
- [ ] Exception handling (`coba`, `kecuali`, `akhirnya`)
- [ ] Better REPL (multiline, history)

### v1.0 (Goal)
- [ ] Stable API
- [ ] Comprehensive standard library
- [ ] Package manager
- [ ] Documentation generator
- [ ] Debugger integration
- [ ] Performance optimizations
- [ ] Cross-platform binary releases

## Contributors
- Initial development: Proyek Indonesiaku Team

## License
Educational and open-source project.

---

**Format Tanggal**: YYYY-MM-DD
**Format Versi**: MAJOR.MINOR.PATCH
