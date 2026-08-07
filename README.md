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
- **Mirip Python** dalam struktur dan filosofi (dinamis, ekspresif); blok kode menggunakan kurung kurawal `{ }`
- **Menggunakan kata kunci Bahasa Indonesia** untuk semua perintah
- **Diimplementasikan dengan C** untuk performa yang baik

## ✨ Fitur

### Saat Ini (v0.2)
- ✅ Ekspresi aritmatika (+, -, *, /, %, **) & assignment gabungan (+=, -=, dll)
- ✅ Tipe data: angka, boolean (`benar`/`salah`), `kosong`, string
- ✅ Percabangan `jika`/`jika_lain`/`selain`, perulangan `selagi`
- ✅ Fungsi, rekursi, **closure** (dengan `variabel` untuk lokal)
- ✅ **List** `[..]` + indexing, **Kamus** via `kamus()` + indexing string
- ✅ **`untuk ... dalam`** (for-in), **`hentikan`/`lanjut`** (break/continue)
- ✅ **Kelas & OOP**: `kelas`, `init`, `diri`, pewarisan (`<`), `super`
- ✅ **Penanganan kesalahan**: `naikkan` + `coba`/`kecuali`
- ✅ **Sistem modul**: `impor "file.idk"` (namespace global bersama, di-cache)
- ✅ **Garbage Collector** mark-sweep
- ✅ REPL interaktif, operator logika `dan`/`atau`/`tidak`

### Akan Datang
- 🔲 `akhirnya` (finally) untuk `coba`/`kecuali`
- 🔲 Literal kamus `{ }`, Set
- 🔲 String methods lanjutan & file I/O
- 🔲 Standard library yang lebih lengkap

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
gcc -Wall -Wextra -std=c11 -O2 -Isrc -o indk.exe src/main.c src/chunk.c src/compiler.c src/debug.c src/memory.c src/native.c src/object.c src/scanner.c src/table.c src/value.c src/vm.c -lm

# Atau gunakan Make (jika tersedia)
make
```

#### Linux/Mac
```bash
# Clone repository
git clone <repository-url>
cd Indonesiaku

# Compile dengan GCC
gcc -Wall -Wextra -std=c11 -O2 -Isrc -o indk src/main.c src/chunk.c src/compiler.c src/debug.c src/memory.c src/native.c src/object.c src/scanner.c src/table.c src/value.c src/vm.c -lm

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

jika nilai >= 90 {
    cetak("Nilai A")
} jika_lain nilai >= 80 {
    cetak("Nilai B")
} jika_lain nilai >= 70 {
    cetak("Nilai C")
} selain {
    cetak("Nilai D")
}
```

### Operator Logika
```python
umur = 20
punya_sim = benar

jika umur >= 17 dan punya_sim {
    cetak("Boleh menyetir")
} selain {
    cetak("Tidak boleh menyetir")
}

jika umur < 13 atau umur > 65 {
    cetak("Diskon khusus")
}

jika tidak punya_sim {
    cetak("Harus punya SIM dulu")
}
```

### Perulangan (While)
```python
# Hitung mundur
i = 5
selagi i > 0 {
    cetak(i)
    i = i - 1
}
cetak("Selesai!")

# Jumlah 1 sampai 10
i = 1
jumlah = 0
selagi i <= 10 {
    jumlah = jumlah + i
    i = i + 1
}
cetak("Jumlah:", jumlah)
```

### Fungsi
```python
# Fungsi tanpa parameter
fungsi sapa() {
    cetak("Halo!")
}

sapa()

# Fungsi dengan parameter
fungsi sapa_nama(nama) {
    cetak("Halo,", nama)
}

sapa_nama("Budi")

# Fungsi dengan return
fungsi tambah(a, b) {
    kembalikan a + b
}

hasil = tambah(5, 3)
cetak("Hasil:", hasil)

# Fungsi rekursif
fungsi faktorial(n) {
    jika n <= 1 {
        kembalikan 1
    } selain {
        kembalikan n * faktorial(n - 1)
    }
}

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
fungsi fibonacci(n) {
    jika n <= 1 {
        kembalikan n
    } selain {
        kembalikan fibonacci(n - 1) + fibonacci(n - 2)
    }
}

i = 0
selagi i < 10 {
    cetak("Fibonacci(", i, ") =", fibonacci(i))
    i = i + 1
}
```

### Cek Bilangan Prima
```python
fungsi adalah_prima(n) {
    jika n <= 1 {
        kembalikan salah
    }

    jika n == 2 {
        kembalikan benar
    }

    jika n % 2 == 0 {
        kembalikan salah
    }

    i = 3
    selagi i * i <= n {
        jika n % i == 0 {
            kembalikan salah
        }
        i = i + 2
    }

    kembalikan benar
}

