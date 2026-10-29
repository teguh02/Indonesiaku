# 🧩 Compiler Setup & Installer Builder

> **Usang sejak 1.4.0.** Installer rilis resmi kini dibangun otomatis oleh
> `.github/workflows/release.yml` memakai `packaging/windows/indonesiaku.iss`.
> Berkas `compiler/indonesiaku_installer.iss` masih menyebut versi 0.1.4,
> biner `indonesiaku.exe` (sekarang `indk.exe`), dan URL repo lama — jangan
> dipakai untuk rilis baru. Dokumen ini dipertahankan sebagai catatan manual
> untuk pembuatan installer secara lokal.

Folder ini berisi semua yang diperlukan untuk membangun dan mendistribusikan Windows installer untuk Bahasa Pemrograman Indonesiaku.

---

## 📁 Struktur Folder

```
compiler/
├── README.md                       # File ini
├── setup_prompt.txt               # Prompt/notes asli dari Teguh
├── indonesiaku_installer.iss      # Inno Setup script (MAIN FILE)
├── SETUP_WINDOWS_INSTALLER.md     # Panduan lengkap setup
├── bin/
│   └── indonesiaku.exe            # Compiled binary (output dari gcc)
└── [dist/ - created after compile]
    └── Indonesiaku-Setup-v*.exe   # Final Windows installer
```

---

## 🚀 Quick Start

### 1. Build Binary

```powershell
cd ..\  # Go to Indonesiaku root
gcc -O2 -s -o compiler/bin/indonesiaku.exe src/*.c -lm
```

### 2. Open in Inno Setup

- Buka **Inno Setup Compiler**
- File → Open → `compiler/indonesiaku_installer.iss`

### 3. Compile Installer

- Klik tombol **Compile** (ikon Play hijau)
- Atau Ctrl+F9
- Output akan ada di: `dist/Indonesiaku-Setup-v0.1.1.exe`

### 4. Test Installer

- Double-click file `.exe` di `dist/` folder
- Follow installation wizard
- Verify PATH works di terminal baru

---

## 📖 Files Reference

### `indonesiaku_installer.iss`
**Script Inno Setup untuk installer**

Features:
- ✅ Professional installer UI
- ✅ Installation ke `C:\Program Files\Indonesiaku\`
- ✅ Start Menu shortcuts
- ✅ Desktop shortcut (optional)
- ✅ Automatic PATH setup
- ✅ .idk file association
- ✅ Bahasa Indonesia messages
- ✅ Uninstaller

### `SETUP_WINDOWS_INSTALLER.md`
**Panduan lengkap** dengan:
- Prasyarat & setup
- Step-by-step instructions
- Compile procedures
- Testing checklist
- Distribusi guide
- Troubleshooting
- Advanced customization

### `setup_prompt.txt`
**Original notes dari Teguh** tentang:
- Konsep installer
- Struktur folder
- Contoh script
- Inno Setup basics

---

## 🔄 Workflow Lengkap

```
1. Code Changes (di src/)
   ↓
2. Rebuild Binary (gcc command)
   → Output: compiler/bin/indonesiaku.exe
   ↓
3. Open Script (Inno Setup)
   → File: compiler/indonesiaku_installer.iss
   ↓
4. Compile Installer (Ctrl+F9)
   → Output: dist/Indonesiaku-Setup-v0.1.1.exe
   ↓
5. Test Installer (double-click .exe)
   → Verify install & PATH
   ↓
6. Upload to GitHub (if ok)
   → Releases page
