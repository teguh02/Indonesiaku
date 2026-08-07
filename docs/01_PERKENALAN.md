# 1. Perkenalan - Bahasa Pemrograman Indonesiaku

## Apa itu Bahasa Pemrograman Indonesiaku?

**Bahasa Pemrograman Indonesiaku** adalah bahasa pemrograman yang dirancang khusus untuk memberikan pengalaman pemrograman dalam bahasa Indonesia. Dengan menggunakan keywords dan syntax yang familiar dalam bahasa Indonesia, bahasa ini memudahkan pemula lokal untuk belajar pemrograman tanpa hambatan bahasa.

---

## Visi & Misi

### Visi
Menjadi bahasa pemrograman pertama yang fully mendukung pemrograman dalam Bahasa Indonesia, membuka akses pendidikan programming untuk jutaan pelajar Indonesia.

### Misi
1. Mendemokratisasi akses ke pemrograman melalui bahasa lokal
2. Menciptakan ekosistem pembelajaran programming yang inklusif
3. Membuktikan bahwa bahasa pemrograman dapat didesain dengan keywords non-Inggris
4. Mendukung perkembangan talent engineering di Indonesia

---

## Fitur Utama

### ✅ v0.4.0 (Saat Ini)

**Tipe Data**
- ✅ Angka (integer & float, direpresentasikan sebagai `double`)
- ✅ Boolean (`benar`, `salah`)
- ✅ String (dengan escape `\n \t \r \" \\`)
- ✅ `kosong` (null)
- ✅ **List** (`[1, 2, 3]`) + pengindeksan baca/tulis
- ✅ **Kamus/dictionary** (`kamus()` + indeks string)

**Kontrol Program**
- ✅ Variabel global & lokal (`variabel` untuk lokal/closure)
- ✅ `jika` / `jika_lain` / `selain`
- ✅ `selagi` (while) dan **`untuk ... dalam`** (for-in)
- ✅ **`hentikan` / `lanjut`** (break/continue)
- ✅ Fungsi, rekursi, dan **closure** (menangkap variabel)
- ✅ **Kelas & OOP**: `kelas`, `init`, `diri`, pewarisan (`<`), `super`
- ✅ **Penanganan kesalahan**: `naikkan`, `coba` / `kecuali`
- ✅ **Modul**: `impor "file.idk"`

**Operator**
- ✅ Aritmatika: `+`, `-`, `*`, `/`, `%`, `**`
- ✅ Assignment gabungan: `+=`, `-=`, `*=`, `/=`, `%=`, `**=`
- ✅ Perbandingan: `==`, `!=`, `<`, `>`, `<=`, `>=` (juga membandingkan string)
- ✅ Logika: `dan`, `atau`, `tidak`

**Fungsi Bawaan** (lengkap di [07_PUSTAKA_STANDAR.md](07_PUSTAKA_STANDAR.md))
- Matematika, string, list, kamus, konversi (`ke_angka`/`ke_teks`/`format`)
- Berkas (File I/O), waktu/tanggal, soket TCP, utilitas

**Infrastruktur**
- ✅ Garbage collector mark-sweep
- ✅ Pustaka standar dalam bahasa sendiri (`pustaka/`)
- ✅ Contoh aplikasi: Todo-list JSON DB (CLI + REST HTTP server)

---

## Roadmap Pengembangan

Semua fitur inti (list, kamus, kelas/OOP, exception, modul, GC, File I/O, soket)
sudah **terimplementasi** di v0.4.0. Kandidat pekerjaan berikutnya:

- 🔮 `akhirnya` (finally) untuk `coba`/`kecuali`
- 🔮 Kesadaran UTF-8 pada fungsi string (kini berbasis byte)
- 🔮 Literal kamus `{ }` dan slicing `list[a:b]`
- 🔮 Standard library yang lebih luas & package manager

---

## Teknologi di Balik Layar

### Arsitektur
```
Source Code (.idk)
     ↓
  Scanner (Lexical Analysis)
     ↓
  Compiler single-pass (Pratt parser → Bytecode langsung, tanpa AST)
     ↓
  Virtual Machine berbasis stack (+ garbage collector mark-sweep)
     ↓
  Output
```

### Tech Stack
- **Language**: C11
- **Build**: GCC
- **Architecture**: Stack-based bytecode VM
- **Optimization**: -O2 flag

### Performance
- Lightning fast compilation
- Efficient bytecode execution
- Memory safe (no buffer overflows)
- Suitable for educational purposes

