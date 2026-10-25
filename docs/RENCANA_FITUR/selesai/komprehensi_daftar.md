# Rencana Fitur: list comprehension

- **Status:** selesai
- **Target versi:** 0.8.0
- **Pemilik:** —
- **Ref:** Fase 2 di `docs/ROADMAP.md`; 02 §B

## 1. Masalah

Membangun daftar turunan hari ini harus memakai loop + `tambah()`:

```indonesiaku
hasil = []
untuk x dalam rentang(5) {
    tambah(hasil, x * x)
}
```

Python menulis ini sebagai satu ekspresi. Tanpa comprehension, setiap contoh
"tutorial Python" yang disalin harus diubah bentuknya.

## 2. Sintaks usulan

```indonesiaku
[x * x untuk x dalam rentang(1, 6)]      # [1, 4, 9, 16, 25]
[x untuk x dalam rentang(10) jika x % 2 == 0]
[[a * b untuk b dalam rentang(1, 3)] untuk a dalam [1, 2]]   # bersarang
```

Satu `untuk` dan satu `jika` opsional; syarat tambahan digabung dengan
`dan`/`atau`. Tidak ada blok `{ }` di sini karena comprehension adalah satu
ekspresi — aturan K4 tidak berubah.

## 3. Semantik

- Variabel loop adalah lokal baru yang hanya hidup di dalam comprehension;
  nama di luar tidak berubah (`x = 99` tetap 99 setelah comprehension).
- Urutan yang dapat diiterasi: apa pun yang diterima `untuk` (list, teks,
  rentang, kamus → per kunci).
- Comprehension mengevaluasi ekspresi elemen sekali per elemen yang lolos
  syarat; `jika` gepeng dievaluasi sebelum ekspresi elemen.
- Galat runtime di dalam ekspresi/syarat merambat seperti biasa dan bisa
  ditangkap `coba/kecuali`.
- `[1, 2 untuk x ...]` (elemen lebih dari satu tanpa tanda kurung) adalah
  galat kompilasi.

## 4. Rencana implementasi

| Bagian | Perubahan |
|---|---|
| `src/scanner.c` | tidak ada (kata kunci `untuk` sudah ada) |
| `src/compiler.c` | `scanComprehension` (lookahead), `listComprehension`, hook di `listLiteral` |
| `src/vm.c` | opcode baru `OP_APPEND` |
| `src/chunk.h`, `src/debug.c` | nama opcode |

**Mengapa fungsi implisit:** dua kendala arsitektur. (a) Token elemen muncul
SEBELUM header yang mendeklarasikan `x`, jadi compiler mengintip (`untuk`) lalu
membaca ulang token elemen dari posisi scanner tersimpan setelah `x` masuk
scope. (b) Slot lokal adalah posisi stack; comprehension yang berada di dalam
argumen pemanggilan fungsi akan menabrak slot callee/argumen yang sudah ada.
Karena itu badan per-elemen dikompilasi menjadi fungsi satu parameter
(`listcomp`) yang dipanggil sekali per elemen — frame sendiri, dan nama luar
tetap terlihat lewat upvalue.

**GC-safety:** `OP_APPEND` memakai `peek(1)`/`peek(0)` (bukan `pop`) selama
`writeValueArray`, karena array bisa tumbuh dan memicu GC.

## 5. Pengujian

- Golden: `examples/komprehensi.idk` + `tests/expected/komprehensi.out`
  (termasuk bersarang, teks, kamus, syarat, argumen fungsi, kosong).
- Interaksi: variabel loop tidak bocor; nama luar tetap; comprehension di dalam
  fungsi menangkap upvalue.
- `-DDEBUG_STRESS_GC`: seluruh contoh, keluaran identik.

## 6. Dampak dokumentasi

- [x] `website/panduan/03_PANDUAN_SINTAKS.md`
- [x] `README.md`, `CHANGELOG.md`, `docs/ROADMAP.md`, `AGENTS.md`
- [x] `editors/vscode-indonesiaku/` (snippet `komprehensi`)

## 7. Risiko & alternatif

- **Alternatif ditolak:** memakai lokal tersembunyi di frame pemanggil (tabrakan
  slot saat comprehension jadi argumen — ketahuan saat uji
  `cetak(panjang([...]))`); atau menyalin bytecode elemen ke dalam loop (gagal
  karena `x` belum dideklarasikan saat elemen di-parse).
- **Batas yang diterima:** satu `untuk`, satu `jika`, tanpa `*args`-style
  unpacking di dalam comprehension.
