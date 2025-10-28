# 🐳 Bahasa Pemrograman Indonesiaku - Docker Image

[![Docker Pulls](https://img.shields.io/docker/pulls/teguh02/indonesiaku?style=flat-square&logo=docker)](https://hub.docker.com/r/teguh02/indonesiaku)
[![Docker Image Size](https://img.shields.io/docker/image-size/teguh02/indonesiaku/latest?style=flat-square&logo=docker)](https://hub.docker.com/r/teguh02/indonesiaku)
[![GitHub](https://img.shields.io/badge/GitHub-teguh02%2FIndonesiaku-blue?style=flat-square&logo=github)](https://github.com/teguh02/Indonesiaku)
[![License](https://img.shields.io/badge/License-MIT-green?style=flat-square)](https://github.com/teguh02/Indonesiaku/blob/main/LICENSE)

Docker image untuk **Bahasa Pemrograman Indonesiaku** - bahasa pemrograman pertama di dunia yang menggunakan Bahasa Indonesia sebagai syntax utamanya!

🇮🇩 **Programmu dalam Bahasa Indonesia** | 🚀 **Ringan & Cepat** | 📦 **Alpine-based**

---

## 📋 Daftar Isi

- [Quick Start](#-quick-start)
- [Persyaratan](#-persyaratan)
- [Build dari Sumber](#-build-dari-sumber)
- [Run Container](#-run-container)
- [Contoh Penggunaan](#-contoh-penggunaan)
- [Spesifikasi Image](#-spesifikasi-image)
- [Push ke Docker Hub](#-push-ke-docker-hub)
- [Troubleshooting](#-troubleshooting)
- [Lisensi](#-lisensi)

---

## ⚡ Quick Start

Gunakan image yang sudah jadi di Docker Hub:

```bash
# Pull image terbaru
docker pull teguh02/indonesiaku:latest

# Verifikasi versi
docker run --rm teguh02/indonesiaku:latest -v
# Output: 0.1.1

# Run interactive shell
docker run -it --rm teguh02/indonesiaku:latest /bin/bash

# Run program dari file
docker run -v $(pwd):/workspace --rm teguh02/indonesiaku:latest /workspace/hello.idk
```

**Docker Hub Repository**: https://hub.docker.com/r/teguh02/indonesiaku

---

## ✅ Persyaratan

### Untuk Build Lokal
- Docker Desktop terinstal (minimal v20.10)
- ~500 MB disk space untuk image
- Koneksi internet untuk download Alpine base image

### Untuk Push ke Docker Hub
- Docker CLI login: `docker login`
- Account Docker Hub: https://hub.docker.com

---

## 🔨 Build dari Sumber

### Opsi 1: Build dari Root Repo (Recommended)

```bash
# Clone repo
git clone https://github.com/teguh02/Indonesiaku.git
cd Indonesiaku

# Build image dengan tag
docker build -f docker/images/Dockerfile -t indonesiaku:latest .

# Verifikasi build
docker run --rm indonesiaku:latest -v
# Output: 0.1.1
```

### Opsi 2: Build dengan Version Tag

```bash
# Build dengan spesifik version
docker build -f docker/images/Dockerfile \
  -t indonesiaku:0.1.1 \
  -t indonesiaku:latest .

# Verify
docker run --rm indonesiaku:0.1.1 -v
```

### Opsi 3: Build dengan Custom Base Image

```bash
# Build menggunakan Alpine versi berbeda (optional)
docker build -f docker/images/Dockerfile \
  --build-arg BASE_IMAGE=alpine:3.19 \
  -t indonesiaku:custom .
```

### Build Tips

```bash
# Build tanpa cache (force rebuild)
docker build --no-cache -f docker/images/Dockerfile -t indonesiaku:latest .

# Build dengan build progress verbose
docker build --progress=plain -f docker/images/Dockerfile -t indonesiaku:latest .

# Check build history
docker history indonesiaku:latest
```

---

## 🚀 Run Container

### Menjalankan Program

#### Mode 1: Pass File sebagai Argument

```bash
# File di current directory
echo 'tulis("Halo dari Docker!")' > hello.idk

# Run di container dengan volume mount
docker run --rm -v $(pwd):/workspace indonesiaku:latest /workspace/hello.idk
# Output: Halo dari Docker!
```

#### Mode 2: Interactive Shell

```bash
# Masuk ke bash di dalam container
docker run -it --rm indonesiaku:latest /bin/bash

# Di dalam container:
/opt/indonesiaku # indk -v
0.1.1

/opt/indonesiaku # indk examples/hello.idk
Halo, Dunia!

/opt/indonesiaku # indk -h
# Tampilkan help message

/opt/indonesiaku # exit
```

#### Mode 3: Direct Command

```bash
# Run langsung tanpa interactive shell
docker run --rm indonesiaku:latest -v
# Output: 0.1.1

# Run dengan argument lain
docker run --rm indonesiaku:latest -h
```

#### Mode 4: Environment Variables (Advanced)

```bash
# Jalankan dengan custom working directory
docker run -it --rm \
  -e LANG=id_ID.UTF-8 \
  -v $(pwd):/workspace \
  -w /workspace \
  indonesiaku:latest /bin/bash
```

---

## 📚 Contoh Penggunaan

### 1. Hello World

```bash
docker run --rm -v $(pwd):/workspace indonesiaku:latest /workspace/hello.idk
```

File `hello.idk`:
```indonesiaku
tulis("Halo, Dunia!")
```

### 2. Jalankan Built-in Examples

```bash
# Fibonacci
docker run --rm indonesiaku:latest /opt/indonesiaku/examples/fibonacci.idk

# Variabel & Expression
docker run --rm indonesiaku:latest /opt/indonesiaku/examples/variabel.idk

# Control Flow
docker run --rm indonesiaku:latest /opt/indonesiaku/examples/percabangan.idk
```

### 3. Interactive REPL Mode

```bash
docker run -it --rm indonesiaku:latest /bin/bash

# Di dalam container:
$ indk
> variabel x = 5
> tulis(x * 2)
10
> keluar
```

### 4. Jalankan Script Kompleks dengan Volume Mount

```bash
# Setup working directory
mkdir -p ~/indonesiaku-dev
cd ~/indonesiaku-dev

# Buat program fibonacci
cat > fibonacci.idk << 'EOF'
fungsi fib(n) {
  jika (n <= 1) {
    kembalikan n
  }
  kembalikan fib(n - 1) + fib(n - 2)
}

untuk (i = 0; i < 10; i++) {
  tulis(fib(i))
}
EOF

# Jalankan di container
docker run --rm -v $(pwd):/app indonesiaku:latest /app/fibonacci.idk
# Output: 0 1 1 2 3 5 8 13 21 34
```

### 5. Compile Program Lokal (Save Output)

```bash
# Mount directory untuk hasil
docker run --rm \
  -v $(pwd):/app \
  -w /app \
  indonesiaku:latest \
  /bin/bash -c "indk program.idk > output.txt 2>&1"

# Check hasil
cat output.txt
```

---

## 🐳 Push ke Docker Hub

### Prerequisites

```bash
# 1. Pastikan image sudah dibuild
docker images | grep indonesiaku

# 2. Login ke Docker Hub
docker login
# Masukkan username & password Docker Hub

# 3. Siapkan username Anda
# (Ganti 'teguh02' dengan username Docker Hub Anda)
```

### Push Image

```bash
# Tag image untuk Docker Hub
docker tag indonesiaku:latest <username>/indonesiaku:latest

# Push ke Docker Hub
docker push <username>/indonesiaku:latest

# Verify push berhasil
docker pull <username>/indonesiaku:latest
docker run --rm <username>/indonesiaku:latest -v
# Output: 0.1.1
```

### Push dengan Version Tags

```bash
# Tag dengan versi spesifik
docker tag indonesiaku:latest teguh02/indonesiaku:0.1.1
docker tag indonesiaku:latest teguh02/indonesiaku:latest

# Push keduanya
docker push teguh02/indonesiaku:0.1.1
docker push teguh02/indonesiaku:latest

# Verify
docker pull teguh02/indonesiaku:0.1.1
docker run --rm teguh02/indonesiaku:0.1.1 -v
```

### Troubleshoot Push Issues

```bash
# Error: denied permission
# Solution: Pastikan sudah docker login dan username benar

# Check login status
docker info | grep Username

# Re-login jika diperlukan
docker logout
docker login

# Push dengan verbose output
docker push <username>/indonesiaku:latest --verbose
```

---

## 🏗️ Spesifikasi Image

| Aspek | Nilai |
|-------|-------|
| **Base Image** | Alpine Linux 3.20 |
| **Compiler** | GCC 14.2.1 |
| **Language Version** | v0.1.1 |
| **Size (Compressed)** | ~1.9 MB |
| **Size (Uncompressed)** | ~60 MB |
| **Maintainer** | Teguh Rijanandi |
| **Repository** | https://hub.docker.com/r/teguh02/indonesiaku |
| **GitHub** | https://github.com/teguh02/Indonesiaku |

### Build Details

```dockerfile
FROM alpine:3.20

LABEL maintainer="Teguh Rijanandi <teguhrijanandi02@gmail.com>"
LABEL description="Docker image for Bahasa Pemrograman Indonesiaku"
LABEL version="0.1.1"

# Installation
RUN apk add --no-cache build-base bash git make

# Build dari source
WORKDIR /opt/indonesiaku
COPY src ./src
COPY include ./include
COPY Makefile .
RUN make clean && make

# Entry point
ENTRYPOINT ["indk"]
CMD ["--help"]
```

---

## 🐛 Troubleshooting

### Error: "failed to solve with frontend dockerfile.v0"
- **Cause**: Dockerfile syntax error atau path tidak valid
- **Solution**: Pastikan build dijalankan dari root repo dengan `-f docker/images/Dockerfile`

### Error: "make: *** No targets specified"
- **Cause**: Makefile tidak ditemukan dalam build context
- **Solution**: Build dari root repo, bukan dari `docker/images`

### Container exit immediately
- **Cause**: CMD syntax salah atau file tidak ditemukan
- **Solution**: Jalankan `docker run -it indonesiaku:latest /bin/bash` untuk debug

### Docker Hub push permission denied
- **Cause**: Belum login atau username salah
- **Solution**: 
  ```bash
  docker logout
  docker login
  docker push <username>/indonesiaku:latest
  ```

### Image too large?
- **Current size**: ~1.9 MB (compressed)
- **Could reduce by**: Removing examples folder dari image
- **Edit Dockerfile**: Comment out `COPY ../../examples`

---

## 📖 Dokumentasi Tambahan

- [Dockerfile Best Practices](https://docs.docker.com/develop/develop-images/dockerfile_best-practices/)
- [Alpine Linux Packages](https://pkgs.alpinelinux.org/)
- [Docker CLI Reference](https://docs.docker.com/engine/reference/commandline/cli/)

---

## 🤝 Kontribusi

Kami sangat terbuka terhadap kontribusi! Silakan:

1. **Fork** repository: https://github.com/teguh02/Indonesiaku
2. **Edit** `docker/images/Dockerfile`
3. **Build & test** locally: `docker build -f docker/images/Dockerfile -t indonesiaku:test .`
4. **Submit** Pull Request dengan deskripsi perubahan

### Development Tips

```bash
# Build tanpa cache
docker build --no-cache -f docker/images/Dockerfile -t indonesiaku:dev .

# Debug di dalam container
docker run -it --rm indonesiaku:dev /bin/sh

# Check image layers
docker history indonesiaku:latest

# Remove image
docker rmi indonesiaku:latest
```

---

## 📜 Lisensi

Proyek ini dilisensikan di bawah **MIT License** - lihat [LICENSE](https://github.com/teguh02/Indonesiaku/blob/main/LICENSE) untuk detail lengkap.

---

## 🔗 Links Penting

| Link | Deskripsi |
|------|-----------|
| 🐳 [Docker Hub](https://hub.docker.com/r/teguh02/indonesiaku) | Repository image di Docker Hub |
| 🐙 [GitHub](https://github.com/teguh02/Indonesiaku) | Source code & issue tracker |
| 📖 [Dokumentasi](https://github.com/teguh02/Indonesiaku/tree/main/docs) | Panduan lengkap bahasa |
| 💬 [Issues](https://github.com/teguh02/Indonesiaku/issues) | Report bug atau request fitur |
| 🌟 [Releases](https://github.com/teguh02/Indonesiaku/releases) | Download installer & binary |

---

## 📞 Support & Questions

Untuk bantuan, pertanyaan, atau laporan bug:

1. **GitHub Issues**: https://github.com/teguh02/Indonesiaku/issues
2. **Discussions**: https://github.com/teguh02/Indonesiaku/discussions (jika tersedia)
3. **Email**: teguhrijanandi02@gmail.com

---

## 🎉 Mulai Programming dalam Bahasa Indonesia!

```bash
# Quick example
docker run -it --rm teguh02/indonesiaku /bin/bash

# Di dalam container:
$ cat > hello.idk << 'EOF'
tulis("Selamat datang di Indonesiaku!")
variabel x = 5
variabel y = 10
tulis(x + y)
EOF

$ indk hello.idk
Selamat datang di Indonesiaku!
15
```

---

**Nikmati programmu dalam Bahasa Indonesia! 🇮🇩�**

*Built with ❤️ by the Indonesiaku community*
