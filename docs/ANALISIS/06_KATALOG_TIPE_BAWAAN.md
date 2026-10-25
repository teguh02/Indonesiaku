# 06 — Katalog Lengkap: Tipe Bawaan, Metode, Kelas Pengecualian, Metode Ajaib

Katalog ini mendaftar **setiap** metode tipe bawaan Python 3.12, seluruh
kelas pengecualian bawaan, dan seluruh metode ajaib (dunder), beserta usulan
nama Indonesianya. Tujuannya: daftar kerja yang bisa dicentang satu per satu,
dan sumber "good first issue" bagi kontributor.

Ringkasan fungsi bawaan tingkat atas (`len`, `range`, `zip`, dll.) ada di
[`02_PETA_FITUR_PYTHON.md`](02_PETA_FITUR_PYTHON.md) §D. Modul pustaka standar
ada di [`07_KATALOG_PUSTAKA_STANDAR.md`](07_KATALOG_PUSTAKA_STANDAR.md).

**Legenda**

- **Ada:** builtin/pustaka Indonesiaku yang sudah menyediakan fungsinya
  (biasanya sebagai *fungsi*, belum *metode*).
- **P:** prioritas — **P1** dibutuhkan pemula (fase 1–2), **P2** umum
  (fase 2–4), **P3** lanjutan/jarang (fase 5+).
- Semua metode diimplementasikan native di C (cepat) kecuali ditandai
  "idk" (boleh ditulis dalam Indonesiaku di `pustaka/`).
- Konvensi metode ajaib: nama Indonesia diapit satu garis bawah
  (`_teks_`), meniru `__str__` tanpa terlalu banyak simbol.

---

## 1. `str` → `teks` (47 metode)

| Python | Usulan | Ada | P |
|---|---|---|---|
| `upper()` | `huruf_besar()` | `huruf_besar(t)` | P1 |
| `lower()` | `huruf_kecil()` | `huruf_kecil(t)` | P1 |
| `capitalize()` | `kapital()` | | P1 |
| `title()` | `judul()` | | P1 |
| `swapcase()` | `tukar_huruf()` | | P3 |
| `casefold()` | `lipat_huruf()` | | P3 |
| `strip(c)` / `lstrip` / `rstrip` | `pangkas()` / `pangkas_kiri()` / `pangkas_kanan()` | `teks_pangkas` | P1 |
| `split(sep, maks)` | `pisah()` | `teks_pisah` | P1 |
| `rsplit()` | `pisah_kanan()` | | P3 |
| `splitlines()` | `pisah_baris()` | | P2 |
| `join(iterable)` | `gabung()` (`", ".gabung(daftar)`) | `teks_gabung` | P1 |
| `replace(a, b, n)` | `ganti()` | `ganti(t, a, b)` | P1 |
| `find()` / `rfind()` | `cari()` / `cari_kanan()` | `teks_cari` | P1 |
| `index()` / `rindex()` | `indeks()` / `indeks_kanan()` (error bila tak ada) | | P2 |
| `count()` | `hitung()` | | P1 |
| `startswith()` / `endswith()` | `mulai_dengan()` / `akhiri_dengan()` | `teks_mulai_dengan`… | P1 |
| `removeprefix()` / `removesuffix()` | `buang_awalan()` / `buang_akhiran()` | | P2 |
| `partition()` / `rpartition()` | `bagi_tiga()` / `bagi_tiga_kanan()` | | P3 |
| `isdigit()` / `isdecimal()` / `isnumeric()` | `adalah_digit()` / `adalah_desimal()` / `adalah_numerik()` | | P1 |
| `isalpha()` / `isalnum()` | `adalah_huruf()` / `adalah_alfanumerik()` | | P1 |
| `isspace()` | `adalah_spasi()` | | P1 |
| `islower()` / `isupper()` / `istitle()` | `adalah_huruf_kecil()` / `adalah_huruf_besar()` / `adalah_judul()` | | P2 |
| `isascii()` / `isprintable()` / `isidentifier()` | `adalah_ascii()` / `bisa_dicetak()` / `adalah_pengenal()` | | P3 |
| `center(n, c)` / `ljust` / `rjust` | `tengah()` / `rata_kiri()` / `rata_kanan()` | | P2 |
| `zfill(n)` | `isi_nol()` | | P2 |
| `expandtabs()` | `lebarkan_tab()` | | P3 |
| `format(...)` / `format_map(d)` | `format()` / `format_kamus()` | `format(t, ...)` | P1 |
| `encode()` | `enkode()` → `bita` | | P3 |
| `maketrans()` / `translate()` | `buat_tabel_terjemah()` / `terjemahkan()` | | P3 |

