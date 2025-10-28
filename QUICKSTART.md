# Panduan Cepat Build & Run - Bahasa Indonesiaku

## Windows (PowerShell/CMD)

### Build Proyek
```powershell
# Cara 1: Menggunakan GCC langsung
gcc -Wall -Wextra -std=c11 -O2 -Isrc -o indk.exe src/main.c src/chunk.c src/compiler.c src/debug.c src/memory.c src/object.c src/scanner.c src/table.c src/value.c src/vm.c -lm

# Cara 2: Menggunakan Make (jika terinstal)
make

# Cara 3: Menggunakan VS Code
# Tekan Ctrl+Shift+B, pilih "Build Indonesiaku (C VM)"
```

### Menjalankan Program

```powershell
# Menjalankan file .idk
.\indk.exe examples\hello.idk
.\indk.exe examples\fibonacci.idk
.\indk.exe examples\kalkulator.idk

# Menjalankan REPL (mode interaktif)
.\indk.exe
```

### Membersihkan Build
```powershell
# Hapus file hasil kompilasi
del src\*.o
del indk.exe

# Atau dengan Make
make clean
```

## Linux/Mac

### Build Proyek
```bash
# Cara 1: Menggunakan GCC langsung
gcc -Wall -Wextra -std=c11 -O2 -Isrc -o indk src/main.c src/chunk.c src/compiler.c src/debug.c src/memory.c src/object.c src/scanner.c src/table.c src/value.c src/vm.c -lm

# Cara 2: Menggunakan Make
make

# Cara 3: Menggunakan VS Code
# Tekan Ctrl+Shift+B, pilih "Build Indonesiaku (C VM)"
```

### Menjalankan Program

```bash
# Menjalankan file .idk
./indk examples/hello.idk
./indk examples/fibonacci.idk
./indk examples/kalkulator.idk

# Menjalankan REPL (mode interaktif)
./indk
```

### Membersihkan Build
```bash
# Hapus file hasil kompilasi
make clean

# Atau manual
rm src/*.o indk
```

## Menggunakan VS Code Tasks

### Task yang Tersedia:
1. **Build Indonesiaku (C VM)** - Kompilasi proyek
2. **Run Indonesiaku (file aktif)** - Jalankan file .idk yang sedang dibuka
3. **Run Indonesiaku REPL** - Jalankan mode interaktif
4. **Clean Build** - Bersihkan hasil kompilasi

### Cara Menggunakan:
- Tekan `Ctrl+Shift+B` untuk build
- Tekan `Ctrl+Shift+P`, ketik "Run Task", pilih task yang diinginkan
- Atau gunakan menu Terminal → Run Task...

## Membuat File .idk Baru

1. Buat file baru dengan ekstensi `.idk`
2. Tulis kode menggunakan sintaks Indonesiaku
3. Simpan file
4. Jalankan dengan: `indk.exe namafile.idk` (Windows) atau `./indk namafile.idk` (Linux/Mac)

## Contoh Program Sederhana

### hello.idk
```python
cetak("Halo, dunia!")
```

### hitung.idk
```python
fungsi kali(a, b):
    kembalikan a * b

hasil = kali(7, 6)
cetak("7 x 6 =", hasil)
```

## Tips Debugging

### Mengaktifkan Debug Mode
Edit file `src/common.h` dan uncomment baris berikut:
```c
#define DEBUG_PRINT_CODE
#define DEBUG_TRACE_EXECUTION
```

Kemudian rebuild proyek untuk melihat bytecode dan trace eksekusi.

### Pesan Error Umum

1. **"String tidak ditutup"** - Pastikan string menggunakan tanda kutip penutup
2. **"Variabel tidak terdefinisi"** - Variabel belum dibuat atau typo nama variabel
3. **"Harapkan ')' setelah argumen"** - Lupa menutup kurung
4. **"Operand harus berupa angka"** - Mencoba operasi matematika pada non-angka

## Struktur Proyek

```
Indonesiaku/
├── src/                # Source code C
│   ├── main.c         # Entry point
│   ├── scanner.c      # Lexer
│   ├── compiler.c     # Parser & compiler
│   ├── vm.c           # Virtual machine
│   └── ...
├── examples/          # Contoh program .idk
│   ├── hello.idk
│   ├── fibonacci.idk
│   └── ...
├── .vscode/          # Konfigurasi VS Code
│   └── tasks.json
├── Makefile          # Build script
└── README.md         # Dokumentasi lengkap
```

## Troubleshooting

### Windows: GCC tidak ditemukan
- Install MinGW-w64 atau MSYS2
- Tambahkan path GCC ke environment variables
- Atau gunakan Visual Studio dengan compiler MSVC

### Linux: error saat compile
```bash
# Install GCC jika belum ada
sudo apt-get install build-essential  # Debian/Ubuntu
sudo yum install gcc                   # RedHat/CentOS
```

### Mac: compiler error
```bash
# Install Xcode Command Line Tools
xcode-select --install
```

## Kontribusi

Jika menemukan bug atau ingin menambahkan fitur:
1. Fork repository
2. Buat branch baru
3. Commit perubahan
4. Push dan buat Pull Request

---

Selamat coding! 🇮🇩
