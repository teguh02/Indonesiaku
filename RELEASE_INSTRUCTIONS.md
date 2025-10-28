# Instruksi Upload Release v0.1.1 ke GitHub

Berikut langkah-langkah untuk membuat release v0.1.1 di GitHub dan mengupload installer.

## Prasyarat

Pastikan Anda memiliki GitHub CLI (`gh`) terinstal. Verifikasi:

```powershell
gh --version
```

Jika belum terinstal, download dari: https://cli.github.com/

## Opsi 1: Menggunakan GitHub CLI (RECOMMENDED - Otomatis)

### Langkah 1: Buat Release dengan Installer

Jalankan perintah berikut di PowerShell di folder root repo:

```powershell
gh release create v0.1.1 `
  --title "Bahasa Pemrograman Indonesiaku v0.1.1" `
  --notes-file RELEASE_NOTES_v0.1.1_GITHUB.md `
  dist/Indonesiaku-Setup-v0.1.1.exe
```

Penjelasan:
- `gh release create v0.1.1` — membuat release dengan tag `v0.1.1`
- `--title "..."` — judul release
- `--notes-file RELEASE_NOTES_v0.1.1_GITHUB.md` — membaca deskripsi dari file (sudah dibuat)
- `dist/Indonesiaku-Setup-v0.1.1.exe` — attach installer sebagai asset

### Langkah 2: Verifikasi

Buka browser ke:
```
https://github.com/teguh02/Indonesiaku/releases/tag/v0.1.1
```

Cek bahwa:
- ✅ Release title terlihat
- ✅ Release notes (markdown) terformat dengan baik
- ✅ Installer `.exe` sudah ter-upload dan siap didownload

---

## Opsi 2: Manual Menggunakan Web GitHub (Jika CLI tidak tersedia)

### Langkah 1: Buka GitHub Releases

1. Buka: https://github.com/teguh02/Indonesiaku/releases
2. Klik **"Draft a new release"** (tombol hijau di sebelah kanan atas)

### Langkah 2: Isi Form Release

- **Tag version**: `v0.1.1`
- **Release title**: `Bahasa Pemrograman Indonesiaku v0.1.1`
- **Description**: Copas isi dari `RELEASE_NOTES_v0.1.1_GITHUB.md` (atau link ke file jika perlu)

### Langkah 3: Upload Installer

1. Scroll ke bagian **"Attach binaries..."**
2. Klik atau drag-and-drop file: `dist/Indonesiaku-Setup-v0.1.1.exe`
3. Tunggu upload selesai

### Langkah 4: Publikasikan

- Klik **"Publish release"** (bukan "Save as draft")
- Release sekarang live! ✅

---

## Opsi 3: Advanced - Gunakan Git Tag + GitHub Web (Manual Tag Push)

Jika ingin kontrol penuh atas process:

```powershell
# 1. Buat local tag
git tag -a v0.1.1 -m "Release v0.1.1: CLI flags & debug fixes"

# 2. Push tag ke remote
git push origin v0.1.1

# 3. Pergi ke GitHub web dan buat release dari tag
#    (Buka https://github.com/teguh02/Indonesiaku/releases)
```

---

## Verifikasi Setelah Upload

Periksa bahwa:

1. **Release tersedia publik**:
   - Buka https://github.com/teguh02/Indonesiaku/releases
   - Lihat `v0.1.1` di daftar releases

2. **Installer bisa didownload**:
   - Klik "Indonesiaku-Setup-v0.1.1.exe" di release page
   - File harus didownload dengan benar (1.91 MB)

3. **Release notes terformat baik**:
   - Cek markdown rendering (headings, code blocks, links)

---

## Troubleshooting

### Error: "Release already exists"
- Jika release v0.1.1 sudah ada, delete terlebih dahulu:
  ```powershell
  gh release delete v0.1.1 -y
  ```
- Atau gunakan `--draft` flag untuk membuat draft terlebih dahulu

### Error: "Tag v0.1.1 already exists"
- Jika git tag sudah ada:
  ```powershell
  git tag -d v0.1.1
  git push origin --delete v0.1.1
  ```
- Kemudian coba lagi

### Installer tidak muncul di release
- Pastikan path file benar: `dist/Indonesiaku-Setup-v0.1.1.exe`
- Cek file ada: `Test-Path dist/Indonesiaku-Setup-v0.1.1.exe`

---

## Langkah Selanjutnya (Optional)

Setelah release live:

1. **Update README.md** untuk menunjuk ke release terbaru
2. **Announce di social media** (Twitter, Dev.to, etc.)
3. **Mulai planning v0.2.0** dengan fitur-fitur baru

---

## Quick Reference - Copas & Jalankan

**Untuk pengguna yang sudah punya `gh` CLI:**

```powershell
# Berada di root repo (C:\Users\teguh\Works\myself\Indonesiaku)
gh release create v0.1.1 --title "Bahasa Pemrograman Indonesiaku v0.1.1" --notes-file RELEASE_NOTES_v0.1.1_GITHUB.md dist/Indonesiaku-Setup-v0.1.1.exe
```

Selesai! 🎉