Operasi string (bukan metode): `+`, `*` (ulang, `teks_ulang`), `dalam`,
indeks negatif, irisan `t[1:3]`, `t[::-1]` (`teks_balik`), perbandingan,
iterasi per karakter, f-string. Semua P1.

## 2. `list` → `daftar` (11 metode)

| Python | Usulan | Ada | P |
|---|---|---|---|
| `append(x)` | `tambah(x)` | `tambah(d, x)` | P1 |
| `extend(it)` | `perluas(it)` | | P1 |
| `insert(i, x)` | `sisip(i, x)` | `sisip(d, i, x)` | P1 |
| `pop(i=-1)` | `ambil_keluar(i)` | `hapus(d)` / `hapus_pada(d, i)` | P1 |
| `remove(x)` | `buang(x)` (berdasarkan nilai) | | P1 |
| `clear()` | `kosongkan()` | | P2 |
| `index(x)` | `indeks(x)` | `daftar_indeks_dari` | P1 |
| `count(x)` | `hitung(x)` | | P1 |
| `sort(key, reverse)` | `urutkan(kunci=, terbalik=)` | `daftar_urutkan` (idk, lambat) | P1 — native, stabil (Timsort/merge sort) |
| `reverse()` | `balik()` | `daftar_balik` | P1 |
| `copy()` | `salin()` | `daftar_salin` | P1 |

Operasi: `+`, `*`, `dalam`, `==`, `<` (leksikografis), irisan baca/tulis,
`buang d[i]`, `min/max/jumlah/terurut`, unpacking. P1–P2.

## 3. `dict` → `kamus` (11 metode)

| Python | Usulan | Ada | P |
|---|---|---|---|
| `keys()` | `kunci()` | `kunci(k)` | P1 |
| `values()` | `nilai()` | `kamus_nilai` | P1 |
| `items()` | `pasangan()` (→ tupel `(k, v)`) | | P1 |
| `get(k, d)` | `ambil(k, bawaan)` | `kamus_ambil` | P1 |
| `pop(k, d)` | `ambil_keluar(k, bawaan)` | `hapus_kunci` | P1 |
| `popitem()` | `ambil_terakhir()` | | P3 |
| `setdefault(k, d)` | `atur_bawaan(k, d)` | | P2 |
| `update(d2)` | `perbarui(d2)` | `kamus_gabung` | P1 |
| `clear()` | `kosongkan()` | | P2 |
| `copy()` | `salin()` | `kamus_salin` | P1 |
| `fromkeys(it, v)` | `kamus.dari_kunci(it, v)` | | P3 |

Operasi: literal `{k: v}`, `dalam` (`punya(k, x)`), `d1 | d2`, iterasi kunci,
urutan penyisipan dipertahankan (seperti Python 3.7+).

## 4. `set` → `himpunan` (17 metode)

| Python | Usulan | P |
|---|---|---|
| `add` / `remove` / `discard` | `tambah` / `buang` / `buang_jika_ada` | P2 |
| `pop` / `clear` / `copy` | `ambil_keluar` / `kosongkan` / `salin` | P2 |
| `union` / `update` | `gabungan` / `perbarui` | P2 |
| `intersection` / `intersection_update` | `irisan` / `perbarui_irisan` | P2 |
| `difference` / `difference_update` | `selisih` / `perbarui_selisih` | P2 |
| `symmetric_difference` / `…_update` | `selisih_simetris` / `perbarui_selisih_simetris` | P3 |
| `issubset` / `issuperset` / `isdisjoint` | `subhimpunan_dari` / `superhimpunan_dari` / `saling_lepas` | P2 |

