# 2. Panduan Instalasi - Bahasa Pemrograman Indonesiaku

Panduan lengkap instalasi Bahasa Pemrograman Indonesiaku di berbagai sistem operasi: Windows, Linux, dan macOS.

---

## 📋 Daftar Isi

1. [Persiapan Umum](#persiapan-umum)
2. [Instalasi di Windows](#instalasi-di-windows)
3. [Instalasi di Linux](#instalasi-di-linux)
4. [Instalasi di macOS](#instalasi-di-macos)
5. [Verifikasi Instalasi](#verifikasi-instalasi)
6. [Troubleshooting](#troubleshooting)
7. [Build dari Source](#build-dari-source)

---

## Persiapan Umum

Sebelum melakukan instalasi, siapkan beberapa tools yang diperlukan:

### Tools yang Dibutuhkan

1. **GCC Compiler** - Untuk compile source code dari C
2. **Git** - Untuk clone repository
3. **Text Editor/IDE** - Untuk menulis program
4. **PowerShell/Bash** - Terminal untuk menjalankan commands

### Verifikasi Koneksi Internet

Pastikan komputer Anda terhubung ke internet untuk:
- Download repository dari GitHub
- Download dependencies (jika diperlukan)
- Mengakses dokumentasi online

---

## Instalasi di Windows

### 🚀 CARA MUDAH - Download Installer (Recommended!)

**Untuk pengguna Windows yang ingin install dengan cepat dan mudah:**

1. **Download Installer**
   - Buka: https://github.com/teguh02/Indonesiaku/releases
   - Download file: `Indonesiaku-Setup-v0.1.0.exe` (latest version)

2. **Jalankan Installer**
   - Double-click file `Indonesiaku-Setup-v0.1.0.exe`
   - Ikuti wizard instalasi: Next → Next → Install → Finish
   - Tunggu hingga instalasi selesai (± 1 menit)

3. **Verifikasi Instalasi**
   - Buka **PowerShell** atau **Command Prompt** yang BARU
   - Ketik: `indonesiaku`
   - Seharusnya melihat REPL prompt `> `

4. **Done! ✅**
   - Sekarang Anda bisa mulai programming dengan Bahasa Indonesiaku!

**Keuntungan menggunakan installer:**
- ✅ 1-click installation (sangat mudah)
- ✅ Automatic PATH setup (tidak perlu manual config)
- ✅ File association (.idk files bisa double-click)
- ✅ Start Menu shortcuts
- ✅ Easy uninstall

---

### 🔧 CARA MANUAL - Build dari Source

**Untuk developer atau yang ingin build dari source code:**

#### Langkah 1: Install Git

**Opsi A - Menggunakan Official Installer:**

1. Buka https://git-scm.com/download/win
2. Download Git untuk Windows
3. Jalankan installer (`.exe`)
4. Ikuti wizard instalasi dengan default settings
5. Finish

**Opsi B - Menggunakan Chocolatey (jika sudah installed):**

```powershell
choco install git
```

**Verifikasi:**

```powershell
git --version
# Output: git version 2.x.x...
```

#### Langkah 2: Install GCC (MinGW)

**Opsi A - Menggunakan MinGW-w64 Installer:**

1. Buka https://www.mingw-w64.org/
2. Download installer (Online atau Offline)
3. Jalankan installer
4. Pilih settings:
   - Version: Latest (e.g., 11.2.0)
   - Architecture: x86_64 (untuk 64-bit) atau i686 (untuk 32-bit)
   - Threads: posix
   - Exception: seh (untuk 64-bit) atau dwarf (untuk 32-bit)
   - Build revision: Latest
5. Finish dan remember installation path

**Opsi B - Menggunakan Chocolatey:**

```powershell
choco install mingw
```

**Langkah Tambahan - Set Environment Variable:**

Setelah install MinGW, tambahkan ke PATH:

1. Tekan `Win + X` → pilih "System"
2. Klik "Advanced system settings"
3. Klik "Environment Variables"
4. Di "System variables", cari "Path" dan klik "Edit"
5. Klik "New" dan tambahkan path ke MinGW bin folder:
   ```
   C:\Program Files\mingw-w64\x86_64-11.2.0-posix-seh-rt_v9-rev0\mingw64\bin
   ```
   (Sesuaikan dengan path instalasi Anda)
6. Klik OK dan tutup windows

**Verifikasi:**

Buka PowerShell baru dan cek:

```powershell
gcc --version
# Output: gcc (Rev5, Built by MSYS2 project) 11.2.0...

g++ --version
# Output: g++ (Rev5, Built by MSYS2 project) 11.2.0...

make --version
# Output: GNU Make 4.x.x...
```

#### Langkah 3: Clone Repository Indonesiaku

Pilih folder tempat Anda ingin menyimpan project:

```powershell
# Navigasi ke folder yang diinginkan
cd C:\Users\YourName\Documents

# Clone repository
git clone https://github.com/teguh02/Indonesiaku.git
cd Indonesiaku
```

**Verifikasi struktur:**

```powershell
ls -la
# Seharusnya melihat: src/, examples/, docs/, tests/, dll
```

#### Langkah 4: Build Project

**Opsi A - Menggunakan Make (Recommended):**

```powershell
make
```

**Opsi B - Menggunakan GCC langsung:**

```powershell
gcc -Wall -Wextra -std=c11 -O2 -Isrc -o indk.exe `
    src/main.c src/chunk.c src/compiler.c src/debug.c `
    src/memory.c src/object.c src/scanner.c src/table.c `
    src/value.c src/vm.c -lm
```

**Output:**

```
indk.exe  # Executable yang berhasil dibuild
```

#### Langkah 5: Test Installation

```powershell
# Run REPL
./indk.exe

# Ketik beberapa commands
cetak("Halo, Windows!")
x = 10
cetak(x)

# Exit dengan Ctrl+C
```

#### Langkah 6 (Opsional): Add ke PATH

Agar bisa run `indk` dari mana saja:

1. Tekan `Win + X` → "System"
2. "Advanced system settings" → "Environment Variables"
3. "System variables" → "Path" → "Edit"
4. "New" dan tambahkan path ke folder Indonesiaku:
   ```
   C:\Users\YourName\Documents\Indonesiaku
   ```
5. OK dan restart terminal

---

## Instalasi di Linux

### Langkah 1: Install Build Tools

**Ubuntu/Debian:**

```bash
sudo apt update
sudo apt install -y build-essential git
```

**Fedora/RHEL/CentOS:**

```bash
sudo dnf install -y gcc gcc-c++ make git
```

**Arch Linux:**

```bash
sudo pacman -S base-devel git
```

**Verifikasi:**

```bash
gcc --version
make --version
git --version
```

### Langkah 2: Clone Repository

```bash
# Navigasi ke folder yang diinginkan
cd ~/

# Clone repository
git clone https://github.com/teguhriyan/Indonesiaku.git
cd Indonesiaku
```

**Verifikasi struktur:**

```bash
ls -la
# Seharusnya melihat: src/, examples/, docs/, tests/, dll
```

### Langkah 3: Build Project

**Opsi A - Menggunakan Make:**

```bash
make
```

**Opsi B - Menggunakan GCC langsung:**

```bash
gcc -Wall -Wextra -std=c11 -O2 -Isrc -o indk \
    src/main.c src/chunk.c src/compiler.c src/debug.c \
    src/memory.c src/object.c src/scanner.c src/table.c \
    src/value.c src/vm.c -lm
```

**Output:**

```
indk  # Executable yang berhasil dibuild
```

### Langkah 4: Test Installation

```bash
# Run REPL
./indk

# Ketik beberapa commands
cetak("Halo, Linux!")
x = 10
cetak(x)

# Exit dengan Ctrl+C
```

### Langkah 5 (Opsional): Add ke PATH

Agar bisa run `indk` dari mana saja:

```bash
# Copy executable ke /usr/local/bin
sudo cp indk /usr/local/bin/indk

# Verifikasi
which indk  # Seharusnya: /usr/local/bin/indk

# Sekarang bisa run dari mana saja
indk
```

### Langkah 6 (Opsional): Create Symlink

Atau gunakan symlink (jika ingin keep original di folder project):

```bash
# Create symlink
sudo ln -s ~/Indonesiaku/indk /usr/local/bin/indk

# Verify
indk --help
```

---

## Instalasi di macOS

### Langkah 1: Install Xcode Command Line Tools

```bash
xcode-select --install
```

Tunggu instalasi selesai (bisa 5-10 menit).

**Verifikasi:**

```bash
gcc --version
make --version
```

### Langkah 2: Install Git

Git sudah included di Xcode Command Line Tools.

**Verifikasi:**

```bash
git --version
# Output: git version 2.x.x...
```

Jika ingin Git yang lebih baru, gunakan Homebrew:

```bash
# Install Homebrew terlebih dahulu jika belum
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install Git
brew install git
```

### Langkah 3: Clone Repository

```bash
# Navigasi ke folder yang diinginkan
cd ~/

# Clone repository
git clone https://github.com/teguhriyan/Indonesiaku.git
cd Indonesiaku
```

**Verifikasi struktur:**

```bash
ls -la
# Seharusnya melihat: src/, examples/, docs/, tests/, dll
```

### Langkah 4: Build Project

**Opsi A - Menggunakan Make:**

```bash
make
```

**Opsi B - Menggunakan GCC langsung:**

```bash
gcc -Wall -Wextra -std=c11 -O2 -Isrc -o indk \
    src/main.c src/chunk.c src/compiler.c src/debug.c \
    src/memory.c src/object.c src/scanner.c src/table.c \
    src/value.c src/vm.c -lm
```

**Output:**

```
indk  # Executable yang berhasil dibuild
```

### Langkah 5: Test Installation

```bash
# Run REPL
./indk

# Ketik beberapa commands
cetak("Halo, macOS!")
x = 10
cetak(x)

# Exit dengan Ctrl+C
```

### Langkah 6 (Opsional): Add ke PATH

Agar bisa run `indk` dari mana saja:

**Opsi A - Copy ke /usr/local/bin:**

```bash
# Copy executable
sudo cp indk /usr/local/bin/indk

# Verifikasi
which indk

# Run dari mana saja
indk
```

**Opsi B - Gunakan Homebrew (Advanced):**

Jika Anda ingin create Homebrew formula, ikuti dokumentasi Homebrew.

---

## Verifikasi Instalasi

### Test 1: Check Compiler

```bash
gcc --version
# Output: gcc (...) version X.X.X
```

### Test 2: Check Git

```bash
git --version
# Output: git version X.X.X
```

### Test 3: Check Indonesiaku Binary

```bash
# Windows
.\indk.exe

# Linux/macOS
./indk
```

Output seharusnya:
```
> 
```
(REPL prompt yang siap untuk input)

### Test 4: Hello World

```indonesiaku
cetak("Halo, dunia!")
```

Expected output:
```
Halo, dunia!
```

### Test 5: Simple Calculation

```indonesiaku
x = 10
y = 5
hasil = x + y
cetak("Hasil:", hasil)
```

Expected output:
```
Hasil: 15
```

### Test 6: Run Example File

**Windows:**
```powershell
.\indk.exe examples/hello.idk
```

**Linux/macOS:**
```bash
./indk examples/hello.idk
```

Expected output:
```
Halo, dunia!
```

---

## Troubleshooting

### Windows Issues

#### Error: "gcc: command not found"

**Solusi:**
- Verifikasi MinGW sudah installed (Run `gcc --version`)
- Check PATH environment variable (Control Panel → System → Environment Variables)
- Restart terminal setelah mengubah PATH
- Reinstall MinGW jika perlu

#### Error: "cannot open source file"

**Solusi:**
```powershell
# Pastikan Anda di folder Indonesiaku
cd C:\path\to\Indonesiaku

# Coba build lagi
make clean
make
```

#### Executable sudah dibuat tapi tidak bisa run

**Solusi:**
```powershell
# Check apakah executable ada
ls indk.exe

# Run dengan full path
.\indk.exe

# Atau dari mana saja jika sudah di PATH
indk.exe
```

### Linux/macOS Issues

#### Error: "make: command not found"

**Solusi:**
```bash
# Install build-essential (Linux)
sudo apt install build-essential

# Atau gunakan GCC langsung (tanpa make)
gcc -Wall -Wextra -std=c11 -O2 -Isrc -o indk \
    src/main.c src/chunk.c src/compiler.c src/debug.c \
    src/memory.c src/object.c src/scanner.c src/table.c \
    src/value.c src/vm.c -lm
```

#### Error: "git: command not found"

**Solusi:**
```bash
# Linux
sudo apt install git

# macOS - Install Xcode Command Line Tools
xcode-select --install
```

#### Permission denied when running ./indk

**Solusi:**
```bash
# Make it executable
chmod +x indk

# Run
./indk
```

#### Error: "libm.so.6: cannot open shared object file"

**Solusi:**
```bash
# Install required libraries
sudo apt install libm  # atau libc6

# Atau rebuild dengan static linking
gcc -static -Wall -Wextra -std=c11 -O2 -Isrc -o indk \
    src/main.c src/chunk.c src/compiler.c src/debug.c \
    src/memory.c src/object.c src/scanner.c src/table.c \
    src/value.c src/vm.c -lm
```

### Universal Issues

#### REPL opens tapi tidak bisa input

**Solusi:**
- Check apakah terminal sudah properly configured
- Try input sederhana: `cetak("test")`
- Exit dengan `Ctrl+C`
- Report issue di GitHub jika persist

#### Test file tidak ketemu

**Solusi:**
```bash
# Pastikan Anda di folder Indonesiaku
pwd  # atau cd untuk verifikasi lokasi

# Check examples folder ada
ls examples/

# Run dengan full path
./indk examples/hello.idk
```

#### Build error dengan specific warning

**Solusi:**
- Baca error message carefully
- Check file yang di-mention di error
- Try `make clean` sebelum build lagi
- Update compiler ke versi terbaru

---

## Build dari Source

Jika ingin build manual atau customize:

### 1. Compile Individual Files

```bash
# Windows (PowerShell)
gcc -Wall -Wextra -std=c11 -c -Isrc src/main.c -o src/main.o
gcc -Wall -Wextra -std=c11 -c -Isrc src/vm.c -o src/vm.o
# ... (repeat untuk semua file)

# Linux/macOS
gcc -Wall -Wextra -std=c11 -c -Isrc src/main.c -o src/main.o
gcc -Wall -Wextra -std=c11 -c -Isrc src/vm.c -o src/vm.o
# ... (repeat untuk semua file)
```

### 2. Link Object Files

```bash
# Windows
gcc -o indk.exe src/main.o src/chunk.o src/compiler.o src/debug.o ^
    src/memory.o src/object.o src/scanner.o src/table.o ^
    src/value.o src/vm.o -lm

# Linux/macOS
gcc -o indk src/main.o src/chunk.o src/compiler.o src/debug.o \
    src/memory.o src/object.o src/scanner.o src/table.o \
    src/value.o src/vm.o -lm
```

### 3. Clean Compilation

```bash
# Linux/macOS
make clean
make

# Windows
del /Q src\*.o indk.exe
gcc -Wall -Wextra -std=c11 -O2 -Isrc -o indk.exe `
    src/main.c src/chunk.c src/compiler.c src/debug.c `
    src/memory.c src/object.c src/scanner.c src/table.c `
    src/value.c src/vm.c -lm
```

### 4. Debug Build

Tanpa optimization untuk debugging:

```bash
gcc -Wall -Wextra -std=c11 -g -Isrc -o indk \
    src/main.c src/chunk.c src/compiler.c src/debug.c \
    src/memory.c src/object.c src/scanner.c src/table.c \
    src/value.c src/vm.c -lm
```

### 5. Release Build

Dengan optimization untuk performance:

```bash
gcc -Wall -Wextra -std=c11 -O3 -Isrc -o indk \
    src/main.c src/chunk.c src/compiler.c src/debug.c \
    src/memory.c src/object.c src/scanner.c src/table.c \
    src/value.c src/vm.c -lm
```

---

## Next Steps Setelah Install

1. **Read Documentation**
   - Baca `docs/01_PERKENALAN.md` untuk overview
   - Baca `docs/02_PANDUAN_SINTAKS.md` untuk learn syntax

2. **Run Examples**
   ```bash
   ./indk examples/hello.idk
   ./indk examples/fibonacci.idk
   ./indk examples/fungsi.idk
   ```

3. **Write Your First Program**
   - Create file: `my_first.idk`
   - Write code
   - Run: `./indk my_first.idk`

4. **Join Community**
   - Star repository di GitHub
   - Follow project updates
   - Share feedback

5. **Consider Contributing**
   - Read `docs/03_KONTRIBUSI.md`
   - Submit improvements

---

## Quick Reference - OS Comparison

| Aspek | Windows | Linux | macOS |
|-------|---------|-------|-------|
| **Compiler** | MinGW-w64 | GCC (apt) | Xcode CLT |
| **Package Mgr** | Chocolatey | apt/dnf | Homebrew |
| **Executable** | `indk.exe` | `indk` | `indk` |
| **Run REPL** | `.\indk.exe` | `./indk` | `./indk` |
| **Run File** | `.\indk.exe file.idk` | `./indk file.idk` | `./indk file.idk` |
| **Add to PATH** | Environment Vars | ~/.bashrc | ~/.zprofile |
| **Make** | MinGW make or choco | apt install make | Xcode CLT |

---

## Support

Jika mengalami masalah:

1. **Check Dokumentasi**
   - Baca section Troubleshooting di atas
   - Cek docs/ folder

2. **GitHub Issues**
   - Search existing issues
   - Create new issue dengan detail:
     - OS & version
     - GCC version
     - Error message lengkap
     - Steps to reproduce

3. **Community**
   - GitHub Discussions
   - Email: teguhriyan@gmail.com

---

**Selamat! Anda sudah berhasil install Bahasa Pemrograman Indonesiaku!**

Mari mulai programming dengan Bahasa Indonesia! 🚀

---

Versi: 1.0  
Terakhir diperbarui: 28 Oktober 2025  
Platform: Windows, Linux, macOS
