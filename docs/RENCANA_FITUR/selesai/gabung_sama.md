# Rencana Fitur: Gabung/ulang teks & daftar, dan kesetaraan struktural

- **Status:** selesai
- **Target versi:** 0.6.0 (Fase 1)
- **Keputusan terkait:** K7

## 1. Masalah

Dua kekurangan yang terasa langsung setelah irisan mendarat:

1. Daftar hasil irisan tidak bisa digabung:
   ```idk
   kembalikan xs[:2] + xs[-2:]
   # Operand harus dua angka atau dua string.
   ```
   Pengulangan juga tidak ada: `"-" * 20` dan `[0] * 5` gagal.
2. `==` pada daftar/kamus membandingkan **identitas**, bukan isi:
   ```idk
   cetak([1, 2] == [1, 2])   # salah — padahal isinya sama
   ```
   Ini melanggar K7 dan mengejutkan pengguna yang datang dari Python.

## 2. Sintaks

Tidak ada token atau kata kunci baru; hanya memperluas operator yang sudah ada.

```idk
# Gabung
cetak([1, 2] + [3])          # [1, 2, 3]
cetak("ha" + "lo")           # halo (sudah ada)

# Ulang
cetak([0] * 3)               # [0, 0, 0]
cetak(3 * [0])               # [0, 0, 0]
cetak("-" * 5)               # -----
cetak("ab" * 2)              # abab

# Kesetaraan struktural
cetak([1, [2]] == [1, [2]])              # benar
cetak({"a": 1} == {"a": 1})              # benar
cetak([1, 2] adalah [1, 2])              # salah — identitas
```

## 3. Semantik

### `+`
- `daftar + daftar` → daftar baru (salinan dangkal kedua sisi).
- `teks + teks` → tetap seperti sekarang.
- Campuran (`daftar + teks`, `daftar + angka`) → `GalatTipe` dengan pesan jelas.

### `*`
- `daftar * n` dan `n * daftar` → daftar baru berisi `n` salinan dangkal.
- `teks * n` dan `n * teks` → teks berulang.
- `n` harus bilangan bulat; `n <= 0` menghasilkan daftar/teks kosong
  (seperti Python). `n` pecahan → `GalatTipe`.
- Batas aman: hasil lebih dari `UINT16_MAX` elemen/karakter → `GalatNilai`,
  agar `[0] * 1e9` tidak menghabiskan memori diam-diam.

### `==` struktural (K7)
- Daftar: sama panjang dan setiap elemen `==` secara rekursif.
- Kamus: sama jumlah kunci hidup, setiap kunci ada di keduanya dan nilainya `==`.
  Urutan penyisipan **tidak** memengaruhi kesetaraan.
- Rentang: sama `start`, `stop`, `step`.
- Teks: sudah benar (string di-intern).
- Instansi kelas, fungsi, closure, berkas, soket: tetap **identitas**.
  (`_sama_` pada instansi ditunda — belum ada kebutuhan nyata.)
- `adalah` / `bukan adalah` tetap **identitas** dan karenanya harus memakai
  fungsi terpisah, bukan `valuesEqual` yang kini struktural.

Struktur melingkar (daftar yang memuat dirinya) akan membuat perbandingan
rekursif tak berujung. Dibatasi kedalaman rekursi; melewati batas dianggap
tidak sama daripada membuat VM mogok.

## 4. Rencana implementasi

| Bagian | Perubahan |
|---|---|
| `src/value.c`, `src/value.h` | `valuesEqual` jadi struktural (rekursif berbatas); tambah `valuesIdentical` untuk identitas |
| `src/vm.c` | `OP_ADD` menerima daftar; `OP_MULTIPLY` menerima daftar/teks × angka; `chainCompare` memakai `valuesIdentical` untuk `adalah` |
| `src/object.c` | — (tidak ada tipe baru) |

**GC-safety:** penggabungan/pengulangan daftar menahan daftar hasil di stack
VM selama `writeValueArray`; operand sumber tetap di stack (`peek`, bukan
`pop`) sampai hasil selesai dibangun. Pengulangan teks memakai pola yang sama
dengan `concatenate()`.

**Jebakan:** `valuesEqual` dipakai juga oleh `dalam` (`containsValue`) dan
`OP_EQUAL`; membuatnya struktural otomatis membuat `[1,2] dalam [[1,2]]`
bernilai benar — itu memang perilaku Python dan diinginkan. Yang **tidak**
boleh ikut berubah adalah `adalah`.

## 5. Pengujian

Golden `examples/gabung_sama.idk` + `tests/expected/gabung_sama.out`:
- gabung daftar, gabung hasil irisan, daftar kosong;
- ulang daftar/teks di kedua arah, faktor 0 dan negatif;
- hasil gabung/ulang adalah objek baru (mengubahnya tidak menyentuh sumber);
- `==` struktural untuk daftar bersarang, kamus, rentang, dan campuran tipe;
- kamus dengan urutan penyisipan berbeda tetap sama;
- `adalah` tetap identitas; `x adalah x` benar;
- `dalam` memakai kesetaraan struktural.

Tes galat:
- `tests/galat/gabung_tipe_salah.idk` — `[1] + "a"`;
- `tests/galat/ulang_pecahan.idk` — `[1] * 2.5`;
- `tests/galat/ulang_terlalu_besar.idk` — `[1] * 999999`.

Plus: suite penuh, `-DDEBUG_STRESS_GC`, ASan+UBSan, build C11.

## 6. Dampak dokumentasi

- [x] `website/panduan/03_PANDUAN_SINTAKS.md`
- [x] `README.md`
- [x] `CHANGELOG.md` (⚠️ `==` berubah) + `docs/ROADMAP.md` + `docs/ANALISIS/02_PETA_FITUR_PYTHON.md`
- [x] `AGENTS.md` (pemisahan `valuesEqual` vs `valuesIdentical`)
- [ ] `editors/vscode-indonesiaku/` — tidak ada keyword/builtin baru

## 7. Risiko & alternatif

**Risiko utama:** `valuesEqual` dipanggil dari beberapa tempat (`OP_EQUAL`,
`containsValue`, `chainCompare`). Mengubahnya di satu titik mengubah semua
pemanggil sekaligus — itu justru yang diinginkan (perbaikan akar), kecuali
untuk `adalah` yang harus dipisahkan secara eksplisit. Lupa memisahkan
`adalah` adalah kegagalan paling mungkin, jadi golden wajib mengujinya.

**Alternatif ditolak:** menambah builtin `sama_dengan(a, b)` alih-alih
mengubah `==`. Ditolak karena K7 sudah disetujui dan pengguna Python akan
tetap menulis `==`; menyediakan dua cara hanya menambah kebingungan.
