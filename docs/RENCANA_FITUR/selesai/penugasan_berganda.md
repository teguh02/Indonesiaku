# Rencana Fitur: penugasan berganda & unpacking

- **Status:** selesai
- **Target versi:** 0.8.0
- **Pemilik:** —
- **Ref:** Fase 2 di `docs/ROADMAP.md`

## 1. Masalah

Tidak ada cara menukar dua nilai atau mengambil beberapa hasil sekaligus:

```indonesiaku
# harus pakai variabel sementara
t = a
a = b
b = t
```

Python menulis `a, b = b, a`, dan fungsi yang mengembalikan beberapa nilai
sebagai daftar tidak bisa langsung dipecah.

## 2. Sintaks usulan

```indonesiaku
a, b = 1, 2
a, b = b, a
x, y, z = [10, 20, 30]
p, q = "hi"            # per karakter
m, n = rentang(2, 4)
g, h = dua()           # dari hasil fungsi
```

## 3. Semantik

- Sisi kanan boleh berupa daftar nilai (`1, 2`) atau **satu** urutan yang
  dapat dibuka: daftar, teks (per karakter), rentang.
- Jumlah elemen harus sama dengan jumlah nama; selain itu `GalatNilai`
  ("Tidak dapat membuka N nilai menjadi M nama.") yang bisa ditangkap.
- Sisi kanan dievaluasi seluruhnya sebelum nama mana pun di-assign, jadi
  `a, b = b, a` menukar dengan benar.
- Nama mengikuti aturan assignment biasa (K3): di dalam fungsi menjadi lokal,
  termasuk bila berada di dalam blok `jika`/`selagi`, karena
  `prescanFunctionBody` kini melacak rantai koma.
- Target hanya nama; `d[0], d[1] = ...` belum didukung.

## 4. Rencana implementasi

| Bagian | Perubahan |
|---|---|
| `src/compiler.c` | `looksLikeUnpack` (lookahead `IDENT (, IDENT)+ =`), `unpackStatement`, `assignToName`; `prescanFunctionBody` melacak rantai koma |
| `src/vm.c` | opcode `OP_UNPACK n`: validasi panjang, dorong nilai dengan urutan terbalik agar nama pertama di atas stack |
| `src/chunk.h`, `src/debug.c` | nama opcode |

**GC-safety:** `OP_UNPACK` tidak melepas urutan sumber (`peek`, bukan `pop`)
selama mengekstrak elemen teks — pembuatan `ObjString` di dalamnya bisa memicu
GC — lalu menggeser elemen ke bawah dengan `memmove` manual.

## 5. Pengujian

- Golden: `examples/penugasan_berganda.idk` (tukar, list, teks, rentang, hasil
  fungsi, lokal di dalam blok).
- Tes galat: `tests/galat/unpack_jumlah_salah.idk`.
- Regresi K3: `examples/lingkup_assignment.idk`, `examples/lingkup_variabel.idk`.
- `-DDEBUG_STRESS_GC`: keluaran identik.

## 6. Dampak dokumentasi

- [x] `website/panduan/03_PANDUAN_SINTAKS.md`
- [x] `README.md`, `CHANGELOG.md`, `docs/ROADMAP.md`, `AGENTS.md`
- [x] `editors/vscode-indonesiaku/` (snippet `unpack`)

## 7. Risiko & alternatif

- **Alternatif ditolak:** menambah tipe `tupel` dulu (Fase 2 juga) agar sisi
  kanan `1, 2` menjadi literal tupel. Tidak perlu: daftar sudah cukup, dan
  tupel tetap direncanakan terpisah.
- **Batas yang diterima:** tanpa `*sisa`, tanpa target indeks/atribut, tanpa
  unpacking bersarang `a, (b, c) = ...`.
