# Rencana Fitur: Fase 3 — ObjModule, `impor x`, `dari ... impor ...`

- **Status:** selesai
- **Target versi:** 0.9 (menutup Fase 3)
- **Keputusan terkait:** K3 (lingkup), K14 (stabilitas)

## 1. Masalah

`impor "berkas.idk"` yang ada **berbagi satu namespace global**: modul yang
diimpor menulis langsung ke `vm.globals`, sehingga dua modul dengan nama fungsi
sama saling menimpa tanpa peringatan. Tidak ada objek modul, tidak ada
`modul.anggota`, tidak ada jalur pencarian, dan tidak ada cara sebuah berkas
tahu apakah ia dijalankan langsung atau diimpor.

Sisa Fase 3 di ROADMAP: `ObjModule` dengan namespace, `impor x`,
`dari x impor y sebagai z`, paket, jalur pencarian (`INDK_PATH`), `__utama__`.

## 2. Sintaks

```indonesiaku
impor matematika                      # modul -> nama `matematika`
impor matematika sebagai mtk          # dengan alias
dari matematika impor mat_fpb         # satu anggota
dari matematika impor mat_fpb sebagai fpb
dari matematika impor mat_fpb, mat_kpk

cetak(matematika.mat_fpb(12, 18))

impor "berkas.idk"                    # BENTUK LAMA, tetap jalan apa adanya
```

Nama modul adalah identifier, bukan teks. `impor a.b` (paket) memakai titik.

## 3. Semantik

### ObjModule

Tipe objek baru `OBJ_MODULE`: `name`, `path`, dan `Table globals` miliknya
sendiri. **Setiap** eksekusi berjalan di dalam sebuah modul — termasuk skrip
utama, yang modulnya bernama `__utama__`.

Pemisahan namespace dikerjakan di akar, bukan ditambal per opcode:

- `vm.globals` menjadi **tabel builtin saja** (native, kelas galat, `argumen`).
- Setiap modul punya tabel globalnya sendiri.
- `ObjFunction` menyimpan `module` tempat ia dikompilasi, sehingga fungsi yang
  didefinisikan di modul A tetap membaca global A walau dipanggil dari B.
- Baca global: tabel modul → `vm.globals` (builtin). Tulis/hapus: tabel modul.

Dengan begitu sebuah modul tidak bisa lagi menimpa global modul lain, dan
builtin tetap terlihat dari mana saja.

### `__nama__` dan `__utama__`

Setiap modul mendapat global `__nama__`. Untuk skrip yang dijalankan langsung
nilainya `"__utama__"`; untuk modul yang diimpor, nama modulnya. Pola Python:

```indonesiaku
jika __nama__ == "__utama__" {
    utama()
}
```

### Jalur pencarian

`impor nama` mencari `nama.idk` berurutan di:

1. direktori berkas yang mengimpor;
2. setiap entri `INDK_PATH` (dipisah `:`, atau `;` di Windows);
3. `pustaka/` relatif terhadap direktori kerja.

`impor a.b` memetakan titik ke pemisah direktori: `a/b.idk`. Modul dicache
berdasar **jalur absolut yang sudah di-resolve**, sehingga dua nama berbeda
yang menunjuk berkas sama hanya dieksekusi sekali.

### Kompatibilitas (K14)

`impor "berkas.idk"` **tidak berubah**: tetap menjalankan berkas di dalam
modul pemanggil (namespace bersama), persis seperti sekarang. Ini yang menjaga
`pustaka/*.idk` dan seluruh contoh lama tetap jalan. Bentuk teks adalah jalur
lama; bentuk identifier adalah jalur bermodul.

## 4. Berkas tersentuh

- `src/object.h`/`.c` — `OBJ_MODULE`, `ObjModule`, `newModule`.
- `src/memory.c` — tandai `name`/`path`/`globals`; bebaskan tabelnya.
- `src/vm.h`/`.c` — `vm.builtins` vs global modul, `vm.mainModule`,
  resolusi global per-frame, `OP_IMPORT_MODULE`, `OP_IMPORT_FROM`,
  `moduleResolvePath`, `__nama__`.
- `src/compiler.c` — `importStatement` mengurai bentuk identifier dan `dari`.
- `src/chunk.h`/`src/debug.c` — opcode baru.
- `src/scanner.c`/`.h` — kata kunci `dari` (cek: mungkin sudah ada).
- `examples/` + `tests/expected/` + `tests/galat/`.
- `website/panduan/`, `editors/`, `CHANGELOG.md`, `ROADMAP.md`, peta fitur,
  `README.md`, `AGENTS.md`.

## 5. Rencana uji

- Golden `examples/modul.idk` + modul pendukung di `examples/modul_contoh/`.
- **Isolasi namespace**: dua modul mendefinisikan nama sama, keduanya benar.
- `__nama__` berbeda saat diimpor vs dijalankan langsung.
- Siklus impor (A impor B, B impor A) tidak menggantung.
- Modul dieksekusi **sekali** walau diimpor dua kali.
- Tes galat: modul tidak ditemukan, anggota tidak ada, alias tanpa nama.
- **Regresi wajib**: seluruh `pustaka/*.idk` dan contoh lama yang memakai
  `impor "..."` harus tetap lulus tanpa diubah.
- Sanitizer wajib (tipe objek baru + tabel baru): golden **dan**
  `tests/galat/` di bawah `-DDEBUG_STRESS_GC -fsanitize=address,undefined`.

## 6. Dampak dokumentasi & tooling

- [x] `website/panduan/03_PANDUAN_SINTAKS.md`
- [x] `website/panduan/07_PUSTAKA_STANDAR.md`
- [x] `editors/vscode-indonesiaku/extension.js` KEYWORDS (`dari`)
- [x] `editors/vscode-indonesiaku/syntaxes/indonesiaku.tmLanguage.json`
- [x] `editors/vscode-indonesiaku/snippets/indonesiaku.code-snippets`
- [x] `README.md`
- [x] `CHANGELOG.md` + `docs/ROADMAP.md` + `ANALISIS/02_PETA_FITUR_PYTHON.md`
- [x] `AGENTS.md`
- [x] Rencana ini → `docs/RENCANA_FITUR/selesai/`

## 7. Hasil

Selesai. Suite **153/153** (naik dari 150: golden `modul` + 2 tes galat).
Sanitizer ASan+UBSan+`DEBUG_STRESS_GC` atas 84 golden + 63 tes galat: nol
temuan **setelah** memperbaiki satu use-after-free yang ditemukannya (kunci
`__nama__` dipakai tanpa di-*root* sebelum `tableSet` yang mengalokasi).
Modul native P1 sengaja tidak dikerjakan; alasannya dicatat di ROADMAP.

## 8. Risiko & alternatif

- **Risiko terbesar: memisahkan `vm.globals` memecah kode lama.** Diredam
  dengan menjadikan builtin tabel terpisah yang selalu terlihat, dan dengan
  membiarkan `impor "teks"` memakai modul pemanggil. Suite 150 tes adalah
  jaring pengamannya.
- Modul native (`matematika`, `acak`, ... sebagai `.c`) **tidak** dikerjakan di
  sini: `pustaka/*.idk` sudah menyediakan isinya dan menulis ulang jadi C tanpa
  kebutuhan nyata adalah pekerjaan mahal tanpa manfaat. Dicatat di ROADMAP
  sebagai sisa yang disengaja.
- Alternatif ditolak: menukar `vm.globals` saat masuk/keluar modul. Itu rapuh
  terhadap callback, closure lintas modul, dan unwinding galat — fungsi harus
  ingat modulnya sendiri, bukan bergantung pada siapa yang memanggil.
