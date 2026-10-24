# Rencana Fitur: `enumerasi()` dan iterasi kamus

- **Status:** selesai
- **Target versi:** 0.6.0 (Fase 1)
- **Pemilik:** teguh02
- **Keputusan terkait:** K9 (protokol iterasi), disetujui

## 1. Masalah

`untuk` belum menerima kamus, dan tidak ada cara langsung untuk memperoleh indeks ketika mengiterasi sebuah urutan.

## 2. Sintaks usulan

```python
untuk kunci dalam data { cetak(kunci) }
untuk pasangan dalam enumerasi(buah) { cetak(pasangan[0], pasangan[1]) }
untuk pasangan dalam enumerasi(buah, 1) { cetak(pasangan) }
```

## 3. Semantik

- `untuk` atas kamus menghasilkan kunci dalam urutan penyisipan aktif.
- `enumerasi(urutan)` menghasilkan list pasangan `[indeks, nilai]`; argumen awal opsional menetapkan indeks pertama (bawaan `0`).
- `enumerasi` menerima list, teks, rentang, dan kamus melalui jalur `OP_LIST_LEN` + `OP_INDEX_GET` yang sama dengan `untuk`.
- Tipe yang tidak dapat diiterasi serta jumlah/tipe argumen salah menghasilkan galat yang dapat ditangkap `coba/kecuali`.

## 4. Rencana implementasi

| Bagian | Perubahan |
|---|---|
| `src/object.h` / `object.c` | simpan urutan penyisipan kunci pada `ObjDict` |
| `src/vm.c` | `OP_LIST_LEN` dan `OP_INDEX_GET` menerima kamus dan membaca kunci berurutan |
| `src/native.c` | builtin `enumerasi()` membangun pasangan dengan GC root yang aman |
| `src/vm.c` | daftar native mendaftarkan `enumerasi` |

Urutan kunci harus dipertahankan saat nilai ditimpa dan mengabaikan kunci yang dihapus. List hasil `enumerasi` dipush selama pasangan ditambahkan agar alokasi tidak kehilangan objek dari GC.

## 5. Pengujian

- Golden `examples/enumerasi.idk`: list, teks, rentang, offset, dan kamus.
- Golden `examples/untuk_kamus.idk`: urutan sisip, update nilai, lalu hapus kunci.
- Galat: `enumerasi(1)` dan offset bukan angka.
- Jalankan dengan `-DDEBUG_STRESS_GC`; stdout harus sama.

## 6. Dampak dokumentasi

- [ ] `website/panduan/03_PANDUAN_SINTAKS.md`
- [ ] `website/panduan/07_PUSTAKA_STANDAR.md`
- [ ] `CHANGELOG.md` + `docs/ROADMAP.md` + `docs/ANALISIS/02_PETA_FITUR_PYTHON.md`
- [ ] `AGENTS.md`

## 7. Risiko & alternatif

Mengiterasi slot hash langsung tidak menjamin urutan sisip. Daftar kunci khusus pada kamus dipilih agar semantik stabil.