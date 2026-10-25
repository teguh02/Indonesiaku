# 01 — Kondisi Indonesiaku Saat Ini (audit v1.0.0)

> Tanggal audit: 2026-10-24. Basis: kode di `src/`, `pustaka/`, `examples/`,
> 51 tes golden (semua lulus), plus ±80 skrip uji coba tambahan yang ditulis
> khusus untuk audit ini untuk membandingkan perilaku dengan Python 3.

## 1. Ringkasan satu paragraf

Indonesiaku sudah melewati fase "mainan". Fondasinya benar secara arsitektur
(VM bytecode ala *Crafting Interpreters*, GC mark-sweep, closure, kelas,
pengecualian, modul, I/O berkas, soket, JSON, HTTP) dan cukup cepat
(`fib(27)` ±0,14 detik, setara CPython). **Tetapi bahasanya belum stabil
untuk pemula**: ada bug yang menghasilkan jawaban salah tanpa pesan error
(`//`, argumen bernama), batas rekursi hanya 64, banyak error tidak bisa
ditangkap `coba/kecuali`, dan beberapa kata kunci sudah diiklankan di
README tetapi belum ada. Sebelum menambah fitur besar, prioritas pertama
adalah **Fase 0 — Stabilisasi** (lihat [`ROADMAP.md`](../ROADMAP.md)).

## 2. Arsitektur yang sudah ada

```
.idk ─► scanner.c ─► compiler.c (Pratt, satu lintasan, tanpa AST) ─► Chunk (bytecode)
                                                                      │
                     native.c (±55 builtin) ◄── vm.c (stack VM, 47 opcode) ◄┘
                                                 │
                                    memory.c (GC mark-sweep), table.c (hash FNV-1a)
```

| Komponen | Ukuran | Catatan |
|---|---|---|
| `compiler.c` | 1.308 baris | Parser + codegen sekaligus; tidak ada AST |
| `vm.c` | 959 baris | Loop interpreter `switch`, impor modul, registrasi native |
| `native.c` | 1.227 baris | Semua builtin, termasuk berkas & soket |
| `pustaka/*.idk` | 6 modul | Ditulis dalam Indonesiaku sendiri (bagus — "dogfooding") |
| Total C | ±5.600 baris | Masih kecil dan mudah dipahami kontributor baru |

**Representasi nilai:** `struct { type; union { bool; double; Obj* } }` — 16
byte per nilai. Hanya ada satu tipe angka (`double`). Objek heap:
`string, function, native, closure, upvalue, list, dict, class, instance,
bound_method, file, socket`.

**Infrastruktur yang sudah bagus (pertahankan):**

- Tes golden lintas OS + CI Linux/macOS/Windows.
- Job CI ASan/UBSan dengan `DEBUG_STRESS_GC`, Valgrind, coverage, fuzzing
  front-end (libFuzzer). Ini level kebersihan yang jarang ada di proyek bahasa
  hobi.
- Pesan error kompilasi dan runtime sudah memakai nomor baris + jejak
  panggilan (`[baris N] dalam f()`). *Catatan: `ROADMAP.md` lama masih
  menyebut "pesan error dengan nomor baris" sebagai fitur belum ada — itu sudah
  selesai.*
- REPL multi-baris, installer Windows, image Docker, situs + playground.

## 3. Fitur yang berfungsi (terverifikasi)

| Area | Status | Bukti |
|---|---|---|
| `cetak(...)` | ✅ | Wajib pakai kurung (seperti Python 3) |
| Variabel global/lokal (`variabel`) | ✅ | `examples/variabel.idk`, `lingkup_variabel.idk` |
| Aritmetika `+ - * / % **`, majemuk `+= -= *= /= %= **=` | ✅ (dengan catatan §4) | `complex_arithmetic.idk` |
| Perbandingan, `dan`/`atau`/`tidak`, `!` | ✅ | probe `24_bool_ops`, `25_ne` |
| `jika` / `jika_lain` / `selain` | ✅ | `percabangan.idk` |
| `selagi`, `untuk x dalam list`, `hentikan`, `lanjut` | ✅ | `hentikan_lanjut.idk` |
| Fungsi, rekursi, closure, fungsi sebagai nilai | ✅ | `closure.idk`, `function_composition.idk` |
| Kelas, `init`, `diri`, pewarisan `<`, `super` | ✅ | `examples/oop/*` |
| `naikkan` / `coba` / `kecuali` (bersarang, lempar ulang) | ✅ | `coba_kecuali.idk` |
| `impor "berkas.idk"` (di-cache) | ✅ | `impor.idk` |
| List `[...]`, indeks, kamus `kamus()` + `d["k"]` | ✅ | `list.idk`, `kamus.idk` |
| String: escape, indeks baca, `<`/`>` leksikografis, Unicode dicetak benar | ✅ | `indeks_string.idk` |
| Berkas, waktu/tanggal, soket TCP, JSON, HTTP | ✅ | `berkas.idk`, `todo_server.idk` |
| Kinerja dasar | ✅ | loop 1 juta iterasi < 0,2 dtk; list 200.000 elemen OK |

