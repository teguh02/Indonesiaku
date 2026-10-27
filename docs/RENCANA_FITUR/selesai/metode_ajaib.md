# Rencana Fitur: Metode ajaib — representasi, perbandingan, aritmetika

- **Status:** selesai (disetujui dan dikerjakan 2026-10-25)
- **Target versi:** Fase 4 (1.1), butir pertama
- **Pemilik:** teguh02
- **Keputusan terkait:** K19 (nama & aturan metode ajaib — disetujui), K7
  (`==` struktural, `_sama_` pada instansi), K8 (aturan hash), K10/K11
- **Ref:** `ANALISIS/06_KATALOG_TIPE_BAWAAN.md` §10, `ANALISIS/04_TEKNIK_LANJUTAN.md` §4

## 1. Masalah

Objek buatan pengguna belum bisa ikut "bahasa" seperti tipe bawaan. Tidak ada
cara mengatur bagaimana objek dicetak, dibandingkan, atau dijumlahkan:

```indonesiaku
kelas Vektor {
    init(x, y) {
        diri.x = x
        diri.y = y
    }
}
a = Vektor(1, 2)
cetak(a)                    # <objek Vektor>
cetak(ke_teks(a), f"{a}")   # <objek> <objek>   ← tidak konsisten dengan cetak
cetak(a == Vektor(1, 2))    # salah             ← selalu identitas
cetak(a + a)                # GalatTipe: Operand '+' harus dua angka, dua teks, ...
cetak(-a)                   # "Operand harus berupa angka." — TANPA kelas galat,
                            # jadi `kecuali GalatTipe` tidak menangkapnya
```

Selain itu ada tiga bug yang tersingkap saat menelusuri jalur operator, dan
semuanya berada tepat di cabang yang akan disentuh fitur ini:

1. **`>=`, `<=`, `!=` dikompilasi sebagai negasi** (`emitComparison` di
   `compiler.c`: `>=` → `OP_LESS` + `OP_NOT`, `<=` → `OP_GREATER` + `OP_NOT`,
   `!=` → `OP_EQUAL` + `OP_NOT`). Dengan NaN hasilnya salah:
   `n = 1e308 * 10 - 1e308 * 10` lalu `n >= 1`, `n <= 1`, `1 <= n` semuanya
   `benar`, padahal Python memberi `False`. Perbandingan berantai
   (`1 <= n <= 2`) sudah benar karena `chainCompare` menangani tiap operator
   langsung. Dengan metode ajaib, negasi juga salah secara semantik:
   `a >= b` harus memanggil `_lebih_sama_`, bukan `tidak a._kurang_dari_(b)`.
2. **Galat "Operand harus berupa angka." tanpa kelas** di `OP_GREATER`,
   `OP_LESS`, `OP_SUBTRACT`, `OP_DIVIDE`, `OP_MODULO`, `OP_FLOOR_DIVIDE`,
   `OP_POWER`, `OP_NEGATE`, dan `chainCompare` — tidak tertangkap
   `kecuali GalatTipe`.
3. **Batas `FRAMES_MAX` tanpa kelas**: pesan "Call stack overflow: ..." tidak
   memakai `GalatRekursi`, padahal kelas itu sudah didaftarkan di `initVM`.

Ditemukan lagi saat implementasi, semuanya di jalur yang sama dan ikut
diperbaiki:

4. **Teks terpotong:** `ke_teks`, f-string, dan `repr` menyalin ke buffer 512
   byte, jadi teks lebih dari 511 karakter terpotong diam-diam (`format` di
   1023).
5. **Dua pencetak yang tidak sepakat:** `cetak` memakai `printObject`,
   sisanya `appendValueText` — `ke_teks(cetak)` memberi `<objek>`,
   `cetak(cetak)` memberi `<fungsi bawaan>`.
6. **Crash pada daftar yang berisi dirinya sendiri:** `a.tambah(a)` lalu
   `cetak(a)` berulang tanpa akhir sampai stack C habis (exit 127). Python
   mencetak `[1, [...]]`.
