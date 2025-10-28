# 🧩 Panduan Setup Installer Windows - Bahasa Pemrograman Indonesiaku

Panduan lengkap untuk membuat dan menggunakan Windows Installer (.exe) untuk Bahasa Pemrograman Indonesiaku menggunakan Inno Setup.

---

## 📋 Daftar Isi

1. [Prasyarat](#prasyarat)
2. [Struktur Folder](#struktur-folder)
3. [Langkah-langkah Setup](#langkah-langkah-setup)
4. [Compile Installer](#compile-installer)
5. [Testing Installer](#testing-installer)
6. [Distribusi](#distribusi)
7. [Troubleshooting](#troubleshooting)

---

## Prasyarat

### Yang Dibutuhkan:
- ✅ **Inno Setup** - Sudah installed di laptop Anda
- ✅ **GCC Compiler** - Untuk build binary
- ✅ **Administrator Access** - Untuk install path ke Windows
- ✅ **Folder Struktur** - Sudah dipersiapkan di `compiler/`

### Download & Install (jika belum):

**Inno Setup:**
```
URL: https://jrsoftware.org/isdl.php
Download: Inno Setup 6.x (latest)
```

---

## Struktur Folder

Struktur yang sudah dipersiapkan:

```
Indonesiaku/
├── compiler/
│   ├── bin/
│   │   └── indonesiaku.exe        ← Binary compiler (sudah dibuild)
│   ├── indonesiaku_installer.iss  ← Script installer (sudah dibuat)
│   └── setup_prompt.txt           ← Dokumentasi original
├── dist/                           ← Output installer hasil compile
│   └── Indonesiaku-Setup-v0.1.0.exe
├── docs/                           ← Documentation
├── examples/                       ← Example programs
├── src/                            ← Source code
└── README.md
```

---

## Langkah-langkah Setup

### ✅ Step 1: Verify Binary Build

Pastikan binary sudah ada:

```powershell
# Windows PowerShell
ls compiler\bin\

# Seharusnya terlihat:
# Mode                 LastWriteTime         Length Name
# ----                 ---------              ------ ----
# -a----        10/28/2025   2:00 PM        xxx KB indonesiaku.exe
```

Jika belum ada, build manual:

```powershell
cd Indonesiaku
gcc -O2 -s -o compiler/bin/indonesiaku.exe `
    src/main.c src/chunk.c src/compiler.c src/debug.c `
    src/memory.c src/object.c src/scanner.c src/table.c `
    src/value.c src/vm.c -lm
```

### ✅ Step 2: Verify Installer Script

Cek file script:

```powershell
ls compiler\indonesiaku_installer.iss
```

File `indonesiaku_installer.iss` sudah dibuat dengan:
- ✅ Informasi aplikasi (name, version, publisher)
- ✅ Installation directory: `C:\Program Files\Indonesiaku`
- ✅ Files to include (bin, docs, examples)
- ✅ Start Menu shortcuts
- ✅ Desktop shortcut (optional)
- ✅ PATH environment variable setup
- ✅ File association (.idk files)
- ✅ Custom messages dalam Bahasa Indonesia

### ✅ Step 3: Buka Inno Setup Compiler

1. **Buka Inno Setup Compiler** dari Start Menu atau desktop shortcut
   - Cari: "Inno Setup"
   - Buka: "Inno Setup Compiler"

2. **Buka Script File:**
   - Menu: `File → Open`
   - Navigate ke: `C:\Users\[YourName]\...\Indonesiaku\compiler\`
   - Pilih: `indonesiaku_installer.iss`
   - Klik: `Open`

   Atau drag-drop file ke window Inno Setup.

---

## Compile Installer

### Method 1: Menggunakan GUI (Recommended)

1. **File sudah terbuka** di Inno Setup Compiler

2. **Klik tombol Compile** (ikon Play hijau di toolbar)
   - Atau Menu: `Build → Compile`
   - Atau Shortcut: `Ctrl+F9`

3. **Tunggu compile selesai** (biasanya < 1 menit)

4. **Jika sukses**, akan melihat:
   ```
   Compile finished at 14:30:45
   Successful compile of...
   ```

5. **Output file** akan tersimpan di:
   ```
   Indonesiaku\dist\Indonesiaku-Setup-v0.1.0.exe
   ```

### Method 2: Compile Manual (Command Line)

Jika ingin compile dari terminal:

```powershell
# Windows PowerShell (as Administrator)
cd C:\Users\[YourName]\...\Indonesiaku\compiler

# Compile dengan iscc.exe (Inno Setup Compiler CLI)
"C:\Program Files (x86)\Inno Setup 6\iscc.exe" indonesiaku_installer.iss

# Tunggu selesai...
# Output: ..\dist\Indonesiaku-Setup-v0.1.0.exe
```

---

## Testing Installer

### ✅ Test 1: Verify Output File

```powershell
# Cek apakah file sudah ada
ls dist\

# Seharusnya terlihat:
# Indonesiaku-Setup-v0.1.0.exe
```

### ✅ Test 2: Run Installer

**Method A - Direct Run:**

```powershell
# Run dari PowerShell (as Administrator)
& "dist\Indonesiaku-Setup-v0.1.0.exe"
```

**Method B - File Explorer:**
1. Buka `File Explorer`
2. Navigate ke `Indonesiaku\dist\`
3. Double-click `Indonesiaku-Setup-v0.1.0.exe`

### ✅ Test 3: Follow Installation Wizard

**Langkah-langkah:**

1. **Welcome Screen**
   - Baca informasi
   - Klik: `Next`

2. **Select Destination Location**
   - Default: `C:\Program Files\Indonesiaku`
   - Atau ubah sesuai preferensi
   - Klik: `Next`

3. **Select Start Menu Folder**
   - Default: `Bahasa Pemrograman Indonesiaku`
   - Klik: `Next`

4. **Select Additional Tasks**
   - ☑ Create a desktop icon (opsional)
   - Klik: `Next`

5. **Ready to Install**
   - Review settings
   - Klik: `Install`

6. **Installing Files**
   - Progress bar akan ditampilkan
   - Tunggu selesai

7. **Installation Complete**
   - ☑ Launch Bahasa Indonesiaku (opsional)
   - Klik: `Finish`

---

## Verifikasi Instalasi

### ✅ Check 1: Files in Program Files

```powershell
ls "C:\Program Files\Indonesiaku"

# Seharusnya ada:
# Mode                 LastWriteTime         Length Name
# ----                 ---------              ------ ----
# d-----                                            bin
# d-----                                            docs
# d-----                                            examples
# -a----                                           README.md
# -a----                                           LICENSE
# -a----                                           CHANGELOG.md
```

### ✅ Check 2: Start Menu Shortcuts

1. Klik **Start Menu**
2. Cari: "Bahasa Pemrograman Indonesiaku"
3. Seharusnya melihat shortcuts:
   - Bahasa Indonesiaku (REPL)
   - Dokumentasi
   - Contoh Program
   - Copot Instalasi

### ✅ Check 3: PATH Environment Variable

```powershell
# Buka PowerShell baru (PENTING: buka terminal baru!)
# Jalankan dari mana saja:

indonesiaku --version
# Output: (version info)

# Atau coba run example:
indonesiaku "C:\Program Files\Indonesiaku\examples\hello.idk"
# Output: Halo, dunia!
```

### ✅ Check 4: File Association

1. Buka **File Explorer**
2. Navigate ke: `C:\Program Files\Indonesiaku\examples\`
3. Right-click file `.idk` (misal `hello.idk`)
4. Seharusnya ada opsi: "Open with → indonesiaku"
5. Double-click file → seharusnya langsung execute

---

## Distribusi

### Distribute Installer

Setelah successful compile:

```
dist/Indonesiaku-Setup-v0.1.0.exe
```

File ini bisa didistribusikan:

1. **GitHub Releases**
   ```
   - Upload ke GitHub Releases
   - URL: github.com/teguhriyan/Indonesiaku/releases
   ```

2. **Direct Download Link**
   ```
   - Host di personal website
   - Share link ke users
   ```

3. **SourceForge / FossHub**
   ```
   - Register dan upload
   - Free hosting untuk open source
   ```

### Version Updates

Untuk update ke versi baru (misal v0.2):

1. **Update source code** di `src/`
2. **Rebuild binary:**
   ```powershell
   gcc -O2 -s -o compiler/bin/indonesiaku.exe src/*.c -lm
   ```

3. **Update version di script:**
   - Buka: `compiler/indonesiaku_installer.iss`
   - Cari: `AppVersion=0.1.0`
   - Ubah: `AppVersion=0.2.0`
   - Cari: `OutputBaseFilename=Indonesiaku-Setup-v0.1.0`
   - Ubah: `OutputBaseFilename=Indonesiaku-Setup-v0.2.0`

4. **Recompile:**
   - Buka Inno Setup
   - Open updated `.iss` file
   - Compile

---

## Troubleshooting

### Error: "The specified file does not exist"

**Penyebab:** Path ke file source tidak valid

**Solusi:**
```
1. Pastikan sedang di folder: Indonesiaku\compiler\
2. Cek apakah binary ada: compiler\bin\indonesiaku.exe
3. Cek apakah docs folder ada: docs\
4. Cek apakah examples folder ada: examples\
```

### Error: "Administrator privileges are required"

**Penyebab:** Installer memerlukan admin rights untuk write ke PATH

**Solusi:**
```
1. Jalankan Inno Setup Compiler as Administrator
2. Atau jalankan installer dengan right-click → Run as Administrator
```

### Installer runs but files not in Program Files

**Penyebab:** Instalasi terganggu atau permissionissue

**Solusi:**
```
1. Copot instalasi dari Control Panel
2. Restart komputer
3. Jalankan installer lagi sebagai Administrator
4. Follow wizard dengan teliti
```

### PATH tidak terupdate setelah install

**Penyebab:** Terminal lama belum reload environment

**Solusi:**
```powershell
# PENTING: Buka terminal PowerShell BARU (jangan lanjutkan yang lama)
# PATH akan otomatis ter-load dari registry

# Verifikasi:
$env:Path -split ';' | findstr Indonesiaku

# Atau cek direct:
test-path "C:\Program Files\Indonesiaku\bin"
```

### Command "indonesiaku" not recognized

**Penyebab:** Terminal tidak mereload environment

**Solusi:**
```powershell
# Close semua terminal/PowerShell
# Buka terminal PowerShell BARU

# Test command:
indonesiaku --version

# Atau use full path:
"C:\Program Files\Indonesiaku\bin\indonesiaku.exe"
```

### .idk files not associated

**Penyebab:** Registry entry tidak dibuat dengan benar

**Solusi:**
```powershell
# Manual registry fix:
1. Tekan: Win + R
2. Ketik: regedit
3. Navigate ke: HKEY_CLASSES_ROOT\.idk
4. Verify ada entry: (Default) = IndonesiakuFile
5. Check: HKEY_CLASSES_ROOT\IndonesiakuFile\shell\open\command
6. Verify path: "C:\Program Files\Indonesiaku\bin\indonesiaku.exe" "%1"
```

---

## Advanced Customization

### Customize Installer Script

Edit `compiler/indonesiaku_installer.iss` untuk:

**1. Change Installation Directory:**
```ini
DefaultDirName={pf}\MyIndonesiaku  ; Change to custom path
```

**2. Add More Files:**
```ini
[Files]
Source: ".\extra\*"; DestDir: "{app}\extra"; Flags: recursesubdirs
```

**3. Custom Messages:**
```ini
[Messages]
WelcomeLabel1=Selamat datang di Installer Mods Saya
```

**4. Run Custom Commands:**
```ini
[Run]
Filename: "cmd.exe"; Parameters: "/c set PATH={app}\bin"; Flags: runhidden
```

---

## Testing Workflow

**Recommended workflow untuk development:**

```
1. Make code changes di src/
2. Build binary: gcc -O2 -s -o compiler/bin/indonesiaku.exe src/*.c -lm
3. Test binary: .\compiler\bin\indonesiaku.exe examples\hello.idk
4. If OK, compile installer: Ctrl+F9 di Inno Setup
5. Test installer: dist\Indonesiaku-Setup-v0.1.0.exe
6. Verify PATH works: Buka terminal baru, cek "indonesiaku" command
7. If OK, commit ke Git
```

---

## Summary

✅ **Setup selesai!** Sekarang Anda punya:

```
✓ Binary compiler optimized:   compiler/bin/indonesiaku.exe
✓ Installer script lengkap:    compiler/indonesiaku_installer.iss
✓ Windows installer ready:     dist/Indonesiaku-Setup-v0.1.0.exe
✓ PATH auto-setup:             Otomatis ditambahkan saat install
✓ File association:            .idk files bisa double-click
✓ Start Menu shortcuts:        Untuk mudah akses
✓ Uninstaller:                 Control Panel → Programs
```

Users tinggal:
1. Download `Indonesiaku-Setup-v0.1.0.exe`
2. Double-click → Next Next Finish
3. Buka terminal baru
4. Langsung bisa: `indonesiaku program.idk`

---

## Next Steps

1. ✅ Test installer di clean Windows (virtual machine recommended)
2. ✅ Test PATH work di berbagai command line tools (CMD, PowerShell, Git Bash)
3. ✅ Test file association dengan double-click .idk file
4. ✅ Upload ke GitHub Releases
5. ✅ Buat tutorial installation video (optional)

---

**Dokumentasi ini menjelaskan complete workflow untuk setup, compile, test, dan distribute Windows installer untuk Bahasa Pemrograman Indonesiaku!** 🚀

Versi: 1.0
Tanggal: 28 Oktober 2025
