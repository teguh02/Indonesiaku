# Rencana Fitur: Fase 6 — Pustaka standar, gelombang C (sistem, teks, jaringan)

- **Status:** dikerjakan (C1 `jalur` dan mesin `pola` C3 selesai diuji; registrasi menunggu `vm.c`)
- **Target versi:** 1.x (lanjutan Fase 6)
- **Pemilik:** kontributor Fase 6
- **Butir ROADMAP:** Fase 6 — `jalur`, `opsi`, `log`, `proses`, `pola`, klien HTTP.
- **Ref:** `ANALISIS/07_KATALOG_PUSTAKA_STANDAR.md` §6, §9, §14, §15

## 1. Masalah

Program Indonesiaku belum bisa melakukan hal-hal yang dilakukan hampir setiap
skrip Python sungguhan:

```indonesiaku-takjalan
impor jalur      # Modul 'jalur' tidak ditemukan.
cetak(jalur.gabung("data", "a.csv"))
```

- Tidak ada cara memecah/menggabung jalur berkas selain memotong teks manual.
- Argumen baris perintah hanya tersedia sebagai daftar mentah `argumen`.
- Tidak ada pencatatan (logging) terstruktur; satu-satunya keluaran adalah
  `cetak` ke stdout, padahal pesan diagnostik seharusnya ke stderr.
- Tidak bisa menjalankan program lain, membaca variabel lingkungan, atau
  melihat isi direktori.
- Tidak ada ekspresi reguler, jadi validasi dan ekstraksi teks ditulis dengan
  loop karakter.
- Soket hanya bisa *mendengar*; program tidak bisa mengambil halaman HTTP.

## 2. Keputusan desain (perlu persetujuan pemilik)

**D1 — Urutan pengerjaan mengikuti kebutuhan native.** Gelombang ini dibagi
supaya bagian tanpa C bisa jalan lebih dulu:

| Tahap | Modul | Native baru? |
|---|---|---|
| C1 | `jalur` (bagian teks) | Tidak — murni Indonesiaku |
| C2 | `opsi`, `log`, `proses`, sisa `jalur`/`os` | Ya — `src/sistem.c` baru |
| C3 | `pola` | Ya — `src/pola.c` baru (mesin regex) |
| C4 | klien HTTP (`http.ambil`) | Ya — `soket_sambung` |

**D2 — Native baru di BERKAS baru**, bukan di `native.c`: `src/sistem.c`
(stderr, lingkungan, direktori, proses) dan `src/pola.c` (regex). `native.c`
sudah 2.700+ baris, dan berkas terpisah menjaga satu modul C = satu berkas.
Akibatnya `Makefile` `SOURCES`, keempat daftar berkas eksplisit di
`.github/workflows/test.yml`, dan `AGENTS.md` ("All N src/*.c") wajib ikut
diperbarui.

