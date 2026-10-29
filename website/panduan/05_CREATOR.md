# 5. Pembuat — Teguh Rijanandi

Dokumentasi tentang pembuat dan motivasi di balik Bahasa Pemrograman Indonesiaku.

---

## 👨‍💻 Profil

**Nama:** Teguh Rijanandi
**Peran:** Pembuat sekaligus pengembang utama Bahasa Pemrograman Indonesiaku
**Bidang:** Bahasa pemrograman, rancang bangun kompilator, arsitektur mesin virtual

---

## 🎯 Motivasi & Visi

### Mengapa Membuat Bahasa Pemrograman Baru?

Sebagai pengembang Indonesia, Teguh Rijanandi ingin membuat sesuatu yang berarti
di ranah bahasa pemrograman. Pertanyaan yang muncul:

> "Bisakah kita membuat bahasa pemrograman yang mudah dipelajari dan memakai
> sintaks dalam Bahasa Indonesia?"

Dari pertanyaan itulah lahir **Indonesiaku**.

### Tujuan Proyek

1. **Keterjangkauan** — membuat pemrograman lebih mudah diakses pelajar Indonesia
2. **Pembelajaran** — memahami konsep mendalam rancang bangun kompilator dan mesin virtual
3. **Komunitas** — membangun komunitas di sekitar bahasa pemrograman lokal
4. **Inovasi** — menunjukkan bahwa Indonesia mampu membuat teknologi yang bermutu

### Filosofi Desain

```
"Sintaks yang jelas dan mudah dipahami adalah fondasi
 pembelajaran pemrograman yang efektif"
```

Desain Indonesiaku mengikuti prinsip-prinsip:

- **Sederhana, bukan dangkal** — mudah bagi pemula, tetap bertenaga bagi pengguna mahir
- **Mengutamakan Bahasa Indonesia** — kata kunci dan dokumentasi dalam Bahasa Indonesia
- **Bernilai didaktis** — dirancang untuk mengajarkan konsep pemrograman
- **Sumber terbuka** — siapa pun boleh berkontribusi dan belajar
- **Sadar kinerja** — dioptimalkan untuk kecepatan eksekusi
- **Mudah dikembangkan** — fitur baru gampang ditambahkan

---

## 🔬 Latar Belakang Teknis

### Pengalaman Implementasi

Teguh Rijanandi memiliki pengalaman dalam:

- **Rancang bangun kompilator** — analisis leksikal, penguraian, pembangkitan kode
- **Mesin virtual** — implementasi VM berbasis tumpukan dengan bytecode
- **Manajemen memori** — penanganan pengumpul sampah dan optimasi memori
- **Optimasi** — penyetelan kinerja dan pemrofilan
- **Sumber terbuka** — kontribusi ke berbagai proyek sumber terbuka

### Pembelajaran dari Proyek Lain

Desain Indonesiaku terinspirasi oleh:

- **Lua** — kesederhanaan dan keanggunan
- **Python** — sintaks yang mudah dibaca dan bernilai didaktis
- **Lisp** — konsep pemrograman fungsional
- **Crafting Interpreters** — panduan menyeluruh untuk implementasi bahasa

### Tumpukan Teknis

```
┌─────────────────────┐
│   Kode Indonesiaku  │
│   (berkas .idk)     │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│     Scanner         │ (analisis leksikal)
│   Tokenisasi        │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│     Compiler        │ (analisis sintaks)
│   Pembangkitan kode │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│   Chunk bytecode    │ (representasi antara)
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│   Mesin Virtual     │ (mesin eksekusi)
│   VM berbasis stack │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│     Keluaran        │
│   Hasil & data      │
└─────────────────────┘
```

---

## 🚀 Perjalanan Proyek

### Tahap 1: Pematangan Gagasan (2024)

```
- Curah gagasan bahasa pemrograman bersintaks Indonesia
- Penelusuran implementasi bahasa yang sudah ada
- Perencanaan arsitektur dan desain
- Keputusan: C11 + VM berbasis tumpukan
```

### Tahap 2: Implementasi Inti (awal 2025)

```
✅ Scanner — memecah masukan menjadi token
✅ Compiler — mengurai dan membangkitkan bytecode
✅ VM — menjalankan instruksi bytecode
✅ Tipe dasar — bilangan, teks, boolean
✅ Variabel — deklarasi dan penugasan
✅ Fungsi — definisi dan rekursi
✅ Manajemen memori — dasar pengumpulan sampah
```

