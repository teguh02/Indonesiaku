# Rencana Fitur: Fungsi anonim & ekspresi kondisional

- **Status:** selesai
- **Target versi:** 0.7.0 (Fase 1)
- **Pemilik:** Teguh
- **Peta fitur:** 02 §B (`lambda`, ekspresi kondisional)

## 1. Masalah

Dua lubang yang keduanya membuat kode jadi bertele-tele.

**(a) Tidak ada fungsi anonim.** Setiap fungsi harus diberi nama, walau dipakai
sekali sebagai argumen:

```indonesiaku
fungsi _bandingkan(x) { kembalikan x * -1 }
hasil = terurut(angka, _bandingkan)      # namanya cuma sampah
```

**(b) Tidak ada ekspresi kondisional.** Memilih satu dari dua nilai butuh empat
baris dan satu variabel yang harus dideklarasikan lebih dulu:

```indonesiaku
variabel label = ""
jika nilai >= 60 { label = "lulus" } jika_lain { label = "gagal" }
```

## 2. Sintaks usulan

Keduanya sudah ditetapkan di peta fitur 02 §B, jadi rencana ini tidak
mengarang sintaks baru:

```indonesiaku
# (a) fungsi anonim — sama persis dengan fungsi biasa, tanpa nama
variabel kuadrat = fungsi(x) { kembalikan x * x }
cetak(peta(fungsi(x) { kembalikan x * 2 }, [1, 2, 3]))

# (b) ekspresi kondisional — urutan Python: nilai, kondisi, alternatif
variabel label = "lulus" jika nilai >= 60 selain "gagal"
```

Tidak ada kata kunci baru: `fungsi`, `jika`, dan `selain` ketiganya sudah ada.
Yang berubah hanya **posisi** kemunculannya — dari pernyataan menjadi ekspresi.

## 3. Semantik

**Fungsi anonim**

- Identik dengan `fungsi nama(...) { ... }`, hanya tidak mengikat nama apa pun.
- Closure penuh: menangkap variabel sekitarnya lewat upvalue yang sudah ada.
- Nilai bawaan parameter (K15) dan argumen bernama (K16) tetap berlaku —
  jalurnya sama, `function()` dipakai apa adanya.
- Namanya di traceback: `<anonim>`, bukan `skrip`. Penting karena `name == NULL`
  sudah berarti "skrip" di `src/vm.c` dan `src/object.c`; memakai NULL akan
  membuat galat di dalam lambda tampak seolah berasal dari tingkat atas.
- Badan tetap `{ }` dan `kembalikan`. **Bukan** lambda satu-ekspresi gaya
  Python: identitas bahasa ini adalah blok kurung tanpa aturan indentasi,
  dan membuat bentuk kedua hanya untuk fungsi anonim akan memecah itu.

**Ekspresi kondisional**

- Urutan penilaian seperti Python: **kondisi dulu**, lalu hanya satu cabang
  yang dinilai. `mahal() jika salah selain murah()` tidak memanggil `mahal()`.
- Presedensnya paling rendah di antara operator — tepat di atas penugasan —
  sehingga `a atau b jika c selain d` berarti `(a atau b) jika c selain d`,
  sama seperti Python.
- `selain` wajib ada. Python juga mewajibkan `else`; tanpa itu nilainya tidak
  terdefinisi saat kondisi salah.

## 4. Rencana implementasi

| Bagian | Perubahan |
|---|---|
| `src/compiler.c` | `anonymousFunction()` sebagai aturan prefix `TOKEN_FUNGSI` |
| `src/compiler.c` | `conditional()` sebagai aturan infix `TOKEN_JIKA` |
| `src/compiler.c` | `PREC_CONDITIONAL` antara `PREC_ASSIGNMENT` dan `PREC_OR` |
| `src/compiler.c` | `initCompiler` menamai fungsi anonim `<anonim>` |

Tidak ada opcode baru, tidak ada perubahan VM, tidak ada token baru.

**(a)** `function(TYPE_FUNCTION)` sudah memancarkan `OP_CLOSURE` beserta
daftar upvalue-nya di akhir. Jadi aturan prefix cukup memanggilnya; nilainya
mendarat di stack seperti ekspresi lain. Satu-satunya penyesuaian: `function()`
mengharapkan `(` "setelah nama fungsi", dan `initCompiler` menyalin
`parser.previous` sebagai nama — untuk anonim `parser.previous` adalah kata
`fungsi` itu sendiri, maka penamaan diambil alih.

Konflik dengan pernyataan: `declaration()` mencocokkan `TOKEN_FUNGSI` lebih
dulu untuk `funDeclaration()`. Dibedakan dengan satu token lookahead memakai
`checkNext()` yang **sudah ada** (dipakai untuk `nama=` di daftar argumen):
`fungsi (` → ekspresi, `fungsi nama` → deklarasi.

**(b)** Aturan infix menerima operand kiri yang **sudah dipancarkan**,
sedangkan Python menilai kondisi lebih dulu. Menerima urutan terbalik bukan
sekadar nuansa — pola penjaga yang lumrah akan rusak:

