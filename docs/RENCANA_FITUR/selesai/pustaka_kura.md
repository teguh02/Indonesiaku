# Rencana Fitur: `kura` (turtle) dan trigonometri bawaan

- **Status:** selesai
- **Target versi:** 1.2.0 (Fase 6)
- **Pemilik:** kontributor Fase 6
- **Butir ROADMAP:** Fase 6 — `kura` (katalog 07 §17); trigonometri (katalog 07 §1).

## 1. Masalah

`turtle` adalah pintu masuk pemrograman di banyak sekolah, dan katalog 07
menargetkan `kura` dengan keluaran SVG. Menggambar kura butuh `sin`/`cos`,
padahal bahasa belum punya fungsi trigonometri sama sekali.

## 2. Sintaks / API

Builtin C: `sin cos tan asin acos atan atan2 radian derajat` (nama katalog
§1, sudut radian seperti `math`).

Modul `pustaka/kura.idk` (bernamespace, `impor kura`): fungsi modul yang
menggerakkan satu kura bawaan, dan kelas `Kura` untuk kura sendiri.

| Nama | turtle |
|---|---|
| `maju`, `mundur`, `kiri`, `kanan` | `forward`, `backward`, `left`, `right` |
| `ke(x, y)`, `hadap(d)`, `pulang()` | `goto`, `setheading`, `home` |
| `lingkaran(jari, busur=360, langkah=kosong)` | `circle` |
| `angkat_pena`, `turunkan_pena`, `warna`, `tebal`, `kecepatan` | `penup`, `pendown`, `pencolor`, `pensize`, `speed` |
| `posisi`, `arah`, `svg`, `selesai(berkas)`, `reset`, `svg_dari` | `position`, `heading`, —, `done`, `reset`, — |

## 3. Semantik

- Mode turtle "standard": mulai di (0, 0) menghadap timur, y ke atas,
  `kiri` berlawanan jarum jam, sudut dalam derajat.
- `lingkaran` memakai algoritma `turtle.circle` apa adanya (jumlah langkah
  `1 + int(min(11 + |r|/6, 59) * busur/360)`, setengah sudut di awal dan
  akhir), jadi titik-titiknya sama dengan Python.
- Tiap rangkaian gerak dengan pena turun dan atribut yang sama menjadi satu
  `<polyline>`; `angkat_pena`, `warna`, dan `tebal` memulai goresan baru.
  Sumbu y dibalik saat menulis SVG; `viewBox` dihitung dari batas gambar
  ditambah tepi.
- Angka dibulatkan 2 desimal (tanpa `.0`/`-0`): keluaran deterministik,
  sehingga bisa diuji golden.
- Warna dicek agar tidak memuat `"`, `<`, `&` (tidak bisa menyuntik atribut
  SVG). Argumen salah → `GalatKura`.
- `asin`/`acos` di luar [-1, 1] dan `sin`/`cos`/`tan` dari tak hingga →
  `GalatNilai`, seperti `ValueError` Python; argumen bukan angka → `GalatTipe`.

## 4. Berkas tersentuh

`src/native.c`, `src/native.h`, `src/vm.c` (trigonometri); `pustaka/kura.idk`;
`examples/trigonometri.idk`, `examples/pustaka_kura.idk` + golden; panduan
07 §1.1 dan §2.21; editor (BUILTINS dihasilkan ulang).

## 5. Rencana uji

- `trigonometri`: semua nilai dibandingkan dengan `math` Python di Windows
  (identik, kecuali `-0.0` yang dicetak `0.0` oleh formatter bahasa).
  `atan(pi)` berbeda 1 ulp antara glibc dan CRT Windows, jadi golden
  membulatkan ke 12 desimal agar sama di Linux, macOS, dan Windows.
- `pustaka_kura`: persegi, bintang, lingkaran (setiap titik berjarak tepat
  `jari` dari pusat, 20 langkah seperti Python), lingkaran negatif,
  pena/warna/beberapa kura, `selesai` menulis berkas, dan semua galat.

## 6. Di luar cakupan

Isian (`begin_fill`), teks (`write`), bentuk kura, dan kanvas beranimasi di
playground (butuh dukungan front-end) — ditunda. Fungsi hiperbolik dan
logaritma katalog §1 belum.