## 4. Bug yang ditemukan (urut berdasarkan bahaya)

"Diam" = menghasilkan jawaban salah **tanpa** pesan error. Ini paling
berbahaya bagi pemula karena mereka akan mengira logika merekalah yang salah.

| # | Bug | Contoh | Hasil sekarang | Seharusnya (Python) | Akar masalah |
|---|---|---|---|---|---|
| B1 | **`//` rusak (diam)** | `cetak(1 + 10 // 3)` | `13` | `4` | `binary()` di `compiler.c` tidak punya `case TOKEN_SLASH_SLASH` → tidak ada opcode dipancarkan; kedua operand tertinggal di stack (stack bocor) |
| B2 | **Argumen bernama jadi assignment global (diam)** | `f(b=1, a=5)` | `-4`, dan membuat global `a`, `b` | `4` | Assignment adalah ekspresi; `b=1` di dalam daftar argumen dikompilasi sebagai `b = 1` |
| B3 | **Batas rekursi 64** | `f(100)` rekursif | Crash "Call stack overflow" | Python: 1000 | `FRAMES_MAX 64` di `vm.h`; stack statis `64*256` |
| B4 | `**` asosiatif kiri | `2 ** 3 ** 2` | `64` | `512` | `parsePrecedence(prec + 1)` dipakai juga untuk `**` |
| B5 | Unary minus lebih kuat dari `**` | `-2 ** 2` | `4` | `-4` | Presedensi unary > power |
| B6 | `%` memakai `fmod` C | `-7 % 3` | `-1` | `2` | Python: hasil mengikuti tanda pembagi |
| B7 | `akhirnya` diam-diam salah parse | `coba {...} kecuali e {...} akhirnya {...}` | Runtime error "Variabel tidak terdefinisi 'akhirnya'" setelah blok jalan | blok finally | Token ada di scanner, tidak ada di compiler |
| B8 | Error internal tidak bisa ditangkap | `coba { cetak(a[5]) } kecuali e {...}` | Program mati (exit 70) | `IndexError` tertangkap | Hanya `naikkan` & `nativeRaise` yang dapat ditangkap; indeks, tipe, nama, panggil-bukan-fungsi memakai `runtimeError` |
| B9 | Bagi nol tidak error | `1 / 0` | `takhingga` | `ZeroDivisionError` | Aritmetika `double` mentah |
| B10 | Presisi cetak angka hilang | `cetak(2 ** 0.5)`, `0.1 + 0.2` | `1.4142135623731`, `0.3` | `1.4142135623730951`, `0.30000000000000004` | `numberToString` memakai presisi tetap, bukan "shortest round-trip" |
| B11 | `panjang` menghitung byte, bukan karakter | `panjang("héllo")` | `6` | `5` | String UTF-8 diperlakukan sebagai byte |
| B12 | `cetak(a, b)` mencetak per baris | `cetak(1, 2)` | `1\n2` | `1 2` | `OP_PRINT` per argumen + newline; `cetak()` kosong tidak mencetak baris |
| B13 | `x //= 3` tidak dikenal | | Compile error | | Tidak ada token `//=` |
| B14 | **Penugasan majemuk memotong sisi kanan (diam)** | `x = 10` lalu `x *= 2 + 3` | `20` | `50` | Sisi kanan diparse pada `PREC_FACTOR`, jadi `+ 3` dihitung terpisah lalu dibuang |
| B15 | **`tidak` terlalu kuat (diam)** | `tidak 1 == 2` | `salah` | `True` | `tidak` memakai `PREC_UNARY`, jadi terbaca `(tidak 1) == 2` |

| B16 | **Akhir baris tidak mengakhiri pernyataan** | `tidak b` lalu baris baru `(a atau b)` | Galat "Hanya dapat memanggil fungsi" — terbaca `b(a atau b)` | Dua pernyataan terpisah | Scanner membuang `\n`; parser menyambung baris yang diawali `(`, `[` atau `-` ke ekspresi sebelumnya. Perlu pernyataan yang diakhiri baris (di luar kurung) — Fase 1 |

B14 dan B15 ditemukan saat memperbaiki B1–B6. B16 ditemukan saat menguji
semua blok kode di dokumentasi; sementara ini ada catatan di panduan sintaks
dan `;` bisa dipakai sebagai pemisah.

