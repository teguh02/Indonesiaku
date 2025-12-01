# 🇮🇩 Bahasa Pemrograman Indonesiaku

**Indonesiaku** adalah bahasa pemrograman dengan sintaks Bahasa Indonesia yang terinspirasi dari Python. Bahasa ini menggunakan Virtual Machine (VM) berbasis bytecode yang diimplementasikan dalam bahasa C.

---

## 📚 DOKUMENTASI LENGKAP

**Dokumentasi terstruktur tersedia di folder `docs/`:**

| Dokumen | Deskripsi |
|---------|-----------|
| **[docs/README.md](docs/README.md)** | 📖 Hub utama dokumentasi |
| **[docs/01_PERKENALAN.md](docs/01_PERKENALAN.md)** | 🎯 Visi, misi, fitur, roadmap |
| **[docs/02_INSTALASI.md](docs/02_INSTALASI.md)** | 🔧 Instalasi untuk Windows, Linux, macOS |
| **[docs/03_PANDUAN_SINTAKS.md](docs/03_PANDUAN_SINTAKS.md)** | 📝 Panduan syntax lengkap dengan contoh |
| **[docs/04_KONTRIBUSI.md](docs/04_KONTRIBUSI.md)** | 🤝 Cara berkontribusi & setup dev |
| **[docs/05_CREATOR.md](docs/05_CREATOR.md)** | 👤 Profile & motivasi creator |
| **[docs/06_PENUTUP.md](docs/06_PENUTUP.md)** | ✅ Kesimpulan, FAQ, resources |

---

## 📋 Daftar Isi

