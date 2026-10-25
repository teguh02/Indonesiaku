# Rencana Fitur: Argumen bawaan (nilai default parameter)

- **Status:** selesai
- **Target versi:** 0.6.0 (Fase 1)
- **Keputusan terkait:** K12, calon K15

## 1. Masalah

Setiap parameter wajib diisi. Fungsi dengan opsi harus ditulis berulang:

```idk
fungsi sapa(nama, salam) {
    cetak(salam + ", " + nama)
}
sapa("Budi", "Halo")
sapa("Siti")     # Harapkan 2 argumen tetapi mendapat 1.
```

Ini juga yang menghalangi K12 (`cetak(pisah=, akhir=)`) dan sebagian besar
pustaka yang ingin punya opsi.

## 2. Sintaks

```idk
fungsi sapa(nama, salam="Halo") {
    cetak(salam + ", " + nama)
}
sapa("Budi")            # Halo, Budi
sapa("Budi", "Hai")     # Hai, Budi

# Default boleh ekspresi apa pun, termasuk memanggil fungsi
fungsi catat(pesan, waktu=jam()) { ... }

# Metode dan init juga
kelas Kotak {
    init(lebar, tinggi=10) { ... }
}
```

Parameter wajib tidak boleh muncul setelah parameter berdefault:
`fungsi f(a=1, b)` → galat kompilasi.

## 3. Semantik

- `arity` = jumlah seluruh parameter; `requiredArity` = yang tanpa default.
  Panggilan sah bila `requiredArity <= argCount <= arity`.
- **Default dievaluasi setiap kali fungsi dipanggil**, bukan sekali saat
  definisi. Ini **sengaja berbeda dari Python**: `fungsi f(a=[])` memberi
  daftar baru tiap panggilan, jadi jebakan *mutable default argument* yang
  terkenal di Python tidak ada di sini. Lihat §7.
- Karena dievaluasi di dalam frame yang slot parameter sebelumnya sudah
  terisi, default boleh merujuk parameter di kirinya: `fungsi f(a, b=a)`.
- Default yang menaikkan galat menaikkannya di dalam frame fungsi tersebut
  (traceback menunjuk ke fungsi itu).
- Fungsi native tidak mendapat default (belum diperlukan).

## 4. Rencana implementasi

Trik intinya: **prolog bertingkat**. Kode default dipancarkan berurutan di
awal chunk fungsi, dan `call()` melompat ke titik masuk yang sesuai dengan
jumlah argumen yang benar-benar diberikan.

```text
chunk fungsi f(a, b=X, c=Y):
  offset[0] ──► <kode X>        ; dipanggil dengan 1 argumen
  offset[1] ──► <kode Y>        ; dipanggil dengan 2 argumen
  offset[2] ──► <badan fungsi>  ; dipanggil dengan 3 argumen
```

Hasil ekspresi default cukup **di-push**; karena slot parameter diisi
berurutan di stack, push mengisi slot yang hilang secara alami. Tidak perlu
`OP_SET_LOCAL`, tidak perlu nilai sentinel, dan tidak ada opcode baru.

| Bagian | Perubahan |
|---|---|
| `src/object.h` / `object.c` | `ObjFunction` dapat `int requiredArity` dan `int* entryPoints` (panjang `arity - requiredArity + 1`) |
| `src/memory.c` | bebaskan `entryPoints` di `freeObject` |
| `src/compiler.c` | `function()` mem-parsing `= ekspresi`, mencatat offset chunk sebelum tiap default, menolak wajib-setelah-opsional |
| `src/vm.c` | `call()` memvalidasi rentang arity dan menyetel `frame->ip` ke `entryPoints[argCount - requiredArity]` |

**GC-safety:** `entryPoints` adalah array `int` biasa, bukan `Value`, jadi
tidak perlu ditandai. Ekspresi default berjalan sebagai bytecode normal di
dalam frame, jadi GC-safety-nya sama dengan kode lain.

**Jebakan:** `frame->slots = vm.stackTop - argCount - 1` tetap benar meski
argumen kurang, karena dihitung dari `argCount` aktual sebelum prolog
mem-push default. Mengubahnya ke `arity` akan merusak seluruh pengalamatan
lokal.

## 5. Pengujian

Golden `examples/argumen_bawaan.idk` + `tests/expected/argumen_bawaan.out`:

- satu, beberapa, dan semua default diisi/dibiarkan;
- default berupa ekspresi (`2 * 3`), pemanggilan fungsi, daftar, dan kamus;
- daftar default tidak dibagi antar panggilan (bukti beda dari Python);
- default merujuk parameter di kirinya;
- metode kelas dan `init` berdefault;
- fungsi berdefault yang dioper sebagai nilai lalu dipanggil;
- rekursi dengan default.

Tes galat:
- `tests/galat/argumen_wajib_setelah_bawaan.idk` — `fungsi f(a=1, b)`;
- `tests/galat/argumen_kurang.idk` — kurang dari `requiredArity`;
- `tests/galat/argumen_lebih.idk` — lebih dari `arity`.

Plus suite penuh, `-DDEBUG_STRESS_GC`, ASan+UBSan, build C11.

## 6. Dampak dokumentasi

- [ ] `website/panduan/03_PANDUAN_SINTAKS.md` (seksi Fungsi)
- [ ] `README.md`
- [ ] `CHANGELOG.md` + `docs/ROADMAP.md` + `docs/ANALISIS/02_PETA_FITUR_PYTHON.md`
- [ ] `docs/ANALISIS/03_KEPUTUSAN_DESAIN.md` — keputusan baru: default per-panggilan
- [ ] `AGENTS.md` — prolog bertingkat & `entryPoints`
- [ ] `editors/vscode-indonesiaku/` — tidak ada keyword baru

## 7. Risiko & alternatif

**Penyimpangan sadar dari Python.** Python mengevaluasi default sekali saat
`def` dijalankan, sehingga `def f(a=[])` berbagi satu list antar panggilan —
jebakan yang menghabiskan waktu pemula setiap tahun. Kita mengevaluasi per
panggilan. Konsekuensi yang harus diterima: default yang mahal dihitung ulang
tiap panggilan, dan `fungsi f(x=waktu())` memberi waktu saat panggil, bukan
saat definisi (justru yang biasanya diinginkan orang). Keputusan ini dicatat
sebagai K15 agar tidak "diperbaiki" menjadi perilaku Python oleh kontributor
berikutnya.

**Alternatif ditolak:** mengisi argumen yang hilang dengan nilai sentinel lalu
memeriksanya di prolog dengan opcode kondisional. Ditolak karena butuh tipe
nilai baru (atau menyalahgunakan `kosong`, yang merupakan nilai sah) dan
opcode tambahan, padahal tabel titik masuk tidak butuh keduanya.

**Di luar lingkup:** argumen bernama (`f(b=2)` di sisi pemanggil) dan `cetak`
sebagai fungsi biasa dengan `pisah=`/`akhir=`. Keduanya butuh nama parameter
tersedia saat runtime dan penyusunan ulang stack di sisi pemanggil — rencana
tersendiri setelah ini.