### Tahap 3: Fitur Lanjutan (pertengahan 2025)

```
✅ Closure — dukungan fungsi bersarang
✅ Fungsi tingkat tinggi — fungsi sebagai argumen
✅ Struktur data majemuk — pengolahan banyak nilai
✅ Optimasi kinerja — perbaikan pembangkitan kode
```

### Tahap 4: Mutu & Pengujian

```
✅ Rangkaian uji menyeluruh
✅ Contoh lanjutan — algoritma yang lebih rumit
✅ Pemrograman berorientasi objek
✅ Dokumentasi — panduan lengkap berbahasa Indonesia
```

### Tahap 5: Pematangan Bahasa (v1.x)

```
✅ Kelas dan pewarisan
✅ Sistem modul
✅ Pustaka standar
✅ Anotasi tipe opsional
```

Status terbaru setiap fitur dicatat di `docs/ROADMAP.md` — berkas itulah
satu-satunya sumber kebenaran untuk peta jalan, bukan halaman ini.

---

## 💡 Capaian Utama

### Capaian Teknis

| Capaian | Keterangan |
|---------|------------|
| **VM berbasis tumpukan** | Mesin eksekusi bytecode yang ringkas |
| **Pengumpul sampah** | Manajemen memori otomatis (mark-sweep) |
| **Closure** | Dukungan lingkup leksikal |
| **Berorientasi objek** | Kelas, pewarisan, dan metode magis |
| **Sistem modul** | Setiap modul memiliki globalnya sendiri |
| **Pustaka standar** | Modul bawaan siap pakai |
| **Penanganan galat** | Pesan galat yang jelas berbahasa Indonesia |
| **Rangkaian uji** | Uji golden, uji galat, dan uji panduan |

### Dampak bagi Komunitas

- Proyek sumber terbuka yang bisa dijadikan rujukan
- Bahan belajar untuk perancangan bahasa
- Wadah berkontribusi bagi pengembang Indonesia
- Bukti bahwa Indonesia mampu berinovasi di bidang teknologi

### Hasil Pembelajaran

Melalui proyek ini, Teguh Rijanandi mempelajari:

1. **Pengetahuan kompilator mendalam** — dari tokenisasi hingga eksekusi
2. **Pemrograman sistem** — manajemen memori dan optimasi
3. **Rekayasa perangkat lunak** — pengelolaan proyek dan dokumentasi
4. **Pembinaan komunitas** — merangkul kontributor
5. **Komunikasi teknis** — menulis dokumentasi yang jernih

---

## 🎓 Nilai Pendidikan

### Untuk Pelajar dan Mahasiswa

Indonesiaku menyediakan:

- **Implementasi bahasa nyata** — melihat langsung cara kerja sebuah bahasa
- **Konsep kompilator** — belajar analisis leksikal, penguraian, pembangkitan kode
- **Arsitektur VM** — memahami bytecode dan mesin tumpukan
- **Latihan algoritma** — berlatih dengan bahasa pemrograman sungguhan
- **Belajar bersama** — berkontribusi sambil belajar dari orang lain

### Untuk Pengajar

Sumber daya yang dapat dimanfaatkan:

- **Kode sumber** — kode C yang rapi dan berkomentar
- **Contoh program** — dari yang sederhana hingga lanjutan
- **Dokumentasi** — panduan lengkap berbahasa Indonesia
- **Rangkaian uji** — contoh pengujian otomatis
- **Pola berorientasi objek** — kelas, pewarisan, dan metode magis

---

## 🤝 Filosofi Berkontribusi

Teguh Rijanandi percaya pada:

### Keterbukaan

```
"Tidak ada kontribusi yang terlalu kecil.
 Setiap perbaikan membuat proyek lebih baik."
```

- Menyambut pemula dan kontributor baru
- Mendampingi mereka yang sedang belajar
- Telaah kode yang sabar

### Mutu

```
"Mutu kode adalah investasi jangka panjang.
 Luangkan waktu untuk mengerjakannya dengan benar."
```

- Pengujian yang menyeluruh
- Dokumentasi yang jelas
- Keputusan desain yang dipikirkan matang

### Keberlanjutan

```
"Sumber terbuka bukan lari cepat, melainkan lari maraton.
 Fokus pada praktik yang berkelanjutan."
```

