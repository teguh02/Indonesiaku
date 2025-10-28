# Dokumentasi Bahasa Pemrograman Indonesiaku

Selamat datang di dokumentasi lengkap **Bahasa Pemrograman Indonesiaku** - Bahasa pemrograman yang menggunakan syntax dan keyword dalam bahasa Indonesia.

---

## 📖 Daftar Isi

### 1. [Perkenalan](01_PERKENALAN.md)
Pengenalan umum tentang Bahasa Pemrograman Indonesiaku, sejarah, visi, dan misi.
- Apa itu Bahasa Pemrograman Indonesiaku?
- Mengapa dibuat?
- Fitur utama v0.1
- Roadmap pengembangan v0.2-v1.0

### 2. [Panduan Instalasi](02_INSTALASI.md)
Panduan lengkap instalasi Bahasa Pemrograman Indonesiaku di berbagai sistem operasi.
- **Windows** - MinGW installation & setup
- **Linux** - GCC installation & setup (Ubuntu, Fedora, Arch)
- **macOS** - Xcode Command Line Tools setup
- Verifikasi instalasi & troubleshooting
- Build dari source

### 3. [Panduan Sintaks](03_PANDUAN_SINTAKS.md)
Panduan lengkap penggunaan syntax dan grammar Bahasa Pemrograman Indonesiaku dengan contoh-contoh praktis.
- Tipe data (numbers, strings, booleans, null)
- Variabel & scope
- Operator (aritmatika, perbandingan, logika)
- Percabangan (if/else/elif)
- Perulangan (while loops)
- Fungsi & rekursi
- Best practices & patterns

### 4. [Panduan Kontribusi](04_KONTRIBUSI.md)
Panduan untuk berkontribusi dalam pengembangan Bahasa Pemrograman Indonesiaku.
- Setup development environment
- Struktur project
- Jenis-jenis kontribusi
- Git workflow & commit messages
- Code style guidelines
- Testing & quality assurance
- Pull request process

### 5. [Tentang Creator](05_CREATOR.md)
Informasi tentang pembuat dan maintainer Bahasa Pemrograman Indonesiaku.
- Profil creator: Teguh Rijanandi
- Motivasi & visi proyek
- Background teknis
- Project journey & achievements
- Philosophy on contribution
- Kontak & social media

### 6. [Penutup & Resources](06_PENUTUP.md)
Kesimpulan, tips, dan informasi tambahan.
- Key takeaways untuk berbagai level
- FAQ komprehensif
- Troubleshooting guide
- Resources & external links
- Next steps
- Support & feedback channels

---

## 🚀 Memulai Cepat

### Instalasi
```bash
# Clone repository
git clone <repository-url>

# Build
gcc -Wall -Wextra -std=c11 -O2 -Isrc -o indk.exe src/*.c -lm

# Run
./indk.exe example.idk
```

### Hello World
```indonesiaku
cetak("Halo, dunia!")
```

### Contoh Program Sederhana
```indonesiaku
# Variabel dan operasi
x = 10
y = 20
jumlah = x + y

cetak("Hasil penjumlahan:", jumlah)
```

---

## 📋 Spesifikasi Bahasa

| Aspek | Keterangan |
|-------|-----------|
| **Versi** | v0.1.0 |
| **Tipe** | Interpreted bytecode VM |
| **Syntax** | C-style dengan keywords Indonesia |
| **Tipe Data** | Number, Boolean, String, Null |
| **Platform** | Windows, Linux, macOS |
| **License** | Lihat LICENSE |

---

## 📚 Dokumentasi Tambahan

Selain panduan di atas, project ini juga menyediakan:

- **SYNTAX_REFERENCE.md** - Referensi syntax lengkap
- **QUICKSTART.md** - Panduan cepat memulai
- **CHANGELOG.md** - Riwayat perubahan
- **OOP_TESTING_GUIDE.md** - Panduan OOP patterns
- **WHY_OOP_TEST_FAILS.md** - Penjelasan limitasi v0.1

---

## 🔗 Links Penting

- **Repository**: https://github.com/teguh02/Indonesiaku
- **Issues**: Laporkan bug melalui GitHub Issues
- **Discussions**: Tanya jawab di GitHub Discussions

---

## 📞 Hubungi Kami

- **Email**: teguh.rijanandi@example.com
- **GitHub**: [@teguh02](https://github.com/teguh02)
- **Twitter**: [@indonesiakudev](https://twitter.com/indonesiakudev)

---

## 📄 License

Bahasa Pemrograman Indonesiaku dirilis di bawah **MIT License**. Silakan lihat file [LICENSE](../LICENSE) untuk detail lengkap.

---

## 🙏 Terima Kasih

Terima kasih telah menggunakan Bahasa Pemrograman Indonesiaku. Kami sangat menghargai feedback, saran, dan kontribusi Anda!

---

**Terakhir diperbarui**: 28 Oktober 2025  
**Maintainer**: Teguh Rijanandi  
**Version**: 1.0
