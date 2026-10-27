# Rencana Fitur: pewarisan berganda + MRO C3

- **Status:** selesai
- **Target versi:** 1.1.0
- **Pemilik:** Teguh Rijanandi

## 1. Masalah

Sebuah kelas hanya boleh punya satu induk. Pola campuran (*mixin*) — perilaku
kecil yang dipakai ulang beberapa kelas tanpa menaruhnya di satu induk besar —
tidak bisa ditulis.

```python
kelas BisaDicetak { tampilkan() { cetak(diri.ringkas()) } }
kelas BisaDisimpan { simpan() { ... } }

kelas Dokumen < BisaDicetak, BisaDisimpan { ... }   # sekarang: galat
```

## 2. Sintaks usulan

Induk dipisah koma, urutan menentukan prioritas:

```python
kelas Dokumen < BisaDicetak, BisaDisimpan {
    ringkas() { kembalikan "dokumen" }
}
```

Yang kiri menang saat nama sama muncul di beberapa induk.

## 3. Semantik: MRO C3

Urutan pencarian metode (*method resolution order*) dihitung dengan
linearisasi C3, persis seperti Python:

```
L(C) = C + merge(L(B1), L(B2), ..., [B1, B2, ...])
```

Sifat yang dijamin C3 dan menjadi alasan memilihnya:

- **Urutan lokal terjaga** — kalau `C < A, B`, maka `A` selalu sebelum `B`.
- **Monotonik** — urutan induk tidak pernah terbalik di anak.
- Kalau tak ada urutan yang konsisten, **kelasnya ditolak saat dibuat**,
  bukan diam-diam memilih salah satu.

Contoh diamond:

```python
kelas A { nama() { kembalikan "A" } }
kelas B < A { nama() { kembalikan "B" } }
kelas C < A { nama() { kembalikan "C" } }
kelas D < B, C { }

cetak(D().nama())      # B
# MRO: D, B, C, A
```

Perhatikan `A` muncul **sekali**, setelah `B` dan `C` — bukan langsung setelah
`B`. Itulah bedanya C3 dengan telusur kiri-dalam yang naif.

**MRO yang mustahil ditolak:**

```python
kelas X < A, B { }
kelas Y < B, A { }
kelas Z < X, Y { }     # galat: tidak ada urutan yang konsisten
```

## 4. Rencana implementasi

**Kunci kemalasan: pencarian metode tidak berubah sama sekali.** `OP_INHERIT`
sudah meratakan metode induk ke subkelas lewat `tableAddAll`, jadi
`tableGet(&klass->methods, ...)` tetap satu lookup. Untuk banyak induk, cukup
salin dari MRO **dari belakang ke depan** — yang lebih awal di MRO menimpa
yang belakangan. Jalur panas nol perubahan.

Yang benar-benar butuh MRO tersimpan hanyalah yang menelusuri rantai:
`classFieldGet`, `classMethodGet`, `specialKind`, `adalah_instansi`, dan
penanda galat.

1. `src/object.h` — `ObjClass` mendapat `ObjClass** mro; int mroCount;`.
   `superclass` **tetap ada** dan berisi induk pertama, supaya `super` dan
   kelas galat bawaan tidak perlu disentuh.
2. `src/memory.c` — **tiga edit wajib**: alokasi array ikut dibebaskan di
   `freeObject`, tiap elemen di-`markObject` di `blackenObject`, dan
   ukurannya ikut dihitung GC. Array `ObjClass*` adalah akar GC sama seperti
   tabel.
3. `src/vm.c` — `c3Linearize(ObjClass* klass, ObjClass** bases, int n)`:
   merge C3 di atas array, melapor galat bila mandek. Dipanggil dari
   `OP_INHERIT` varian banyak-induk.
4. `src/chunk.h`/`compiler.c` — `OP_INHERIT` memakai operand jumlah induk;
   `classDeclaration` mengurai daftar dipisah koma.
5. Enam penelusuran rantai diganti penelusuran MRO.

**Batas:** jumlah induk langsung dibatasi (`UINT8_MAX` lewat operand) dan
panjang MRO dibatasi wajar; keduanya dilaporkan sebagai galat, bukan ditimpa
diam-diam.

## 5. Rencana uji

- Golden `examples/oop/pewarisan_berganda.idk` + expected.
- Kasus: dua induk sederhana; prioritas kiri; **diamond** dengan MRO
  `D, B, C, A` dibuktikan lewat urutan cetak; tiga induk; mixin tanpa induk
  bersama; atribut kelas lewat MRO; `@properti`/`@metode_statis` warisan dari
  induk kedua; metode ajaib (`_teks_`) dari induk kedua; `adalah_instansi`
  benar untuk semua leluhur; `super` tetap menunjuk induk pertama.
- Galat: MRO mustahil (`Z < X, Y`); induk duplikat (`kelas C < A, A`);
  mewarisi diri sendiri; induk bukan kelas.
- Wajib lewat build sanitizer atas seluruh golden + `tests/galat/` — ada
  alokasi array baru pada objek heap.

## 6. Dampak dokumentasi & tooling

- [x] `website/panduan/03_PANDUAN_SINTAKS.md`
- [x] `README.md`, `CHANGELOG.md`, `docs/ROADMAP.md`,
      `docs/ANALISIS/02_PETA_FITUR_PYTHON.md`
- [x] `AGENTS.md` bila ada jebakan baru
- [x] Rencana ini → `selesai/`

## 7. Risiko & alternatif

**Risiko utama: metode yang sudah diratakan vs MRO yang ditelusuri.** Dua
mekanisme hidup berdampingan — `methods` diratakan, `special`/`fields`
ditelusuri. Jebakan ini sudah menggigit sekali di `@properti` (penanda tidak
ikut tersalin `OP_INHERIT`). Karena itu penelusuran `special` **wajib** lewat
MRO, bukan lewat `superclass` saja.

**Alternatif yang ditolak: tidak meratakan metode, selalu telusuri MRO saat
lookup.** Lebih "benar" secara konsep dan menghemat memori, tapi mengubah
setiap pemanggilan metode dari satu lookup menjadi perulangan sepanjang MRO —
jalur terpanas di VM. Tidak dibayar oleh manfaatnya.

**Alternatif yang ditolak: telusur kiri-dalam (DFS) tanpa C3.** Jauh lebih
pendek, tapi salah pada diamond: `A` akan muncul sebelum `C`, sehingga
`D().nama()` bisa mengambil metode leluhur alih-alih saudara yang lebih
dekat. Bug seperti ini sangat sulit dilacak pengguna — kemalasan yang salah
tempat.


## 8. Catatan hasil

- **`own` tidak ada di rencana dan ternyata wajib.** Meratakan dari
  `methods` induk salah untuk cucu: `B.methods` sudah memuat `f()` warisan
  `A`, sehingga menyalinnya ke `D` menimpa `C.f()` yang menurut C3 harus
  menang. Ketahuan lewat uji `S < Q, R` yang hanya `R`-nya menimpa.
- **Kelas berinduk tunggal tidak mengalokasi MRO sama sekali.** `AncestorIter`
  jatuh balik ke rantai `superclass` saat `mroCount == 0`, jadi kelas galat
  bawaan yang dibuat dari C juga tidak perlu disentuh.
- `MRO_MAX` 32, bukan 128: worklist merge berbentuk array persegi di stack C.
