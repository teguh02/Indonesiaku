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

### ✅ v0.1.1 (Saat Ini)

**Tipe Data**
- ✅ Numbers (integer dan float)
- ✅ Booleans (`benar`, `salah`)
- ✅ Strings (text)
- ✅ Null (`kosong`)

**Kontrol Program**
- ✅ Variabel global
- ✅ If/else statements (`jika`, `selain`)
- ✅ While loops (`selagi`)
- ✅ Fungsi dengan parameter dan return value
- ✅ Recursion support

**Operators**
- ✅ Arithmetic: `+`, `-`, `*`, `/`, `%`
- ✅ Comparison: `==`, `!=`, `<`, `>`, `<=`, `>=`
- ✅ Logical: `dan`, `atau`, `tidak`

**Built-in Functions**
- ✅ `cetak()` - Print output
- ✅ `jam()` - Get current time

**Features Lainnya**
- ✅ Comments (`#`)
- ✅ OOP patterns (via functions)
- ✅ Recursion mendalam
- ✅ Nested loops dan functions

---

## Roadmap Pengembangan

### v0.2 (Planned)
- ⏳ For...in loops (`untuk...dalam`)
- ⏳ Array/List support
- ⏳ Break dan Continue statements
- ⏳ String operations (concatenation, indexing)
- ⏳ More built-in functions

### v0.3 (Planned)
- ⏳ Native class support (`kelas`)
- ⏳ Inheritance dan polymorphism
- ⏳ Access modifiers (public/private)
- ⏳ Constructor methods
- ⏳ `diri` (self) keyword
- ⏳ Method chaining

### v0.4+ (Future)
- 🔮 Exception handling (`coba...kecuali`)
- 🔮 Module system (`impor`)
- 🔮 File I/O operations
- 🔮 Standard library expansion
- 🔮 Package manager

---

## Teknologi di Balik Layar

### Arsitektur
```
Source Code (.idk)
     ↓
  Scanner (Lexical Analysis)
     ↓
  Parser (Syntax Analysis)
     ↓
  Compiler (Code Generation)
     ↓
  Bytecode
     ↓
  Virtual Machine (Execution)
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
    jika n <= 1:
        kembalikan n
    kembalikan fibonacci(n - 1) + fibonacci(n - 2)
}

cetak("Fibonacci ke-10:", fibonacci(10))
```

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

---

## Statistik Project

| Metrik | Nilai |
|--------|-------|
| Lines of Code (VM) | ~3.5K |
| Number of Opcodes | 31+ |
| Test Coverage | 95% |
| Example Programs | 20 |
| Documentation | 3K+ lines |
| Git Commits | 12+ |

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
**Latest Version**: v0.1.1  
**Release Date**: Oktober 2025  
**Maintainer**: Teguh Rijanandi
