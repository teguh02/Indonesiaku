# Rencana Fitur: Scope assignment fungsi, `global`, dan `nonlokal`

- **Status:** selesai
- **Target versi:** 0.6.0 (Fase 1)
- **Keputusan terkait:** K3

## 1. Masalah

Assignment nama baru di dalam fungsi saat ini membuat/mengubah global. Ini berlawanan dengan Python dan membuat closure tidak dapat memakai assignment biasa.

## 2. Sintaks

```idk
variabel luar = 1
fungsi ubah_global() {
    global luar
    luar = 2
}
fungsi pembuat() {
    nilai = 0
    fungsi tambah() {
        nonlokal nilai
        nilai = nilai + 1
        kembalikan nilai
    }
    kembalikan tambah
}
```

Assignment nama yang tidak dideklarasikan `global`/`nonlokal` di dalam fungsi menjadi lokal fungsi. Pada tingkat skrip tetap global.

## 3. Semantik

- Deklarasi `global` dan `nonlokal` berlaku untuk seluruh badan fungsi dan harus muncul sebelum penggunaan/assignment nama tersebut.
- `nonlokal` harus menemukan local pada fungsi pembungkus; tidak boleh merujuk global.
- Compiler melakukan prescan badan fungsi untuk menyiapkan stack slot local sebelum memasuki blok bersarang, sehingga local hasil assignment dapat hidup sepanjang fungsi.
- `variabel` tetap sah dan merujuk local yang sama.

## 4. Rencana implementasi

| Bagian | Perubahan |
|---|---|
| `src/scanner.[ch]` | Keyword `global` dan `nonlokal` |
| `src/compiler.c` | Prescan deklarasi/assignment fungsi; resolve mode lokal/global/upvalue dan statement deklarasi |
| `examples/`, `tests/expected/`, `tests/galat/` | Golden closure/global dan galat nonlokal invalid |

Tidak ada objek/opcode baru. Prescan wajib menyimpan dan mengembalikan seluruh `ScannerState`.

## 5. Pengujian

- Local fungsi tidak mengubah global senama.
- Assignment dalam blok fungsi tetap hidup setelah blok.
- `global` mengubah global.
- `nonlokal` mengubah upvalue closure dan galat bila tidak ada binding pembungkus.
- Golden penuh, stress GC, dan ASan/UBSan.

## 6. Dampak dokumentasi

- [x] Panduan sintaks, README, changelog/roadmap, AGENTS

## 7. Risiko & alternatif

Mengubah assignment saat ditemukan tanpa prescan akan membuat slot local setelah slot scope dalam dan merusak stack cleanup. Prescan token ringan dipilih daripada mengubah VM slot model.