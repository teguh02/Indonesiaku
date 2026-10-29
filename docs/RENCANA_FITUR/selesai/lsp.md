# Rencana Fitur: Language Server Protocol (LSP)

- **Status:** selesai
- **Target versi:** 1.4.0
- **Pemilik:** Indonesiaku contributors

## 1. Masalah

Ekstensi VS Code saat ini menyediakan syntax highlighting, snippets,
completion statis, dan formatter `indk rapikan`, tetapi belum memberikan
diagnostics saat mengetik, hover dokumentasi, atau go-to-definition. Pengguna
harus menjalankan program untuk menemukan kesalahan sintaks.

LSP harus menganalisis teks dokumen tanpa mengeksekusi program pengguna dan
tetap stabil saat dokumen belum lengkap atau sedang diedit.

## 2. Sintaks usulan

Tidak ada sintaks bahasa baru. Fitur ini menambah `server.js` mandiri yang
dijalankan oleh host ekstensi VS Code dan integrasi ke
`editors/vscode-indonesiaku`; tidak menambah executable C `indk-lsp`.

## 3. Semantik

- Server menggunakan JSON-RPC 2.0 framing `Content-Length` melalui stdin/stdout.
- stdout hanya untuk pesan protokol; log diagnostik internal memakai stderr.
- Diagnostics sintaks berasal dari `indk --dis <file sementara>` dan warning
  variabel lokal tidak terpakai dari `indk periksa <file sementara>`; keduanya
  mengompilasi/analisis statis, tetapi tidak menjalankan program.
- Temp source ditempatkan di direktori dokumen agar resolusi `impor` relatif
  tetap mengikuti lokasi file; file sementara selalu dihapus.
- Diagnostics memetakan nomor baris/kolom compiler (1-based) ke rentang LSP
  (0-based UTF-16). Baris yang tidak dapat dipetakan dengan aman diabaikan,
  bukan menebak posisi yang menyesatkan.
- Completion berisi keyword, builtin, magic methods, dan simbol deklarasi yang
  ditemukan dalam dokumen yang sedang dibuka.
- Hover menampilkan ringkasan singkat untuk keyword/builtin yang diketahui.
- Go-to-definition mencakup deklarasi fungsi/kelas/variabel lokal yang dapat
  diidentifikasi dengan pemindaian sumber sederhana; impor lintas file tidak
  termasuk cakupan awal.
- Server tidak mengeksekusi source, tidak memulai server/socket pengguna, dan
  membatasi ukuran dokumen/proses analisis.

## 4. Rencana implementasi

| Bagian | Perubahan |
|---|---|
| `editors/vscode-indonesiaku/server.js` | JSON-RPC stdio, lifecycle dokumen, diagnostics, completion, hover, definition |
| `editors/vscode-indonesiaku/extension.js` | spawn/stop server, sambungkan dokumen `.idk` ke provider LSP |
| `editors/vscode-indonesiaku/package.json` | konfigurasi jalur `indk`, activation/capabilities, masukkan server ke paket |
| `editors/vscode-indonesiaku/test-lsp.js` | uji protokol tanpa membuka VS Code |
| `editors/vscode-indonesiaku/README.md` | instalasi, konfigurasi executable, fitur dan batasan LSP |
| `docs/ROADMAP.md` | tandai Fase 9 selesai hanya setelah acceptance criteria terpenuhi |
| `CHANGELOG.md` | catat kemampuan editor LSP |
| `AGENTS.md` | catat jebakan teknis LSP/compiler bila ditemukan |

## 5. Pengujian

- Uji protokol initialize/shutdown dan request-response dengan Content-Length;
  pastikan proses tidak mencampur log server ke stdout JSON-RPC.
- Uji diagnostics untuk sintaks valid/error, blok belum ditutup, Unicode sebelum
  lokasi error, dokumen `untitled`, dan warning lokal tak terpakai.
- Uji completion/hover/definition untuk keyword, builtin, simbol dokumen dan
  deklarasi fungsi lokal.
- Uji perubahan dokumen yang membatalkan hasil diagnostics usang, dan `keluar()`
  untuk memastikan server menganalisis source tanpa mengeksekusinya.
- Pastikan `indk --dis` dipakai dan source tidak dieksekusi (program yang
  memanggil `keluar()`, `input()`, loop tak hingga, atau file I/O tidak boleh
  berjalan ketika diagnostics diperbarui).
- Jalankan `node editors/vscode-indonesiaku/test-lsp.js`, tes formatter yang
  sudah ada, `powershell -ExecutionPolicy Bypass -File tests/test_all.ps1`,
  `bash tests/test_panduan.sh`, dan CI lintas platform.
- Periksa paket `.vsix` berisi `server.js` dan dapat menemukan executable dari
  konfigurasi `indonesiaku.jalurIndk`.

## 6. Dampak dokumentasi & tooling

- [x] `editors/vscode-indonesiaku/README.md`
- [x] `docs/ROADMAP.md` dan `CHANGELOG.md`
- [x] `AGENTS.md` jika ada jebakan baru
- [x] Berkas rencana ini: status → `selesai`, lalu pindahkan ke
      `docs/RENCANA_FITUR/selesai/`

## Hasil implementasi

- `editors/vscode-indonesiaku/server.js`: JSON-RPC stdio, compile-only
  diagnostics lewat `indk --dis`, completion, hover, definition lokal, debounce,
  timeout, dan pembatalan proses compile usang saat dokumen berubah.
- `editors/vscode-indonesiaku/extension.js`: lifecycle server dan provider VS
  Code; formatter yang sudah ada tetap dipertahankan.
- `editors/vscode-indonesiaku/test-lsp.js`: tes integrasi protokol termasuk
  UTF-8 byte column → UTF-16 position, dokumen untitled, dan bukti source tidak
  dieksekusi.
- CI job `editor-lsp` menjalankan tes LSP dan formatter pada Linux, macOS, dan
  Windows.

Go-to-definition versi ini dibatasi pada deklarasi dalam dokumen yang sama;
resolusi simbol lintas modul belum termasuk.

## 7. Risiko & alternatif

- Compiler single-pass mengeluarkan diagnostics melalui stderr dan tidak
  dirancang untuk pohon sintaks parsial. Server harus menoleransi hasil compile
  gagal, mengabaikan diagnostics yang posisinya tidak valid, dan tidak mengubah
  perilaku compiler normal.
- Pemindaian sederhana untuk go-to-definition bukan symbol database penuh;
  jangan mengklaim resolusi scope/import yang tidak diuji.
- Tidak menambahkan dependency npm LSP pada iterasi awal: implementasi JSON-RPC
  kecil memakai Node.js bawaan agar paket editor ringan dan mudah diuji. Jika
  kompleksitas protokol bertambah, evaluasi library resmi pada rencana terpisah.
