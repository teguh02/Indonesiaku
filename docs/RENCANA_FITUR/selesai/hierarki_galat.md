# Rencana Fitur: Hierarki `Galat*` dan `kecuali Tipe sebagai e`

- **Status:** selesai
- **Target versi:** 0.6.0 (Fase 1)
- **Keputusan terkait:** K5

## 1. Masalah

`runtimeError()` dan `nativeRaise()` saat ini memberi teks biasa kepada `kecuali e`. Program belum dapat membedakan galat indeks, tipe, nama, atau pembagian nol, dan bentuk `kecuali GalatTipe sebagai e` belum diparse.

## 2. Sintaks

```idk
coba {
    nilai = daftar[99]
} kecuali GalatIndeks sebagai e {
    cetak(e)
} kecuali e {
    cetak("galat lain", e)
}
```

Bentuk lama `kecuali e { ... }` tetap menangkap semua galat. Satu `kecuali Tipe sebagai e` diprioritaskan sebagai implementasi minimal; tuple beberapa tipe dari K5 ditunda sampai pola handler berulang tersedia.

## 3. Semantik

- VM membuat instansi kelas galat bawaan, dengan pesan tersedia sebagai `e.pesan` dan representasi cetak yang jelas.
- Hierarki minimum: `Galat`, `GalatAritmetika`, `GalatBagiNol`, `GalatPencarian`, `GalatIndeks`, `GalatKunci`, `GalatTipe`, `GalatNilai`, `GalatNama`, `GalatAtribut`, `GalatImpor`, dan `GalatRekursi`.
- Error runtime dipetakan di titik sumber yang sudah ada; error internal VM tetap fatal dan tidak dapat ditangkap.
- Handler bertipe hanya menjalankan bloknya apabila galat adalah instansi kelas tersebut atau turunannya; bila tidak, galat diteruskan keluar.

## 4. Berkas yang disentuh

| Berkas | Perubahan |
|---|---|
| `src/vm.[ch]` | Buat dan lempar instance galat; pencocokan handler |
| `src/compiler.c`, `src/chunk.h`, `src/debug.c` | Parse handler bertipe dan bytecode pencocokan |
| `src/object.[ch]`, `src/memory.c` | Gunakan kelas/instance yang sudah ada dengan GC root aman |
| `examples/`, `tests/expected/`, `tests/galat/` | Golden typed catch dan galat tak tertangani |
| Dokumentasi | Roadmap, changelog, panduan sintaks, dan peta fitur |

## 5. Rencana uji

1. Tangkap `GalatBagiNol` melalui induk `GalatAritmetika` dan melalui kelas konkret.
2. Pastikan handler bertipe salah meneruskan galat ke handler luar.
3. Pastikan `kecuali e` lama tetap menangkap galat runtime dan native.
4. Uji `DEBUG_STRESS_GC`, ASan/UBSan, serta golden stdout tanpa stderr.