**D3 — `jalur` memakai aturan POSIX (`/`) di semua platform, persis
`posixpath`.** Hasil identik di Windows, Linux, dan macOS sehingga golden test
sama di ketiga CI, dan Windows menerima `/` di hampir semua API. Hanya `/`
yang memisahkan; jalur bergaya Windows diubah dulu dengan
`jalur.dari_windows()`. Mencampur `\` sebagai pemisah di sebagian fungsi
ditolak: perilakunya tidak lagi bisa dibandingkan satu-satu dengan Python.
`jalur.pemisah` bernilai `"/"`.

**D4 — Mode `--sandbox` dan playground.** `proses`, `lingkungan`, isi
direktori, dan klien HTTP memberi kode akses ke sistem. Playground web
menjalankan kode dari siapa saja, jadi:

- `indk --sandbox berkas.idk` mematikan semua native itu; memanggilnya
  menghasilkan `GalatIzin` yang bisa ditangkap.
- `docker/playground/server.py` selalu menjalankan `indk --sandbox`.
- Ini lapisan tambahan, bukan pengganti: kontainer runner tetap read-only,
  tanpa jaringan keluar, `cap_drop ALL`.

**D5 — `log` menulis ke stderr secara bawaan**, seperti `logging` Python.
Butuh native `tulis_galat(teks)` (C2). Format bawaan `LEVEL:nama:pesan`
(sama dengan `logging.basicConfig()`), level `DEBUG/INFO/PERINGATAN/GALAT/
KRITIS`, ambang bawaan `PERINGATAN`.

**D6 — `pola` mendukung subset `re` yang bisa dijalankan tanpa backtracking
eksponensial** (gaya Pike VM, Russ Cox `re1`): literal, `.`, kelas `[...]`/
`[^...]`, `\d \w \s \D \W \S \b \B`, `^ $`, `* + ? {m,n}` dan versi malasnya,
grup `(...)`/`(?:...)`/`(?P<nama>...)`, alternasi `|`, bendera
`ABAIKAN_KAPITAL`, `MULTIBARIS`, `TITIK_SEMUA`. **Tidak** ada backreference
(`\1`) dan lookaround (`(?=...)`): keduanya memaksa backtracking dan membuka
celah ReDoS. Pola yang memakainya ditolak dengan `GalatPola` yang jelas.
Pencocokan per karakter (code point UTF-8), konsisten dengan K13.

**D6a — Catatan implementasi `pola` (ditemukan saat pengujian):**

- Kelas `\d \w \s`, `\b`, dan `ABAIKAN_KAPITAL` hanya ASCII, persis
  `re.ASCII`. Unicode penuh butuh tabel kategori; ditunda.
- Kuantifier posesif (`a?+`, Python 3.11+) ditolak seperti backreference:
  semantik atomiknya butuh backtracking.
- Pengulangan yang badannya bisa cocok kosong (`(a|)*`, `(\S?)+`) meniru
  sre: iterasi kosong diterima sekali lalu keluar dari loop (slot posisi
  tersembunyi + instruksi `I_PROGRESS`), dan bintang atas badan seperti itu
  dikompilasi sebagai DUA salinan bergantian supaya iterasi berurutan tidak
  berbagi pc (dedupe Pike VM akan membunuh iterasi kosong terakhir). Tanpa
  ini, isi grup -- dan kadang panjang cocokan -- berbeda dari Python.
- `\B` pada teks kosong cocok, mengikuti Python 3.14 (3.12 tidak).
- `kecuali pola.GalatPola` belum bisa ditulis: klausa `kecuali` hanya
  menerima nama polos. Pengguna menulis `dari pola impor GalatPola`.
  Mendukung nama bertitik adalah perubahan compiler terpisah.
- `jenis()` memberi `"angka"` untuk bulat dan desimal, jadi modul memeriksa
  bulat dengan `ke_bulat(x) adalah x`.

**Bukti uji `pola`:** 240.000 kasus fuzz acak mesin (search/match/fullmatch/
finditer termasuk semua rentang grup) identik dengan `re` Python 3.14;
9.000 keluaran API modul (`cari_semua`, `ganti`, `ganti_n` dengan fungsi,
`pisah`, `cari`+`grup_semua`+`kamus_grup`) identik; ASan + UBSan +
`DEBUG_STRESS_GC` bersih; `(a*)*b` atas 30.000 karakter selesai ±0,05 dtk.

**D7 — Klien HTTP hanya `http://`.** HTTPS butuh TLS (mbedTLS) dan tetap P3,
seperti di katalog. `http.ambil("https://...")` → `GalatJaringan` yang
menyebut alasannya.

## 3. Sintaks usulan

```indonesiaku-takjalan
impor jalur
cetak(jalur.gabung("data", "2026", "a.csv"))   # data/2026/a.csv
cetak(jalur.pisah_ekstensi("laporan.tar.gz"))   # ("laporan.tar", ".gz")

dari opsi impor PenguraiArgumen
p = PenguraiArgumen("hitung")
p.tambah_argumen("berkas")
p.tambah_argumen("--jumlah", bawaan=1, jenis_nilai="bulat")
hasil = p.urai(["data.txt", "--jumlah", "3"])
cetak(hasil.berkas, hasil.jumlah)               # data.txt 3

impor log
log.peringatan("disk hampir penuh")             # stderr: PERINGATAN:root:disk hampir penuh

impor pola
m = pola.cari("(\\d+)-(\\d+)", "tel 021-555")
cetak(m.grup(1), m.grup(2))                     # 021 555

impor proses
h = proses.jalankan(["git", "--version"])
cetak(h.kode_keluar, h.keluaran)
```

