# Rencana Fitur: Literal angka, string `"""`, dan f-string

- **Status:** selesai
- **Target versi:** 0.7.0 (Fase 1)
- **Pemilik:** Teguh
- **Peta fitur:** 02 §B

## 1. Masalah

Satu butir roadmap, tiga hal yang semuanya ditolak scanner hari ini:

```indonesiaku
cetak(1e3)        # Kesalahan di 'e3'
cetak(0xFF)       # Kesalahan di 'xFF'
cetak(1_000_000)  # Kesalahan di '_000_000'
cetak("""baris 1
baris 2""")       # Kesalahan
cetak(f"halo {nama}")   # Kesalahan
```

Akibatnya: konstanta besar ditulis `1000000` tanpa pemisah, bitmask ditulis
desimal, teks multi-baris disambung `+ "\n" +`, dan setiap penggabungan teks
jadi rantai `+` yang panjang.

## 2. Yang sudah gratis dari `strtod`

Diuji langsung dengan program C kecil sebelum menulis kode apa pun:

| Literal | `strtod` | Perlu kode? |
|---|---|---|
| `1e3`, `1.5e-3` | 1000, 0.0015 | tidak |
| `0x1F` | 31 | tidak |
| `0b1010` | 0, sisa `b1010` | **ya** |
| `0o17` | 0, sisa `o17` | **ya** |
| `1_000` | 1, sisa `_000` | **ya** |

Jadi eksponen dan heksadesimal hanya perlu **diizinkan lewat scanner**;
`strtod` di `number()` sudah mengurus nilainya. Hanya biner, oktal, dan
garis bawah yang butuh konversi sendiri.

## 3. Sintaks & semantik

```indonesiaku
1e3      1E3     1.5e-3            # eksponen
0xFF     0xff    0XFF              # heksadesimal
0b1010   0B1010                    # biner
0o17     0O17                      # oktal
1_000_000        3.141_592         # pemisah ribuan
```

- Semua tetap bertipe angka (`double`) — K3 belum berubah, bilangan bulat
  presisi penuh menunggu Fase 2.
- Garis bawah hanya **di antara** digit: `1_000` sah, `_1`/`1_`/`1__0` ditolak.
  `_1` memang sudah terbaca sebagai nama variabel, jadi tidak perlu aturan
  khusus; sisanya ditolak scanner.
- Basis non-desimal hanya untuk bilangan bulat: `0x1.8` ditolak.

```indonesiaku
variabel teks = """baris pertama
baris kedua"""
```

- Mempertahankan baris baru apa adanya; escape tetap berlaku (`\n`, `\"`).
- Pembuka dan penutup sama-sama `"""`.

```indonesiaku
variabel nama = "Budi"
cetak(f"Halo {nama}, umurmu {umur} tahun")
cetak(f"1 + 2 = {1 + 2}")
```

- Hanya ekspresi, tanpa format spec (`{x:.2f}`) — itu menunggu metode teks di
  butir Fase 1 berikutnya.
- `{{` dan `}}` menghasilkan `{` dan `}` harfiah.
- Nilai non-teks dikonversi dengan aturan `ke_teks` yang sudah ada.
- f-string tidak boleh digabung dengan `"""` pada tahap ini (`f"""..."""`
  ditolak dengan pesan jelas) — menunggu kebutuhan nyata.

## 4. Rencana implementasi

| Bagian | Perubahan |
|---|---|
| `src/scanner.c` | `number()` menerima `e/E`, `0x/0b/0o`, `_` |
| `src/scanner.c` | `string()` mengenali `"""` |
| `src/scanner.c` | awalan `f"` → `TOKEN_FSTRING` |
| `src/scanner.h` | `TOKEN_FSTRING` |
| `src/compiler.c` | `number()` → konversi basis + buang `_` |
| `src/compiler.c` | `fstring()` → pecah bagian, rangkai dengan `OP_ADD` |

**Angka.** Scanner melebar; konversi di compiler:
`0b`/`0o` lewat `strtol(.., 2/8)`, selain itu `strtod` (sudah menangani `0x`
dan eksponen). Garis bawah disalin keluar ke buffer sementara lebih dulu.
Buffer berukuran tetap dengan batas panjang literal — angka lebih panjang dari
itu ditolak, bukan di-`strcpy` membabi buta.

**f-string.** Didesugar di compiler menjadi rangkaian `+` yang sudah ada,
sehingga **tidak ada opcode baru**:

