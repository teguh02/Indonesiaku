# Changelog - Bahasa Pemrograman Indonesiaku

Semua perubahan penting pada proyek ini akan didokumentasikan dalam file ini.

Format berdasarkan [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
dan proyek ini mengikuti [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]
### Planned
- Perulangan `untuk...dalam` (for...in loop)
- List, Dictionary, Set operations
- `hentikan` (break) dan `lanjut` (continue) statements
- String concatenation dan operations
- Operator assignment gabungan (+=, -=, *=, /=)

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
