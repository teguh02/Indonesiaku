# Rencana Fitur: Rantai perbandingan dan identitas

- **Status:** selesai
- **Target versi:** 0.6.0 (Fase 1)
- **Pemilik:** teguh02
- **Keputusan terkait:** K7 (identitas), disetujui

## 1. Masalah

Perbandingan berturut-turut saat ini dievaluasi dari kiri, sehingga `1 < x < 3` membandingkan boolean dengan `3`. Bahasa juga belum memiliki operator identitas `adalah` / `bukan adalah`.

## 2. Sintaks usulan

```python
cetak(1 < 2 < 3)
cetak(a dalam b == c)
cetak(daftar adalah daftar)
cetak(daftar bukan adalah daftar_lain)
```

`bukan` hanya valid sebagai bagian dari `bukan adalah`; `tidak dalam` tetap operator keanggotaan yang sudah ada.

## 3. Semantik

- Rantai `a op1 b op2 c` setara dengan `(a op1 b) dan (b op2 c)`.
- Operand tengah dievaluasi tepat satu kali; rantai berhenti saat perbandingan sebelumnya salah.
- `adalah` membandingkan identitas objek. Untuk nilai langsung (angka, boolean, `kosong`), hasilnya sama dengan kesetaraan nilai. `bukan adalah` menegasikan hasilnya.
- Operator tetap pada tingkat presedensi perbandingan dan dapat dirantai dengan `<`, `==`, `dalam`, dan bentuk negasinya.

## 4. Rencana implementasi

| Bagian | Perubahan |
|---|---|
| `src/scanner.h` / `src/scanner.c` | token dan kata kunci `adalah` serta `bukan` |
| `src/compiler.c` | aturan Pratt perbandingan yang mengompilasi rantai; `bukan adalah` wajib lengkap |
| `src/chunk.h` / `src/debug.c` | opcode bantu untuk menyimpan operand tengah dan membersihkan jalur short-circuit |
| `src/vm.c` | handler opcode bantu tanpa alokasi; identitas memakai kesetaraan nilai langsung / pointer objek |

Tidak ada alokasi pada opcode rantai; stack harus menyisakan boolean hasil akhir pada kedua jalur.

## 5. Pengujian

- Golden `examples/rantai_perbandingan.idk`: angka, string, `dalam`, operand tengah dengan efek samping, dan short-circuit.
- Golden `examples/operator_identitas.idk`: objek sama/berbeda, nilai langsung, serta `bukan adalah`.
- Galat: `bukan` tanpa `adalah`.
- Jalankan dengan `-DDEBUG_STRESS_GC`; stdout harus sama.

## 6. Dampak dokumentasi

- [ ] `website/panduan/03_PANDUAN_SINTAKS.md`
- [ ] `CHANGELOG.md` + `docs/ROADMAP.md` + `docs/ANALISIS/02_PETA_FITUR_PYTHON.md`
- [ ] `AGENTS.md`

## 7. Risiko & alternatif

Menyusun ulang hasil boolean biasa akan mengevaluasi operand tengah dua kali. Opcode rantai kecil dipilih agar evaluasi sekali dan short-circuit tetap benar.