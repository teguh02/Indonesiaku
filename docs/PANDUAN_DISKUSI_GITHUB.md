# Panduan GitHub Discussions — Indonesiaku

GitHub Discussions merupakan tempat untuk bertanya, bertukar ide, menunjukkan karya, dan mengikuti pengumuman resmi tentang Indonesiaku (Idk-Lang).

Halaman: https://github.com/teguh02/Indonesiaku-Lang/discussions

## Kategori

| Kategori | Tujuan | Template |
|---|---|---|
| Announcements | Pengumuman resmi maintainer, rilis dan perubahan penting | `.github/DISCUSSION_TEMPLATE/announcements.yml` |
| General | Percakapan komunitas dan topik umum | `.github/DISCUSSION_TEMPLATE/general.yml` |
| Ideas | Usulan fitur/pustaka dan masukan desain bahasa | `.github/DISCUSSION_TEMPLATE/ideas.yml` |
| Q&A | Pertanyaan penggunaan, troubleshooting, dan bantuan teknis | `.github/DISCUSSION_TEMPLATE/q-a.yml` |
| Show and tell | Berbagi proyek, demo, dan karya komunitas | `.github/DISCUSSION_TEMPLATE/show-and-tell.yml` |
| Polls | Jajak pendapat komunitas, bila dibuka oleh maintainer | Tidak didukung GitHub Discussion Forms |

Nama file harus cocok dengan **slug kategori** pada GitHub. Bila suatu form tidak muncul, buka kategori pada Discussions, pastikan slug sebenarnya, dan sesuaikan nama file. Pada tampilan awal repository, kategori-kategori di atas sudah terlihat; slug final tetap perlu diperiksa lewat pengaturan GitHub jika telah diubah.

## Aturan ringkas komunitas

1. Bersikap sopan, saling menghormati, dan fokus pada masalah teknis atau pengembangan komunitas.
2. Cari diskusi dan dokumentasi yang sudah ada sebelum membuat topik baru.
3. Gunakan **Q&A** untuk meminta bantuan; sertakan versi `indk`, OS, kode minimal, langkah reproduksi, dan pesan galat. Tandai jawaban yang menyelesaikan masalah.
4. Gunakan **Ideas** untuk pembahasan awal fitur; fitur yang diterima belum otomatis masuk rilis. Rujuk `docs/ROADMAP.md`.
5. Gunakan **Issues** untuk bug yang dapat direproduksi dan perlu dilacak sebagai pekerjaan, bukan untuk obrolan umum.
6. Jangan publikasikan password, access token, private key, data pribadi, atau log yang memuat rahasia.
7. Untuk keamanan, **jangan ungkap eksploit yang belum ditangani di diskusi publik**; gunakan pelaporan privat melalui GitHub Security Advisories bila tersedia atau kanal privat resmi maintainer yang dipublikasikan.
8. Pertahankan bahasa Indonesia sebagai bahasa utama; pertanyaan berbahasa Inggris tetap boleh selama sopan dan relevan.
9. Maintainer boleh memindahkan, mengunci, atau menutup diskusi yang duplikat, off-topic, spam, atau melanggar aturan.

## Langkah pengaturan

1. Pastikan fitur Discussions aktif pada **Settings > General > Features > Discussions**.
2. Pada halaman Discussions, klik ikon edit di bagian **Categories** untuk melihat nama, slug, dan format kategori. Pastikan Q&A memakai format **Question/Answer** supaya jawaban dapat diterima.
3. Form YAML pada `.github/DISCUSSION_TEMPLATE/` harus berada di branch default `main`; setiap nama file cocok dengan slug kategori.
4. GitHub tidak mendukung form kustom untuk kategori **Polls**.
5. Buka **New discussion** dan uji General, Ideas, Q&A, Show and tell, serta Announcements untuk memastikan field template tampil.
6. Disarankan membatasi pengumuman resmi hanya pada maintainer menurut hak akses kategori yang disediakan GitHub.
7. Buat diskusi sambutan di General lalu **pin**; buat pengumuman rilis terbaru di Announcements dengan tautan ke GitHub Releases.
8. Gunakan diskusi sebagai tempat percakapan; perubahan kode tetap melalui PR dan bug melalui Issues.

## Draft diskusi sambutan

**Kategori:** General

**Judul:** Selamat datang di komunitas Indonesiaku (Idk-Lang)!

Selamat datang! Indonesiaku adalah bahasa pemrograman open-source dengan sintaks Bahasa Indonesia dan runtime VM berbasis bytecode.

Ruang diskusi ini ditujukan untuk saling membantu belajar Indonesiaku, mendiskusikan pengembangan pustaka dan ekosistem, serta berbagi proyek nyata.

Mulai dari sini:
- Baca [README dan instalasi](../README.md).
- Pelajari [roadmap pengembangan](ROADMAP.md).
- Pilih **Q&A** jika mengalami kendala, **Ideas** jika ingin mengusulkan fitur, dan **Show and tell** untuk berbagi karya.
- Ikuti **Announcements** untuk kabar rilis resmi.

Perkenalkan diri atau ceritakan proyek apa yang ingin Anda buat menggunakan Indonesiaku. Terima kasih sudah bergabung!

## Draft pengumuman rilis

**Kategori:** Announcements

**Judul:** Indonesiaku v1.3.2 tersedia

Versi v1.3.2 dirilis untuk memperbaiki kompatibilitas artefak Linux pada CPU yang tidak mendukung penanda ISA lebih tinggi.

Detail dan unduhan resmi: https://github.com/teguh02/Indonesiaku-Lang/releases/tag/v1.3.2

Pastikan memeriksa rilis terbaru sebelum memposting ulang pengumuman ini.

## Pemeliharaan

- Ketika kategori diganti namanya, periksa apakah slug berubah; bila berubah, ubah nama file form.
- Sesuaikan contoh versi dan tautan dengan rilis terbaru.
- Perbarui pedoman dan template bila komunitas memerlukan field baru.
