# Rencana Fitur: <nama fitur>

- **Status:** usulan | disetujui | dikerjakan | selesai
- **Target versi:** <mis. 0.5.0>
- **Pemilik:** <nama>

## 1. Masalah

Apa yang tidak bisa dilakukan pengguna hari ini? Beri contoh kode yang gagal
atau canggung, dan tuliskan error/pesan yang muncul sekarang.

## 2. Sintaks usulan

```python
# contoh pemakaian yang diinginkan
```

Sebutkan kata kunci baru (Bahasa Indonesia), operator, dan bagaimana ia
berinteraksi dengan `jika`/`selagi`/`untuk`/`fungsi`/`kelas`.

## 3. Semantik

- Tipe yang terlibat dan hasil operasinya.
- Aturan scope (ingat: `variabel` membuat lokal, assignment implisit selalu
  menargetkan global dengan semantik definisi).
- Perilaku pada nilai `kosong`, tipe salah, dan argumen kurang/berlebih.
- Apakah error-nya bisa ditangkap `coba/kecuali` (`nativeRaise`) atau fatal
  (`runtimeError`, exit 70)?

## 4. Rencana implementasi

| Bagian | Perubahan |
|---|---|
| `src/scanner.c` | token / kata kunci baru |
| `src/compiler.c` | aturan parsing (precedence), codegen |
| `src/vm.c` | opcode baru, handler, unwinding |
| `src/native.c` | builtin baru (jika ada) |
| `src/value.c` / `src/object.c` | tipe objek baru (jika ada) |
| `src/debug.c` | nama opcode untuk disassembler |

Catat kebutuhan **GC-safety**: objek yang dialokasikan sambil dipegang harus
tetap reachable (pakai `peek`, bukan `pop`, sebelum `tableSet`/`writeValueArray`).

## 5. Pengujian

- Contoh golden: `examples/<nama>.idk` + `tests/expected/<nama>.out`
  (stdout saja; stderr harus kosong; exit 0).
- Kasus batas yang wajib diuji: <daftar>.
- Jalankan juga dengan `-DDEBUG_STRESS_GC` — output harus identik.
- Kalau menyentuh scope/upvalue: tambahkan regresi seperti
  `examples/lingkup_variabel.idk`.

## 6. Dampak dokumentasi

- [ ] `website/panduan/03_PANDUAN_SINTAKS.md`
- [ ] `website/panduan/07_PUSTAKA_STANDAR.md` (kalau ada builtin baru)
- [ ] `README.md`
- [ ] `CHANGELOG.md` + `docs/ROADMAP.md`
- [ ] `AGENTS.md` (kalau ada jebakan baru yang perlu diketahui agen/kontributor)

## 7. Risiko & alternatif

Apa yang bisa rusak (kompatibilitas program lama, perilaku GC, performa), dan
pendekatan lain yang dipertimbangkan beserta alasan tidak dipilih.