- [Tentang Proyek](#tentang-proyek)
- [Fitur](#fitur)
- [Instalasi](#instalasi)
- [Cara Menggunakan](#cara-menggunakan)
- [Sintaks Dasar](#sintaks-dasar)
- [Contoh Program](#contoh-program)
- [Kata Kunci](#kata-kunci)
- [Roadmap](#roadmap)
- [Kontribusi](#kontribusi)

## 🎯 Tentang Proyek

Proyek ini bertujuan untuk membuat bahasa pemrograman yang:
- **Mudah dipahami** oleh penutur Bahasa Indonesia
- **Mirip Python** dalam struktur dan filosofi (indentasi, titik dua, dll)
- **Menggunakan kata kunci Bahasa Indonesia** untuk semua perintah
- **Diimplementasikan dengan C** untuk performa yang baik

## ✨ Fitur

### Saat Ini (v0.1)
- ✅ Ekspresi aritmatika (+, -, *, /, %, **)
- ✅ Variabel dinamis
- ✅ Tipe data: angka (number), boolean (benar/salah), kosong (null), string
- ✅ Percabangan: `jika`, `jika_lain`, `selain`
- ✅ Perulangan: `selagi` (while)
- ✅ Fungsi dengan parameter dan nilai kembalian
- ✅ Operator logika: `dan`, `atau`, `tidak`
- ✅ Operator perbandingan: `==`, `!=`, `<`, `<=`, `>`, `>=`
- ✅ Fungsi cetak (print)
- ✅ Rekursi
- ✅ REPL interaktif

### Akan Datang (v0.2+)
- 🔲 Perulangan `untuk...dalam` (for...in) dengan list
- 🔲 List, Dictionary, Set operations
- 🔲 Kelas dan Object-Oriented Programming
- 🔲 Import modul
- 🔲 Garbage Collection
- 🔲 Error handling yang lebih baik
- 🔲 Standard library

## 🔧 Instalasi

### Prasyarat
- GCC atau Clang compiler
- Make (opsional, untuk menggunakan Makefile)
- Git (untuk clone repository)

### Langkah Instalasi

#### Windows (PowerShell/CMD)
```bash
# Clone repository (jika dari Git)
git clone <repository-url>
cd Indonesiaku

# Compile dengan GCC
gcc -Wall -Wextra -std=c11 -O2 -Isrc -o indk.exe src/main.c src/chunk.c src/compiler.c src/debug.c src/memory.c src/object.c src/scanner.c src/table.c src/value.c src/vm.c -lm

# Atau gunakan Make (jika tersedia)
make
```

#### Linux/Mac
```bash
# Clone repository
git clone <repository-url>
cd Indonesiaku

# Compile dengan GCC
gcc -Wall -Wextra -std=c11 -O2 -Isrc -o indk src/main.c src/chunk.c src/compiler.c src/debug.c src/memory.c src/object.c src/scanner.c src/table.c src/value.c src/vm.c -lm

# Atau gunakan Make
make
```

### Menggunakan VS Code

1. Buka folder proyek di VS Code
2. Tekan `Ctrl+Shift+B` untuk build
3. Pilih task **"Build Indonesiaku (C VM)"**
4. Untuk menjalankan file .idk yang sedang dibuka, pilih task **"Run Indonesiaku (file aktif)"**

## 🚀 Cara Menggunakan

### Menjalankan File .idk
```bash
# Windows
indk.exe examples/hello.idk

# Linux/Mac
./indk examples/hello.idk
```

### Mode REPL (Interactive)
```bash
# Windows
indk.exe

# Linux/Mac
./indk
```

Dalam mode REPL, Anda bisa menulis kode langsung:
```
> cetak("Halo!")
Halo!
> x = 10
> cetak(x * 2)
20
```

## 📖 Sintaks Dasar

### Komentar
```python
# Ini adalah komentar
cetak("Halo")  # Komentar di akhir baris
```

### Variabel
```python
nama = "Budi"
umur = 25
tinggi = 170.5
aktif = benar
data = kosong
```

### Cetak (Print)
```python
cetak("Halo, dunia!")
cetak("Nama:", nama, "Umur:", umur)
```

### Operasi Aritmatika
```python
x = 10
y = 3

jumlah = x + y      # 13
selisih = x - y     # 7
kali = x * y        # 30
bagi = x / y        # 3.333...
sisa = x % y        # 1
pangkat = x ** 2    # 100
```

### Percabangan
```python
nilai = 85

jika nilai >= 90:
    cetak("Nilai A")
jika_lain nilai >= 80:
    cetak("Nilai B")
jika_lain nilai >= 70:
    cetak("Nilai C")
selain:
    cetak("Nilai D")
```

### Operator Logika
```python
umur = 20
punya_sim = benar

jika umur >= 17 dan punya_sim:
    cetak("Boleh menyetir")
selain:
    cetak("Tidak boleh menyetir")

jika umur < 13 atau umur > 65:
    cetak("Diskon khusus")

jika tidak punya_sim:
    cetak("Harus punya SIM dulu")
```

### Perulangan (While)
```python
# Hitung mundur
i = 5
selagi i > 0:
    cetak(i)
    i = i - 1
cetak("Selesai!")

# Jumlah 1 sampai 10
i = 1
jumlah = 0
selagi i <= 10:
    jumlah = jumlah + i
    i = i + 1
cetak("Jumlah:", jumlah)
```

### Fungsi
```python
# Fungsi tanpa parameter
fungsi sapa():
    cetak("Halo!")

sapa()

# Fungsi dengan parameter
fungsi sapa_nama(nama):
    cetak("Halo,", nama)

sapa_nama("Budi")

# Fungsi dengan return
fungsi tambah(a, b):
    kembalikan a + b

hasil = tambah(5, 3)
cetak("Hasil:", hasil)

# Fungsi rekursif
fungsi faktorial(n):
    jika n <= 1:
        kembalikan 1
    selain:
        kembalikan n * faktorial(n - 1)

cetak("5! =", faktorial(5))  # Output: 120
```

## 🎨 Contoh Program

### Hello World
```python
cetak("Halo, dunia!")
cetak("Selamat datang di Bahasa Indonesiaku!")
```

### Fibonacci
```python
fungsi fibonacci(n):
    jika n <= 1:
        kembalikan n
    selain:
        kembalikan fibonacci(n - 1) + fibonacci(n - 2)

i = 0
selagi i < 10:
    cetak("Fibonacci(", i, ") =", fibonacci(i))
    i = i + 1
```

### Cek Bilangan Prima
```python
fungsi adalah_prima(n):
    jika n <= 1:
        kembalikan salah
    
    jika n == 2:
        kembalikan benar
    
    jika n % 2 == 0:
        kembalikan salah
    
    i = 3
    selagi i * i <= n:
        jika n % i == 0:
            kembalikan salah
        i = i + 2
    
    kembalikan benar

angka = 1
selagi angka <= 20:
    jika adalah_prima(angka):
        cetak(angka, "adalah bilangan prima")
    angka = angka + 1
```

Lebih banyak contoh tersedia di folder `examples/`.

## 🔤 Kata Kunci

### Tabel Pemetaan (Python → Indonesiaku)

| Python | Indonesiaku | Keterangan |
|--------|-------------|------------|
| `True` | `benar` | Boolean benar |
| `False` | `salah` | Boolean salah |
| `None` | `kosong` | Null/None |
| `and` | `dan` | Operator logika AND |
| `or` | `atau` | Operator logika OR |
| `not` | `tidak` | Operator logika NOT |
| `if` | `jika` | Percabangan if |
| `elif` | `jika_lain` | Percabangan elif |
| `else` | `selain` | Percabangan else |
| `while` | `selagi` | Perulangan while |
| `for` | `untuk` | Perulangan for |
| `in` | `dalam` | Operator in |
| `def` | `fungsi` | Definisi fungsi |
| `return` | `kembalikan` | Return value |
| `print` | `cetak` | Print/output |
| `class` | `kelas` | Definisi kelas (v0.3+) |
| `self` | `diri` | Self reference (v0.3+) |

### Kata Kunci Tambahan (Untuk Versi Mendatang)
- `break` → `hentikan`
- `continue` → `lanjut`
- `pass` → `lewati`
- `import` → `impor`
- `from` → `dari`
- `as` → `sebagai`
- `with` → `dengan`
- `try` → `coba`
- `except` → `kecuali`
- `finally` → `akhirnya`
- `raise` → `naikkan`

## 🗺️ Roadmap

### v0.1 (Saat Ini) ✅
- Ekspresi dan variabel
- Percabangan (jika/jika_lain/selain)
- Perulangan (selagi)
- Fungsi dengan parameter dan return
- Operator aritmatika dan logika
- REPL

### v0.2 (Dalam Pengembangan)
- Perulangan `untuk...dalam`
- List operations
- `hentikan` (break) dan `lanjut` (continue)
- Dictionary dan Set (dasar)
- Operator assignment gabungan (+=, -=, dll)

### v0.3 (Rencana)
- Object-Oriented Programming (kelas, objek)
- String methods
- Garbage Collection
- Error messages yang lebih baik

### v0.4 (Rencana)
- Import modul
- Standard library
- File I/O
- Exception handling

## 🏗️ Arsitektur

Proyek ini menggunakan arsitektur bytecode VM:

```
Kode Sumber (.idk)
    ↓
Scanner/Lexer (Token)
    ↓
Parser (AST)
    ↓
Compiler (Bytecode)
    ↓
Virtual Machine (Eksekusi)
```

### Struktur Proyek
```
Indonesiaku/
├── src/
│   ├── main.c          # Entry point
│   ├── scanner.c/.h    # Lexer (tokenizer)
│   ├── compiler.c/.h   # Parser & code generator
│   ├── chunk.c/.h      # Bytecode container
│   ├── vm.c/.h         # Virtual machine
│   ├── value.c/.h      # Value representation
│   ├── object.c/.h     # Object system
│   ├── table.c/.h      # Hash table
│   ├── memory.c/.h     # Memory management
│   └── debug.c/.h      # Debugging tools
├── examples/           # Contoh program .idk
├── .vscode/           # VS Code tasks
└── Makefile           # Build system
```

## 🤝 Kontribusi

Kontribusi sangat diterima! Silakan baca panduan lengkap di **[docs/04_KONTRIBUSI.md](docs/04_KONTRIBUSI.md)**.

Beberapa area yang bisa dikontribusikan:

1. **Fitur Baru**: Implementasi fitur yang ada di roadmap
2. **Bug Fixes**: Perbaikan bug yang ditemukan
3. **Dokumentasi**: Perbaikan atau penambahan dokumentasi
4. **Contoh Program**: Tambahkan contoh program .idk yang menarik
5. **Testing**: Buat test cases dan laporan bug
6. **Optimasi**: Perbaikan performa

### Quick Start untuk Kontributor

```bash
# 1. Clone repository
git clone https://github.com/teguhriyan/Indonesiaku.git
cd Indonesiaku

# 2. Build
make

# 3. Run tests
powershell -ExecutionPolicy Bypass -File tests/test_all.ps1

# 4. Create feature branch
git checkout -b feature/nama-fitur

# 5. Buat perubahan, commit, dan submit PR
```

Untuk detail lengkap, baca **[docs/04_KONTRIBUSI.md](docs/04_KONTRIBUSI.md)**.

## 📝 Lisensi

Proyek ini dibuat untuk tujuan edukasi dan pembelajaran bahasa pemrograman.

## 👨‍💻 Penulis

Proyek Indonesiaku - Bahasa Pemrograman dengan Sintaks Bahasa Indonesia

---

**Selamat Coding dengan Bahasa Indonesia! 🇮🇩**

Untuk pertanyaan, saran, atau laporan bug, silakan buat issue di repository ini.
