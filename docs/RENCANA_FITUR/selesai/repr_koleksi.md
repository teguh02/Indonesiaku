# Rencana Fitur: `repr` untuk elemen koleksi

- **Status:** selesai
- **Target versi:** 0.7.0 (sisa Fase 1)
- **Keputusan terkait:** K12 (sisa)

## 1. Masalah

Teks di dalam koleksi dicetak tanpa kutip, sehingga isinya ambigu:

```indonesiaku
cetak(["a", 1])          # [a, 1]      <- "a" dan 1 tak terbedakan
cetak(["1", 1])          # [1, 1]
cetak([""])              # []          <- terlihat seperti daftar kosong
cetak(["a, b"])          # [a, b]      <- terlihat seperti dua elemen
```

Python membedakan `str` dan `repr`: `str("a")` → `a`, tetapi `str(["a"])` →
`['a']`. Kita belum.

## 2. Akar masalah: dua jalur cetak, bukan satu

Pemeriksaan sebelum menulis kode — ada **tiga** tempat yang memformat nilai,
dan dua di antaranya punya bug yang sama:

| Jalur | Dipakai oleh | Teks bersarang |
|---|---|---|
| `printObject()` di `src/object.c` | `cetak`, `printValue` | tanpa kutip ❌ |
| `appendValueText()` di `src/native.c` | `ke_teks`, `format`, f-string | tanpa kutip ❌ |
| `nativeFnRepr()` di `src/native.c` | `repr` | dikutip, **tanpa escape** ❌ |

Menambal hanya `printObject` akan membuat `cetak(x)` dan `ke_teks(x)`
menghasilkan string berbeda untuk nilai yang sama. Ketiganya harus diperbaiki
bersamaan.

Selain itu ketiganya tidak meng-escape apa pun, jadi `repr("a\"b")` hari ini
menghasilkan `"a"b"` yang tidak bisa dibaca balik.

## 3. Desain

Aturan escape **ditaruh di satu tempat** sebagai helper kecil:

```c
// object.h — NULL bila karakter tidak perlu di-escape.
const char* stringEscape(char c);
```

Mengembalikan `\\"`, `\\\\`, `\\n`, `\\t`, `\\r` untuk karakter terkait.
Mekanisme keluarannya tetap lokal (satu menulis ke `stdout`, satu ke buffer),
tetapi **aturannya tunggal** sehingga ketiga jalur tidak bisa menyimpang.

Yang berubah:

- `printObject`: elemen daftar, nilai kamus, dan **kunci** kamus dicetak
  dalam bentuk repr.
- `appendValueText`: sama, lewat parameter `bool repr`.
- `nativeFnRepr`: memakai aturan escape yang sama.

Yang **tidak** berubah: `cetak("a")` tetap `a`, `ke_teks("a")` tetap `a`.
Hanya saat teks berada **di dalam** koleksi.

## 4. Risiko

- **Banyak golden berubah.** Setiap contoh yang mencetak daftar/kamus berisi
  teks akan berbeda. Harus dibaca satu per satu, bukan di-regenerate buta.
- **Kedalaman rekursi.** `appendValueText` sudah rekursif tanpa pelindung
  siklus (utang lama, di luar lingkup ini) — tidak diperburuk.
- Kunci kamus yang mengandung `"` sebelumnya menghasilkan keluaran rusak;
  ini sekalian memperbaikinya.

## 5. Pengujian

- `cetak(["a", 1, benar, kosong])`, kamus bersarang, daftar dalam daftar
- teks berisi `"`, `\`, newline, tab di dalam koleksi
- `cetak("a")` dan `ke_teks("a")` **tetap** tanpa kutip (tidak ada regresi)
- `cetak(x)` dan `ke_teks(x)` menghasilkan teks identik untuk koleksi
- `repr("a\"b")` bisa dibaca balik
- f-string dan `format` ikut benar karena lewat `appendValueText`

## 6. Dampak dokumentasi

- [x] `website/panduan/03_PANDUAN_SINTAKS.md` — seksi Teks di Dalam Koleksi (2 blok diuji); 6 komentar hasil di panduan 03/07 yang jadi basi ikut diperbaiki dan diverifikasi
- [x] `README.md` + `CHANGELOG.md`
- [x] `docs/ROADMAP.md` — butir ⬜ → ✅
- [x] `docs/ANALISIS/03_KEPUTUSAN_DESAIN.md` — K12 jadi terlaksana penuh
- [ ] `AGENTS.md` — tertunda: berkas diproteksi Hermes, perlu persetujuan interaktif pemilik repo

## 7. Hasil (2026-10-25)

```text
tests/test_all.sh                  124/124 PASSED
ASan+UBSan+DEBUG_STRESS_GC         bersih (300 iterasi kamus/escape bersarang)
```

Catatan dari pengerjaan:

- Dugaan awal "tambal `printObject`" **salah**: ada dua jalur cetak terpisah
  (`printObject` dan `appendValueText`) plus `repr`. Menambal satu akan
  membuat `cetak(x)` dan `ke_teks(x)` berbeda. Ketiganya kini memakai satu
  `stringEscape()`.
- `repr` lama tidak meng-escape apa pun, jadi `repr("a\"b")` menghasilkan
  `"a"b"` yang tidak bisa dibaca balik. Ikut diperbaiki.
- **Bug lain yang ketemu saat berada di fungsi yang sama:** `ObjDict`
  menyimpan `keys` untuk urutan sisip, tetapi kedua jalur cetak mengiterasi
  tabel hash. Akibatnya `cetak(d)` bisa berbeda urutan dari `kunci(d)` dan
  dari `untuk ... dalam d` pada kamus yang sama. Diperbaiki sekalian —
  membiarkannya berarti mengunci urutan yang salah ke dalam golden baru.
- 7 golden berubah, semuanya diperiksa satu per satu: tidak ada perubahan di
  luar teks-dalam-koleksi.