## 4. Semantik per modul

### `jalur` (C1 teks, C2 sistem berkas)

| Fungsi | Python | Catatan |
|---|---|---|
| `gabung(a, b, ...)` | `os.path.join` | bagian yang diawali `/` membuang bagian sebelumnya |
| `pisah(j)` | `os.path.split` | → tupel `(kepala, ekor)` |
| `pisah_ekstensi(j)` | `os.path.splitext` | titik di awal nama bukan ekstensi (`.bashrc`) |
| `nama_dasar(j)` / `nama_direktori(j)` | `basename` / `dirname` | |
| `normalisasi(j)` | `os.path.normpath` | buang `.`, selesaikan `..`, rapatkan `//` |
| `adalah_absolut(j)` | `isabs` | `/...` atau `C:/...`, `C:\...` |
| `ada(j)` | `exists` | memakai `ada_berkas` yang sudah ada |
| `adalah_berkas`, `adalah_direktori`, `ukuran`, `isi_direktori`, `direktori_kini`, `absolut` | | C2 (native) |

Tidak ada builtin bernama sama yang tertutupi.

### `opsi` (C2)

Dipindah dari C1: argparse **keluar** dari program setelah `-h` (kode 0) dan
pada galat pemakaian (kode 2, pesan ke stderr). Bahasa ini belum punya
`keluar()` (`henti()` adalah *sleep*) maupun penulis stderr, jadi `opsi`
menunggu `keluar` dan `tulis_galat` di `src/sistem.c` agar API-nya benar sejak
awal, bukan berubah kemudian.


`PenguraiArgumen(nama_program, deskripsi="")`; `tambah_argumen(nama,
bawaan=kosong, jenis_nilai="teks", bantuan="", wajib=salah, aksi="simpan")`.
Argumen posisi (`"berkas"`) dan opsi (`"--jumlah"`, singkatan `"-j"` lewat
parameter `pendek`). `aksi="benar"` untuk bendera tanpa nilai. `jenis_nilai`
`"teks"`/`"bulat"`/`"desimal"`. `urai(daftar=argumen)` mengembalikan objek
`Hasil` dengan atribut per argumen (nama tanpa `--`, `-` diganti `_`).
`-h`/`--bantuan` mencetak bantuan yang dibangkitkan otomatis. Galat
pemakaian (argumen tak dikenal, nilai hilang, bukan angka) → `GalatOpsi`
yang bisa ditangkap (Python keluar dengan kode 2; di sini diserahkan ke
program, dan `urai_atau_keluar()` meniru perilaku Python).

### `log` (C2)

`debug/info/peringatan/galat/kritis(pesan)`, `atur(ambang=, format=,
berkas=)` (`basicConfig`), `ambil_pencatat(nama)` → `Pencatat` dengan
metode yang sama. Format memakai kunci `{level}`, `{nama}`, `{pesan}`,
`{waktu}`. Ke berkas bila `berkas=` diberikan, selain itu stderr.

### `proses` (C2)

`jalankan(perintah, masukan="", periksa=salah)` → `Hasil` dengan
`kode_keluar`, `keluaran`, `galat`. `perintah` WAJIB daftar (`["ls", "-l"]`),
bukan teks — tidak ada shell, jadi tidak ada injeksi lewat `;`/`|`.
`keluaran(perintah)` = `check_output`. Dimatikan di `--sandbox` (D4).

### `pola` (C3)

Sesuai katalog: `kompilasi`, `cari`, `cocok`, `cocok_penuh`, `cari_semua`,
`iter_cari`, `ganti`, `ganti_n`, `pisah`, `lolos`; objek `Cocokan`
(`grup`, `grup_semua`, `rentang`, `awal`, `akhir`). Indeks dalam karakter,
bukan byte. Fungsi pengganti di `ganti` boleh berupa fungsi.

