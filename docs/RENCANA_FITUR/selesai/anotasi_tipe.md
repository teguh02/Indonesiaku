# Rencana Fitur: Anotasi tipe opsional

- **Status:** selesai (dirilis sebagai anotasi yang di-parse lalu dibuang)
- **Target versi:** 1.3.0
- **Pemilik:** Teguh R

## 1. Masalah

Tidak ada cara menuliskan tipe yang diharapkan sebuah parameter, nilai
kembali, atau variabel. Dokumentasi tipe hanya hidup di komentar, sehingga
ikut basi saat kode berubah, dan editor tidak punya sumber informasi apa pun.

Hari ini keduanya ditolak mentah oleh parser:

```text
$ indk anot.idk
[baris 1, kolom 11] Kesalahan di ':': Harapkan ')' setelah parameter.
    fungsi f(x: bilangan) {
              ^

[baris 1, kolom 11] Kesalahan di ':': Harapkan ekspresi.
    variabel a: bilangan = 5
              ^
```

## 2. Sintaks usulan

```python
fungsi sapa(nama: teks, ulang: bilangan = 1) -> teks {
    kembalikan nama * ulang
}

variabel umur: bilangan = 30
variabel daftar: daftar = [1, 2, 3]
```

- **Tidak ada kata kunci baru.** `:` memakai `TOKEN_COLON` yang sudah ada
  (dipakai irisan `a[1:3]` dan literal kamus).
- `->` adalah token baru untuk tipe kembali.
- Nama tipe adalah **identifier biasa**, bukan kata kunci — `bilangan`,
  `teks`, `daftar`, `kamus`, `logika`, `kosong`, atau nama kelas pengguna.
  Ini penting: menjadikannya kata kunci akan merusak kode lama yang memakai
  `teks` sebagai nama variabel.
- Interaksi dengan `jika`/`selagi`/`untuk`/`kelas`: tidak ada. Anotasi hanya
  muncul di deklarasi parameter, deklarasi `variabel`, dan setelah `)` sebuah
  `fungsi`.

## 3. Semantik

**Anotasi tidak dieksekusi dan tidak memengaruhi jalannya program.** Ia
di-parse lalu dibuang; tidak ada opcode baru, tidak ada pemeriksaan saat
runtime. Program dengan dan tanpa anotasi harus menghasilkan bytecode yang
identik.

Alasan memilih "buang" daripada "periksa saat runtime":

- Pemeriksaan runtime mengubah semantik bahasa (program yang tadinya jalan
  jadi gagal) — itu perubahan besar yang menyalahi sifat "opsional".
- Biayanya nyata: satu pemeriksaan per panggilan per argumen.
- Python sebagai referensi juga tidak memeriksa anotasi saat runtime.

Konsekuensi yang harus diterima: `fungsi f(x: bilangan)` lalu `f("abc")`
tetap jalan. Pemeriksa statis (`indk periksa`) adalah tempat yang benar untuk
melaporkannya, dan itu **di luar lingkup rencana ini**.

- Tipe yang terlibat: tidak ada nilai runtime baru.
- Aturan scope: tidak berubah. `variabel` tetap membuat lokal.
- `kosong`, tipe salah, argumen kurang/berlebih: perilakunya persis seperti
  sekarang, karena anotasi tidak dilihat saat eksekusi.
- Bisa ditangkap `coba/kecuali`? Tidak relevan — tidak ada galat runtime baru.
  Anotasi yang salah bentuk adalah galat **kompilasi**, seperti sintaks lain.

## 4. Rencana implementasi

| Bagian | Perubahan |
|---|---|
| `src/scanner.c` | token `->` (`TOKEN_PANAH`). `:` sudah ada. |
| `src/scanner.h` | `TOKEN_PANAH` di enum |
| `src/compiler.c` | `parseAnotasiTipe()` yang mengonsumsi-dan-membuang; dipanggil dari parameter, `varDeclaration`, dan setelah `)` fungsi |
| `src/vm.c` | **tidak ada** |
| `src/native.c` | **tidak ada** |
| `src/value.c` / `src/object.c` | **tidak ada** |
| `src/debug.c` | **tidak ada** (tidak ada opcode baru) |