**Status perbaikan (2026-10-24):** B1–B15 semuanya sudah diperbaiki dan
diberi tes golden (`examples/`) atau tes galat (`tests/galat/`). Akar masalah
B7 ternyata salah hitung panjang kata kunci `akhirnya` di `scanner.c`.
`kembalikan`/`hentikan`/`lanjut` yang keluar dari `coba` ber-`akhirnya` kini
menjalankan blok akhirnya. B16 dijadwalkan di Fase 1. Lihat
[`ROADMAP.md`](../ROADMAP.md).

## 5. Perbedaan semantik dengan Python (bukan bug, tapi keputusan yang belum dibuat)

| Hal | Indonesiaku sekarang | Python | Rekomendasi |
|---|---|---|---|
| Truthiness | ✅ Sudah mengikuti Python sejak Fase 1 (dulu `0`, `""`, `[]` = benar) | semuanya **salah** | Selesai (K1) |
| Tipe angka | Hanya `double` → `2**64` jadi `1.84467440737096e+19` | `int` presisi tak terbatas + `float` | Tambah `bulat` (int64 → bignum) K2 |
| `[1,2] == [1,2]` | `salah` (identitas) | `True` | Kesetaraan struktural K7 |
| Assignment di dalam fungsi | `x = 5` di fungsi membuat/ubah **global** | membuat **lokal** | Ikuti Python + `global`/`nonlokal` K3 |
| Kunci kamus | Hanya string | Semua nilai *hashable* | Hash untuk angka/bool/tupel K8 |
| Kunci kamus hilang | `kosong` | `KeyError` | Error + metode `.ambil(k, bawaan)` K6 |
| `untuk` atas string/kamus | Error "hanya dapat mengiterasi list" | Bisa | Protokol iterator K9 |
| Metode bawaan | `tambah(a, 4)`; `a.tambah(4)` error | `a.append(4)` | Metode pada tipe bawaan K10 |
| `"ab" * 3`, `[1] + [2]` | Error | Bisa | Tambah ke `OP_MULTIPLY`/`OP_ADD` |
| `1 < 2 < 3` | Error tipe | `True` | Rantai perbandingan |
| `jenis(obj)` | `"objek"` | `<class 'A'>` | Kembalikan kelasnya K11 |
| Blok | Kurung kurawal `{ }` | Indentasi | Keputusan terbesar — K4 |

## 6. Ketidaksesuaian dokumentasi vs implementasi

- `README.md` bagian "Kata Kunci Tambahan" mendaftar `lewati`, `dari`,
  `sebagai`, `dengan`, `akhirnya` — token-nya **ada di scanner** (jadi
  tidak bisa dipakai sebagai nama variabel) tapi **tidak ada di compiler**.
  Pengguna yang mencoba akan mendapat error membingungkan (lihat B7).
- `docs/ROADMAP.md` (sebelum audit ini) mencantumkan "pesan error dengan nomor
  baris" sebagai belum ada; sebenarnya sudah ada.
- `website/panduan/03_PANDUAN_SINTAKS.md` mengajarkan `**` tanpa menyebut
  asosiatifnya; `//` disebut di beberapa tempat padahal rusak.
- `scanner.h` masih mendeklarasikan `TOKEN_INDENT/DEDENT/SEMICOLON/COLON`
  yang tidak dipakai — sisa desain lama berbasis indentasi.

## 7. Kesimpulan audit

| Aspek | Nilai (1–5) | Komentar |
|---|---|---|
| Arsitektur inti | 4 | Pola clox solid; tanpa AST membatasi fitur lanjut (f-string, comprehension tetap mungkin) |
| Kebenaran (correctness) | 2 | B1, B2 menghasilkan jawaban salah diam-diam |
| Kesesuaian dengan Python | 2 | Sintaks dasar mirip; semantik (truthiness, int, scope) berbeda |
| Penanganan error | 3 | Nomor baris sudah ada; tapi kebanyakan error tidak bisa ditangkap |
| Pustaka standar | 3 | Cakupan bagus untuk ukurannya (JSON, HTTP, soket) |
| Infrastruktur uji & CI | 5 | Sanitizer, fuzz, valgrind, golden lintas OS |
| Dokumentasi | 3 | Banyak, tapi sebagian mengiklankan fitur yang belum ada |

Langkah berikutnya: [`02_PETA_FITUR_PYTHON.md`](02_PETA_FITUR_PYTHON.md)
untuk daftar lengkap fitur Python yang akan ditiru, dan
[`03_KEPUTUSAN_DESAIN.md`](03_KEPUTUSAN_DESAIN.md) untuk keputusan semantik
yang harus dikunci sebelum versi 1.0.
