# Panduan Sponsorship — Indonesiaku (Idk-Lang)

Indonesiaku merupakan proyek bahasa pemrograman open-source untuk komunitas. Dukungan bersifat **sukarela**, bukan syarat untuk memakai kode, mengajukan fitur, atau berkontribusi.

## Status konfigurasi

- Pada **Settings → General → Features**, kotak centang **Sponsorships** sudah tampak aktif pada screenshot pengelola.
- File `.github/FUNDING.yml` tersedia sebagai **template nonaktif** sampai akun/URL pendanaan terverifikasi.
- Belum dikonfirmasi bahwa akun `teguh02` telah aktif menerima dana di GitHub Sponsors atau bahwa ada kanal donasi eksternal yang sah.
- **Jangan publikasikan tautan placeholder sebagai kanal pembayaran**; jangan menjanjikan tier/perk tanpa kemampuan pemenuhan.

## Mengaktifkan tombol Sponsor

1. Tentukan opsi pendanaan yang benar-benar dimiliki/dioperasikan proyek:
   - **GitHub Sponsors:** daftar/aktifkan profil sponsor pada akun yang benar-benar menerima dana. Pastikan profil dapat dibuka dan tier sudah tersedia jika diperlukan.
   - **Platform lain:** gunakan tautan donasi langsung yang aktif dari penyedia yang memenuhi syarat, lewat `custom` (maksimal 4 URL).
2. Edit `.github/FUNDING.yml` pada branch default `main`: aktifkan misalnya `github: teguh02` **hanya jika profil tersebut benar-benar aktif**, atau `custom: ["https://URL-VALID"]` dengan URL nyata. Hapus contoh yang tidak digunakan.
3. Commit perubahan. Fitur Sponsorships pada pengaturan repository harus tetap dicentang.
4. Kembali ke halaman utama repository dan pastikan tombol **Sponsor** muncul dan semua opsi mengarah ke akun penerima yang tepat.
5. Uji tampilan sebagai pengunjung yang tidak login bila memungkinkan; **jangan mengirim dana percobaan tanpa sengaja**.
6. Jika mengubah opsi pendanaan, perbarui berkas ini, README dan pemberitahuan komunitas.

Referensi resmi: https://docs.github.com/en/repositories/managing-your-repositorys-settings-and-features/customizing-your-repository/displaying-a-sponsor-button-in-your-repository

## Teks yang dapat dipakai di profil sponsor

**Ringkasan:**

Dukung pengembangan Indonesiaku (Idk-Lang), bahasa pemrograman open-source dengan sintaks Bahasa Indonesia dan VM berbasis bytecode. Dukungan membantu pemeliharaan, dokumentasi berbahasa Indonesia, pengujian lintas platform, pustaka standar, serta pengalaman belajar dan pengembangan aplikasi.

**Penggunaan dukungan:**

- Pengujian dan peningkatan kualitas compiler/VM.
- Dokumentasi, tutorial, dan referensi fungsi.
- Perbaikan keamanan dan kompatibilitas lintas sistem operasi.
- Pengembangan pustaka database, HTTP/REST API, SSR, dan otomasi server sesuai roadmap.
- Infrastruktur pendukung proyek apabila diperlukan.

**Prinsip:**

Dukungan bersifat opsional dan tidak membeli pengaruh terhadap prioritas teknis, review kontribusi, kebijakan keamanan, maupun penerimaan pull request. Fitur direncanakan dan dievaluasi secara terbuka melalui roadmap dan Discussions. Donasi tidak memberikan jaminan hasil atau tenggat pengembangan tertentu.

## Usulan tier (opsional, bukan harga yang ditetapkan)

- **Pendukung Komunitas:** apresiasi publik jika sponsor setuju.
- **Pendukung Dokumentasi:** apresiasi dan pembaruan progres publik.
- **Pendukung Ekosistem:** apresiasi dan laporan perkembangan terbuka.

Jika membuat tier berbayar, maintainer menetapkan harga, kelayakan, benefit, pajak/payout dan ketentuan platform secara mandiri di GitHub Sponsors. Hindari janji fitur eksklusif atau prioritas pengembangan yang bertentangan dengan tujuan komunitas.

## Keamanan

- Jangan menaruh nomor rekening, data pajak, dokumen identitas, access token, atau informasi payout pribadi di repository.
- Verifikasi domain penerima; waspadai tautan impersonasi/pengalihan.
- Data pajak dan rekening diisi hanya lewat halaman resmi penyedia pendanaan.