7. **Galat atribut tanpa kelas** (`(5).x` → "Hanya objek yang memiliki
   properti.") dan `0 ** -1` tanpa kelas (Python: `ZeroDivisionError`).

## 2. Sintaks usulan

Tidak ada kata kunci atau token baru. Metode ajaib adalah metode biasa di
dalam `kelas` yang namanya diapit garis bawah (K19); interpreter memanggilnya
otomatis.

```indonesiaku
kelas Vektor {
    init(x, y) {
        diri.x = x
        diri.y = y
    }
    _teks_() {
        kembalikan f"({diri.x}, {diri.y})"
    }
    _repr_() {
        kembalikan f"Vektor({diri.x}, {diri.y})"
    }
    _sama_(lain) {
        kembalikan diri.x == lain.x dan diri.y == lain.y
    }
    _kurang_dari_(lain) {
        kembalikan diri.panjang2() < lain.panjang2()
    }
    _tambah_(lain) {
        kembalikan Vektor(diri.x + lain.x, diri.y + lain.y)
    }
    _kali_(k) {
        kembalikan Vektor(diri.x * k, diri.y * k)
    }
    _negatif_() {
        kembalikan Vektor(-diri.x, -diri.y)
    }
    panjang2() {
        kembalikan diri.x * diri.x + diri.y * diri.y
    }
}

a = Vektor(1, 2)
b = Vektor(3, 4)
cetak(a)                  # (1, 2)
cetak([a, b])             # [Vektor(1, 2), Vektor(3, 4)]
cetak(f"a = {a}")         # a = (1, 2)
cetak(a + b, a * 3, -a)   # (4, 6) (3, 6) (-1, -2)
cetak(a == Vektor(1, 2))  # benar
cetak(a != b)             # benar   (dari _sama_, karena _tidak_sama_ tidak ada)
cetak(a < b, b > a)       # benar benar   (b > a memakai a._kurang_dari_(b))
cetak(a dalam [b, Vektor(1, 2)])   # benar
a += b
cetak(a)                  # (4, 6)
```

## 3. Semantik (mengikuti Python 3, nama menurut K19)

### 3.1 Cakupan tahap ini

| Kelompok | Nama Indonesiaku | Python | Dipicu oleh |
|---|---|---|---|
| Representasi | `_teks_` | `__str__` | `cetak`, `ke_teks`, f-string |
| | `_repr_` | `__repr__` | `repr`, elemen di dalam koleksi yang dicetak, cadangan `_teks_` |
| Perbandingan | `_sama_` | `__eq__` | `==`, `dalam`, `==` antar-daftar/tupel/kamus |
| | `_tidak_sama_` | `__ne__` | `!=` |
| | `_kurang_dari_` / `_kurang_sama_` | `__lt__` / `__le__` | `<` / `<=` (termasuk berantai) |
| | `_lebih_dari_` / `_lebih_sama_` | `__gt__` / `__ge__` | `>` / `>=` (termasuk berantai) |
| Aritmetika | `_tambah_` `_kurangi_` `_kali_` `_bagi_` `_bagi_bulat_` `_sisa_` `_pangkat_` | `__add__` `__sub__` `__mul__` `__truediv__` `__floordiv__` `__mod__` `__pow__` | `+ - * / // % **` dan `+= -= ...` |
| Unary | `_negatif_` | `__neg__` | `-x` |

**Sengaja belum (rencana terpisah):** varian sisi kanan (`_ktambah_` ↔
`__radd__`) dan di tempat (`_tambah_di_` ↔ `__iadd__`) — keduanya butuh nilai
penanda `TidakDiterapkan` (`NotImplemented`) yang belum ada; `_positif_`,
`_mutlak_`, bitwise; `_logika_`, `_panjang_`, `_hash_`, `_ambil_`/`_atur_`,
`_iter_`, `_panggil_`, `_format_`; `terurut`/`min`/`max` atas instansi
(`terurut` memakai `qsort`, yang pembandingnya tidak boleh memanggil kode
pengguna — butuh merge sort sendiri, sekalian stabil seperti `sorted`).

### 3.2 Aturan pencarian

- Metode dicari di **kelas** (`klass->methods`, termasuk yang diwarisi lewat
  `OP_INHERIT`), **tidak** di field instansi — sama seperti Python yang
  mencari dunder di tipe. `diri._tambah_ = ...` tidak mengubah perilaku `+`.
- Hanya **instansi** yang memicu metode ajaib. Angka, teks, koleksi, dan kelas
  itu sendiri tidak berubah sedikit pun; jalur cepatnya tidak tersentuh.
- Metode ajaib tetap metode biasa: `a._tambah_(b)` boleh dipanggil langsung.

### 3.3 Aritmetika

`a + b`:

1. `a` instansi dan kelasnya punya `_tambah_` → hasil = `a._tambah_(b)`, apa
   pun tipenya (tidak dipaksa menjadi tipe tertentu).
2. Selain itu, bila salah satu operand instansi → `GalatTipe`
   "Operator '+' tidak didukung antara 'Vektor' dan 'bulat'." (Python:
   *unsupported operand type(s)*). Python masih akan mencoba `b.__radd__(a)`;
   di sini itu baru datang bersama `_ktambah_`.
3. Selain itu → perilaku sekarang, tetapi semua galat operand diberi kelas
   `GalatTipe` (bug 2).

`a += b` memakai `_tambah_` (opcode yang sama) dan mengikat ulang `a` ke
hasilnya — persis Python saat `__iadd__` tidak ada. `-a` memakai `_negatif_`.

### 3.4 Perbandingan

`a < b`:

1. `a` instansi dengan `_kurang_dari_` → `a._kurang_dari_(b)`.
2. Selain itu, `b` instansi dengan **cerminannya** `_lebih_dari_` →
   `b._lebih_dari_(a)`. Cerminan: `<` ↔ `>`, `<=` ↔ `>=`, `==` ↔ `==`,
   `!=` ↔ `!=`. Ini aturan refleksi Python untuk kasus sisi kiri tidak punya
   metodenya; tidak butuh `TidakDiterapkan`.
3. Selain itu, ada instansi → `GalatTipe` "Operator '<' tidak didukung antara
   'Vektor' dan 'bulat'."

Hasil tidak dipaksa menjadi `benar`/`salah` (Python juga tidak); `jika`
memakai truthiness K1.

`==`: `_sama_` dari kiri, lalu dari kanan (cerminan); tidak ada keduanya →
**identitas** (perilaku sekarang, K7). `!=`: `_tidak_sama_` (kiri, lalu
kanan); kalau tidak ada tetapi `_sama_` ada → `tidak` hasil `_sama_` (bawaan
`__ne__` Python); tidak ada keduanya → bukan-identitas.

`adalah` / `bukan` **tetap identitas murni** dan tidak pernah memanggil
`_sama_` (K7) — golden `gabung_sama` menjaganya.

**Di dalam koleksi:** `[a] == [Vektor(1, 2)]`, `(a,) == (b,)`, dan `a dalam xs`
membandingkan elemen instansi lewat `_sama_`, seperti Python — setelah
jalan pintas identitas yang juga diambil Python (`x dalam [x]` tidak
memanggil `_sama_`). Bahasa ini belum punya `.indeks()`/`.hitung()` pada
daftar.

**Hash (K8):** kelas yang mendefinisikan `_sama_` membuat instansinya **tidak
dapat di-hash** — menjadi kunci kamus atau anggota himpunan → `GalatTipe`
(Python mengeset `__hash__ = None`). Kelas tanpa `_sama_` tetap hashable
berdasarkan identitas seperti sekarang. Akibatnya pencarian tabel kamus/
himpunan **tidak pernah** memanggil kode pengguna. `_hash_` menyusul.

### 3.5 Representasi

| Pemanggil | Urutan yang dicoba | Bawaan |
|---|---|---|
| `cetak(x)`, `ke_teks(x)`, `f"{x}"` | `_teks_`, lalu `_repr_` | `<objek Vektor>` |
| `repr(x)`, `x` sebagai elemen koleksi yang dicetak | `_repr_` | `<objek Vektor>` |

- Hasil wajib teks; selain itu `GalatTipe` "_teks_ harus mengembalikan teks,
  bukan bulat." (Python: *`__str__` returned non-string*).
- Bawaan disatukan menjadi `<objek Kelas>` di **semua** jalur. Sekarang
  `cetak` memberi `<objek Vektor>` tetapi `ke_teks`/`repr`/f-string memberi
  `<objek>`. Tidak ada golden atau panduan yang memuat teks itu (sudah dicek),
  jadi tidak merusak apa pun.
- Instansi `Galat*` bawaan tetap dicetak sebagai pesannya.

### 3.6 Galat dan rekursi

- Galat di dalam metode ajaib merambat seperti galat biasa (bisa ditangkap,
  traceback memuat baris di dalam metode).
- `_teks_` yang memanggil `cetak(diri)` adalah rekursi tak terbatas. Jalur
  cetak masuk ulang ke VM lewat `callFromNative` → `run()`, jadi setiap
  tingkat memakai stack **C** (frame `run()` + buffer teks native 512–1024
  byte). Batas `FRAMES_MAX` (1000) bisa datang terlambat di Windows (stack
  1 MB). Karena itu kedalaman masuk-ulang native dibatasi (usulan
  `NATIVE_CALL_MAX` 200) → `GalatRekursi`. Bug 3 ikut diperbaiki: batas
  `FRAMES_MAX` juga memakai `GalatRekursi`.

## 4. Rencana implementasi

| Bagian | Perubahan |
|---|---|
| `src/compiler.c` | `emitComparison`: `!=`, `>=`, `<=` memakai opcode sendiri, bukan negasi (memperbaiki NaN) |
| `src/chunk.h`, `src/debug.c` | `OP_NOT_EQUAL`, `OP_GREATER_EQUAL`, `OP_LESS_EQUAL` + nama disassembler |
| `src/object.h` / `object.c` | `ObjClass.ajaib` (bitmask metode ajaib yang ada) supaya cek di VM cukup satu `&` tanpa pencarian tabel; cetak bawaan `<objek Kelas>` |
| `src/vm.c` | Tabel nama metode ajaib (string ter-intern di `vm`, seperti `initString`). `OP_METHOD` menyalakan bit; `OP_INHERIT` meng-OR bitmask induk. Helper `dispatchMagic(nama, cermin, …)` dipanggil **hanya** di cabang yang sekarang melempar galat operand, lalu menata stack (penerima di `peek(1)`, argumen di `peek(0)`; cerminan = tukar dulu) dan memanggil `call(closure, n)` — frame baru, tanpa masuk-ulang C. `OP_EQUAL`/`OP_NOT_EQUAL` cek instansi lebih dulu. `chainCompare` (fungsi C) memakai `callFromNative`. Semua galat operand diberi `vm.galatTipe`; `FRAMES_MAX` diberi `vm.galatRekursi`; batas `NATIVE_CALL_MAX` di `callFromNative` |
| `src/value.c` | `valuesEqualDepth`: instansi lewat hook yang dipasang VM (value.c tidak mengenal `vm`); `hashValue`: instansi dengan bit `_sama_` → tidak hashable |
| `src/object.c` | **Satu formatter** `formatValue` ke `TextBuf` (malloc, tanpa batas panjang, penjaga siklus) menggantikan `printObject` dan `appendValueText`; instansi lewat `formatInstanceHook` yang dipasang VM |
| `src/native.c` | `cetak`, `ke_teks`, `repr`, `format` memakai `formatValue` (`valueText`); `_teks_`/`_repr_` jalan lewat `callFromNative` |
| `src/memory.c` | Tandai string nama metode ajaib di `markRoots` |

**Jalur cepat tidak berubah:** pengecekan instansi hanya ada di cabang yang
hari ini sudah berakhir dengan galat, jadi program yang ada tidak melambat.

**GC & masuk-ulang (wajib diuji ASan + stress GC):**

- Saat `callFromNative` menjalankan kode pengguna, GC bisa jalan dan objek
  apa pun bisa dialokasi. Setiap nilai yang dipegang C (teks hasil sementara
  di `cetak`, daftar yang sedang dirangkai) harus ter-root di stack VM.
- Kode pengguna di `_sama_` **boleh mengubah daftar yang sedang ditelusuri**
  (`dalam`, `.indeks()`, `==` daftar). Loop C tidak boleh menyimpan pointer
  `items->values` atau `count` di luar iterasi — `writeValueArray` bisa
  me-`realloc` dan meninggalkan pointer basi. Baca ulang setiap iterasi, dan
  hentikan loop begitu `vm.nativeErrorPending`/galat tertunda menyala.
- Hook kesetaraan tidak punya saluran galat (mengembalikan `bool`): galat
  dicatat sebagai galat tertunda, hook mengembalikan `salah`, dan pemanggil
  di VM memeriksa flag setelahnya.

## 5. Pengujian

- **Golden** `examples/oop/metode_ajaib.idk` + `tests/expected/metode_ajaib.out`:
  setiap baris tabel §3.1, cerminan (`b > a`, `1 == a` memakai `a._sama_`),
  `!=` turunan dari `_sama_`, `+=`, pewarisan metode ajaib, `[a] == [b]`,
  `dalam`, `.indeks`, `adalah` tetap identitas, f-string, cadangan `_repr_`,
  bawaan `<objek Kelas>` di semua jalur. Dibandingkan dengan padanan Python
  (`__dunder__`) yang dijalankan sungguhan.
- **Galat tertangkap** (di golden): operand tidak didukung (kiri dan kanan),
  `_teks_` mengembalikan bukan-teks, instansi ber-`_sama_` sebagai kunci
  kamus/anggota himpunan, galat dari dalam `_tambah_`, semua galat operand
  angka kini `GalatTipe`.
- **NaN** golden: `>=`, `<=`, `!=` dengan NaN sama dengan Python, berantai dan
  tidak.
- **`tests/galat/`**: operand tidak didukung tanpa `coba` (format pesan),
  rekursi `_teks_` → `GalatRekursi`, rekursi fungsi biasa → `GalatRekursi`.
- **Masuk-ulang**: `_sama_` yang menambah elemen ke daftar yang sedang dicari
  `dalam` — wajib bersih di ASan.
- `-DDEBUG_STRESS_GC` + ASan/UBSan untuk golden **dan** `tests/galat/`, fuzz
  pengganti, todo CLI/HTTP. Golden yang berubah dibaca satu per satu.

## 6. Dampak dokumentasi & tooling

- [x] `website/panduan/03_PANDUAN_SINTAKS.md` — subbagian "Metode ajaib" di
      bawah "Kelas & OOP" (kedua blok kode dijalankan dan cocok)
- [x] `website/panduan/07_PUSTAKA_STANDAR.md` — `ke_teks`/`repr` memanggil
      `_teks_`/`_repr_`
- [x] `editors/vscode-indonesiaku/extension.js` — `MAGIC_METHODS` di saran
      pelengkapan
- [x] `editors/vscode-indonesiaku/syntaxes/indonesiaku.tmLanguage.json` —
      `support.function.magic` (pola diuji dengan mencocokkan teks nyata)
- [x] `editors/vscode-indonesiaku/snippets/indonesiaku.code-snippets` —
      snippet `_teks_` dan `_sama_`
- [x] `README.md`
- [x] `CHANGELOG.md` + `docs/ROADMAP.md` (Fase 4 sebagian)
      + `docs/ANALISIS/02_PETA_FITUR_PYTHON.md`
      + `docs/ANALISIS/06_KATALOG_TIPE_BAWAAN.md` §10
- [x] `docs/ANALISIS/03_KEPUTUSAN_DESAIN.md` — K19 → selesai
- [x] `AGENTS.md` — bitmask `magic`, dua jalur dispatch, formatter tunggal,
      jebakan masuk-ulang di loop C, `NATIVE_CALL_MAX`
- [x] Berkas rencana ini: status → `selesai`, lalu `git mv` ke
      `docs/RENCANA_FITUR/selesai/`

## 7. Risiko & alternatif

- **Masuk-ulang dari C** adalah risiko terbesar (pointer basi, GC di tengah
  loop). Dibatasi dengan memakai `call()` biasa di semua opcode, sehingga
  `callFromNative` hanya dipakai di `cetak`/`ke_teks`/`repr`, `chainCompare`,
  dan hook kesetaraan koleksi.
- **Perubahan perilaku yang terlihat:** `>=`/`<=`/`!=` dengan NaN kini
  `salah`/`benar` seperti Python (perbaikan bug); galat operand kini
  tertangkap `kecuali GalatTipe`; `ke_teks`/`repr`/f-string instansi kini
  `<objek Kelas>`. Tidak ada program contoh di repo yang bergantung pada
  perilaku lama.
- **Alternatif ditolak — opcode khusus per metode ajaib:** memperbanyak
  opcode tanpa manfaat; cabang galat yang ada sudah titik sisip yang tepat.
- **Alternatif ditolak — `_sama_` hanya untuk `==` tingkat atas:** melanggar
  K7 dan membuat `dalam` berbeda dari `==`, sumber kebingungan pemula.
- **Alternatif ditolak — nama Python apa adanya (`__add__`):** bertentangan
  dengan prinsip penamaan Indonesia; nama dikunci di K19.
