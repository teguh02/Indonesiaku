# 🐳 Bahasa Pemrograman Indonesiaku — Docker Image

[![Docker Pulls](https://img.shields.io/docker/pulls/teguh02/indonesiaku?style=flat-square&logo=docker)](https://hub.docker.com/r/teguh02/indonesiaku)
[![Docker Image Size](https://img.shields.io/docker/image-size/teguh02/indonesiaku/latest?style=flat-square&logo=docker)](https://hub.docker.com/r/teguh02/indonesiaku)
[![GitHub](https://img.shields.io/badge/GitHub-teguh02%2FIndonesiaku-blue?style=flat-square&logo=github)](https://github.com/teguh02/Indonesiaku)

**Indonesiaku** — bahasa pemrograman dengan sintaks Bahasa Indonesia, dibangun
sebagai bytecode VM dalam C. Image ini berisi interpreter `indk`, pustaka
standar (`pustaka/`), dan contoh program (`examples/`).

📖 **Dokumentasi & sintaks lengkap ada di GitHub:** https://github.com/teguh02/Indonesiaku

---

## ⚡ Quick Start

```bash
# Pull image
docker pull teguh02/indonesiaku:latest

# Cek versi
docker run --rm teguh02/indonesiaku:latest -v

# Jalankan contoh bawaan
docker run --rm teguh02/indonesiaku:latest examples/hello.idk

# Jalankan program sendiri (mount folder kerja)
echo 'cetak("Halo dari Docker!")' > hello.idk
docker run --rm -v "$(pwd):/kerja" teguh02/indonesiaku:latest /kerja/hello.idk

# REPL interaktif
docker run -it --rm teguh02/indonesiaku:latest indk
```

---

## 🔨 Build dari Sumber

```bash
git clone https://github.com/teguh02/Indonesiaku.git
cd Indonesiaku
docker build -f docker/images/Dockerfile -t teguh02/indonesiaku:latest .
```

---

## 🏗️ Spesifikasi

| Aspek | Nilai |
|-------|-------|
| Base image | Alpine Linux 3.20 (multi-stage build) |
| Ukuran | ~12.6 MB |
| Isi | biner `indk` + `pustaka/` + `examples/` |
| Versi bahasa | v0.4.0 |
| Lisensi | MIT |

---

## 🔗 Tautan

- 🐙 GitHub (kode + dokumentasi lengkap): https://github.com/teguh02/Indonesiaku
- 📖 Panduan sintaks: https://github.com/teguh02/Indonesiaku/tree/main/docs
- 💬 Issues: https://github.com/teguh02/Indonesiaku/issues

*Built with ❤️ — programmu dalam Bahasa Indonesia 🇮🇩*