```indonesiaku
x[0] jika panjang(x) > 0 selain "kosong"    # harus aman pada daftar kosong
```

Kalau nilai-benar dinilai duluan, baris itu galat indeks. Jadi urutannya wajib
benar. Caranya memakai pola yang **sudah ada** di `function()` untuk nilai
bawaan parameter: `saveScanner()` + rewind, ditambah memotong bytecode operand
kiri yang terlanjur dipancarkan (`currentChunk()->count = awal`).

Ditangani di `expression()`, bukan sebagai aturan infix — karena kondisional
berpresedens paling rendah, operand kirinya selalu dimulai di awal sebuah
ekspresi, dan semua jalur (kurung, argumen, elemen daftar) lewat `expression()`.

Urutannya:

1. parse operand kiri seperti biasa, catat `awal` scanner + offset chunk
2. lihat `jika` → potong bytecode kiri, konsumsi `jika`, parse kondisi
3. catat posisi scanner setelah kondisi (tepat di `selain`)
4. `OP_JUMP_IF_FALSE` → alt, `OP_POP`
5. rewind ke `awal`, parse ulang operand kiri (kini di cabang benar)
6. `OP_JUMP` → selesai; di `alt`: `OP_POP`, lalu rewind ke posisi (3),
   konsumsi `selain`, parse nilai-salah

Hasilnya kondisi dinilai pertama dan hanya satu cabang berjalan, sama seperti
Python.

Biayanya operand kiri **diparsing dua kali** saat kondisional dipakai. Bytecode
buangan dipotong, tetapi konstanta yang sempat ditambahkan tetap tinggal di
tabel konstanta — beberapa byte per kondisional, tidak ada efek runtime.
Ditandai `ponytail:` di kode.

## 5. Pengujian

Golden `examples/fungsi_anonim.idk` + `tests/expected/`:
- anonim disimpan di variabel, dipanggil langsung, dioper sebagai argumen
- closure menangkap variabel luar; dua closure dari satu pabrik saling bebas
- nilai bawaan + argumen bernama pada anonim
- rekursi lewat variabel yang menampungnya
- traceback galat di dalam anonim menyebut `<anonim>`, bukan `skrip`
- kondisional: bersarang, di dalam argumen, dengan `atau`/`dan`
- **bukti cabang tak terpilih tidak dinilai** (fungsi yang mencetak)
- **bukti urutan penilaian** nilai-benar vs kondisi, apa adanya

Tes galat: `fungsi` anonim tanpa `(`, kondisional tanpa `selain`.
Sanitizer ASan+UBSan+`DEBUG_STRESS_GC`: closure mengalokasi.

## 6. Dampak dokumentasi

- [x] `website/panduan/03_PANDUAN_SINTAKS.md` — seksi Fungsi Anonim + Ekspresi Kondisional (5 blok diuji jalan)
- [x] `README.md` + `CHANGELOG.md`
- [x] `docs/ROADMAP.md` — butir Fase 1 → ✅
- [x] `docs/ANALISIS/02_PETA_FITUR_PYTHON.md` — `lambda` + kondisional → ✅
- [x] `editors/vscode-indonesiaku/` — snippet `fungsianonim`, `jikaselain`
- [ ] `AGENTS.md` — tertunda: berkas terproteksi, perlu persetujuan pemilik repo

## 7. Risiko

- **Ambiguitas `fungsi`.** Satu token lookahead memisahkan keduanya; `fungsi (`
  di awal pernyataan menjadi ekspresi. Program lama tidak terpengaruh karena
  `fungsi (` sebelumnya memang galat sintaks.
- **`jika` menjadi infix.** Aturan infix hanya aktif ketika `jika` muncul di
  tengah ekspresi. Pernyataan `jika` dicocokkan di `statement()` sebelum
  ekspresi sempat dimulai, jadi tidak bertabrakan — tapi wajib diuji dengan
  `jika` bersarang dan `jika_lain`.

## 8. Hasil (2026-10-25)

```text
tests/test_all.sh                  110/110 PASSED
ASan+UBSan+DEBUG_STRESS_GC         bersih (400 closure + kondisional)
```

Dicatat dari pengerjaan:

- Perkiraan §4 bahwa kondisional bisa menerima urutan penilaian terbalik
  **dibatalkan sebelum ditulis**: pola `x[0] jika panjang(x) > 0 selain ...`
  akan galat indeks. Implementasinya memotong bytecode operand kiri lalu
  mem-parsing ulang di cabang benar, sehingga urutannya sama dengan Python.
  Dibuktikan dengan fungsi penjejak di golden.
- Ditangani di `expression()`, bukan sebagai aturan infix di tabel presedens:
  kondisional berpresedens terendah sehingga operand kirinya selalu berawal
  di awal ekspresi.
- Sempat dikira regresi: `jika ... { } jika_lain { }` gagal kompilasi. Setelah
  diuji pada commit sebelum perubahan, galatnya **identik** — `jika_lain`
  memang memerlukan kondisi (else-if), dan else polos adalah `selain`.
  Bukan regresi; pesan galatnya memang membingungkan dan itu tetap butir
  tersendiri di Fase 1 (traceback + saran).