### Klien HTTP (C4)

`http.ambil(url, metode="GET", kepala=kamus(), badan="")` →
`Respons(status, kepala, badan)`. Native baru `soket_sambung(host, port)`
di samping soket yang ada; resolusi nama lewat `getaddrinfo`. Dimatikan di
`--sandbox`.

## 5. Rencana implementasi

| Bagian | Perubahan |
|---|---|
| `pustaka/jalur.idk`, `opsi.idk`, `log.idk`, `proses.idk`, `pola.idk` | modul bernamespace, tanpa awalan |
| `pustaka/http.idk` | tambah `ambil` dan kelas `Respons` |
| `src/sistem.c` + `sistem.h` (baru) | `keluar(kode)`, `tulis_galat`, `ambil_lingkungan`, `direktori_kini`, `isi_direktori`, `adalah_direktori`, `ukuran_berkas`, `jalankan_proses`, `soket_sambung`; cek sandbox |
| `src/pola.c` + `pola.h` (baru) | compiler regex → program Pike VM, eksekusi, objek hasil |
| `src/main.c` | bendera `--sandbox` |
| `src/vm.c` | `defineNative` untuk native baru; `GalatIzin`, `GalatPola`, `GalatJaringan` |
| `Makefile`, `.github/workflows/test.yml` (4 daftar berkas) | sumber baru |
| `docker/playground/server.py` | jalankan dengan `--sandbox` |

GC: native yang membangun daftar (`isi_direktori`, hasil `pola`) me-root
daftar hasil di stack sebelum mengisi, pola `OP_BUILD_LIST`.

## 6. Pengujian

- Golden per modul: `examples/pustaka_jalur.idk`, `pustaka_opsi.idk`,
  `pustaka_log.idk` (log ke berkas lalu dibaca balik, karena golden menuntut
  stderr kosong), `pustaka_proses.idk` (menjalankan `indk` sendiri, satu-
  satunya program yang pasti ada di ketiga CI), `pustaka_pola.idk`.
- `jalur` dan `pola` dibandingkan dengan `posixpath` / `re` Python lewat
  program kembaran.
- `tests/galat/`: argumen opsi tak dikenal (tanpa coba), `pola` dengan
  backreference, `proses` di bawah `--sandbox`.
- Klien HTTP: integrasi terhadap `examples/todo_server.idk` (server lokal
  yang sudah dijalankan CI `todo-http`), bukan internet.
- `-DDEBUG_STRESS_GC` + ASan/UBSan untuk semua native baru; fuzz untuk
  compiler regex (input pola acak tidak boleh crash/menggantung).

## 7. Dampak dokumentasi & tooling

- [ ] `website/panduan/07_PUSTAKA_STANDAR.md` — bagian per modul (blok kode
      dijalankan `tests/test_panduan.sh`)
- [ ] `editors/vscode-indonesiaku/` — builtin baru (daftar dibangkitkan dari
      `defineNative`)
- [ ] `README.md`, `CHANGELOG.md`, `docs/ROADMAP.md` (tabel Fase 6),
      `docs/ANALISIS/07_KATALOG_PUSTAKA_STANDAR.md` (kolom "Ada")
- [ ] `AGENTS.md` — berkas C baru, `--sandbox`, jebakan modul
- [ ] Berkas rencana ini → `selesai/`

## 8. Risiko & alternatif

- **Keamanan** adalah risiko terbesar (D4). Alternatif ditolak: tidak
  menyediakan `proses` sama sekali — skrip otomasi adalah salah satu alasan
  orang belajar bahasa ini.
- **Regex dalam Indonesiaku murni** ditolak: lambat, dan katalog sudah
  menetapkan C.
- **Embed PCRE2** ditolak untuk sekarang: ±100 rb baris, mendukung fitur yang
  justru sengaja ditolak (D6), dan menambah lisensi baru.
