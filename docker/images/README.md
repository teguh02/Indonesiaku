# 🐳 Docker Image - Bahasa Pemrograman Indonesiaku

Dockerfile untuk membangun image Docker dari Bahasa Pemrograman Indonesiaku.

---

## 📋 Daftar Isi

- [Quick Start](#quick-start)
- [Persyaratan](#persyaratan)
- [Build dari Sumber](#build-dari-sumber)
- [Run Container](#run-container)
- [Contoh Penggunaan](#contoh-penggunaan)
- [Push ke Docker Hub](#push-ke-docker-hub)
- [Troubleshooting](#troubleshooting)

---

## ⚡ Quick Start

Gunakan image yang sudah jadi di Docker Hub:

```bash
# Pull image
docker pull teguh02/indonesiaku:latest

# Run container dengan version flag
docker run --rm teguh02/indonesiaku:latest -v

# Run interactive shell
docker run -it --rm teguh02/indonesiaku:latest /bin/bash

# Run program dari file (mount volume)
docker run -v $(pwd):/workspace teguh02/indonesiaku:latest /workspace/hello.idk
```

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

# Build image
docker build -f docker/images/Dockerfile -t indonesiaku:latest .

# Verifikasi
docker run --rm indonesiaku:latest -v
# Output: 0.1.1
```

### Opsi 2: Build dari Docker Image Folder

```bash
cd docker/images

# Copy src, include, Makefile, examples ke folder ini terlebih dahulu
# Kemudian:
docker build -t indonesiaku:latest .
```

### Options Build

Tambahkan build arguments untuk customize:

```bash
docker build -f docker/images/Dockerfile \
  --build-arg BASE_IMAGE=alpine:3.19 \
  -t indonesiaku:v0.1.1 .
```

---

## 🚀 Run Container

### Menjalankan Program

#### Mode 1: Pass File sebagai Argument

```bash
# File di current directory
echo 'tulis("Halo dari Docker!")' > hello.idk
docker run --rm -v $(pwd):/workspace indonesiaku:latest /workspace/hello.idk
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

/opt/indonesiaku # exit
```

#### Mode 3: Direct Command

```bash
# Run langsung tanpa REPL
docker run --rm indonesiaku:latest -v
```

---

## 📚 Contoh Penggunaan

### 1. Jalankan Hello World

```bash
docker run --rm -v $(pwd):/workspace indonesiaku:latest /workspace/hello.idk
```

File `hello.idk`:
```indonesiaku
tulis("Halo, Dunia!")
```

### 2. Jalankan dari Built-in Examples

```bash
docker run --rm indonesiaku:latest /opt/indonesiaku/examples/fibonacci.idk
```

### 3. REPL Mode (Interactive)

```bash
docker run -it --rm indonesiaku:latest /bin/bash
# Di dalam container:
$ indk
Indonesiaku 0.1.1 REPL
> variabel x = 5
> tulis(x * 2)
10
> keluar
```

### 4. Jalankan Script dengan Volume Mount

```bash
# Setup folder
mkdir -p ~/indonesiaku-scripts
cd ~/indonesiaku-scripts

# Buat program
cat > program.idk << 'EOF'
fungsi fibonacci(n) {
  jika (n <= 1) {
    kembalikan n
  }
  kembalikan fibonacci(n - 1) + fibonacci(n - 2)
}

untuk (i = 0; i < 10; i++) {
  tulis(fibonacci(i))
}
EOF

# Run di container
docker run --rm -v $(pwd):/app indonesiaku:latest /app/program.idk
```

---

## 🐳 Push ke Docker Hub

### Prerequisites

```bash
# Login ke Docker Hub
docker login

# Siapkan username Anda (ganti teguh02 dengan username Anda)
docker tag indonesiaku:latest <username>/indonesiaku:latest
```

### Push Image

```bash
# Push ke Docker Hub
docker push teguh02/indonesiaku:latest

# Verify push
docker pull teguh02/indonesiaku:latest
docker run --rm teguh02/indonesiaku:latest -v
```

### Push dengan Version Tag

```bash
# Tag dengan versi spesifik
docker tag indonesiaku:latest teguh02/indonesiaku:0.1.1

# Push keduanya
docker push teguh02/indonesiaku:latest
docker push teguh02/indonesiaku:0.1.1

# Verify
docker pull teguh02/indonesiaku:0.1.1
docker run --rm teguh02/indonesiaku:0.1.1 -v
```

---

## 🏗️ Dockerfile Details

### Base Image
- **Alpine Linux 3.20** - Lightweight (5 MB base image)
- **GCC 14.2.1** - Standard C compiler
- **Build tools**: build-base, git, make

### Build Process
1. Copy sumber code ke `/opt/indonesiaku`
2. Install dependencies dengan `apk add`
3. Compile dengan `make clean && make`
4. Copy binary ke `/usr/local/bin/indk`

### Image Size
- **Compressed**: ~1.9 MB
- **Uncompressed**: ~60 MB (setelah extract)

### Default Entry Point
```dockerfile
ENTRYPOINT ["indk"]
CMD ["--help"]
```

Artinya:
- `docker run indonesiaku` → `indk --help`
- `docker run indonesiaku -v` → `indk -v`
- `docker run indonesiaku /path/to/file.idk` → `indk /path/to/file.idk`

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

Untuk perbaikan Dockerfile atau improvements:

1. Fork repo
2. Edit `docker/images/Dockerfile`
3. Build dan test locally: `docker build -f docker/images/Dockerfile -t indonesiaku:test .`
4. Submit PR dengan deskripsi perubahan

---

**Nikmati Indonesiaku di Docker! 🇮🇩🐳**
