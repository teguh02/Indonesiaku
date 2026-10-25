# Rencana Fitur: Himpunan (`himpunan`)

- **Status:** selesai
- **Target versi:** Fase 2
- **Pemilik:** teguh02
- **Keputusan terkait:** K17 (hanya lewat konstruktor, tanpa literal — disetujui 2026-10-25), K8 (aturan hash), K7 (`==` struktural)

## 1. Masalah

Tidak ada koleksi anggota unik. Menghapus duplikat, menguji keanggotaan
cepat, atau mencari irisan dua daftar harus ditulis manual dengan perulangan
bersarang:

```
cetak(himpunan([1, 2, 2]))   # Variabel tidak terdefinisi 'himpunan'.
```

## 2. Sintaks (K17 — tanpa literal `{1, 2}`)

```indonesiaku
hampa = himpunan()
angka = himpunan([3, 1, 3, 2])      # himpunan([3, 1, 2])
huruf = himpunan("halo")            # himpunan(["h", "a", "l", "o"])

cetak(2 dalam angka, panjang(angka))
cetak(angka | himpunan([5]))        # gabungan
cetak(angka & himpunan([1, 9]))     # irisan
cetak(angka - himpunan([1]))        # selisih
cetak(angka ^ himpunan([1, 9]))     # selisih simetris
angka.tambah(7)
angka.hapus(3)
```

## 3. Semantik (mengikuti Python 3)

| Operasi | Hasil |
|---|---|
| `himpunan()`, `himpunan(urutan)` | dari daftar, tupel, teks (per karakter), rentang, kamus (kuncinya), himpunan; duplikat dibuang |
| anggota | harus hashable — aturan K8 yang sama dengan kunci kamus (angka, teks, logika, kosong, tupel berisi nilai hashable); daftar/kamus/himpunan → `GalatTipe` |
| `x dalam s`, `panjang`, `untuk`, comprehension, `jumlah`/`min`/`max`/`terurut`/`tupel(s)` | seperti Python |
| `a \| b`, `a & b`, `a - b`, `a ^ b` | himpunan **baru**; kedua operand harus himpunan, kalau tidak `GalatTipe` (Python juga menolak `set \| list`) |
| `\|= &= -= ^=` | lewat operator majemuk yang ada (`a = a \| b`) |
| `==` | anggota sama, urutan tidak penting; `himpunan([1]) == [1]` → `salah` |
| kebenaran | `himpunan()` salah |
| `.tambah(x)`, `tambah(s, x)` | menambah anggota (tidak bergalat bila sudah ada) |
| `.hapus(x)` | membuang anggota; tidak ada → `GalatKunci` (Python: KeyError) |
| `.panjang()`, `.terurut()`, `.jumlah()`, `.semua()`, `.ada_yang()` | seperti fungsi bebasnya |
| `s[0]`, `buang s[x]` | `GalatTipe` — himpunan tidak berindeks (Python: TypeError) |
| sebagai kunci kamus / anggota himpunan lain | `GalatTipe` — himpunan tidak hashable, sama seperti `set` Python |
| cetak / `ke_teks` | `himpunan([1, 2])`, `himpunan()` (K17: selalu kode yang sah) |
| `jenis(s)` | `"himpunan"` |

**Urutan:** Python tidak menjamin urutan anggota. Indonesiaku memakai urutan
penyisipan (deterministik, mudah diuji), yang tetap sah untuk koleksi tak
berurutan. Hasil operasi: `|` urutan `a` lalu anggota baru dari `b`; `&` dan
`-` urutan `a`; `^` anggota `a` lalu anggota `b`.

**Sengaja belum:** perbandingan subset `<=`/`<`, metode `gabungan()`/`irisan()`
bernama, `himpunan_beku` (frozenset, hashable), dan `|=` yang mengubah objek di
tempat (sekarang membuat objek baru — sama seperti `+=` pada daftar saat ini).

## 4. Rencana implementasi

| Bagian | Perubahan |
|---|---|
| `src/object.h` / `object.c` | `OBJ_SET`, `ObjSet` dengan tata letak sama persis seperti `ObjDict` (`table` + `keys`) supaya `dictSet`/`dictDelete`/`dictLength`/`dictKeyAt` dipakai ulang lewat `setAsDict()`; `_Static_assert` menjaga tata letaknya; `newSet`, `setAdd`; cetak |
| `src/memory.c` | tandai tabel + daftar urutan, bebaskan |
| `src/value.c` | `hashValue`: himpunan tidak hashable; `valuesEqualDepth` |
| `src/vm.c` | `\| & ^` di `bitwiseBinary` (cabang yang memang disiapkan), `-` di `OP_SUBTRACT`, `dalam`, kebenaran, iterasi (`OP_LIST_LEN`/`OP_SEQ_GET`), metode |
| `src/native.c` | `himpunan()`, `panjang`, `jenis`, `ke_teks`, helper urutan, `tambah`, metode `.tambah`/`.hapus` |
| `editors/vscode-indonesiaku/` | `himpunan` ke BUILTINS |

**GC:** himpunan baru di-root di stack sebelum `dictSet` (yang bisa memperbesar
tabel dan memicu GC); `himpunan("teks")` mengalokasi string per karakter, jadi
setiap anggota juga di-root sebelum disisipkan.

## 5. Pengujian

- Golden `examples/himpunan.idk`: semua baris tabel; dibandingkan dengan
  `python3` lewat `terurut()` (urutan himpunan Python berbeda).
- Galat tertangkap: anggota tak hashable, operand campuran, `.hapus` yang
  tidak ada, indeks, himpunan sebagai kunci kamus.
- `tests/galat/himpunan_anggota_tidak_hashable.idk`.
- `-DDEBUG_STRESS_GC`, ASan/UBSan (WSL), fuzz pengganti.

## 6. Dampak dokumentasi

- [x] `website/panduan/03_PANDUAN_SINTAKS.md`, `07_PUSTAKA_STANDAR.md`
- [x] `editors/vscode-indonesiaku/`
- [x] `README.md`, `CHANGELOG.md`
- [x] `docs/ROADMAP.md`, `docs/ANALISIS/02_PETA_FITUR_PYTHON.md`
- [x] `AGENTS.md`