Operator `| & - ^ <= >=`, literal `{1, 2}`, `himpunan()` kosong.
`frozenset` → `himpunan_beku` (P3).

## 5. `tuple` → `tupel`

`count` → `hitung`, `index` → `indeks`. Literal `(1, 2)`, `(1,)`; unpacking;
bisa jadi kunci kamus. P2.

## 6. Angka: `int` → `bulat`, `float` → `desimal`

| Python | Usulan | P |
|---|---|---|
| `int.bit_length()` / `bit_count()` | `panjang_bit()` / `hitung_bit()` | P3 |
| `int.to_bytes()` / `from_bytes()` | `ke_bita()` / `bulat.dari_bita()` | P3 |
| `float.is_integer()` | `adalah_bulat()` | P2 |
| `as_integer_ratio()` | `sebagai_pecahan()` | P3 |
| `float.hex()` / `fromhex()` | `heksa()` / `desimal.dari_heksa()` | P3 |
| `bool` sebagai subkelas `int` (`benar + 1 == 2`) | putuskan di spesifikasi | — |

Konstanta: `takhingga` (sudah tercetak untuk inf) dan `nan` → sediakan di
modul `matematika`.

## 7. `bytes` / `bytearray` → `bita` / `larik_bita` (P3)

`decode` → `dekode`, `hex` → `heksa`, `fromhex` → `dari_heksa`, ditambah
sebagian besar metode `teks` (split, strip, find, …) untuk data biner.
Dibutuhkan oleh berkas biner, `hash`, `base64`, soket biner.

## 8. Objek fungsi, kelas, modul (introspeksi, P3)

| Python | Usulan |
|---|---|
| `f.__name__`, `__doc__`, `__module__` | `f._nama_`, `f._dok_`, `f._modul_` |
| `obj.__class__`, `cls.__bases__`, `cls.__mro__` | `obj._kelas_`, `K._induk_`, `K._mro_` |
| `obj.__dict__` | `obj._atribut_` |
| `cls.__subclasses__()` | `K._turunan_()` |

---

## 9. Kelas pengecualian bawaan (hierarki penuh Python 3.12)

