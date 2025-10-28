# 2. Panduan Instalasi - Bahasa Pemrograman Indonesiaku

Panduan lengkap instalasi Bahasa Pemrograman Indonesiaku di berbagai sistem operasi: Windows, Linux, dan macOS.

---

## 📋 Daftar Isi

1. [Persiapan Umum](#persiapan-umum)
2. [Instalasi di Windows](#instalasi-di-windows)
3. [Instalasi dengan Docker](#-instalasi-dengan-docker)
4. [Instalasi di Linux](#instalasi-di-linux)
5. [Instalasi di macOS](#instalasi-di-macos)
6. [Verifikasi Instalasi](#verifikasi-instalasi)
7. [Troubleshooting](#troubleshooting)
8. [Build dari Source](#build-dari-source)
9. [Perbandingan Instalasi Methods](#perbandingan-instalasi-methods)

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
   - Download file: `Indonesiaku-Setup-v0.1.1.exe` (latest version)

2. **Jalankan Installer**
   - Double-click file `Indonesiaku-Setup-v0.1.1.exe`
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

## 🐳 Instalasi dengan Docker

### Persiapan

Sebelum menggunakan Docker, pastikan:

1. **Install Docker Desktop**
   - Windows: https://www.docker.com/products/docker-desktop
   - macOS: https://www.docker.com/products/docker-desktop
   - Linux: `sudo apt install docker.io` (Ubuntu/Debian)

2. **Verifikasi Docker Installation**
   ```bash
   docker --version
   # Output: Docker version XX.XX.XX
   ```

### Langkah 1: Pull Docker Image

Ambil image Indonesiaku dari Docker Hub:

```bash
docker pull teguh02/indonesiaku:latest
```

**Atau gunakan versi specific:**

```bash
docker pull teguh02/indonesiaku:v0.1.2
```

**Verifikasi image sudah di-pull:**

```bash
docker images
# Output:
# REPOSITORY                TAG       IMAGE ID      CREATED       SIZE
# teguh02/indonesiaku       latest    abc123def456  2 weeks ago    95MB
```

### Langkah 2: Run Container - Interactive (REPL)

**Opsi A - Run REPL dengan Interactive Mode:**

```bash
docker run -it teguh02/indonesiaku:latest
```

Sekarang Anda dalam container dan bisa langsung gunakan REPL:

```
> cetak("Halo dari Docker!")
Halo dari Docker!
> x = 10
> cetak(x)
10
> exit()  # atau Ctrl+D
```

**Opsi B - Run dengan Custom Name:**

```bash
docker run -it --name indonesiaku-app teguh02/indonesiaku:latest
```

**Exit dari container:**

```
Ctrl+C atau Ctrl+D
```

### Langkah 3: Run Container - File Execution

Untuk menjalankan file `.idk` dari host machine:

**Opsi A - Mount Volume Lokal:**

```bash
# Buat folder project lokal
mkdir -p ~/indonesiaku-projects
cd ~/indonesiaku-projects

# Buat file program
cat > hello.idk << 'EOF'
cetak("Halo dari file di host!")
x = 100
cetak("Nilai x:", x)
EOF

# Run file melalui Docker
docker run -it -v $(pwd):/app teguh02/indonesiaku:latest /usr/local/bin/indk /app/hello.idk
```

**Output:**
```
Halo dari file di host!
Nilai x: 100
```

**Opsi B - Windows PowerShell:**

```powershell
# Buat folder project
New-Item -ItemType Directory -Path "$env:USERPROFILE\indonesiaku-projects" -Force
cd "$env:USERPROFILE\indonesiaku-projects"

# Buat file program
@"
cetak("Halo dari Windows Docker!")
hasil = 5 + 10
cetak(hasil)
"@ | Out-File -Encoding UTF8 hello.idk

# Run file
docker run -it -v ${PWD}:/app teguh02/indonesiaku:latest /usr/local/bin/indk /app/hello.idk
```

### Langkah 4: Multiple Files

Untuk menjalankan beberapa file program:

```bash
# Buat beberapa file
cat > program1.idk << 'EOF'
cetak("Program 1")
EOF

cat > program2.idk << 'EOF'
cetak("Program 2")
EOF

# Run program 1
docker run -it -v $(pwd):/app teguh02/indonesiaku:latest /usr/local/bin/indk /app/program1.idk

# Run program 2
docker run -it -v $(pwd):/app teguh02/indonesiaku:latest /usr/local/bin/indk /app/program2.idk
```

### Langkah 5: Persistent Container

Untuk container yang dapat digunakan kembali:

**Opsi A - Create Named Container:**

```bash
# Create container (tidak running)
docker create -it --name my-indonesiaku -v $(pwd):/app teguh02/indonesiaku:latest

# Start container untuk REPL
docker start -i my-indonesiaku

# Gunakan lagi nanti
docker start -i my-indonesiaku
```

**Opsi B - Run Detached (Background):**

```bash
# Run di background
docker run -d --name indonesiaku-bg -v $(pwd):/app teguh02/indonesiaku:latest sleep 1000

# Execute command di container yang sudah running
docker exec -it indonesiaku-bg /usr/local/bin/indk /app/hello.idk

# Stop container
docker stop indonesiaku-bg

# Remove container
docker rm indonesiaku-bg
```

### Langkah 6: Verifikasi Installation dalam Docker

**Check binary version:**

```bash
docker run --rm teguh02/indonesiaku:latest indk -v
# Output: Indonesiaku v0.1.2
```

**Check binary location:**

```bash
docker run --rm teguh02/indonesiaku:latest which indk
# Output: /usr/local/bin/indk
```

**List available tools dalam image:**

```bash
docker run --rm teguh02/indonesiaku:latest apk list --installed
# Menampilkan semua packages terinstall
```

---

## Perbandingan Instalasi Methods

| Method | Pros | Cons | Cocok untuk |
|--------|------|------|-------------|
| **Installer (Windows)** | Mudah, auto PATH, 1-click | Windows only | Pengguna casual |
| **Build dari Source** | Full control, bisa customize | Butuh tools, lebih kompleks | Developer, contributor |
| **Docker** | Isolated, portable, konsisten | Butuh Docker, sedikit overhead | Testing, CI/CD, cross-platform |

---

## Helpful Docker Commands

```bash
# List running containers
docker ps

# List all containers (including stopped)
docker ps -a

# View container logs
docker logs <container_name>

# Remove container
docker rm <container_name>

# Remove image
docker rmi teguh02/indonesiaku:latest

# View image info
docker inspect teguh02/indonesiaku:latest

# Tag image with custom name
docker tag teguh02/indonesiaku:latest my-indonesiaku:custom

# Save image to tar
docker save teguh02/indonesiaku:latest > indonesiaku.tar

# Load image from tar
docker load < indonesiaku.tar

# Export container as image
docker commit <container_id> my-indonesiaku:snapshot
```

---

## Docker Troubleshooting

### Image Tidak Ketemu

```bash
# Search image
docker search indonesiaku

# Pull dengan tag spesifik
docker pull teguh02/indonesiaku:v0.1.2

# List local images
docker images
```

### Permission Denied (Linux)

```bash
# Option 1: Use sudo
sudo docker run -it teguh02/indonesiaku:latest

# Option 2: Add user to docker group
sudo usermod -aG docker $USER
newgrp docker
docker run -it teguh02/indonesiaku:latest  # Sekarang bisa tanpa sudo
```

### Container Cannot Access Host Files

```bash
# Pastikan volume mounting syntax benar
docker run -it -v /host/path:/container/path teguh02/indonesiaku:latest

# Windows: gunakan full path
docker run -it -v C:\Users\YourName\projects:/app teguh02/indonesiaku:latest
```

### Image Too Large / Slow Download

```bash
# Check image size
docker images

# Gunakan lighter base image (sudah digunakan Alpine 3.20)
# Alpine ~ 95MB, jauh lebih kecil dari Ubuntu ~1GB

# Pull hanya specific layer
docker pull --no-parallel teguh02/indonesiaku:latest
```

---



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
