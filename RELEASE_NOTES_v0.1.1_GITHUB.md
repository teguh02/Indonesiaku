# 🎉 Bahasa Pemrograman Indonesiaku v0.1.1

**Rilis Pembaruan & Perbaikan - 28 Oktober 2025**

Kami dengan senang hati merilis versi patch **v0.1.1** yang mencakup perbaikan runtime, fitur CLI baru, dan peningkatan dokumentasi.

---

## ✨ Yang Baru

### 🔧 CLI Flags (Command-Line Arguments)
- **`-v`, `--version`**: Tampilkan nomor versi interpreter  
  ```bash
  $ indk.exe -v
  0.1.1
  ```
- **`-h`, `--help`**: Tampilkan pesan bantuan (akan ditambahkan di versi mendatang)

### 🐛 Bug Fixes & Improvements
- **Disable Debug Printing**: Bytecode disassembly tidak lagi ditampilkan pada build rilis (membuatnya lebih bersih dan cepat)
- **Version Bumped**: Versi internal dinaikan ke `0.1.1` untuk konsistensi rilis
- **Documentation Updated**: Release notes dan panduan instalasi diperbarui untuk mencerminkan versi terbaru

---

## 📋 Changelog Lengkap

### v0.1.1 (28 Oktober 2025)

#### Changed
- Bumped version to `0.1.1`
- Added CLI flags `-v/--version` dan `-h/--help`
- Disabled compiler debug bytecode printing by default untuk build rilis
- Updated installer metadata dan dokumentasi ke v0.1.1

#### Fixed
- Bytecode disassembly no longer prints in release builds unless explicitly enabled

---

## 📦 Download & Instalasi

### Windows (Recommended - Installer)
**Opsi Termudah untuk Windows!**

1. Download: [Indonesiaku-Setup-v0.1.1.exe](#) ↓ (1.91 MB)
2. Double-click installer
3. Ikuti wizard: Next → Next → Install → Finish
4. Selesai! Jalankan `indk` dari Command Prompt atau PowerShell

```powershell
# Verify installation
indk.exe -v
# Output: 0.1.1
```

### macOS / Linux (Build dari Sumber)

Persyaratan: `gcc` dan `make`

```bash
git clone https://github.com/teguh02/Indonesiaku.git
cd Indonesiaku
make clean
make
./indk examples/hello.idk
```

---

## 🚀 Quick Start

### Jalankan Program dari File
```bash
indk.exe nama_program.idk
```

### Jalankan REPL (Interactive Mode)
```bash
indk.exe
# Ketik kode Indonesiaku langsung dan tekan Enter
# Ketik 'keluar' untuk menutup REPL
```

### Contoh Program Sederhana
File: `hello.idk`
```indonesiaku
tulis("Halo, Dunia!")
```

Jalankan:
```bash
$ indk.exe hello.idk
Halo, Dunia!
```

---

## 📚 Dokumentasi

- 📖 **[Panduan Lengkap](https://github.com/teguh02/Indonesiaku/tree/main/docs)**
  - [01_PERKENALAN.md](https://github.com/teguh02/Indonesiaku/blob/main/docs/01_PERKENALAN.md) - Pengenalan bahasa
  - [02_INSTALASI.md](https://github.com/teguh02/Indonesiaku/blob/main/docs/02_INSTALASI.md) - Instalasi & setup
  - [03_PANDUAN_SINTAKS.md](https://github.com/teguh02/Indonesiaku/blob/main/docs/03_PANDUAN_SINTAKS.md) - Referensi sintaks

- 📂 **[Contoh Program](https://github.com/teguh02/Indonesiaku/tree/main/examples)**
  - Dasar: `hello.idk`, `variabel.idk`, `fungsi.idk`
  - Menengah: `recursion_deep.idk`, `fibonacci.idk`, `state_management.idk`
  - Lanjut: `oop/` directory untuk OOP patterns

---

## 🐛 Known Issues & Limitations

- Debug printing (`DEBUG_TRACE_EXECUTION`) masih tersedia untuk developer (requires rebuild)
- REPL tidak menyimpan history antar session
- Error messages masih dalam bahasa Inggris (akan dilokal-isasi di versi mendatang)

---

## 🤝 Kontribusi

Kami sangat terbuka terhadap kontribusi! Silakan:

1. Fork repository
2. Buat branch fitur (`git checkout -b fitur-baru`)
3. Commit perubahan (`git commit -m "feat: deskripsi fitur"`)
4. Push ke branch (`git push origin fitur-baru`)
5. Buat Pull Request

Lihat [CONTRIBUTING.md](https://github.com/teguh02/Indonesiaku/blob/main/CONTRIBUTING.md) untuk detail lengkap.

---

## 📝 Lisensi

Proyek ini dilisensikan di bawah lisensi MIT - lihat [LICENSE](https://github.com/teguh02/Indonesiaku/blob/main/LICENSE) untuk detail.

---

## 👨‍💻 Pencipta

**Bahasa Pemrograman Indonesiaku** adalah proyek open-source yang dibuat dengan ❤️ untuk membawa pemrograman lebih dekat dengan Bahasa Indonesia.

Untuk pertanyaan, saran, atau laporan bug, silakan buka [GitHub Issues](https://github.com/teguh02/Indonesiaku/issues).

---

**Selamat menggunakan Indonesiaku! 🇮🇩💻**