| Python | Indonesiaku | Kapan dinaikkan oleh VM | P |
|---|---|---|---|
| `BaseException` | `GalatDasar` | — (akar) | P1 |
| ├ `SystemExit` | `KeluarSistem` | `keluar()` / `henti()` | P2 |
| ├ `KeyboardInterrupt` | `InterupsiKeyboard` | Ctrl+C | P2 |
| ├ `GeneratorExit` | `KeluarGenerator` | generator ditutup | P3 |
| └ `Exception` | `Galat` | — (induk semua galat biasa) | P1 |
|   ├ `ArithmeticError` | `GalatAritmetika` | | P1 |
|   │ ├ `ZeroDivisionError` | `GalatBagiNol` | `x / 0`, `x // 0`, `x % 0` | P1 |
|   │ ├ `OverflowError` | `GalatLuapan` | int64 meluap (sebelum bignum) | P2 |
|   │ └ `FloatingPointError` | `GalatTitikMengambang` | (jarang) | P3 |
|   ├ `AssertionError` | `GalatPenegasan` | `pastikan` gagal | P1 |
|   ├ `AttributeError` | `GalatAtribut` | properti tak ada | P1 |
|   ├ `EOFError` | `GalatAkhirBerkas` | `input()` di EOF (sekarang `kosong`) | P2 |
|   ├ `ImportError` | `GalatImpor` | | P1 |
|   │ └ `ModuleNotFoundError` | `GalatModulTidakDitemukan` | | P1 |
|   ├ `LookupError` | `GalatPencarian` | | P1 |
|   │ ├ `IndexError` | `GalatIndeks` | indeks di luar jangkauan | P1 |
|   │ └ `KeyError` | `GalatKunci` | kunci kamus tak ada | P1 |
|   ├ `MemoryError` | `GalatMemori` | alokasi gagal | P2 |
|   ├ `NameError` | `GalatNama` | variabel tak terdefinisi | P1 |
|   │ └ `UnboundLocalError` | `GalatLokalBelumTerikat` | baca lokal sebelum diisi (K3) | P2 |
|   ├ `OSError` | `GalatOS` | | P1 |
|   │ ├ `FileNotFoundError` | `GalatBerkasTidakDitemukan` | `buka` | P1 |
|   │ ├ `FileExistsError` | `GalatBerkasSudahAda` | | P2 |
|   │ ├ `PermissionError` | `GalatIzin` | | P2 |
|   │ ├ `IsADirectoryError` / `NotADirectoryError` | `GalatAdalahDirektori` / `GalatBukanDirektori` | | P3 |
|   │ ├ `TimeoutError` | `GalatWaktuHabis` | soket | P2 |
|   │ ├ `ConnectionError` | `GalatKoneksi` | soket | P2 |
|   │ │ ├ `BrokenPipeError` | `GalatPipaPutus` | | P3 |
|   │ │ ├ `ConnectionRefusedError` | `GalatKoneksiDitolak` | | P2 |
|   │ │ ├ `ConnectionResetError` | `GalatKoneksiDireset` | | P3 |
|   │ │ └ `ConnectionAbortedError` | `GalatKoneksiDibatalkan` | | P3 |
|   │ ├ `BlockingIOError` / `InterruptedError` | `GalatIOTerblokir` / `GalatTerinterupsi` | | P3 |
|   │ └ `ChildProcessError` / `ProcessLookupError` | `GalatProsesAnak` / `GalatProsesTidakDitemukan` | modul `proses` | P3 |
|   ├ `RuntimeError` | `GalatRuntime` | | P1 |
|   │ ├ `RecursionError` | `GalatRekursi` | kedalaman panggilan terlampaui | P1 |
|   │ └ `NotImplementedError` | `GalatBelumDiimplementasi` | metode abstrak | P2 |
|   ├ `StopIteration` | `HentiIterasi` | iterator habis | P2 |
|   ├ `StopAsyncIteration` | `HentiIterasiAsinkron` | | P3 |
|   ├ `SyntaxError` | `GalatSintaks` | `evaluasi`/`impor` sumber rusak | P2 |
|   │ └ `IndentationError` / `TabError` | — (tidak relevan, K4) | | — |
|   ├ `SystemError` | `GalatSistem` | bug VM | P3 |
|   ├ `TypeError` | `GalatTipe` | operand/argumen salah tipe, jumlah argumen salah | P1 |
|   ├ `ValueError` | `GalatNilai` | `ke_bulat("abc")` | P1 |
|   │ └ `UnicodeError` (+ Decode/Encode/Translate) | `GalatUnicode` (+ `…Dekode`, `…Enkode`) | | P3 |
|   ├ `BufferError` / `ReferenceError` | `GalatBuffer` / `GalatReferensi` | | P3 |
|   └ `ExceptionGroup` | `KelompokGalat` | asinkron | P3 |
| `Warning` (+ `DeprecationWarning`, `UserWarning`, `RuntimeWarning`, `SyntaxWarning`, `FutureWarning`, `ResourceWarning`, …) | `Peringatan` (+ `PeringatanUsang`, `PeringatanPengguna`, `PeringatanRuntime`, `PeringatanSintaks`, `PeringatanMasaDepan`, `PeringatanSumberDaya`) | modul `peringatan` | P2 |

Atribut setiap galat: `pesan`, `argumen`, `jejak` (traceback), `sebab`
(rantai `naikkan ... dari e`).

---

## 10. Metode ajaib (dunder) → `_nama_`

### 10.1 Siklus hidup & representasi

| Python | Indonesiaku | P |
|---|---|---|
| `__init__` | `init` (sudah ada, pertahankan) | ✅ |
| `__new__` | `_baru_` | P3 |
| `__del__` | `_hapus_` | P3 |
| `__str__` | `_teks_` (dipakai `cetak`, `ke_teks`) | ✅ (K19) |
| `__repr__` | `_repr_` | ✅ (K19) |
| `__format__` | `_format_` | P3 |
| `__bool__` | `_logika_` | P2 |
| `__hash__` | `_hash_` | P2 |

