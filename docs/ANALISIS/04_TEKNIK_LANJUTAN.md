# 04 — Dari Dasar ke Level Tertinggi: Teknik Pembuatan Bahasa

Dokumen ini memetakan "tangga" teknik implementasi bahasa pemrograman, dari
yang sudah dimiliki Indonesiaku sampai yang dimiliki CPython, PyPy, dan VM
kelas industri (V8, LuaJIT). Untuk setiap anak tangga: apa itu, posisi
Indonesiaku, dan kapan layak dikerjakan. Urutan pengerjaan resminya ada di
[`ROADMAP.md`](../ROADMAP.md).

```
Level 1  Interpreter dasar        scanner, parser, evaluator               ✅
Level 2  VM bytecode              chunk, stack VM, closure, GC             ✅
Level 3  Bahasa lengkap           tipe kaya, pengecualian, modul, iterator 🟡 (fase 0–5)
Level 4  Ekosistem                stdlib, paket, perkakas, LSP, debugger   ❌ (fase 6–8)
Level 5  Kinerja                  NaN-boxing, inline cache, superinstruksi ❌ (fase 7)
Level 6  Runtime canggih          GC generasional, konkurensi, FFI, embed  ❌ (fase 7–8)
Level 7  Kompilasi lanjut         AST+IR, optimisasi, JIT, AOT/WASM        ❌ (riset)
```

---

## 1. Front-end: dari satu lintasan ke AST

**Sekarang:** compiler Pratt satu lintasan memancarkan bytecode langsung.
Sederhana dan cepat, tetapi menyulitkan:

- Analisis sebelum codegen (K3: tahu variabel mana yang lokal).
- Perkakas: formatter, linter, LSP butuh pohon sintaks lengkap dengan posisi.
- Optimisasi (constant folding, eliminasi kode mati).
- Pesan error yang menunjuk kolom dan menampilkan potongan baris.

**Rencana bertahap:**

1. **Fase 1:** tetap satu lintasan; tambahkan *pre-scan* badan fungsi untuk K3.
   Tambahkan nomor **kolom** ke `Token` dan tabel baris di `Chunk`.
2. **Fase 6–7:** parser baru yang menghasilkan AST (`src/ast.c`), dipakai
   bersama oleh compiler bytecode, formatter (`indk rapikan`), linter, dan LSP.
   Simpan *trivia* (komentar, spasi) agar formatter bisa mempertahankannya.
3. **Riset:** IR menengah (SSA) untuk optimisasi/JIT.

**Spesifikasi formal:** tulis tata bahasa EBNF di `docs/SPESIFIKASI.md`.
Fuzzer yang sudah ada (`tests/fuzz_compile.c`) bisa ditambah fuzzer
*grammar-aware* yang membangkitkan program valid dari EBNF tersebut.

## 2. Representasi nilai

| Teknik | Ukuran nilai | Status |
|---|---|---|
| Tagged union `{type, union}` | 16 byte | ✅ sekarang |
| **NaN-boxing** (double + pointer + int kecil dalam 64 bit) | 8 byte | Fase 7 |
| Pointer tagging (int 63-bit di pointer) | 8 byte | alternatif |

NaN-boxing (bab 30 *Crafting Interpreters*) memotong memori stack/list
separuh dan biasanya mempercepat 10–30%. Lakukan **setelah** K2 (int) agar
desain tag langsung menampung int kecil.

## 3. Angka

- Int64 dengan deteksi overflow (`__builtin_add_overflow`).
- **Bignum:** implementasi sendiri (array limb 32-bit, perkalian
  Karatsuba untuk angka besar) atau pakai pustaka berlisensi bebas
  (mini-gmp, LGPL/GPL — periksa kompatibilitas dengan MIT; alternatif
  *libtommath*, public domain). Rekomendasi: libtommath.
- Cetak float *shortest round-trip*: port algoritma Ryu (Apache-2.0/Boost)
  atau `%.17g` + loop pemendekan (lebih sederhana, cukup untuk 1.0).
- `desimal` presisi tetap (modul `desimal`) dan `pecahan` — fase 6,
  ditulis di atas bignum.

## 4. Model objek: "semuanya objek"