```

---

## ⚙️ Inno Setup Script Configuration

### Key Variables:
```ini
[Setup]
AppName=Bahasa Pemrograman Indonesiaku
AppVersion=0.1.1
DefaultDirName={pf}\Indonesiaku      ; C:\Program Files\Indonesiaku
OutputBaseFilename=Indonesiaku-Setup-v0.1.0
OutputDir=..\dist
```

### Files Included:
```ini
[Files]
- bin\indonesiaku.exe             ; Main executable
- ..\docs\*                       ; Documentation
- ..\examples\*                   ; Example programs
- ..\README.md, LICENSE, CHANGELOG.md
```

### Shortcuts Created:
```
Start Menu:
  └─ Bahasa Pemrograman Indonesiaku
    ├─ Bahasa Indonesiaku (REPL)
    ├─ Dokumentasi
    ├─ Contoh Program
    └─ Copot Instalasi

Desktop: (optional)
  └─ Bahasa Indonesiaku
```

### Registry Changes:
```
PATH Environment:
  → Adds: C:\Program Files\Indonesiaku\bin

File Association:
  .idk files → Open with indonesiaku.exe
```

---

## 📋 Checklist Before Compile

- [ ] Source code compiled successfully (`gcc -O2 -s ...`)
- [ ] Binary exists: `compiler/bin/indonesiaku.exe`
- [ ] Docs folder updated & complete
- [ ] Examples folder updated & complete
- [ ] Version number in script matches release
- [ ] README.md & LICENSE in root folder
- [ ] Test with examples: `indonesiaku examples/hello.idk`

---

## 🧪 Testing After Install

```powershell
# 1. Check installation path
ls "C:\Program Files\Indonesiaku"

# 2. Check PATH (IMPORTANT: Open NEW terminal!)
indonesiaku --version

# 3. Run example
indonesiaku "C:\Program Files\Indonesiaku\examples\hello.idk"

# 4. Test REPL
indonesiaku
# Type: cetak("Hello")
# Type: Ctrl+C to exit

# 5. Check Start Menu
# Click Start → Bahasa Pemrograman Indonesiaku
```

---

## 🔧 Customization

### Change Installation Path
Edit `indonesiaku_installer.iss`:
```ini
DefaultDirName={pf}\MyCustomPath
```

### Add More Shortcuts
```ini
[Icons]
Name: "{group}\My Custom Shortcut"; Filename: "{app}\bin\indonesiaku.exe"
```

### Modify Messages (Bahasa Indonesia/English)
```ini
[Messages]
WelcomeLabel1=Custom welcome message
```

### Add Silent Install Support
```ini
[Setup]
; Allow /SILENT and /VERYSILENT parameters
```

---

## 📦 Distribution

After successful compile:

1. **Find installer:**
   ```
   dist/Indonesiaku-Setup-v0.1.0.exe
   ```

2. **Upload options:**
   - GitHub Releases (recommended)
   - SourceForge
   - Personal website
   - Google Drive (shared link)

3. **Share with users:**
   - Direct download link
   - Instructions to run installer
   - Notes about Administrator access requirement

---

## 🆘 Common Issues

### "File not found" error
→ Check paths in `.iss` script are correct relative to current location

### "Permission denied"
→ Run Inno Setup Compiler as Administrator

### PATH doesn't work after install
→ Open NEW terminal (not the old one)

### .idk association not working
→ Check registry `HKEY_CLASSES_ROOT\.idk`

---

## 📚 Resources

- **Inno Setup Official:** https://jrsoftware.org/isinfo.php
- **Script Help:** Help menu in Inno Setup Compiler
- **Examples:** Install Inno Setup → Examples folder
- **Documentation:** `SETUP_WINDOWS_INSTALLER.md` (detailed guide)

---

## 📝 Notes

- Binary built with `-O2 -s` flags for optimization & small size
- Installer requires Administrator privileges for PATH modification
- .idk file association works for file browser & command line
- REPL launches when double-clicking compiler from Start Menu
- Uninstaller available in Control Panel → Programs & Features

---

**Ready to distribute Bahasa Pemrograman Indonesiaku untuk Windows users!** 🎉

Versi: 1.0
Tanggal: 28 Oktober 2025
Author: Teguh Rijanandi
