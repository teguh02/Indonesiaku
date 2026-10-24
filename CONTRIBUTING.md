# Panduan Kontribusi - Bahasa Pemrograman Indonesiaku

Terima kasih atas minat Anda untuk berkontribusi pada proyek Indonesiaku! 🎉

## 📋 Daftar Isi

- [Code of Conduct](#code-of-conduct)
- [Cara Berkontribusi](#cara-berkontribusi)
- [Development Setup](#development-setup)
- [Panduan Coding](#panduan-coding)
- [Mengirim Pull Request](#mengirim-pull-request)
- [Pelaporan Bug](#pelaporan-bug)
- [Permintaan Fitur](#permintaan-fitur)

## 🤝 Code of Conduct

- Bersikap sopan dan menghormati semua kontributor
- Gunakan bahasa yang inklusif dan ramah
- Terima kritik konstruktif dengan lapang dada
- Fokus pada apa yang terbaik untuk komunitas

## 🚀 Cara Berkontribusi

Ada banyak cara untuk berkontribusi:

### 1. Melaporkan Bug 🐛
Jika menemukan bug, buat issue dengan informasi:
- Versi Indonesiaku yang digunakan
- OS dan compiler yang digunakan
- Langkah-langkah untuk reproduce bug
- Kode .idk yang menyebabkan bug
- Output error yang diterima
- Perilaku yang diharapkan

### 2. Mengusulkan Fitur Baru 💡
Buat issue untuk diskusi fitur baru:
- Jelaskan fitur yang diusulkan
- Berikan use case dan contoh
- Diskusikan implementasi yang mungkin
- Tunjukkan syntax yang diinginkan

### 3. Menulis Kode 💻
Kontribusi kode sangat dihargai:
- Implementasi fitur baru
- Perbaikan bug
- Optimasi performa
- Refactoring kode

### 4. Menulis Dokumentasi 📚
Dokumentasi sama pentingnya:
- Perbaiki typo
- Tambahkan contoh
- Perjelas penjelasan
- Terjemahkan dokumentasi

### 5. Membuat Contoh Program 📝
Buat contoh program .idk yang menarik:
- Algoritma klasik
- Game sederhana
- Utility programs
- Tutorial interaktif

## 🔧 Development Setup

### Prerequisites
```bash
# Windows: Install MinGW-w64 atau MSYS2
# Linux: Install build-essential
sudo apt-get install build-essential

# Mac: Install Xcode Command Line Tools
xcode-select --install
```

### Clone dan Build
```bash
# Clone repository
git clone https://github.com/yourusername/Indonesiaku.git
cd Indonesiaku

# Build
make

# Test
make test
```

### Struktur Kode
```
src/
├── main.c          # Entry point & CLI
├── scanner.c/.h    # Lexical analysis (tokenizer)
├── compiler.c/.h   # Parser & code generation
├── vm.c/.h         # Virtual machine execution
├── chunk.c/.h      # Bytecode chunks
├── value.c/.h      # Value representation
├── object.c/.h     # Object system (strings, functions)
├── table.c/.h      # Hash tables
├── memory.c/.h     # Memory management
└── debug.c/.h      # Debugging utilities
```

## 📝 Panduan Coding

### C Code Style

#### Naming Conventions
```c
// Functions: camelCase atau snake_case
void initScanner(const char* source);
void reset_stack();

// Types: PascalCase
typedef struct Compiler Compiler;
typedef enum TokenType TokenType;

// Constants: UPPER_CASE
#define MAX_FRAMES 64
#define STACK_SIZE 256

// Variables: snake_case
int local_count;
Token current_token;
```

#### Formatting
```c
// Indentasi: 4 spasi (tidak tab)
if (condition) {
    doSomething();
} else {
    doOtherthing();
}

// Kurung kurawal: same line untuk control structures
while (i < count) {
    process(i);
    i++;
}

// Spasi di sekitar operator
int result = a + b * c;
Value value = pop();
```

#### Komentar
```c
// Single line comment untuk penjelasan singkat
int count = 0;  // Initialize counter

/* Multi-line comment untuk
   penjelasan yang lebih panjang
   atau dokumentasi fungsi */

/**
 * Function documentation dengan
 * penjelasan parameter dan return value
 */
```

### Indonesiaku Code (.idk)

#### Blok dan indentasi
Blok ditandai kurung kurawal `{ }` (bukan indentasi seperti Python).
Gunakan 4 spasi agar mudah dibaca:
```indonesiaku
jika kondisi {
    cetak("Level 1")
    jika kondisi_lain {
        cetak("Level 2")
    }
}
```

#### Naming Variables
```python
# Gunakan snake_case untuk variabel
nama_lengkap = "Budi Santoso"
total_nilai = 100

# Nama deskriptif
jarak_tempuh = 100  # Baik
jt = 100           # Kurang baik
```

## 🔍 Testing

### Manual Testing
```bash
# Test dengan contoh program
./indk examples/hello.idk
./indk examples/fibonacci.idk

# Test REPL
./indk
> cetak("Test")
> x = 10
> cetak(x * 2)
```

### Debug Mode
Edit `src/common.h`:
```c
#define DEBUG_PRINT_CODE        // Print bytecode
#define DEBUG_TRACE_EXECUTION   // Trace VM execution
```

Rebuild dan jalankan untuk melihat debug output.

## 📤 Mengirim Pull Request

### Langkah-langkah

1. **Fork Repository**
   ```bash
   # Fork di GitHub UI
   # Clone fork Anda
   git clone https://github.com/yourusername/Indonesiaku.git
   ```

2. **Buat Branch Baru**
   ```bash
   git checkout -b fitur-baru
   # atau
   git checkout -b fix-bug-123
   ```

3. **Tulis Kode**
   - Ikuti panduan coding style
   - Tambahkan komentar yang jelas
   - Test kode Anda

4. **Commit Changes**
   ```bash
   git add .
   git commit -m "Tambah: Implementasi fitur X"
   # atau
   git commit -m "Perbaiki: Bug Y pada scanner"
   ```

5. **Push ke Fork**
   ```bash
   git push origin fitur-baru
   ```

6. **Buat Pull Request**
   - Buka GitHub repository Anda
   - Klik "New Pull Request"
   - Jelaskan perubahan Anda
   - Link ke issue terkait (jika ada)

### Commit Message Format

```
Tipe: Ringkasan singkat (max 50 karakter)

Penjelasan lebih detail jika diperlukan.
Jelaskan apa yang berubah dan mengapa.

Fixes #123
```

**Tipe:**
- `Tambah:` - Fitur baru
- `Perbaiki:` - Bug fix
- `Ubah:` - Perubahan fitur existing
- `Hapus:` - Menghapus fitur
- `Docs:` - Perubahan dokumentasi
- `Style:` - Formatting, missing semicolons, dll
- `Refactor:` - Refactoring kode
- `Test:` - Menambah atau memperbaiki tests
- `Chore:` - Maintenance tasks

**Contoh:**
```
Tambah: Implementasi operator modulo (%)

- Tambah OP_MODULO di chunk.h
- Implementasi modulo di VM
- Update scanner untuk recognize %
- Tambah test di examples/

Fixes #45
```

## 🐛 Pelaporan Bug

### Template Issue Bug

```markdown
## Deskripsi Bug
Penjelasan singkat bug yang ditemukan.

## Langkah Reproduce
1. Buat file `test.idk` dengan kode berikut:
   ```
   cetak("test")
   ```
2. Jalankan `indk test.idk`
3. Error muncul

## Perilaku yang Diharapkan
Seharusnya cetak "test".

## Perilaku Aktual
Error: ...

## Informasi Sistem
- OS: Windows 10
- Compiler: GCC 11.2.0
- Versi Indonesiaku: 0.1.1

## Kode yang Menyebabkan Bug
```python
# kode.idk
x = 10
cetak(x
```

## Output Error
```
[baris 2] Kesalahan: Harapkan ')' setelah argumen.
```
```

## 💡 Permintaan Fitur

### Template Issue Fitur

```markdown
## Deskripsi Fitur
Penjelasan fitur yang diusulkan.

## Use Case
Kapan dan mengapa fitur ini berguna.

## Syntax yang Diusulkan
```indonesiaku
# Contoh syntax
untuk i dalam rentang(10) {
    cetak(i)
}
```

## Implementasi yang Mungkin
Ide tentang bagaimana implementasinya.

## Alternatif
Fitur atau pendekatan alternatif yang sudah dipertimbangkan.
```

## 🎯 Area Kontribusi

Daftar pekerjaan dan prioritasnya ada di satu tempat:
[`docs/ROADMAP.md`](docs/ROADMAP.md) (Fase 0–8 menuju 1.0). Untuk tugas kecil
yang cocok bagi kontributor baru, lihat katalog fungsi yang belum ada di
[`docs/ANALISIS/06_KATALOG_TIPE_BAWAAN.md`](docs/ANALISIS/06_KATALOG_TIPE_BAWAAN.md)
dan [`docs/ANALISIS/07_KATALOG_PUSTAKA_STANDAR.md`](docs/ANALISIS/07_KATALOG_PUSTAKA_STANDAR.md).

## 📚 Resources

### Referensi
- [Crafting Interpreters](https://craftinginterpreters.com/)
- [Python Language Reference](https://docs.python.org/3/reference/)
- [C Programming Language](https://en.cppreference.com/)

### Tools
- GCC/Clang - C Compiler
- GDB - Debugger
- Valgrind - Memory leak detection
- VS Code - Editor

## ❓ Pertanyaan?

Jika ada pertanyaan:
1. Cek dokumentasi di README.md
2. Lihat issues yang sudah ada
3. Buat issue baru dengan tag "question"
4. Diskusi di forum atau chat (jika ada)

---

**Terima kasih telah berkontribusi pada Proyek Indonesiaku! 🇮🇩**

Setiap kontribusi, besar atau kecil, sangat berarti untuk proyek ini.
