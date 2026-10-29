# Rencana Fitur: paket native per-OS dan pemberitahuan pembaruan

- **Status:** selesai
- **Target versi:** 1.4.0
- **Roadmap:** pendukung rilis (bukan fase bahasa)

## 1. Masalah

**(a) Rilis hanya berisi arsip.** Pengguna harus mengunduh `.tar.gz`/`.zip`,
mengekstrak, lalu mengatur PATH sendiri. Tidak ada paket yang bisa dipasang.

**(b) Tidak ada jalur pembaruan.** Pengguna yang memasang v1.0.0 tidak punya
cara tahu bahwa 1.3.2 sudah terbit, dan menduga harus menghapus pasangan lama
sebelum memasang yang baru.

## 2. Temuan yang membatasi desain

Diverifikasi sebelum menulis rencana ini:

- **Biner telanjang tidak cukup.** `indk` sendirian menjalankan `cetak("x")`,
  tetapi `impor web` gagal: *"Modul 'web' tidak ditemukan"*. Pustaka standar
  harus ikut terpasang. Jadi "lampirkan .exe saja" tidak bisa berupa biner
  polos — harus paket yang memasang `pustaka/` juga.
- **`pustaka/http.idk` menolak HTTPS.** Pengecekan versi tidak bisa memakai
  klien HTTP sendiri. `proses.jalankan(["curl", ...])` berhasil menghubungi
  API GitHub (diuji: `tag=v1.3.2`, 4 aset terbaca).
- **Layout `.deb` yang benar sudah diuji:** `/usr/bin/indk` +
  `/usr/share/indonesiaku/pustaka` — `impor web` berhasil dari cwd mana pun,
  karena `findInInterpreterRoots` sudah mencari `<exe>/../share/indonesiaku/pustaka`.
- `compiler/indonesiaku_installer.iss` **basi**: `AppVersion=0.1.4`, handle
  GitHub `teguhriyan` (salah), biner `indonesiaku.exe` (sekarang `indk.exe`).
  Ditulis ulang, bukan dipakai apa adanya.

## 3. Yang dibuat

### (a) Paket native

| OS | Aset baru | Pasang | Upgrade |
|---|---|---|---|
| Linux Debian/Ubuntu | `indonesiaku_<versi>_amd64.deb` | `sudo dpkg -i` | `sudo dpkg -i` versi baru — **menimpa, tanpa uninstall** |
| Windows | `Indonesiaku-Setup-<versi>.exe` | klik dua kali | installer mendeteksi pasangan lama dan menimpanya |
| Linux/macOS lain | `.tar.gz` (tetap ada) | ekstrak | ekstrak |

`.tar.gz` **tidak dihapus**: Fedora/Arch/Alpine tidak memakai `.deb`, dan
macOS tanpa DMG bertanda tangan justru lebih mulus (DMG tak bertanda tangan
memicu Gatekeeper).

### (b) `indk --periksa-pembaruan`

Membandingkan `INDK_VERSION` dengan rilis terbaru di GitHub, lalu mencetak
perintah pemutakhiran **yang sesuai OS yang sedang berjalan**.

```
$ indk --periksa-pembaruan
Versi terpasang : 1.3.2
Versi terbaru   : 1.4.0

Pembaruan tersedia.

Cara memperbarui (Linux, paket .deb):
  curl -LO https://github.com/teguh02/Indonesiaku-Lang/releases/download/v1.4.0/indonesiaku_1.4.0_amd64.deb
  sudo dpkg -i indonesiaku_1.4.0_amd64.deb

Tidak perlu menghapus versi lama; paket baru menimpa yang lama.
```

## 4. Keputusan

- **Tidak ada pembaruan otomatis.** Mengunduh lalu mengganti binernya sendiri
  berarti: butuh hak root, harus memverifikasi tanda tangan, dan bisa
  meninggalkan pasangan setengah jalan bila mati listrik. Memberi tahu +
  mencetak perintah persis sudah menyelesaikan keluhan ("tidak perlu uninstall
  dulu") tanpa satu pun risiko itu.
- **Tidak ada pengecekan otomatis saat menjalankan skrip.** Setiap `indk x.idk`
  menghubungi jaringan = lambat, bocor privasi, dan gagal saat offline.
  Pengecekan hanya jalan bila diminta.
- **`curl` dipanggil, bukan ditulis klien HTTPS sendiri.** `http.idk` tidak
  punya TLS; menambah OpenSSL mengakhiri sifat biner yang hanya menaut
  libc/libm. `curl` ada di Windows 10+, macOS, dan hampir semua Linux.
  Bila tidak ada, pesannya menyebutkan hal itu dan memberi URL manual.
- **Perbandingan versi numerik per komponen**, bukan `!=` teks: `1.10.0`
  harus lebih baru dari `1.9.0`. Membandingkan teks menyatakan `1.9.0` lebih
  besar.

## 5. Berkas tersentuh

| Berkas | Perubahan |
|---|---|
| `.github/workflows/release.yml` | bangun `.deb`, bangun installer Windows, unggah keduanya |
| `packaging/deb/` | berkas `control` + skrip bangun |
| `packaging/windows/indonesiaku.iss` | skrip Inno Setup baru (yang lama basi) |
| `src/main.c` | opsi `--periksa-pembaruan` |
| `src/sistem.c` | `_versi_terbaru()` bila perlu |
| `pustaka/pembaruan.idk` | logika banding versi + pesan per OS |
| `tests/` | uji banding versi (murni, tanpa jaringan) |
| `website/panduan/02_INSTALASI.md` | cara pasang & perbarui |
| `README.md`, `CHANGELOG.md` | catatan |

## 6. Rencana uji

- Banding versi: `1.9.0 < 1.10.0`, `1.3.2 == 1.3.2`, `1.3.2 < 1.4.0`,
  `2.0.0 > 1.99.99`, awalan `v` diabaikan — **tanpa jaringan**, sebagai golden.
- `.deb` dibangun lalu diekstrak ke root palsu; `impor web` harus berhasil
  dari cwd lain.
- `.deb` versi lama dipasang, lalu versi baru dipasang di atasnya tanpa
  `dpkg -r` — harus sukses dan `indk -v` menunjukkan versi baru.
- Perintah pemutakhiran yang dicetak diuji benar-benar dapat dijalankan.

## 7. Yang TIDAK dikerjakan

- `.rpm` (tidak ada `rpmbuild` di runner; tambah bila ada permintaan).
- `.dmg` macOS — tanpa tanda tangan Apple memicu Gatekeeper, lebih buruk
  daripada `.tar.gz`.
- Pembaruan otomatis tanpa interaksi.
- Homebrew tap / APT repository sendiri (butuh infrastruktur terpisah).
