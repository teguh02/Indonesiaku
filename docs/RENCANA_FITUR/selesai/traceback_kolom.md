# Rencana Fitur: Traceback dengan kolom, potongan baris, dan saran

- **Status:** selesai (bagian A dan B; C ditunda sebagai butir tersendiri)
- **Target versi:** 0.7.0 (Fase 1)
- **Pemilik:** Teguh
- **Keputusan terkait:** 04 §5, 05 §5

## 1. Masalah

Pesan galat sekarang hanya menyebut nomor baris:

```
[baris 2] Kesalahan di akhir: Harapkan ekspresi.
```

```
Operand '+' harus dua angka, dua teks, atau dua daftar.
[baris 2] dalam skrip
```

Pada baris panjang, pengguna masih harus menebak bagian mana yang salah. Dan
salah ketik nama (`cetk`, `panjng`) hanya berbunyi "Variabel tidak
terdefinisi" tanpa petunjuk.

## 2. Sasaran

```
[baris 2, kolom 14] Kesalahan di akhir: Harapkan ekspresi.
    variabel y = (1 +
                 ^
```

```
Variabel tidak terdefinisi 'cetk'. Mungkin maksudmu 'cetak'?
```

## 3. Lingkup yang diambil (dan yang tidak)

Butir ini mudah membengkak. Dibagi tiga, dan hanya dua pertama dikerjakan
sekarang:

| Bagian | Status | Alasan |
|---|---|---|
| **A. Kolom + potongan baris untuk galat kompilasi** | dikerjakan | Scanner masih memegang sumber; murah dan mengenai galat yang paling sering dilihat pemula |
| **B. Saran "mungkin maksudmu" untuk nama tak dikenal** | dikerjakan | Data sudah ada di `vm.globals`; jarak Levenshtein ±20 baris |
| **C. Kolom untuk galat runtime** | **ditunda** | `chunk->lines` hanya menyimpan baris per-bytecode. Kolom berarti menambah array kedua (atau mengganti dengan struct) di setiap chunk, dan teks sumber harus ikut hidup selama program berjalan — termasuk sumber tiap modul yang di-`impor`. Itu perubahan struktur data, bukan pemolesan pesan |

Bagian C didaftarkan sebagai butir ⬜ tersendiri di ROADMAP, bukan dianggap
selesai.

## 4. Rencana implementasi

### A. Kolom + potongan baris (galat kompilasi)

`Scanner` belum menyimpan awal buffer, sehingga kolom tidak bisa dihitung.
Tambahkan satu field:

| Berkas | Perubahan |
|---|---|
| `src/scanner.c` | field `const char* source` diisi di `initScanner`/`initScannerAt` |
| `src/scanner.h` | `const char* scannerSourceStart(void);` |
| `src/compiler.c` | `errorAt()` menghitung kolom dan mencetak potongan baris |

Kolom = `token->start - awal_baris + 1`, dengan awal baris dicari mundur dari
token sampai `\n` atau awal buffer. Caret diletakkan di bawah token.

Hal yang harus dijaga:
- **Tab** dalam baris sumber: salin tab apa adanya ke baris caret supaya
  posisi caret tetap sejajar di terminal mana pun.
- **Baris sangat panjang**: potong di sekitar token (jendela ±40 karakter)
  dengan penanda `…`, jangan muntahkan 500 karakter.
- **Sub-scanner f-string**: `initScannerAt` dipakai dengan potongan sumber
  ber-NUL sendiri, jadi `source` ikut diset ke potongan itu — kolomnya relatif
  terhadap potongan. Lebih baik daripada menunjuk posisi acak di berkas.
- **`TOKEN_EOF`**: `token->start` menunjuk NUL akhir; kolomnya tetap valid.

### B. Saran "mungkin maksudmu"

| Berkas | Perubahan |
|---|---|
| `src/vm.c` | `namaTerdekat()` — Levenshtein terbatas atas tabel global |

Dipakai pada galat "Variabel tidak terdefinisi". Ambang: jarak ≤ 2 **dan**
≤ sepertiga panjang nama, supaya `xyz` tidak disarankan jadi `abs`.

Jarak dihitung dengan dua baris buffer (bukan matriks penuh); nama > 64
karakter dilewati. Tabel global ±100 entri, hanya dijalankan **saat galat**,
jadi biayanya tidak terasa.

## 5. Pengujian

- Galat kompilasi: kolom benar di awal/tengah/akhir baris, baris ber-tab,
  baris sangat panjang (terpotong), galat di EOF, galat di dalam f-string.
- Saran: `cetk` → `cetak`, `panjng` → `panjang`; nama yang tidak mirip
  (`zzzzz`) **tidak** memunculkan saran; variabel lokal tidak ikut disarankan
  sebagai global.
- Semua golden `tests/galat/*.err` yang ada akan berubah karena formatnya
  bertambah — harus diperbarui dan **dibaca satu per satu**, bukan di-regenerate
  buta.

## 6. Dampak dokumentasi

- [x] `website/panduan/03_PANDUAN_SINTAKS.md` — seksi Membaca Pesan Kesalahan (3 klaim diuji)
- [x] `README.md` + `CHANGELOG.md`
- [x] `docs/ROADMAP.md` — butir ✅, bagian C didaftarkan sebagai butir ⬜ baru
- [ ] `AGENTS.md` — tertunda: berkas diproteksi Hermes, perlu persetujuan interaktif pemilik repo

## 7. Hasil (2026-10-25)

```text
tests/test_all.sh                  123/123 PASSED
ASan+UBSan+DEBUG_STRESS_GC         bersih (600 panggilan namaTerdekat)
```

Catatan dari pengerjaan:

- **10 golden `.err` berubah** karena formatnya bertambah. Semuanya dibaca
  satu per satu sebelum diperbarui, bukan di-regenerate buta: kolom dan posisi
  caret dicocokkan dengan sumbernya masing-masing.
- Ditemukan saat memeriksa golden: pada `kondisional_tanpa_selain` tokennya
  adalah newline, sehingga pesan lama mencetak baris baru literal di tengah
  kalimat. Ditambahkan kasus khusus → "di akhir baris".
- Kolom sempat dihitung dua kali (di `errorAt` dan di `printSourceSnippet`);
  disatukan menjadi helper `lineStartOf()`.
- Di dalam f-string, kolom relatif terhadap potongan ekspresinya, bukan
  terhadap berkas. Itu konsekuensi sub-scanner dan lebih berguna daripada
  menunjuk posisi yang salah di berkas.