angka = 1
selagi angka <= 20 {
    jika adalah_prima(angka) {
        cetak(angka, "adalah bilangan prima")
    }
    angka = angka + 1
}
```

Lebih banyak contoh tersedia di folder `examples/`.

## 🚀 Fitur Lanjutan

### List & Kamus
```python
angka = [10, 20, 30]
cetak(angka[0])          # 10
tambah(angka, 40)        # [10, 20, 30, 40]
cetak(panjang(angka))    # 4

orang = kamus()
orang["nama"] = "Budi"
cetak(orang["nama"])     # Budi
cetak(punya(orang, "umur"))  # salah
```

### Perulangan untuk...dalam, hentikan, lanjut
```python
untuk x dalam [1, 2, 3, 4, 5] {
    jika x == 4 { hentikan }   # break
    jika x == 2 { lanjut }     # continue
    cetak(x)
}
```

### Closure
```python
fungsi buat_pencacah() {
    variabel n = 0             # 'variabel' wajib agar bisa ditangkap closure
    fungsi naik() {
        n = n + 1
        kembalikan n
    }
    kembalikan naik
}
c = buat_pencacah()
cetak(c())  # 1
cetak(c())  # 2
```

### Kelas & Pewarisan
```python
kelas Hewan {
    init(nama) { diri.nama = nama }
    bersuara() { cetak(diri.nama, "bersuara") }
}
kelas Anjing < Hewan {
    bersuara() {
        super.bersuara()
        cetak(diri.nama, "menggonggong")
    }
}
Anjing("Rex").bersuara()
```

### Penanganan Kesalahan
```python
coba {
    naikkan "terjadi kesalahan"
} kecuali e {
    cetak("tertangkap:", e)
}
```

### Impor Modul
```python
impor "examples/pustaka_matematika.idk"
cetak(luas_lingkaran(2))
```

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

## 🧰 Fungsi Bawaan (Built-in)

Fungsi berikut tersedia langsung tanpa impor (didefinisikan di `src/native.c`):

### Matematika
| Fungsi | Deskripsi |
|--------|-----------|
| `min(a, b, ...)` | Nilai terkecil (min. 2 argumen) |
| `max(a, b, ...)` | Nilai terbesar (min. 2 argumen) |
| `abs(x)` | Nilai absolut |
| `akar(x)` | Akar kuadrat (x tidak boleh negatif) |
| `pangkat(x, y)` | x pangkat y |
| `bulat(x)` | Pembulatan ke bilangan terdekat |
| `lantai(x)` | Pembulatan ke bawah (floor) |
| `atap(x)` | Pembulatan ke atas (ceil) |
| `acak()` | Angka acak antara 0 dan 1 |

### String
| Fungsi | Deskripsi |
|--------|-----------|
| `panjang(x)` | Panjang string, list, atau kamus |
| `huruf_besar(s)` | Ubah ke huruf kapital |
| `huruf_kecil(s)` | Ubah ke huruf kecil |
| `ganti(teks, cari, ganti)` | Ganti semua kemunculan substring |

### List
| Fungsi | Deskripsi |
|--------|-----------|
| `tambah(list, nilai)` | Tambah elemen ke akhir list |
| `hapus(list)` | Hapus & kembalikan elemen terakhir |

### Kamus
| Fungsi | Deskripsi |
|--------|-----------|
| `kamus()` | Buat kamus kosong |
| `punya(kamus, kunci)` | Apakah kunci ada |
| `kunci(kamus)` | List berisi semua kunci |
| `hapus_kunci(kamus, kunci)` | Hapus pasangan kunci-nilai |

### Utilitas
| Fungsi | Deskripsi |
|--------|-----------|
| `jenis(x)` | Nama tipe: `"angka"`, `"string"`, `"boolean"`, `"kosong"`, `"list"`, `"kamus"`, `"objek"`, `"kelas"` |
| `input(prompt)` | Baca satu baris dari stdin (prompt opsional) |
| `henti(detik)` | Jeda eksekusi selama `detik` detik |
| `jam()` | Waktu CPU berjalan (detik) |

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
Compiler single-pass (Pratt parser → Bytecode langsung, tanpa AST)
    ↓
Virtual Machine (Eksekusi bytecode berbasis stack)
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

Kontribusi sangat diterima! Silakan baca panduan lengkap di **[docs/03_KONTRIBUSI.md](docs/03_KONTRIBUSI.md)**.

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

Untuk detail lengkap, baca **[docs/03_KONTRIBUSI.md](docs/03_KONTRIBUSI.md)**.

## 📝 Lisensi

Proyek ini dibuat untuk tujuan edukasi dan pembelajaran bahasa pemrograman.

## 👨‍💻 Penulis

Proyek Indonesiaku - Bahasa Pemrograman dengan Sintaks Bahasa Indonesia

---

**Selamat Coding dengan Bahasa Indonesia! 🇮🇩**

Untuk pertanyaan, saran, atau laporan bug, silakan buat issue di repository ini.