```text
f"Halo {nama}!"   ==   "Halo " + ke_teks(nama) + "!"
```

Setiap bagian ekspresi di dalam `{ }` dikompilasi dengan memanggil scanner
pada potongan sumbernya — pola `saveScanner()`/`restoreScanner()` yang sudah
dipakai nilai bawaan parameter dan ekspresi kondisional.

`ke_teks` dibungkus lewat native yang sudah terdaftar, bukan opcode baru:
angka/daftar/kamus di dalam f-string harus tampil sama seperti `ke_teks`.

## 5. Pengujian

Golden `examples/literal_fstring.idk` + expected:
- tiap basis dan eksponen, termasuk huruf besar-kecil
- `1_000_000` == `1000000`, `3.141_592` == `3.141592`
- `0xFF` == `255`, `0b1010` == `10`, `0o17` == `15`
- `"""` dengan baris baru, kutip di dalamnya, dan escape
- f-string: variabel, ekspresi aritmetika, pemanggilan fungsi, `{{`/`}}`,
  nilai non-teks (angka, daftar, kamus, `kosong`), f-string kosong `f""`
- f-string di dalam fungsi dan closure (nama lokal, bukan global)

Tes galat: `1__0`, `0x`, `0b2`, `"""` tak ditutup, `{` tak ditutup,
`f"""..."""`.

Sanitizer ASan+UBSan+`DEBUG_STRESS_GC`: f-string mengalokasi banyak `ObjString`
perantara.

## 6. Dampak dokumentasi

- [x] `website/panduan/03_PANDUAN_SINTAKS.md` — 3 seksi baru (5 blok diuji jalan)
- [x] `README.md` + `CHANGELOG.md`
- [x] `docs/ROADMAP.md` — butir Fase 1 → ✅
- [x] `docs/ANALISIS/02_PETA_FITUR_PYTHON.md`
- [x] `editors/vscode-indonesiaku/` — tmLanguage (angka + f-string + `"""`), snippet `fstring`/`tigakutip`
- [ ] `AGENTS.md` — tertunda: berkas terproteksi, perlu persetujuan pemilik repo

## 7. Risiko

- **`0x`/`0b` memakai huruf** yang juga sah sebagai awal nama variabel.
  Scanner hanya masuk jalur ini setelah melihat digit `0`, jadi `b1010`
  sebagai nama tetap aman — wajib diuji.
- **`number()` di compiler memakai `parser.previous.start`** yang menunjuk ke
  dalam sumber, bukan string ber-NUL. `strtod` berhenti sendiri di karakter
  tak valid, tetapi penyalinan untuk membuang `_` harus memakai
  `parser.previous.length`, bukan `strlen`.
- **f-string menyalakan parser rekursif di tengah token.** Kesalahan di dalam
  `{ }` harus melaporkan baris yang benar, bukan baris 1.

## 8. Hasil (2026-10-25)

```text
tests/test_all.sh                  114/114 PASSED
ASan+UBSan+DEBUG_STRESS_GC         bersih (300 iterasi f-string + `"""`)
```

Catatan dari pengerjaan:

- Perkiraan §2 terbukti: `strtod` sudah menangani `1e3` dan `0x1F`, jadi hanya
  biner/oktal/garis bawah yang butuh konversi sendiri. Scanner hanya perlu
  dilebarkan.
- Rencana §4 menyebut f-string dibatasi tanpa `"""`. Pembatasan itu **dihapus
  sebagai kode mati**: scanner f-string berdiri sendiri (`fstringBody`), jadi
  `f"""..."""` tidak pernah sampai ke jalur itu.
- f-string **tidak bisa** memakai pemindai string biasa: `f"{d["a"]}"` akan
  terpotong di kutip pertama. `fstringBody()` melacak kedalaman `{ }` agar
  kutip di dalam ekspresi aman.
- Token f-string dimulai di huruf `f`, sehingga offsetnya `+2/-3`, bukan
  `+1/-2` seperti string biasa. Sempat salah dan membuat kutip bocor ke output.
- `f"kamus: {{"a": 1}}"` ditolak — `{{` adalah kurawal harfiah. **Python
  menolak hal yang sama**; diuji langsung dengan `python3` sebelum memutuskan
  ini bukan bug. Solusinya beri spasi: `f"{ {"a": 1} }"`.
- `scanner.current[2]` dipakai dua kali untuk lookahead tiga karakter; keduanya
  diberi penjaga agar tidak membaca melewati NUL di akhir sumber.
