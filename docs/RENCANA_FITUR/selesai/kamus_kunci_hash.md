# Rencana Fitur: kunci kamus non-teks (K8)

- **Status:** selesai
- **Target versi:** 0.8.0
- **Pemilik:** —
- **Ref:** K8, Fase 2 di `docs/ROADMAP.md`

## 1. Masalah

Kamus hanya bisa berkunci teks:

```indonesiaku
d = {}
d[1] = "satu"      # GalatTipe: Kunci kamus harus berupa teks.
```

Selain itu `kamus[0]` dipakai sebagai "kunci pertama" (indeks posisi), padahal
Python memakai angka sebagai kunci sungguhan. Dua kebutuhan ini tidak bisa
dipenuhi oleh tabel berkunci `ObjString*`.

## 2. Sintaks usulan

```indonesiaku
d = {1: "satu", 2.5: "dua koma lima", benar: "ya", kosong: "hampa", "a": 1}
cetak(d[1], d[2.5], d[benar], d[kosong])
cetak(d[1.0])        # kunci yang sama dengan d[1]
```

## 3. Semantik

- Kunci hashable: teks, `bulat`, `desimal`, `benar`/`salah`, `kosong`, dan objek
  (identitas). Daftar, kamus, rentang → `GalatTipe` **sebelum** penyisipan.
- Kesetaraan kunci memakai `valuesEqual`, jadi `1` dan `1.0` adalah kunci yang
  sama (`hashValue` menormalkan desimal utuh ke int64).
- Urutan penyisipan dipertahankan: `d.keys` tetap daftar kunci hidup yang
  padat (penghapusan menggeser, tanpa tombstone).
- **`kamus[angka]` berubah arti**: dari "kunci ke-n" menjadi "kunci angka".
  Untuk posisi pakai `kunci(kamus)[i]`. Ini perubahan yang merusak.
- `untuk k dalam kamus` tetap per kunci sesuai urutan penyisipan (opcode baru
  `OP_SEQ_GET`, karena `OP_INDEX_GET` kini murni pencarian kunci).
- `K6` (GalatKunci saat kunci tidak ada) belum diberlakukan: `d[kunci]` yang
  tidak ada masih mengembalikan `kosong`.

## 4. Rencana implementasi

| Bagian | Perubahan |
|---|---|
| `src/table.h/.c` | tipe baru `ValueTable`/`ValueEntry` (state 0/1/2) khusus kamus; tabel string tetap untuk global/intern/metode/field |
| `src/value.c` | `hashValue` + dokteran hash desimal utuh; `valuesEqual` kamus memakai `valueTableGet` |
| `src/object.c` | `ObjDict.table` menjadi `ValueTable`; `dictSet/dictDelete/dictKeyAt` menerima `Value`; kunci yang dihapus dibuang dari daftar urutan |
| `src/vm.c` | `OP_BUILD_DICT`, `OP_INDEX_GET/SET/DELETE`, `OP_IN` menerima kunci apa pun; opcode baru `OP_SEQ_GET` untuk iterasi |
| `src/native.c`, `src/db.c` | `punya`, `hapus_kunci`, `kunci`, pencetakan kamus, `dictSet` berkunci string dibungkus `OBJ_VAL` |
| `src/memory.c` | `markValueTable`/`freeValueTable` |

**GC-safety:** pola lama dipertahankan — kunci dan nilai tetap di stack saat
`dictSet`/`writeValueArray` (keduanya bisa memicu GC); `ValueTable` hanya
menyimpan salinan `Value`, dan `markValueTable` menandai kunci maupun nilai.

## 5. Pengujian

- Golden: `examples/kamus_kunci.idk` (semua tipe kunci, `1` vs `1.0`, iterasi,
  `buang`, kunci tidak hashable).
- Regresi: `enumerasi`, `kamus_irisan`, `json_pakai`, `db_sqlite`, `todo_cli`
  (semua memakai kamus).
- Tes galat: `tests/galat/kamus_kunci_bukan_teks.idk` kini harus tetap gagal
  untuk kunci yang tidak hashable — pesannya diperbarui.
- `-DDEBUG_STRESS_GC`: keluaran identik.

## 6. Dampak dokumentasi

- [x] `website/panduan/03_PANDUAN_SINTAKS.md`, `07_PUSTAKA_STANDAR.md`
- [x] `CHANGELOG.md` (⚠️), `docs/ROADMAP.md`, `AGENTS.md`

## 7. Risiko & alternatif

- **Alternatif ditolak:** memakai satu `Table` berkunci `Value` untuk semuanya —
  memperlambat global/metode dan menyentuh intern string yang butuh
  `ObjString*` untuk deduplikasi.
- **Batas yang diterima:** belum ada `GalatKunci` untuk kunci yang hilang (K6),
  dan `benar`/`1` sengaja tetap kunci berbeda sampai spesifikasi memutuskan
  bool-sebagai-subkelas-int.
