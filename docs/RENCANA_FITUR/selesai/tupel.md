# Rencana Fitur: Tupel (`tupel`)

- **Status:** selesai (2026-10-25)
- **Target versi:** Fase 2
- **Pemilik:** teguh02
- **Keputusan terkait:** K17 (sintaks, disetujui 2026-10-25), K8 (kunci kamus hashable), K7 (`==` struktural)

## 1. Masalah

Tidak ada urutan yang tidak bisa diubah. Akibatnya:

```
koordinat = (3, 4)           # Kesalahan di ',': Harapkan ')' setelah ekspresi.
lokasi = {}
lokasi[[3, 4]] = "rumah"     # GalatTipe: daftar tidak hashable — tidak ada pengganti
```

Pasangan nilai (koordinat, rentang tanggal, baris basis data) dan kunci
kamus majemuk tidak bisa ditulis seperti di Python.

## 2. Sintaks (K17)

```indonesiaku
titik = (3, 4)
satu = (5,)          # koma wajib untuk satu elemen
hampa = ()       # tupel kosong
bukan_tupel = (5)    # pengelompokan biasa -> 5

x, y = titik         # unpacking
cetak(titik[0], titik[-1], titik[0:1], panjang(titik))
jarak = {}
jarak[(0, 0)] = 0    # tupel bisa jadi kunci kamus
cetak(tupel([1, 2]), tupel("ab"), tupel(rentang(3)))
```

Tupel tanpa kurung (`x = 1, 2`, `kembalikan a, b`) **tidak** termasuk
(lihat K17 — belum diputuskan).

## 3. Semantik (mengikuti Python 3)

| Operasi | Hasil |
|---|---|
| `(a, b)`, `(a,)`, `()`, koma akhir `(a, b,)` | tupel; `(a)` tetap pengelompokan |
| `t[i]`, indeks negatif | elemen; di luar jangkauan → `GalatIndeks` |
| `t[a:b:c]` | **tupel** baru |
| `panjang`, `untuk x dalam t`, comprehension, `enumerasi`, `jumlah`, `min`/`max`, `terurut` (→ daftar), `dalam` | seperti daftar |
| `a, b = t` | unpacking |
| `t1 + t2`, `t * n` | tupel baru; `daftar + tupel` → `GalatTipe` (seperti Python) |
| `==` | struktural antar tupel; `(1,) == [1]` → `salah` (beda tipe, seperti Python) |
| hash / kunci kamus | boleh bila semua elemennya hashable; `([1],)` → `GalatTipe` |
| kebenaran | `()` salah, lainnya benar |
| `t[i] = x`, `buang t[i]`, `tambah(t, x)`, `sisip`, `hapus`, `hapus_pada` | `GalatTipe`: "Tupel tidak dapat diubah." |
| cetak / `ke_teks` | `(1, 2)`, `(1,)`, `()`; teks di dalamnya dikutip (repr) |
| `jenis(t)` | `"tupel"` |
| `tupel()`, `tupel(urutan)` | dari daftar, teks, rentang, kamus (kuncinya), tupel |

Belum: perbandingan `<` (daftar pun belum punya), metode `.hitung()`/`.indeks()`
(daftar pun belum punya) — keduanya menyusul bersama untuk daftar dan tupel.

## 4. Rencana implementasi

| Bagian | Perubahan |
|---|---|
| `src/object.h` / `object.c` | `OBJ_TUPLE`, `ObjTuple` (tata letak sama dengan `ObjList`: `Obj` + `ValueArray items`), `newTuple`, helper `sequenceItems()` (item daftar/tupel untuk jalur baca), cetak |
| `src/memory.c` | tandai & bebaskan |
| `src/value.c` | `hashValue` (gabungan hash elemen), `valuesEqualDepth` |
| `src/chunk.h` / `debug.c` | `OP_BUILD_TUPLE` |
| `src/compiler.c` | `grouping()`: `()` dan koma setelah ekspresi pertama → tupel |
| `src/vm.c` | build, unpack, indeks, irisan, panjang/iterasi, `+`, `*`, `dalam`, kebenaran; set/hapus menolak |
| `src/native.c` | `tupel()`, `panjang`, `jenis`, `ke_teks`/repr, helper urutan, `enumerasi`, penolakan di builtin pengubah daftar |
| `src/db.c` | parameter SQL boleh tupel (seperti `sqlite3` Python) |
| `editors/vscode-indonesiaku/` | `tupel` ke BUILTINS |

**GC:** setiap tupel baru di-root di stack VM sebelum `writeValueArray`
(pola `OP_BUILD_LIST`). `tupel("teks")` mengalokasi string per karakter, jadi
tupelnya harus ter-root selama pengisian.

## 5. Pengujian

- Golden `examples/tupel.idk`: semua baris tabel di atas; setiap baris
  non-galat dibandingkan dengan `python3`.
- Galat tertangkap di golden: indeks luar jangkauan, mengubah, unpacking
  salah jumlah, kunci tidak hashable, `daftar + tupel`.
- `tests/galat/tupel_tidak_dapat_diubah.idk`.
- `-DDEBUG_STRESS_GC`, ASan/UBSan (WSL), fuzz pengganti.

## 6. Dampak dokumentasi

- [x] `website/panduan/03_PANDUAN_SINTAKS.md` (bagian tupel), `07_PUSTAKA_STANDAR.md` (`tupel()`)
- [x] `editors/vscode-indonesiaku/`
- [x] `README.md`, `CHANGELOG.md`
- [x] `docs/ROADMAP.md`, `docs/ANALISIS/02_PETA_FITUR_PYTHON.md`
- [x] `AGENTS.md`
