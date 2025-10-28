# Referensi Sintaks Lengkap - Bahasa Indonesiaku v0.1

## Daftar Isi
- [Tipe Data](#tipe-data)
- [Variabel](#variabel)
- [Operator](#operator)
- [Percabangan](#percabangan)
- [Perulangan](#perulangan)
- [Fungsi](#fungsi)
- [Kata Kunci](#kata-kunci)

## Tipe Data

### Number (Angka)
```python
# Integer
x = 42
y = -17
z = 0

# Float
pi = 3.14159
suhu = -40.5
nol = 0.0
```

### Boolean
```python
benar_nilai = benar    # True
salah_nilai = salah    # False
```

### Kosong (None/Null)
```python
data = kosong          # None/null
```

### String
```python
nama = "Budi"
pesan = 'Halo, dunia!'
kosong_str = ""
```

## Variabel

### Deklarasi dan Assignment
```python
# Langsung assign
x = 10
nama = "Budi"
aktif = benar

# Reassignment
x = 20
x = x + 5
```

### Naming Rules
- Harus dimulai dengan huruf atau underscore
- Bisa mengandung huruf, angka, dan underscore
- Case-sensitive (x dan X berbeda)
- Tidak boleh menggunakan kata kunci

```python
# Valid
nama = "Budi"
nama_lengkap = "Budi Santoso"
umur1 = 25
_private = 42

# Invalid (akan error)
# 1nama = "Error"      # Tidak boleh diawali angka
# jika = 10            # Tidak boleh pakai kata kunci
```

## Operator

### Operator Aritmatika
```python
a = 10
b = 3

jumlah = a + b        # 13 (penjumlahan)
selisih = a - b       # 7  (pengurangan)
kali = a * b          # 30 (perkalian)
bagi = a / b          # 3.333... (pembagian)
sisa = a % b          # 1  (modulo/sisa bagi)
pangkat = a ** 2      # 100 (pangkat)
```

### Operator Perbandingan
```python
x = 10
y = 5

x == y    # salah (sama dengan)
x != y    # benar (tidak sama dengan)
x > y     # benar (lebih besar)
x < y     # salah (lebih kecil)
x >= y    # benar (lebih besar atau sama dengan)
x <= y    # salah (lebih kecil atau sama dengan)
```

### Operator Logika
```python
a = benar
b = salah

a dan b   # salah (AND)
a atau b  # benar (OR)
tidak a   # salah (NOT)
tidak b   # benar (NOT)

# Kombinasi
(a atau b) dan (tidak b)  # benar
```

### Operator Unary
```python
x = 10
y = -x        # -10 (negasi)

a = benar
b = tidak a   # salah (NOT)
```

### Prioritas Operator (dari tertinggi ke terendah)
1. `**` (pangkat)
2. `-`, `tidak` (unary)
3. `*`, `/`, `%` (perkalian, pembagian, modulo)
4. `+`, `-` (penjumlahan, pengurangan)
5. `<`, `<=`, `>`, `>=` (perbandingan)
6. `==`, `!=` (kesetaraan)
7. `dan` (AND)
8. `atau` (OR)

## Percabangan

### If Statement
```python
# If sederhana
jika x > 0:
    cetak("Positif")

# If-else
jika x > 0:
    cetak("Positif")
selain:
    cetak("Tidak positif")

# If-elif-else
jika x > 0:
    cetak("Positif")
jika_lain x < 0:
    cetak("Negatif")
selain:
    cetak("Nol")

# Nested if
jika x > 0:
    jika x > 10:
        cetak("Lebih dari 10")
    selain:
        cetak("Antara 1 dan 10")
```

### Kondisi dengan Operator Logika
```python
# AND
jika x > 0 dan x < 100:
    cetak("Antara 0 dan 100")

# OR
jika x < 0 atau x > 100:
    cetak("Di luar range 0-100")

# NOT
jika tidak aktif:
    cetak("Tidak aktif")

# Kombinasi
jika (x > 0 dan x < 100) atau x == 0:
    cetak("Valid")
```

## Perulangan

### While Loop (Selagi)
```python
# Basic while
i = 0
selagi i < 5:
    cetak(i)
    i = i + 1

# While dengan kondisi kompleks
x = 10
selagi x > 0 dan x != 5:
    cetak(x)
    x = x - 1

# Infinite loop (hati-hati!)
# selagi benar:
#     cetak("Loop tak terbatas")
```

### For Loop (Untuk...dalam) - Coming in v0.2
```python
# Akan tersedia di versi 0.2
# untuk i dalam range(10):
#     cetak(i)
```

## Fungsi

### Definisi Fungsi
```python
# Fungsi tanpa parameter
fungsi sapa():
    cetak("Halo!")

# Fungsi dengan parameter
fungsi sapa_nama(nama):
    cetak("Halo,", nama)

# Fungsi dengan banyak parameter
fungsi tambah(a, b):
    kembalikan a + b

# Fungsi dengan return
fungsi kali(x, y):
    hasil = x * y
    kembalikan hasil
```

### Memanggil Fungsi
```python
# Tanpa parameter
sapa()

# Dengan parameter
sapa_nama("Budi")

# Dengan return value
hasil = tambah(5, 3)
cetak(hasil)  # 8

# Langsung di print
cetak(kali(4, 7))  # 28
```

### Fungsi Rekursif
```python
# Faktorial
fungsi faktorial(n):
    jika n <= 1:
        kembalikan 1
    selain:
        kembalikan n * faktorial(n - 1)

cetak(faktorial(5))  # 120

# Fibonacci
fungsi fib(n):
    jika n <= 1:
        kembalikan n
    kembalikan fib(n - 1) + fib(n - 2)
```

### Scope
```python
x = 10  # Global

fungsi test():
    y = 20  # Lokal
    cetak(x)  # Bisa akses global
    cetak(y)  # Bisa akses lokal

test()
# cetak(y)  # Error: y tidak terdefinisi di scope global
```

## Cetak (Print)

### Basic Print
```python
cetak("Halo")
cetak(42)
cetak(3.14)
cetak(benar)
cetak(kosong)
```

### Multiple Arguments
```python
nama = "Budi"
umur = 25

cetak("Nama:", nama)
cetak("Umur:", umur)
cetak("Nama:", nama, "Umur:", umur)
```

### Ekspresi dalam Print
```python
x = 10
y = 5

cetak(x + y)
cetak("Hasil:", x * y)
cetak("x =", x, "y =", y, "x+y =", x+y)
```

## Kata Kunci

### Daftar Lengkap Kata Kunci (v0.1)
```
benar       - Boolean True
salah       - Boolean False
kosong      - None/Null
dan         - Logical AND
atau        - Logical OR
tidak       - Logical NOT
jika        - If statement
jika_lain   - Elif statement
selain      - Else statement
selagi      - While loop
untuk       - For loop (v0.2+)
dalam       - In operator (v0.2+)
fungsi      - Function definition
kembalikan  - Return statement
cetak       - Print function
kelas       - Class (v0.3+)
diri        - Self (v0.3+)
```

### Reserved untuk Versi Mendatang
```
hentikan    - Break
lanjut      - Continue
lewati      - Pass
impor       - Import
dari        - From
sebagai     - As
dengan      - With
coba        - Try
kecuali     - Except
akhirnya    - Finally
naikkan     - Raise
```

## Contoh Program Lengkap

### Program 1: FizzBuzz
```python
fungsi fizzbuzz(n):
    i = 1
    selagi i <= n:
        jika i % 15 == 0:
            cetak("FizzBuzz")
        jika_lain i % 3 == 0:
            cetak("Fizz")
        jika_lain i % 5 == 0:
            cetak("Buzz")
        selain:
            cetak(i)
        i = i + 1

fizzbuzz(20)
```

### Program 2: Menghitung Rata-rata
```python
fungsi hitung_rata_rata(a, b, c, d, e):
    jumlah = a + b + c + d + e
    rata = jumlah / 5
    kembalikan rata

nilai1 = 80
nilai2 = 85
nilai3 = 90
nilai4 = 75
nilai5 = 88

rata_rata = hitung_rata_rata(nilai1, nilai2, nilai3, nilai4, nilai5)
cetak("Rata-rata:", rata_rata)
```

### Program 3: Bilangan Perfect
```python
fungsi adalah_perfect(n):
    jika n <= 1:
        kembalikan salah
    
    jumlah = 1
    i = 2
    
    selagi i * i <= n:
        jika n % i == 0:
            jumlah = jumlah + i
            jika i != n / i:
                jumlah = jumlah + (n / i)
        i = i + 1
    
    kembalikan jumlah == n

# Cari bilangan perfect sampai 1000
angka = 1
selagi angka <= 1000:
    jika adalah_perfect(angka):
        cetak(angka, "adalah bilangan perfect")
    angka = angka + 1
```

## Batasan dan Limitasi (v0.1)

### Belum Didukung
- ❌ List, Array, Dictionary
- ❌ String operations (concatenation, indexing)
- ❌ File I/O
- ❌ Import modul
- ❌ Class dan OOP
- ❌ Exception handling
- ❌ Lambda/anonymous functions
- ❌ Comprehensions
- ❌ Decorators

### Akan Datang
- ⏳ For...in loops (v0.2)
- ⏳ List operations (v0.2)
- ⏳ Break dan Continue (v0.2)
- ⏳ Classes (v0.3)
- ⏳ Import system (v0.4)

## Best Practices

### 1. Naming Conventions
```python
# Baik
nama_variabel = 10
hitung_total = fungsi()

# Hindari
x1234 = 10
fungsi_yang_namanya_sangat_panjang_sekali = fungsi()
```

### 2. Indentasi
```python
# Konsisten gunakan 4 spasi atau tab
jika benar:
    cetak("Indentasi benar")
    jika benar:
        cetak("Nested indentasi benar")
```

### 3. Komentar
```python
# Gunakan komentar untuk menjelaskan kode kompleks
fungsi hitung_kompleks(n):
    # Inisialisasi variabel
    hasil = 0
    
    # Loop untuk menghitung
    i = 1
    selagi i <= n:
        hasil = hasil + i * i  # Tambah kuadrat
        i = i + 1
    
    kembalikan hasil
```

### 4. Fungsi Kecil dan Fokus
```python
# Baik - fungsi kecil dan fokus
fungsi hitung_luas(panjang, lebar):
    kembalikan panjang * lebar

fungsi hitung_keliling(panjang, lebar):
    kembalikan 2 * (panjang + lebar)

# Kurang baik - fungsi terlalu banyak tanggung jawab
fungsi hitung_semua(panjang, lebar):
    luas = panjang * lebar
    keliling = 2 * (panjang + lebar)
    cetak("Luas:", luas)
    cetak("Keliling:", keliling)
    # ... terlalu banyak hal
```

---

Untuk informasi lebih lanjut, lihat README.md atau dokumentasi lengkap.
