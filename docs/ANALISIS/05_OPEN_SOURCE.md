# 05 — Indonesiaku sebagai Proyek Open Source

Bahasa pemrograman hidup karena komunitasnya. Dokumen ini membandingkan
kondisi repo dengan praktik proyek bahasa open source yang sehat (Python,
Rust, Lua, Wren) dan mendaftar yang perlu ditambahkan.

## 1. Yang sudah ada

| Hal | Berkas | Catatan |
|---|---|---|
| Lisensi | `LICENSE` (MIT) | Pilihan baik: permisif, cocok untuk pendidikan dan industri |
| Panduan kontribusi | `CONTRIBUTING.md`, `website/panduan/04_KONTRIBUSI.md` | Periksa agar keduanya tidak saling bertentangan |
| CI lintas OS + sanitizer + fuzz | `.github/workflows/test.yml` | Sangat baik |
| Rilis otomatis | `.github/workflows/release.yml` | Tes dijalankan sebelum publikasi |
| Proses rencana fitur (setara RFC/PEP) | `docs/RENCANA_FITUR/TEMPLATE.md` | Dasar yang baik |
| Changelog | `CHANGELOG.md` | |
| Situs + panduan + playground | `website/` | |
| Distribusi | installer Windows, Docker Hub | Belum ada paket Linux/macOS |

## 2. Yang perlu ditambahkan

| Prioritas | Berkas/aksi | Kenapa |
|---|---|---|
| Tinggi | `SECURITY.md` | Riwayat commit menunjukkan insiden workflow berbahaya (GhostAction, `f34d6d9`). Proyek butuh jalur pelaporan kerentanan privat + kebijakan workflow |
| Tinggi | Proteksi cabang `main` + review wajib + `CODEOWNERS` untuk `.github/` | Mencegah workflow berbahaya masuk lagi |
| Tinggi | Pin action GitHub ke SHA, `permissions:` minimal per job | Pengerasan rantai pasok CI |
| Tinggi | `CODE_OF_CONDUCT.md` (Contributor Covenant, terjemahan Indonesia) | Syarat dasar komunitas terbuka |
| Sedang | `.github/ISSUE_TEMPLATE/` (laporan bug, usulan fitur, pertanyaan) + `PULL_REQUEST_TEMPLATE.md` | Laporan bug yang bisa direproduksi |
| Sedang | Label "good first issue" / "bantuan dibutuhkan" + daftar tugas pemula | Pintu masuk kontributor; katalog 06/07 berisi ratusan fungsi kecil yang cocok untuk pemula |
| Sedang | `docs/SPESIFIKASI.md` | Kontrak bahasa; dasar bila ada implementasi kedua |
| Sedang | GitHub Discussions / forum / grup Telegram-Discord | Tempat tanya jawab terpisah dari issue |
| Sedang | Paket: Homebrew, Scoop/winget, `.deb`, AUR | Instal satu perintah |
| Rendah | `GOVERNANCE.md` | Siapa memutuskan apa (awalnya BDFL = pemilik proyek; nanti tim inti) |
| Rendah | Sponsor (GitHub Sponsors/Open Collective/Saweria) | Keberlanjutan |

## 3. Proses keputusan bahasa (setara PEP)

Gunakan folder `docs/RENCANA_FITUR/` sebagai proses RFC resmi:

1. Usulan dibuka sebagai issue berlabel `usulan-bahasa`.
2. Bila layak, penulis membuat `RENCANA_FITUR/<nama>.md` dari template
   lewat PR. Diskusi terjadi di PR.
3. Pemilik/tim inti mengubah status → `disetujui` atau `ditolak`
   (yang ditolak tetap diarsipkan beserta alasannya — sangat berguna agar
   diskusi yang sama tidak berulang).
4. Implementasi di PR terpisah yang menautkan rencana.
5. Keputusan semantik besar (`03_KEPUTUSAN_DESAIN.md`) mengikuti jalur ini.

## 4. Kebijakan kualitas untuk PR

Sudah tertulis sebagian di `AGENTS.md`; jadikan aturan publik di
`CONTRIBUTING.md`:

- Setiap fitur/bug fix: tes golden baru (`examples/` + `tests/expected/`).
- Perubahan VM/GC: lulus `DEBUG_STRESS_GC` + ASan (sudah di CI).
- Perubahan bahasa: perbarui panduan di `website/panduan/` dan `CHANGELOG.md`.
- Pesan commit berawalan Indonesia (`Tambah:`, `Perbaiki:`, …).

## 5. Pendidikan sebagai pembeda

Indonesiaku paling berpeluang besar di **pendidikan** (SD–SMA, kursus
pemula), bukan menggantikan Python di industri. Implikasinya:

- Pesan error yang **mengajar** ("Mungkin maksudmu `panjang`?" — saran nama
  terdekat dengan jarak Levenshtein).
- Modul `kura` (turtle graphics) di playground web.
- Kurikulum/latihan bertingkat di situs, dengan pemeriksaan jawaban otomatis
  memakai playground.
- Kemampuan "ekspor ke Python": penerjemah sederhana Indonesiaku → Python
  agar siswa bisa melanjutkan ke Python sungguhan. Karena semantik
  diselaraskan dengan Python (03), penerjemah ini menjadi realistis.