Catatan GC-safety: **tidak ada alokasi baru sama sekali**, karena anotasi
dibuang saat parsing. Ini keuntungan utama pendekatan ini.

Perangkap yang sudah diketahui:

- `-` dan `>` sudah menjadi token sendiri. `->` harus dikenali di scanner
  **sebelum** `-` tunggal, dengan pola `peek` seperti `==`/`!=`.
- `:` dalam literal kamus dan irisan **tidak boleh** ikut terbaca sebagai
  anotasi. Anotasi hanya dikonsumsi di tiga posisi di atas, bukan di
  sembarang ekspresi.
- `prescanFunctionBody` dan `skipBalancedBraces` menghitung kurung mentah;
  anotasi tidak menambah kurung sehingga keduanya aman — tapi wajib diuji,
  bukan diasumsikan.
- Argumen bawaan sudah ada (`b=2`). Urutannya menjadi
  `nama : tipe = bawaan`, dan parser harus menerima keempat kombinasi
  (polos / hanya tipe / hanya bawaan / keduanya).

## 5. Pengujian

- Golden: `examples/anotasi_tipe.idk` + `tests/expected/anotasi_tipe.out`.
- **Uji identitas bytecode**: `indk --dis` pada file beranotasi dan file
  identik tanpa anotasi harus menghasilkan keluaran yang sama persis
  (setelah nomor baris dinormalisasi). Ini bukti terkuat bahwa anotasi
  benar-benar tidak mengubah arti program — pola yang sama dipakai saat
  memvalidasi `indk rapikan`.
- Tes galat `tests/galat/`: `:` tanpa nama tipe, `->` tanpa tipe, anotasi di
  posisi yang tidak sah.
- Kasus batas wajib: `f(a: bilangan = 1)` (tipe + bawaan); `f(a, b: teks)`
  (sebagian beranotasi); `{"a": 1}` dan `d[1:3]` **tetap** bekerja; anotasi
  di dalam f-string; `fungsi() -> kosong`.
- Jalankan dengan `-DDEBUG_STRESS_GC` — keluaran harus identik.
- Sanitizer tidak wajib di sini (tidak ada alokasi baru), tapi tetap
  dijalankan sekali untuk memastikan scanner `->` tidak membaca lewat akhir
  buffer.

## 6. Dampak dokumentasi & tooling

- [ ] `website/panduan/03_PANDUAN_SINTAKS.md` — bagian baru anotasi tipe
- [ ] `editors/vscode-indonesiaku/syntaxes/indonesiaku.tmLanguage.json` —
      `->` dan posisi tipe
- [ ] `editors/vscode-indonesiaku/snippets/indonesiaku.code-snippets`
- [ ] `CHANGELOG.md`
- [ ] `docs/ROADMAP.md` — centang butir Fase 8
- [ ] `README.md` kalau contoh utamanya ikut berubah

Tidak ada builtin baru, jadi `07_PUSTAKA_STANDAR.md` dan daftar BUILTINS di
`extension.js` **tidak** tersentuh.

## 7. Sengaja tidak dikerjakan

- **Pemeriksaan tipe statis di `indk periksa`** — rencana terpisah. Butuh
  tabel tipe dan inferensi; mencampurnya ke sini membuat perubahan parser
  yang kecil jadi proyek besar.
- **Tipe generik** (`daftar[bilangan]`) — `[` setelah nama tipe bentrok
  dengan indexing. Butuh aturan disambiguasi tersendiri.
- **Union / opsional** (`bilangan | kosong`).
- **`kelas` dengan atribut beranotasi** — bisa menyusul setelah bentuk dasar
  terbukti stabil.
