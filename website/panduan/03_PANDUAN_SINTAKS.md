# 3. Panduan Sintaks - Bahasa Pemrograman Indonesiaku

Panduan lengkap untuk memahami dan menggunakan syntax Bahasa Pemrograman Indonesiaku dengan contoh-contoh praktis.

---

## 📋 Daftar Isi

1. [Tipe Data](#tipe-data)
2. [Variabel](#variabel)
3. [Operator](#operator)
4. [Percabangan](#percabangan)
5. [Perulangan](#perulangan)
6. [Fungsi](#fungsi)
7. [Built-in Functions](#built-in-functions)
8. [Comments](#comments)
9. [Best Practices](#best-practices)

---

## Tipe Data

Bahasa Pemrograman Indonesiaku mendukung 4 tipe data utama:

### 1. Numbers (Angka)

Digunakan untuk menyimpan nilai numerik (integer dan float).

```indonesiaku
# Integer
x = 42
y = -17
z = 0

# Float
pi = 3.14159
suhu = -40.5
nol = 0.0

# Operasi math
hasil = x + y
print_value = pi * 2
```

### 2. Strings (Text)

Digunakan untuk menyimpan teks.

```indonesiaku
# String dengan double quotes
nama = "Teguh Rijanandi"
pesan = "Halo, dunia!"

# String dengan single quotes
greeting = 'Selamat pagi'
alamat = 'Jakarta, Indonesia'

# String kosong
text_kosong = ""
```

### 3. Booleans (Nilai Logika)

Digunakan untuk nilai benar/salah.

```indonesiaku
# Boolean values
aktif = benar      # True
nonaktif = salah   # False

# Dari operasi perbandingan
x = 10
y = 5
hasil = x > y      # benar
hasil2 = x < y     # salah
```

### 4. Null (Kosong)

Digunakan untuk nilai yang tidak ada atau belum diinisialisasi.

```indonesiaku
# Null value
data = kosong
hasil = kosong
```

---

## Variabel

Variabel adalah tempat menyimpan data yang dapat berubah nilainya.

### Deklarasi dan Assignment

```indonesiaku
# Sederhana
x = 10
nama = "Budi"
aktif = benar

# Multiple assignment (satu per satu)
a = 1
b = 2
c = 3

# Reassignment
x = 20
x = x + 5
nama = "Teguh"
```

### Naming Rules (Aturan Penamaan)

```indonesiaku
# VALID - Boleh digunakan
nama = "Budi"
nama_lengkap = "Budi Santoso"
umur1 = 25
_private = 42
myVar = "test"

# INVALID - TIDAK boleh digunakan
1nama = "Error"       # Tidak boleh diawali angka
jika = 10            # Tidak boleh pakai keyword
untuk-loop = 0       # Tidak boleh gunakan hyphen
```

### Scope (Jangkauan)

```indonesiaku
# Global scope
x = 10

fungsi test() {
    # Local scope
    y = 20
    cetak(x)  # Bisa akses global x
    cetak(y)  # Bisa akses local y
}

test()
# cetak(y)   # ERROR: y tidak ada di global scope
```

---

## Operator

### Operator Aritmatika

```indonesiaku
a = 10
b = 3

jumlah = a + b        # 13 (penjumlahan)
selisih = a - b       # 7  (pengurangan)
kali = a * b          # 30 (perkalian)
bagi = a / b          # 3.333... (pembagian)
bagi_bulat = a // b   # 3  (pembagian bulat, dibulatkan ke bawah)
sisa = a % b          # 1  (modulo/sisa bagi)
pangkat = 2 ** 3      # 8  (perpangkatan)
```

Aturannya sama dengan Python:

```indonesiaku
-7 // 2       # -4  (dibulatkan ke bawah, bukan ke arah nol)
-7 % 3        # 2   (hasil % mengikuti tanda pembagi)
2 ** 3 ** 2   # 512 (pangkat dihitung dari kanan: 2 ** 9)
-2 ** 2       # -4  (sama dengan -(2 ** 2))
```

> Contoh: [`examples/operator_aritmetika.idk`](../examples/operator_aritmetika.idk)

### Operator Perbandingan

```indonesiaku
x = 10
y = 5

# Hasil adalah boolean (benar atau salah)
x == y    # salah (sama dengan)
x != y    # benar (tidak sama dengan)
x > y     # benar (lebih besar)
x < y     # salah (lebih kecil)
x >= y    # benar (lebih besar atau sama)
x <= y    # salah (lebih kecil atau sama)
```

### Identitas dan rantai perbandingan

`adalah` membandingkan identitas objek; `bukan adalah` adalah kebalikannya. Perbandingan dapat dirantai seperti Python: operand tengah dihitung sekali dan evaluasi berhenti saat salah.

```indonesiaku
variabel daftar = [1]
variabel alias = daftar
cetak(daftar adalah alias)       # benar
cetak(daftar bukan adalah [1])   # benar
cetak(1 < x < 3)
cetak("apel" dalam buah == buah)
```

### Operator Logika

```indonesiaku
a = benar
b = salah

cetak(a dan b)    # salah (AND - keduanya harus benar)
cetak(a atau b)   # benar (OR - salah satu harus benar)
cetak(tidak a)    # salah (NOT - kebalikan nilai)
cetak(tidak b)    # benar (NOT - kebalikan nilai)

# Kombinasi
cetak((a atau b) dan (tidak b))  # benar
```

> **Perhatian:** akhir baris belum mengakhiri pernyataan. Baris yang diawali
> `(` atau `-` bisa tersambung ke baris sebelumnya (`tidak b` lalu baris
> `(a atau b)` terbaca sebagai pemanggilan `b(...)`). Pisahkan dengan `;` bila
> perlu, atau hindari baris yang diawali kurung buka.

### Operator Keanggotaan (`dalam`, `tidak dalam`)

```indonesiaku
buah = ["apel", "jeruk"]
cetak("apel" dalam buah)          # benar  (elemen list)
cetak("mangga" tidak dalam buah)  # benar
cetak("ana" dalam "pisang")       # salah  (substring teks)
cetak("san" dalam "pisang")       # benar
cetak(5 dalam rentang(10))        # benar

harga = kamus()
harga["apel"] = 5000
cetak("apel" dalam harga)         # benar  (kunci kamus)
```

> Contoh: [`examples/operator_dalam.idk`](../examples/operator_dalam.idk)

### Prioritas Operator

Urutan eksekusi dari tertinggi ke terendah:

```
1. **           (pangkat, dihitung dari kanan)
2. -            (minus unary)
3. *, /, //, %  (perkalian, pembagian, pembagian bulat, modulo)
4. +, -         (penjumlahan, pengurangan)
5. <, <=, >, >=, dalam, tidak dalam (perbandingan & keanggotaan)
6. ==, !=       (kesetaraan)
7. tidak        (NOT: tidak a == b berarti tidak (a == b))
8. dan          (AND)
9. atau         (OR)
```

**Contoh:**
```indonesiaku
hasil1 = 2 + 3 * 4      # 14, bukan 20 (* dikerjakan terlebih dahulu)
hasil2 = (2 + 3) * 4    # 20 (kurung diprioritaskan)
hasil3 = 10 > 5 dan 3 < 2 atau 4 == 4  # benar
```

---

## Percabangan

### If Statement

Struktur dasar percabangan:

```indonesiaku
jika kondisi {
    # code jika kondisi benar
}
```

Kondisi tidak harus `benar`/`salah`. Seperti Python, nilai berikut dianggap
**salah**: `salah`, `kosong`, `0`, `""` (teks kosong), `[]` (list kosong),
kamus kosong, dan `rentang(0)`. Semua nilai lain dianggap **benar**:

```indonesiaku
antrian = [1, 2, 3]
selagi antrian {                 # berhenti saat list kosong
    cetak(hapus_pada(antrian, 0))
}

nama = ""
cetak(nama atau "tanpa nama")    # tanpa nama
```

> Contoh: [`examples/kebenaran.idk`](../examples/kebenaran.idk)

**Contoh sederhana:**
```indonesiaku
x = 10

jika x > 0 {
    cetak("Positif")
}
```

### If-Else

```indonesiaku
x = -5

jika x > 0 {
    cetak("Positif")
} selain {
    cetak("Tidak positif")
}
```

### If-Elif-Else (Multiple Conditions)

```indonesiaku
nilai = 85

jika nilai >= 90 {
    cetak("Grade A")
}
jika nilai >= 80 {
    cetak("Grade B")
}
jika nilai >= 70 {
    cetak("Grade C")
}
selain {
    cetak("Grade F")
}
```

### Nested If

```indonesiaku
x = 15

jika x > 0 {
    jika x > 10 {
        cetak("Lebih dari 10")
    } selain {
        cetak("Antara 1 dan 10")
    }
} selain {
    cetak("Nol atau negatif")
}
```

### Percabangan dengan Operator Logika

```indonesiaku
# AND condition
x = 50
jika x > 0 dan x < 100 {
    cetak("Di antara 0 dan 100")
}

# OR condition
x = -5
jika x < 0 atau x > 100 {
    cetak("Di luar range 0-100")
}

# NOT condition
aktif = salah
jika tidak aktif {
    cetak("Tidak aktif")
}

# Kombinasi kompleks
x = 50
jika (x > 0 dan x < 100) atau x == 0 {
    cetak("Valid")
}
```

---

## Perulangan

### While Loop

Struktur dasar loop:

```indonesiaku
selagi kondisi {
    # code yang diulang
}
```

**Contoh: Counting**
```indonesiaku
i = 0
selagi i < 5 {
    cetak(i)
    i = i + 1
}
# Output: 0, 1, 2, 3, 4
```

**Contoh: Countdown**
```indonesiaku
hitungan = 5
selagi hitungan > 0 {
    cetak(hitungan)
    hitungan = hitungan - 1
}
# Output: 5, 4, 3, 2, 1
```

### Nested Loops

```indonesiaku
# Multiplication table
i = 1
selagi i <= 3 {
    j = 1
    selagi j <= 3 {
        cetak(i * j)
        j = j + 1
    }
    i = i + 1
}
# Output: 1,2,3,2,4,6,3,6,9
```

### Loop dengan Kondisi Kompleks

```indonesiaku
x = 10
selagi x > 0 dan x != 5 {
    cetak(x)
    x = x - 1
}
# Output: 10, 9, 8, 7, 6
```

---

## Fungsi

### Definisi Fungsi

Struktur dasar fungsi:

```indonesiaku
fungsi nama_fungsi() {
    # code di dalam fungsi
}
```

### Fungsi Tanpa Parameter

```indonesiaku
fungsi sapa() {
    cetak("Halo!")
}

# Memanggil fungsi
sapa()
```

### Fungsi dengan Parameter

```indonesiaku
fungsi sapa_nama(nama) {
    cetak("Halo,", nama)
}

sapa_nama("Teguh")      # Output: Halo, Teguh
sapa_nama("Budi")       # Output: Halo, Budi
```

### Fungsi dengan Multiple Parameters

```indonesiaku
fungsi tambah(a, b) {
    cetak("Menambah", a, "dan", b)
}

fungsi kurangi(a, b) {
    cetak("Mengurangi", a, "dan", b)
}

tambah(5, 3)           # Menambah 5 dan 3
kurangi(10, 4)         # Mengurangi 10 dan 4
```

### Fungsi dengan Return Value

```indonesiaku
fungsi jumlah(a, b) {
    kembalikan a + b
}

hasil = jumlah(5, 3)
cetak("Hasilnya:", hasil)  # Output: Hasilnya: 8
```

### Fungsi Rekursif

**Factorial:**
```indonesiaku
fungsi faktorial(n) {
    jika n <= 1 {
        kembalikan 1
    }
    kembalikan n * faktorial(n - 1)
}

cetak(faktorial(5))  # Output: 120
```

**Fibonacci:**
```indonesiaku
fungsi fibonacci(n) {
    jika n <= 1 {
        kembalikan n
    }
    kembalikan fibonacci(n - 1) + fibonacci(n - 2)
}

cetak(fibonacci(10))  # Output: 55
```

---

## Operator Assignment Gabungan

```indonesiaku
x = 10
x += 5    # x = x + 5  -> 15
x -= 3    # -> 12
x *= 2    # -> 24
x /= 4    # -> 6
x %= 4    # -> 2
x **= 3   # -> 8
x //= 3   # -> 2
x *= 2 + 3  # -> 10 (sisi kanan dihitung utuh dulu: x = x * (2 + 3))
```
> Contoh: [`examples/compound_operators.idk`](../examples/compound_operators.idk)

---

## List (Senarai)

```indonesiaku
angka = [10, 20, 30]
cetak(angka[0])          # 10 (pengindeksan)
angka[1] = 99            # ubah elemen
tambah(angka, 40)        # tambah ke akhir
cetak(panjang(angka))    # 4
x = hapus(angka)         # hapus & kembalikan elemen terakhir
hapus_pada(angka, 0)     # hapus pada indeks
sisip(angka, 0, 5)       # sisipkan pada indeks

# List bisa bersarang & bertipe campuran
matriks = [[1, 2], [3, 4]]
cetak(matriks[0][1])     # 2
```
> Contoh: [`examples/list.idk`](../examples/list.idk)

{{embed:examples/list.idk}}

---

## Kamus (Dictionary)

```indonesiaku
orang = kamus()
orang["nama"] = "Budi"
orang["umur"] = 25
cetak(orang["nama"])         # Budi
cetak(punya(orang, "umur"))  # benar
hapus_kunci(orang, "umur")
untuk k dalam orang {             # iterasi kunci sesuai penyisipan
    cetak(k, "=", orang[k])
}
untuk pasangan dalam enumerasi(orang, 1) {
    cetak(pasangan[0], pasangan[1])
}
```
> Contoh: [`examples/kamus.idk`](../examples/kamus.idk)

---

## Perulangan untuk...dalam (for-in)

```indonesiaku
untuk buah dalam ["apel", "mangga", "jeruk"] {
    cetak(buah)
}

total = 0
untuk n dalam [1, 2, 3, 4, 5] {
    total = total + n
}
cetak("total:", total)   # 15

# Teks diiterasi per karakter
untuk huruf dalam "halo" {
    cetak(huruf)
}
```
> Contoh: [`examples/untuk_dalam.idk`](../examples/untuk_dalam.idk)

### Perulangan angka dengan `rentang()`

`rentang()` menghasilkan deret bilangan bulat, seperti `range()` di Python:

```indonesiaku
untuk i dalam rentang(5) { cetak(i) }          # 0 1 2 3 4
untuk i dalam rentang(2, 6) { cetak(i) }       # 2 3 4 5
untuk i dalam rentang(0, 10, 3) { cetak(i) }   # 0 3 6 9
untuk i dalam rentang(5, 0, -1) { cetak(i) }   # 5 4 3 2 1

r = rentang(0, 10, 2)
cetak(r, panjang(r), r[1])                     # rentang(0, 10, 2) 5 2
```

`akhir` tidak termasuk. Rentang tidak membuat list, jadi `rentang(1000000)`
tetap hemat memori.

> Contoh: [`examples/rentang.idk`](../examples/rentang.idk)

---

## hentikan & lanjut (break & continue)

```indonesiaku
untuk x dalam [1, 2, 3, 4, 5, 6] {
    jika x == 4 { hentikan }   # keluar dari loop
    jika x == 2 { lanjut }     # lompat ke iterasi berikut
    cetak(x)
}
# Output: 1, 3
```
> Contoh: [`examples/hentikan_lanjut.idk`](../examples/hentikan_lanjut.idk)

---

## Pengindeksan String

String dapat diakses per karakter (baca-saja; string bersifat immutable).

```indonesiaku
s = "halo"
cetak(s[0])           # h
cetak(panjang(s))     # 4
untuk i dalam [0, 1, 2, 3] {
    cetak(s[i])
}
```
> Contoh: [`examples/indeks_string.idk`](../examples/indeks_string.idk)

---

## Closure

Gunakan kata kunci `variabel` untuk membuat variabel lokal fungsi yang dapat
ditangkap oleh closure (fungsi bersarang).

```indonesiaku
fungsi buat_pencacah() {
    variabel hitung = 0
    fungsi naik() {
        hitung = hitung + 1
        kembalikan hitung
    }
    kembalikan naik
}

c = buat_pencacah()
cetak(c())   # 1
cetak(c())   # 2
```
> Contoh: [`examples/closure.idk`](../examples/closure.idk)

---

## Kelas & OOP

```indonesiaku
kelas Hewan {
    init(nama, suara) {
        diri.nama = nama
        diri.suara = suara
    }
    bersuara() {
        cetak(diri.nama, "berkata:", diri.suara)
    }
}

kelas Anjing < Hewan {          # pewarisan dengan '<'
    init(nama) {
        super.init(nama, "Guk")  # panggil konstruktor induk
    }
}

a = Anjing("Rex")
a.bersuara()   # Rex berkata: Guk
```
> Contoh: [`examples/oop/kelas.idk`](../examples/oop/kelas.idk)

{{embed:examples/oop/kelas.idk}}

---

## Penanganan Kesalahan (naikkan / coba / kecuali / akhirnya)

```indonesiaku
coba {
    naikkan "terjadi kesalahan"
} kecuali e {
    cetak("tertangkap:", e)
}

# Kesalahan fungsi bawaan juga dapat ditangkap:
coba {
    x = ke_angka("bukan angka")
} kecuali e {
    cetak("gagal konversi:", e)
}

# Semua kesalahan saat program berjalan bisa ditangkap. Nilai `e` adalah
# pesan kesalahannya:
coba {
    cetak(10 / 0)
} kecuali e {
    cetak(e)            # Pembagian dengan nol.
}
```

Blok `akhirnya` **selalu** dijalankan, baik terjadi kesalahan maupun tidak.
Cocok untuk menutup berkas atau soket. Tanpa `kecuali`, kesalahan diteruskan
ke luar setelah blok `akhirnya` selesai:

```indonesiaku
f = buka_berkas("data.txt", "b")
coba {
    cetak(baca(f))
} kecuali e {
    cetak("gagal membaca:", e)
} akhirnya {
    tutup(f)
}
```

Blok `akhirnya` juga dijalankan saat `kembalikan`, `hentikan`, atau `lanjut`
keluar dari `coba`. Nilai yang dikembalikan dihitung lebih dulu:

```indonesiaku
fungsi baca_angka() {
    coba {
        kembalikan 42
    } akhirnya {
        cetak("selalu dijalankan")
    }
}
cetak(baca_angka())   # "selalu dijalankan", lalu 42
```

> Contoh: [`examples/coba_kecuali.idk`](../examples/coba_kecuali.idk),
> [`examples/coba_akhirnya.idk`](../examples/coba_akhirnya.idk),
> [`examples/galat_tertangkap.idk`](../examples/galat_tertangkap.idk),
> [`examples/coba_akhirnya_lompat.idk`](../examples/coba_akhirnya_lompat.idk)

### `lewati` — blok kosong

`lewati` adalah pernyataan yang tidak melakukan apa-apa (seperti `pass` di
Python). Berguna sebagai penanda tempat kode yang belum ditulis:

```indonesiaku
fungsi nanti_saja() { lewati }
kelas Kosong { lewati }
```

---

## Modul (impor)

`impor "file.idk"` menjalankan file lain dalam namespace global yang sama
(di-cache, sehingga impor ulang diabaikan).

```indonesiaku
impor "pustaka/teks.idk"
impor "pustaka/daftar.idk"

cetak(teks_pisah("a,b,c", ","))        # [a, b, c]
cetak(daftar_urutkan([3, 1, 2]))       # [1, 2, 3]
```
> Contoh: [`examples/impor.idk`](../examples/impor.idk)
> Lihat daftar pustaka standar di [07_PUSTAKA_STANDAR.md](07_PUSTAKA_STANDAR.md).

---

## Built-in Functions

> Referensi lengkap semua fungsi bawaan (matematika, string, list, kamus,
> konversi, berkas, waktu/tanggal, soket) ada di
> [07_PUSTAKA_STANDAR.md](07_PUSTAKA_STANDAR.md).

### cetak() - Print Output

Menampilkan output ke layar.

```indonesiaku
# Satu argument
cetak("Halo")

# Multiple arguments: dicetak dalam satu baris, dipisah spasi
cetak("Nama:", "Teguh")   # Nama: Teguh
cetak("Umur:", 25)        # Umur: 25

# Tanpa argument: baris kosong
cetak()

# Expressions
x = 10
y = 5
cetak("Jumlah:", x + y)

# Various types
cetak(42)           # Number
cetak(benar)        # Boolean
cetak(kosong)       # Null
```

### jam() - Get Current Time

Mendapatkan waktu saat ini (dalam detik sejak epoch).

```indonesiaku
waktu = jam()
cetak("Waktu saat ini:", waktu)
```

---

## Comments

Komentar digunakan untuk menjelaskan code dan tidak akan dieksekusi.

```indonesiaku
# Ini adalah single-line comment

# Variabel untuk menyimpan nama
nama = "Teguh"

# Fungsi untuk menghitung luas
fungsi hitung_luas(p, l) {
    # Rumus: panjang x lebar
    kembalikan p * l
}

# Menampilkan hasil
cetak("Luas:", hitung_luas(10, 5))
```

---

## Best Practices

### 1. Naming Conventions

```indonesiaku
# ✅ BAIK - Deskriptif dan jelas
nama_lengkap = "Teguh Rijanandi"
hitung_total_pendapatan = 0
adalah_aktif = benar

# ❌ BURUK - Tidak jelas
nlk = "Teguh Rijanandi"
x = 0
flag = benar
```

### 2. Indentasi Konsisten

```indonesiaku
# ✅ BAIK
jika benar {
    cetak("Level 1")
    jika benar {
        cetak("Level 2")
    }
}

# ❌ BURUK - Indentasi tidak konsisten
jika benar {
cetak("Level 1")
   jika benar {
        cetak("Level 2")
   }
}
```

### 3. Gunakan Comments untuk Logic Kompleks

```indonesiaku
# ✅ BAIK
# Menghitung total dengan diskon 10%
total = harga * 0.9
cetak(total)

# ❌ BURUK - Comment tidak informatif
# Menambah variabel
total = harga * 0.9
```

### 4. Fungsi Kecil dan Fokus

```indonesiaku
# ✅ BAIK - Satu fungsi, satu tanggung jawab
fungsi hitung_luas_persegi(sisi) {
    kembalikan sisi * sisi
}

fungsi hitung_luas_lingkaran(radius) {
    kembalikan 3.14159 * radius * radius
}

# ❌ BURUK - Terlalu banyak tanggung jawab
fungsi hitung_semua(sisi, radius) {
    luas_persegi = sisi * sisi
    luas_lingkaran = 3.14159 * radius * radius
    cetak("Persegi:", luas_persegi)
    cetak("Lingkaran:", luas_lingkaran)
}
```

### 5. Deklarasikan Variabel di Awal

```indonesiaku
# ✅ BAIK
x = 0
y = 0
nama = ""

selagi x < 10 {
    y = x * 2
    nama = "data"
    x = x + 1
}

# ❌ KURANG BAIK - Deklarasi di tengah kode
selagi x < 10 {
    y = x * 2
    nama = "data"
    x = x + 1
}
```

---

## Contoh Program Lengkap

### Program 1: FizzBuzz

```indonesiaku
# FizzBuzz game
fungsi fizzbuzz(n) {
    i = 1
    selagi i <= n {
        jika i % 15 == 0 {
            cetak("FizzBuzz")
        }
        jika i % 3 == 0 dan i % 15 != 0 {
            cetak("Fizz")
        }
        jika i % 5 == 0 dan i % 15 != 0 {
            cetak("Buzz")
        }
        jika i % 3 != 0 dan i % 5 != 0 {
            cetak(i)
        }
        i = i + 1
    }
}

fizzbuzz(15)
```

### Program 2: Hitung Rata-Rata

```indonesiaku
fungsi hitung_rata_rata(a, b, c, d, e) {
    jumlah = a + b + c + d + e
    rata = jumlah / 5
    kembalikan rata
}

nilai1 = 80
nilai2 = 85
nilai3 = 90
nilai4 = 75
nilai5 = 88

rata_rata = hitung_rata_rata(nilai1, nilai2, nilai3, nilai4, nilai5)
cetak("Rata-rata nilai:", rata_rata)
```

### Program 3: Bilangan Prima

```indonesiaku
fungsi adalah_prima(n) {
    jika n <= 1 {
        kembalikan salah
    }
    
    i = 2
    selagi i * i <= n {
        jika n % i == 0 {
            kembalikan salah
        }
        i = i + 1
    }
    
    kembalikan benar
}

# Cari bilangan prima sampai 20
angka = 2
selagi angka <= 20 {
    jika adalah_prima(angka) {
        cetak(angka)
    }
    angka = angka + 1
}
```

---

## Ringkasan Keywords

| Kategori | Keywords |
|----------|----------|
| **Tipe/Literal** | `benar`, `salah`, `kosong` |
| **Percabangan** | `jika`, `jika_lain`, `selain` |
| **Perulangan** | `selagi`, `untuk`, `dalam`, `hentikan`, `lanjut` |
| **Fungsi** | `fungsi`, `kembalikan`, `variabel` |
| **Kelas/OOP** | `kelas`, `diri`, `super` |
| **Kesalahan** | `naikkan`, `coba`, `kecuali` |
| **Modul** | `impor` |
| **Operator logika** | `dan`, `atau`, `tidak` |
| **Output** | `cetak` |

---

## Tips & Tricks

1. **Gunakan descriptive names** - Nama yang jelas mempermudah debugging
2. **Test di REPL** - Coba command secara interaktif terlebih dahulu (blok multi-baris didukung)
3. **Break kompleks logic** - Pisahkan menjadi fungsi-fungsi kecil
4. **Use functions** - Untuk menghindari code duplication
5. **Comment wisely** - Untuk logic yang kompleks, bukan yang obvious

---

**Dokumen ini memberikan fondasi untuk mulai menggunakan Bahasa Pemrograman Indonesiaku. Untuk pertanyaan lebih lanjut, silakan lihat contoh di folder `/examples` atau [07_PUSTAKA_STANDAR.md](07_PUSTAKA_STANDAR.md).**

Versi bahasa: v0.4.0