### 10.2 Perbandingan (P1–P2)

`__eq__` `_sama_`, `__ne__` `_tidak_sama_`, `__lt__` `_kurang_dari_`,
`__le__` `_kurang_sama_`, `__gt__` `_lebih_dari_`, `__ge__` `_lebih_sama_`.
✅ semuanya (K19).

### 10.3 Aritmetika (P2)

| Python | Indonesiaku | Python | Indonesiaku |
|---|---|---|---|
| `__add__` | `_tambah_` | `__sub__` | `_kurangi_` |
| `__mul__` | `_kali_` | `__truediv__` | `_bagi_` |
| `__floordiv__` | `_bagi_bulat_` | `__mod__` | `_sisa_` |
| `__pow__` | `_pangkat_` | `__divmod__` | `_bagi_sisa_` |
| `__neg__` / `__pos__` / `__abs__` | `_negatif_` / `_positif_` / `_mutlak_` | `__matmul__` | `_matkali_` (P3) |
| `__and__` `__or__` `__xor__` `__invert__` `__lshift__` `__rshift__` | `_dan_bit_` `_atau_bit_` `_xor_` `_balik_bit_` `_geser_kiri_` `_geser_kanan_` | | P3 |

✅ (K19): `_tambah_` `_kurangi_` `_kali_` `_bagi_` `_bagi_bulat_` `_sisa_`
`_pangkat_` `_negatif_`. Belum: `_bagi_sisa_`, `_positif_`, `_mutlak_`, bitwise.

Varian sisi kanan (`__radd__` → `_ktambah_`) dan di tempat
(`__iadd__` → `_tambah_di_`) — P3. Catatan: kata "kurang" bisa berarti
"less than" maupun "subtract", jadi namanya sengaja dibedakan:
`_kurang_dari_` untuk `<` dan `_kurangi_` untuk `-`. Kunci nama ini di
rencana fitur metode ajaib.

### 10.4 Konversi (P2)

`__int__` `_bulat_`, `__float__` `_desimal_`, `__index__` `_indeks_`,
`__round__` `_bulatkan_`, `__trunc__` `_potong_`, `__floor__` `_lantai_`,
`__ceil__` `_atap_`.

### 10.5 Wadah & iterasi (P2)

`__len__` `_panjang_`, `__getitem__` `_ambil_`, `__setitem__` `_atur_`,
`__delitem__` `_buang_`, `__contains__` `_berisi_`, `__iter__` `_iter_`,
`__next__` `_berikutnya_`, `__reversed__` `_terbalik_`, `__missing__`
`_hilang_` (untuk subkelas kamus).

### 10.6 Atribut & pemanggilan (P3)

`__getattr__` `_ambil_atribut_`, `__setattr__` `_atur_atribut_`,
`__delattr__` `_buang_atribut_`, `__dir__` `_daftar_atribut_`,
`__call__` `_panggil_` (P2), deskriptor `__get__/__set__/__delete__`
`_dapat_/_tetapkan_/_cabut_`, `__init_subclass__` `_init_turunan_`.

### 10.7 Konteks & asinkron

`__enter__` / `__exit__` → `_masuk_` / `_keluar_` (P2, untuk `dengan`).
`__aenter__/__aexit__/__aiter__/__anext__/__await__` → `_masuk_asinkron_` dst.
(P3).

---

## 11. Ringkasan jumlah

| Kelompok | Jumlah item Python | Sudah ada di Indonesiaku (dalam bentuk apa pun) |
|---|---|---|
| Metode `str` | 47 | ±12 (sebagai fungsi) |
| Metode `list` | 11 | ±8 (sebagai fungsi) |
| Metode `dict` | 11 | ±6 (sebagai fungsi) |
| Metode `set` | 17 | 0 |
| Kelas pengecualian | ±65 | 0 (hanya string/nilai yang di-`naikkan`) |
| Metode ajaib | ±80 | 1 (`init`) |
