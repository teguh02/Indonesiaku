# Rencana Fitur: `pustaka/` ditemukan dari lokasi interpreter

- **Status:** selesai
- **Target versi:** 1.x (bersama Fase 6)
- **Pemilik:** teguh02
- **Keputusan terkait:** Fase 3 (sistem modul), Fase 6 (modul pustaka bernamespace)

## 1. Masalah

Pencarian modul standar memakai `pustaka/` **relatif terhadap direktori
kerja**. Setelah dipasang, modul Fase 6 hanya bisa diimpor dari folder yang
kebetulan berisi `pustaka/`:

```indonesiaku-takjalan
# dijalankan dari C:\Users\budi\proyek
impor pola        # Modul 'pola' tidak ditemukan.
```

Lebih buruk lagi:

- **Installer Windows sama sekali tidak memasang `pustaka/`** (hanya
  `bin\`, dokumentasi, dan contoh).
- **Image Docker** meletakkan `indk` di `/usr/local/bin` dan pustaka di
  `/opt/indonesiaku/pustaka`; impor hanya jalan selama direktori kerja tetap
  `WORKDIR` bawaan, gagal untuk `docker run -w /app ...`.
- Bentuk lama `impor "pustaka/json.idk"` hanya dicari relatif direktori
  kerja.

## 2. Keputusan

Satu daftar **akar pustaka**, dipakai kedua bentuk impor, dalam urutan:

1. `pustaka/` di direktori kerja (perilaku lama, tetap pertama);
2. setiap direktori di `INDK_PATH`;
3. relatif terhadap interpreter yang sedang berjalan (`jalur_indk()`):
   `<dir exe>/pustaka` (build di repo: `indk` di akar repo),
   `<dir exe>/../pustaka` (installer Windows: `{app}\bin` + `{app}\pustaka`),
   `<dir exe>/../share/indonesiaku/pustaka` (konvensi Unix `PREFIX/bin` +
   `PREFIX/share`).

- **Impor bernama** (`impor pola`): direktori berkas pengimpor →
  `INDK_PATH` → `pustaka/` direktori kerja → akar relatif interpreter. Tiga
  langkah pertama tidak berubah, jadi tidak ada program yang perilakunya
  berubah; hanya impor yang tadinya gagal yang kini berhasil.
- **Impor teks** (`impor "pustaka/x.idk"`): bila jalur itu tidak ada apa
  adanya dan diawali `pustaka/` (atau `pustaka\`), sisanya dicari di setiap
  akar `INDK_PATH` dan relatif interpreter.
- Lokasi interpreter dihitung sekali (`GetModuleFileNameW`,
  `/proc/self/exe`, `_NSGetExecutablePath`, `argv[0]`) lewat helper yang sama
  dengan `jalur_indk()` di `src/sistem.c`, bukan disalin.
- **Docker:** `ENV INDK_PATH=/opt/indonesiaku/pustaka` -- memakai mekanisme
  yang sudah ada, tanpa memindahkan berkas yang dipakai `server.py`.
- **Installer Windows:** memasang `pustaka\*` ke `{app}\pustaka`.

Keterbatasan yang dicatat: di Windows berkas dibuka dengan `fopen` (kode
halaman ANSI), jadi jalur instalasi yang memuat karakter non-ASCII belum
didukung -- sudah begitu untuk semua impor sebelum perubahan ini.

## 3. Pengujian

- `tests/test_impor_lokasi.sh` / `.ps1`, dijalankan `test_all`: menyalin
  `indk` ke direktori sementara dalam tiga tata letak (sebelah `pustaka/`,
  `bin/` + `pustaka/`, `bin/` + `share/indonesiaku/pustaka/`), lalu
  menjalankan program dari direktori kerja LAIN tanpa `INDK_PATH`, yang
  mengimpor modul bernama, modul bersarang, dan bentuk teks
  `impor "pustaka/teks.idk"`. Juga memastikan modul lokal tetap menang atas
  pustaka (urutan lama).
- Image Docker dibangun dan dijalankan dengan `-w` di luar `/opt/indonesiaku`.
- Seluruh suite Windows/Linux + `DEBUG_STRESS_GC` + ASan.

## 4. Dampak dokumentasi

- [x] `website/panduan/02_INSTALASI.md` (lokasi `pustaka/`), bagian modul di
      `03_PANDUAN_SINTAKS.md` (urutan pencarian), `docker/images/README.md`
- [x] `CHANGELOG.md`, `AGENTS.md`
- [x] Berkas rencana ini → `selesai/`

Hasil: dengan interpreter lama ketiga tata letak di
`tests/test_impor_lokasi.sh` gagal ("Modul 'pola' tidak ditemukan"); dengan
perubahan ini semuanya lulus, kontrol negatif tetap gagal. Image Docker rilis
dan playground dibangun dan diuji: impor dari `-w /app`, `pustaka/` read-only
untuk user `runner`, dan pola `server.py` (symlink + `--sandbox`) berjalan.
Pengubahan `ENV INDK_PATH` di Docker ditolak karena `INDK_PATH` mendahului
`pustaka/` direktori kerja, sehingga akan menutupi pustaka milik proyek.