Python memperlakukan tipe bawaan sebagai kelas (`int`, `str`, `list`).
Untuk Indonesiaku:

1. Buat `ObjClass` untuk setiap tipe bawaan saat `initVM`
   (`vm.kelasBulat`, `vm.kelasTeks`, `vm.kelasDaftar`, ...), dengan tabel
   metode berisi fungsi native.
2. `OP_INVOKE` pada nilai non-instansi → cari di kelas tipe tersebut (K10).
3. Metode ajaib (`_tambah_`, `_sama_`, `_teks_`, `_ambil_`, `_panjang_`,
   `_iter_`, `_panggil_`): opcode aritmetika mencoba jalur cepat untuk tipe
   primitif, lalu jatuh ke pencarian metode ajaib pada instansi.
4. **Pewarisan berganda + MRO C3**: ganti "salin metode ke subkelas"
   (`OP_INHERIT` sekarang menyalin tabel) dengan pencarian sepanjang MRO
   ber-cache.
5. **Inline cache** per situs panggilan untuk properti/metode (level 5).

## 5. Pengecualian & traceback

- Kelas pengecualian bawaan (02 §F) sebagai `ObjClass` sungguhan.
- Unwinding saat ini via `OP_TRY_BEGIN/END` — tambahkan blok `akhirnya`
  yang dijalankan pada *tiga* jalur: normal, pengecualian, dan
  `kembalikan`/`hentikan`/`lanjut` yang melompat keluar dari `coba`.
  Ini bagian tersulit; kerjakan dengan tes golden untuk ketiga jalur.
- Traceback bergaya Python, dalam bahasa Indonesia:

```
Jejak (panggilan terakhir paling bawah):
  Berkas "hitung.idk", baris 12, dalam rata_rata
    kembalikan total / panjang(data)
                       ^^^^^^^^^^^^^
GalatBagiNol: pembagian dengan nol
```

- Rantai pengecualian (`naikkan GalatBaru dari e`), `e.pesan`, `e.jejak`.

## 6. Modul, paket, dan namespace

- `ObjModule` dengan tabel global sendiri; fungsi menyimpan pointer ke
  modul pemiliknya (seperti `__globals__` Python). Ini memperbaiki masalah
  "semua `pustaka/` harus diberi awalan `teks_`, `daftar_`".
- Resolusi jalur: direktori skrip → `INDK_PATH` → direktori instalasi
  `pustaka/`. Modul bawaan C (mis. `matematika`) didaftarkan di tabel
  modul native, tidak lagi sebagai global.
- Siklus impor: tandai modul "sedang dimuat" agar tidak rekursi tak terbatas.
- Cache bytecode `.idkc` (fase 7): serialisasi `ObjFunction` + konstanta,
  dengan header versi dan hash sumber.

## 7. Iterator, generator, coroutine

- Generator (`hasilkan`) butuh **frame yang bisa dijeda**: salin slot stack
  frame ke objek `ObjGenerator` saat yield, kembalikan saat lanjut. Pola ini
  dipakai Wren dan Lua.
- Coroutine `asinkron/tunggu` dibangun di atas generator + event loop
  (modul `asinkron`) yang memakai `select`/`poll`/IOCP — cocok dengan modul
  soket yang sudah ada (server HTTP non-blocking).

## 8. Manajemen memori

| Teknik | Kapan |
|---|---|
| Mark-sweep stop-the-world | ✅ sekarang |
| Tuning ambang GC (`GC_HEAP_GROW_FACTOR`) + statistik `gc.statistik()` | fase 1 |
| String interning (sudah) + *small string optimization* | fase 7 |
| GC generasional (nursery + old gen, write barrier) | fase 7 — besar manfaatnya untuk program yang membuat banyak objek kecil |
| GC inkremental (tri-color + barrier) | fase 8 — untuk aplikasi interaktif/game |
| Weak reference (`ref_lemah`) | fase 6 |

Tetap pertahankan job CI `DEBUG_STRESS_GC` + ASan; setiap perubahan GC wajib
lulus di sana.

## 9. Kinerja interpreter

Urutan dari yang termurah:

