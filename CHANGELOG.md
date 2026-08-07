# Changelog - Bahasa Pemrograman Indonesiaku

Semua perubahan penting pada proyek ini akan didokumentasikan dalam file ini.

Format berdasarkan [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
dan proyek ini mengikuti [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

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