- Peta jalan yang jelas
- Target waktu yang realistis
- Perawatan yang rutin

---

## 📞 Kontak

Untuk menghubungi Teguh Rijanandi:

- **Surel:** teguhrijanandi02@gmail.com
- **GitHub:** [@teguh02](https://github.com/teguh02)

### Proyek

- **Repositori:** https://github.com/teguh02/Indonesiaku-Lang
- **Dokumentasi:** folder `website/panduan/`
- **Laporan masalah:** GitHub Issues pada repositori di atas

---

## 🎁 Ikut Terlibat

Ingin berkontribusi atau mendukung proyek?

### Cara Berkontribusi

1. **Kode** — kirim pull request berisi perbaikan
2. **Pengujian** — uji coba dan laporkan kutu yang ditemukan
3. **Dokumentasi** — perbaiki panduan dan contoh
4. **Masukan** — bagikan gagasan dan saran
5. **Sebarkan** — ceritakan Indonesiaku kepada teman!

### Cara Memulai

```bash
# 1. Klon repositori
git clone https://github.com/teguh02/Indonesiaku-Lang.git

# 2. Baca panduan kontribusi
cat website/panduan/04_KONTRIBUSI.md

# 3. Pilih sebuah isu atau rancang fitur baru
# 4. Kirim pull request

# 5. Bergabunglah dengan komunitas!
```

---

## 🏆 Penghargaan & Terima Kasih

Terima kasih kepada:

### Komunitas Sumber Terbuka

- Kontributor yang telah mengirim perbaikan
- Penguji yang menemukan dan melaporkan kutu
- Pengajar yang memakai Indonesiaku untuk mengajar
- Semua yang telah memberi masukan

### Sumber Inspirasi

- Penulis buku "Crafting Interpreters"
- Pencipta Lua, Python, dan bahasa lainnya
- Para peneliti di bidang perancangan bahasa pemrograman
- Komunitas teknologi Indonesia

### Ucapan Khusus

Kepada semua yang percaya pada visi Indonesiaku dan mendukung perjalanan ini!

---

## 🎯 Visi ke Depan

### Jangka Pendek (1 tahun)

- Memperluas pustaka standar
- Menumbuhkan komunitas
- Menambah contoh dan tutorial
- Memperluas pemakaian di kalangan pengajar

### Jangka Panjang (3–5 tahun)

- Beberapa target keluaran (JavaScript, Python)
- Lingkungan pengembangan berbasis web
- Pengelola paket
- Pustaka standar yang lebih luas
- Kurikulum pembelajaran
- Pemakaian di dunia industri

---

## 💭 Renungan Pribadi

### Mengapa Ini Penting

```
"Bahasa pemrograman adalah salah satu karya seni terindah dalam teknologi.
Setiap bahasa membawa filosofi dan cara berpikir tertentu.

Dengan membuat Indonesiaku, saya ingin menunjukkan bahwa Indonesia
bisa tidak hanya menggunakan teknologi, tapi juga menciptakannya.

Bahwa pemrograman bisa lebih terjangkau dalam bahasa ibu kita.
Bahwa belajar memrogram tidak harus dimulai dari bahasa Inggris yang rumit.

Semoga Indonesiaku bisa menginspirasi generasi pemrogram Indonesia
untuk tidak sekadar memakai teknologi, tetapi juga menciptakannya."
```

---

## 📚 Bahan Bacaan

Untuk mendalami perancangan bahasa pemrograman:

### Buku yang Disarankan

1. **Crafting Interpreters** — Robert Nystrom
   - Buku gratis daring: craftinginterpreters.com
   - Panduan menyeluruh untuk implementasi bahasa

2. **Language Implementation Patterns** — Terence Parr
   - Pola praktis untuk penguraian dan eksekusi

3. **Programming Language Pragmatics** — Michael Scott
   - Landasan akademis untuk konsep bahasa pemrograman

### Kursus Daring

- MIT OpenCourseWare — Compiler Design
- Udacity — Programming Languages
- Coursera — Compilers

### Komunitas

- r/compilers
- r/ProgrammingLanguages
- Hacker News
- Dev.to

---

**Untuk pertanyaan lebih lanjut atau ajakan berkolaborasi, silakan hubungi lewat
kanal yang tersedia. Selamat datang di komunitas Indonesiaku!**