---

## Mengapa Bahasa Indonesia?

### Keuntungan
1. **Aksesibilitas** - Pemula tidak perlu belajar Inggris untuk coding
2. **Inklusi** - Membuka peluang untuk 270 juta penutur Bahasa Indonesia
3. **Pembelajaran** - Fokus pada concept, bukan language barrier
4. **Kebanggaan** - Programming dalam bahasa sendiri
5. **Eksperimen** - Proving bahwa non-English programming languages work

### Tantangan
1. **Community** - Ecosystem yang lebih kecil dibanding Python/JavaScript
2. **Library** - Fewer third-party libraries available
3. **Documentation** - Lebih sedikit resources online
4. **Performance** - Trade-off untuk accessibility

---

## Contoh Penggunaan

### Program 1: Fibonacci Recursion
```indonesiaku
fungsi fibonacci(n) {
    jika n <= 1 {
        kembalikan n
    }
    kembalikan fibonacci(n - 1) + fibonacci(n - 2)
}

cetak("Fibonacci ke-10:", fibonacci(10))
```
> Contoh lengkap yang bisa dijalankan: [`examples/fibonacci.idk`](../examples/fibonacci.idk)

{{embed:examples/fibonacci.idk}}

### Program 2: Kontrol Program
```indonesiaku
x = 1
selagi x <= 5 {
    jika x % 2 == 0 {
        cetak("Genap:", x)
    } selain {
        cetak("Ganjil:", x)
    }
    x = x + 1
}
```

### Program 3: Fungsi dengan Parameter
```indonesiaku
fungsi hitung_luas(panjang, lebar) {
    kembalikan panjang * lebar
}

cetak("Luas:", hitung_luas(10, 5))
```

> Kumpulan contoh lengkap ada di folder [`examples/`](../examples/) — mis.
> `list.idk`, `kamus.idk`, `oop/kelas.idk`, `coba_kecuali.idk`, `todo_cli.idk`,
> `todo_server.idk`.

---

## Statistik Project

| Metrik | Nilai |
|--------|-------|
| Baris kode (src) | ~4.9K |
| Jumlah opcode VM | 47 |
| Contoh program (`examples/`) | 50+ |
| Golden test | 51 |
| Pustaka standar (`pustaka/`) | teks, daftar, matematika, kamus, json, http |

---

## Perbandingan dengan Bahasa Lain

| Feature | Indonesiaku | Python | JavaScript |
|---------|-------------|--------|------------|
| Syntax Bahasa Indonesia | ✅ Ya | ❌ Tidak | ❌ Tidak |
| Easy to Learn | ✅ Ya | ✅ Ya | ⚠️ Medium |
| Performance | ⚠️ Medium | ⚠️ Medium | ✅ Good |
| Community | ⚠️ Kecil | ✅ Besar | ✅ Besar |
| Libraries | ❌ Sedikit | ✅ Banyak | ✅ Banyak |
| Educational | ✅ Sempurna | ✅ Sempurna | ✅ Baik |

---

## Siapa yang Cocok Menggunakan?

### ✅ Cocok Untuk:
- Pelajar Indonesia baru belajar programming
- Sekolah/universitas yang mengajar programming
- Proyek pendidikan lokal
- Penelitian tentang programming languages
- Enthusiasts yang ingin support bahasa lokal

### ⚠️ Kurang Cocok Untuk:
- Production systems (use Python/Go/Rust)
- High-performance computing
- Large-scale applications
- Proyek dengan tight deadlines

---

## Lisensi & Attribution

Bahasa Pemrograman Indonesiaku dirilis di bawah **MIT License**, yang berarti:
- ✅ Gratis digunakan untuk keperluan komersial dan non-komersial
- ✅ Boleh dimodifikasi dan didistribusikan
- ✅ Harus menyertakan attribution

---

## Kontribusi & Support

Kami sangat menerima:
- 🐛 Bug reports
- 💡 Feature suggestions
- 📝 Documentation improvements
- 🔧 Code contributions
- 📢 Word of mouth support

Lihat file **KONTRIBUSI.md** untuk detail lengkap.

---

## Resources

- **GitHub Repository**: https://github.com/teguh02/Indonesiaku
- **Issue Tracker**: https://github.com/teguh02/Indonesiaku/issues
- **Documentation**: `/docs` folder
- **Examples**: `/examples` folder

---

**Status**: Aktif dikembangkan  
**Latest Version**: v0.4.0  
**Maintainer**: Teguh Rijanandi