1. **Batas rekursi & stack dinamis** (B3) — stack yang tumbuh (realloc +
   perbaiki pointer upvalue/frame) atau minimal `FRAMES_MAX` 1000 + stack
   heap-allocated. *(Bukan optimasi, tapi wajib.)*
2. **Computed goto** (`&&label`, GCC/Clang) menggantikan `switch` — 15–25%.
3. **Superinstruksi** (`OP_GET_LOCAL_0`, `OP_ADD_CONST`, `OP_JUMP_IF_FALSE_POP`).
4. **Inline cache** untuk `OP_GET_GLOBAL`, `OP_GET_PROPERTY`, `OP_INVOKE`.
5. **Constant folding** di compiler.
6. NaN-boxing (§2).
7. Benchmark suite tetap (`tests/benchmark.*` sudah ada) dengan pelacakan
   regresi di CI (mis. simpan hasil sebagai artifact dan bandingkan).

**JIT** (level 7): hanya setelah semua di atas, dan hanya bila ada kebutuhan
nyata. Opsi realistis: *copy-and-patch* JIT (dipakai CPython 3.13) atau
memakai backend seperti MIR. Ini proyek riset bertahun-tahun; jangan
menghalangi 1.0.

## 10. Konkurensi

- **Utas (threading):** VM sekarang memakai global `vm` tunggal. Langkah
  pertama: jadikan `VM*` parameter eksplisit (juga diperlukan untuk embedding).
  Lalu pilih model: GIL seperti CPython (sederhana) atau *isolate* per utas
  dengan pengiriman pesan (lebih aman, model Dart/JS worker). Rekomendasi:
  isolate.
- **Async I/O:** lihat §7.

## 11. FFI, embedding, dan ekstensi C

- **API embedding** (`include/indonesiaku.h`): `indk_vm_baru()`,
  `indk_jalankan(vm, sumber)`, `indk_daftar_fungsi(vm, nama, fn)`. Membuka
  jalan untuk memakai Indonesiaku sebagai bahasa skrip di aplikasi lain
  (game, perangkat lunak sekolah). Folder `include/` sudah ada — manfaatkan.
- **Modul ekstensi C** yang dimuat dinamis (`.dll`/`.so`) dengan ABI stabil.
- **FFI** (gaya `ctypes`) via libffi — fase 8; membuka akses ke sqlite,
  GUI, dll.

## 12. Target kompilasi lain

- **WebAssembly** (Emscripten/WASI): menjalankan `indk` langsung di browser
  → playground tanpa server, tanpa risiko keamanan backend. **Sangat
  direkomendasikan** sebagai backend playground (fase 7), menggantikan rencana
  endpoint API.
- AOT ke C atau executable mandiri (`indk bangun app.idk` → bundel VM +
  bytecode) — fase 8.

## 13. Keamanan & sandbox

Relevan untuk playground dan pemakaian di sekolah:

- Mode `--sandbox`: matikan berkas/soket/proses, batasi memori dan jumlah
  instruksi (hitung di loop dispatch).
- Fuzzing berkelanjutan (OSS-Fuzz bila proyek cukup populer).
- Kebijakan pelaporan kerentanan (`SECURITY.md`) — lihat `05_OPEN_SOURCE.md`.

## 14. Internasionalisasi pesan

Semua pesan error ada di satu tabel (`src/pesan.h`) dengan ID, bukan
literal yang tersebar. Manfaat: konsistensi istilah, dokumentasi otomatis
"daftar error", dan kemungkinan pesan bahasa daerah/Inggris di masa depan.

## 15. Daftar bacaan

- Robert Nystrom, *Crafting Interpreters* — dasar arsitektur sekarang
  (bab 30: optimisasi, NaN-boxing).
- *CPython Internals* (Anthony Shaw) dan `Python/ceval.c` — model objek,
  pengecualian, generator.
- Kode sumber **Wren** dan **Lua 5.4** — VM kecil berkualitas tinggi yang
  ukurannya sebanding dengan Indonesiaku.
- *The Garbage Collection Handbook* (Jones, Hosking, Moss).
- Dokumentasi PEP Python (mis. PEP 8 gaya kode, PEP 257 docstring, PEP 3333)
  sebagai model proses RFC.
