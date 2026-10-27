# Rencana Fitur: Scanner penyimpan komentar + `indk rapikan`

- **Status:** selesai (tahap 1-4; penulisan ulang/wrapping sengaja tidak dikerjakan)
- **Target versi:** Fase 7 (perkakas)
- **Pengusul:** Teguh
- **Keputusan terkait:** K4 (tanpa aturan indentasi), K14 (kompatibilitas)

## 1. Masalah

`indk rapikan` (formatter) tidak bisa dibuat hari ini. `skipWhitespace()` di
`src/scanner.c` membuang komentar sepenuhnya:

```c
case '#':
    // A comment goes until the end of the line.
    while (peek() != '\n' && !isAtEnd()) advance();
    break;
```

Tidak ada `TOKEN_COMMENT`. Komentar tidak pernah sampai ke token stream.

**Dibuktikan, bukan diduga.** Dua berkas yang hanya berbeda komentar
menghasilkan bytecode identik (dibandingkan lewat `indk --dis`, nomor baris
dinormalkan):

```idk
# berkas A
fungsi f(x) { kembalikan x + 1 }

# berkas B
# komentar penting
fungsi f(x) { # inline
    kembalikan x + 1 }
```

Konsekuensinya: formatter berbasis token akan **menghapus seluruh komentar
pengguna**. Formatter yang memakan komentar lebih buruk daripada tidak ada
formatter, jadi fiturnya diblokir sampai masalah ini selesai.

## 2. Bentuk yang diusulkan

Dua tahap terpisah. Tahap 1 tidak berguna bagi pengguna akhir, tetapi tanpa
itu tahap 2 tidak mungkin benar.

### Tahap 1 — scanner bisa menyimpan komentar (prasyarat)

Mode opt-in. Default **mati**, sehingga compiler dan 13 pemanggil
`scanToken()` yang ada tidak berubah sama sekali.

```c
void scannerKeepComments(bool enabled);   // default false
```

Saat menyala, `scanToken()` mengembalikan `TOKEN_KOMENTAR` alih-alih
melewatinya. Saat mati, perilakunya persis seperti sekarang.

### Tahap 2 — `indk rapikan`

```text
$ indk rapikan berkas.idk          # tulis hasil ke stdout
$ indk rapikan --tulis berkas.idk  # timpa berkas di tempat
$ indk rapikan --periksa berkas.idk # exit 1 kalau belum rapi, tidak menulis
```

`--periksa` ada supaya CI bisa menolak kode yang belum diformat tanpa
mengubah berkas.

## 3. Semantik

### Aturan format

Mengikuti gaya yang sudah dipakai di `examples/` dan `pustaka/`, bukan gaya
baru:

- Indentasi 4 spasi per tingkat blok.
- `{` tetap di akhir baris yang sama; `}` di barisnya sendiri.
- Satu spasi di sekitar operator biner dan setelah koma.
- Tidak ada spasi tepat di dalam `(`, `[`, `{` literal.
- Maksimal satu baris kosong berturut-turut.
- Berkas selalu diakhiri tepat satu newline.

**K4 tetap berlaku.** Formatter *menulis* indentasi rapi, tetapi parser tetap
tidak pernah membacanya. Kode flush-left harus tetap kompilasi, dan
`examples/tanpa_indentasi.idk` **tidak boleh** diformat oleh tes mana pun —
berkas itu justru golden yang membuktikan aturan indentasi tidak ada.

### Penempatan komentar

Ini bagian yang paling mudah salah, jadi diputuskan di muka:

| Bentuk | Aturan |
|---|---|
| Komentar di barisnya sendiri | Ikut indentasi baris **berikutnya** |
| Komentar di akhir baris kode | Tetap di baris itu, dua spasi sebelum `#` |
| Komentar sebelum `}` penutup | Ikut indentasi isi blok, bukan `}` |
| Beberapa komentar berturut-turut | Jarak antar barisnya dipertahankan |

### Jaminan utama

**Idempoten dan aman secara semantik.** Dua hal yang wajib dijamin:

1. `rapikan(rapikan(x)) == rapikan(x)`.
2. Bytecode sebelum dan sesudah format **identik** (kecuali nomor baris).

Jaminan 2 adalah pengamannya: kalau formatter mengubah arti program, tesnya
gagal. Ini bisa diperiksa otomatis untuk seluruh `examples/` dan `pustaka/`.

## 4. Rencana implementasi

| Bagian | Perubahan |
|---|---|
| `src/scanner.h` | `TOKEN_KOMENTAR`; `void scannerKeepComments(bool)` |
| `src/scanner.c` | flag statis; `skipWhitespace()` berhenti di `#` saat flag menyala; `scanToken()` memproduksi token komentar |
| `src/format.c` (baru) | penulis ulang dari token stream |
| `src/format.h` (baru) | `char* formatSource(const char* source)` |
| `src/main.c` | subperintah `rapikan` + flag `--tulis` / `--periksa` |
| `Makefile` | tambah `format.c`; **juga audit kedua Dockerfile dan daftar sumber CI** |

Formatter **tidak memakai parser**. Ia bekerja dari token stream saja,
sehingga tidak perlu AST dan tidak ikut terpengaruh `prescanFunctionBody`.
Kedalaman blok dilacak dari `{` / `}` mentah — cara yang sama yang sudah
dipakai `braceDelta()` di `src/main.c` untuk REPL.

### Jebakan yang sudah diketahui

- **Jangan menyalakan flag secara global.** Kalau `TOKEN_KOMENTAR` bocor ke
  compiler, 13 pemanggil `scanToken()` harus menyaringnya satu per satu —
  termasuk `prescanFunctionBody()` dan `skipBalancedBraces()` yang menghitung
  brace mentah. Nyalakan hanya di `formatSource()`, matikan lagi setelahnya.
- **Sub-scanner f-string.** `ScannerState` **sudah** menyimpan `source` dan
  `sourceEnd` (bukan pekerjaan baru), tetapi flag komentar belum ada di sana.
  Kalau nanti ada jalur yang menyarangkan scanner dengan flag menyala, flag
  itu wajib ikut diselamatkan — kalau tidak, komentar bocor ke sub-scanner
  f-string.
- **Teks mentah lebih penting daripada token.** Formatter harus menulis ulang
  dari `token.start`/`token.length`, jangan pernah merekonstruksi literal
  teks dari nilainya — escape dan tanda kutip asli akan hilang.
- **Baris baru itu signifikan di Indonesiaku.** `TOKEN_NEWLINE` adalah
  pemisah pernyataan, dan `scanner.groupingDepth` menekannya di dalam
  kurung. Formatter wajib menghormati keduanya atau ia akan menggabungkan
  dua pernyataan jadi satu.

**Tidak ada perubahan runtime.** Murni waktu-kompilasi dan alat terpisah:
tidak ada opcode baru, tidak ada pertimbangan GC.

## 5. Pengujian

- `tests/format/<nama>.idk` + `<nama>.rapi` (hasil yang diharapkan), dengan
  runner `tests/test_format.sh` yang disambungkan ke `tests/test_all.sh` —
  pola yang sama dengan `tests/test_lint.sh` dan `tests/test_uji.sh`.
- **Properti idempoten**: format dua kali, hasil kedua harus sama persis.
- **Properti bytecode**: untuk setiap berkas di `examples/` dan `pustaka/`,
  bandingkan `indk --dis` sebelum dan sesudah format; harus identik setelah
  nomor baris dinormalkan. Inilah tes yang paling penting.
- `examples/tanpa_indentasi.idk` dipakai sebagai input: hasilnya harus
  tetap **berjalan dan mencetak keluaran golden yang sama**, membuktikan K4
  tidak dilanggar.

Kasus wajib:

- komentar di barisnya sendiri, di akhir baris, dan tepat sebelum `}`;
- komentar di dalam `(`/`[` yang membentang beberapa baris;
- f-string dengan `{}` dan tanda `#` **di dalam** teks (bukan komentar);
- teks yang mengandung `#` dan `{`;
- berkas kosong, berkas hanya komentar, berkas tanpa newline akhir;
- blok bersarang dalam, `jika_lain`, `coba/kecuali/akhirnya`, `cocokkan`;
- berkas yang sudah rapi (harus tidak berubah sama sekali).

## 6. Dampak dokumentasi & tooling

- [ ] `website/panduan/03_PANDUAN_SINTAKS.md` — seksi perkakas
- [ ] `README.md` + `CHANGELOG.md` + `docs/ROADMAP.md` (Fase 7)
- [ ] `AGENTS.md` — aturan "flag komentar jangan bocor ke compiler"
- [ ] `editors/vscode-indonesiaku/` — task `rapikan`, dan format-on-save
      hanya kalau `--periksa` sudah terbukti stabil
- [ ] `Makefile`, kedua Dockerfile, dan daftar sumber di CI (`format.c` baru)
- [ ] Berkas rencana ini → `selesai/`

## 7. Risiko & alternatif

**Risiko terbesar: formatter yang merusak kode.** Diredam oleh tes properti
bytecode di §5 — kalau arti program berubah, tes gagal sebelum rilis.

**Risiko kedua: perang gaya.** Aturan di §3 sengaja diambil dari kode yang
sudah ada, bukan selera baru. Tidak ada opsi konfigurasi: formatter yang bisa
dikonfigurasi akan menghabiskan waktu untuk memperdebatkan setelan.

**Alternatif ditolak — formatter berbasis AST.** Lebih bersih secara teori,
tetapi butuh CST yang menyimpan trivia, dan parser kita single-pass tanpa
AST. Biayanya jauh lebih besar daripada penulis ulang berbasis token untuk
hasil yang sama.

**Alternatif ditolak — `TOKEN_KOMENTAR` selalu aktif.** Memaksa 13 pemanggil
`scanToken()` menyaring token yang tidak mereka pedulikan, dan satu yang
terlewat jadi bug parser yang halus. Opt-in jauh lebih murah.

## 8b. Catatan pelaksanaan

**Langkah 1 dan 2 selesai.** `TOKEN_KOMENTAR` + `scannerKeepComments()`
terpasang, dan `formatSource()` saat ini adalah **formatter identitas**:
keluarannya byte-identik dengan masukan.

Itu bukan setengah jadi, melainkan urutan yang disengaja di §8 — jaring
pengamannya dipasang lebih dulu, selagi belum ada yang bisa jatuh:

- 117/117 berkas `examples/` + `pustaka/` keluar **byte-identik**, komentar
  utuh. Ini membuktikan token stream sanggup merekonstruksi sumber.
- 117/117 **bytecode identik** sebelum/sesudah format.
- Idempoten dan `--periksa` diuji di `tests/test_format.sh`.

**Flag mati terbukti tidak mengubah apa pun:** bytecode 117 berkas dibanding
sebelum/sesudah tahap 1 — identik. Jadi 12 pemanggil `scanToken()` di
compiler memang tidak terpengaruh.

**Langkah 3 dan 4 selesai.** Berkas `.rapi` di `tests/format/` kini benar-benar
berbeda dari `.idk`-nya (input sengaja ditulis berantakan), jadi tes bytecode
sudah menjadi penjaga yang sesungguhnya — dan memang menangkap satu regresi
nyata saat aturan tata letak pertama kali masuk.

Empat bug yang ditangkap jaring itu, semuanya mengubah arti atau merusak kode:

1. **String `"""multi-baris""" terbelah.** Scanner melaporkan baris token
   tempat ia **berakhir**, bukan dimulai. Akibatnya token sesudahnya tampak
   berada di baris lebih awal dan formatter menyisipkan baris baru di tengah
   penugasan. Diperbaiki dengan menyimpan `line` (awal) dan `endLine` (akhir):
   `line` dihitung mundur dari jumlah `\n` di dalam token.
2. **`{` kamus vs `{` blok.** `{"a": 1}` sempat jadi `{ "a": 1}`. Keduanya
   token yang sama, jadi dibedakan lewat token sebelumnya (`braceIsDict`) dan
   dipasangkan dengan tumpukan supaya `}` tahu apa yang ia tutup.
3. **`fungsi(x)` anonim** menjadi `fungsi (x)`.
4. **Komentar tak boleh diikuti token di baris yang sama** — ini aturan
   kebenaran, bukan gaya: komentar berjalan sampai akhir baris, jadi token
   sesudahnya akan tertelan. Dipaksa pindah baris.

**Tidak dikerjakan (sengaja):** formatter tidak memecah atau menyambung baris.
Ia mengikuti baris asli penulis dan hanya memperbaiki indentasi serta spasi.
Aturan panjang baris baru ditambahkan kalau memang diminta.

**Catatan CRLF.** Masukan CRLF keluar sebagai LF karena `readFile()`
menormalkan baris baru sebelum formatter melihatnya. Ini perilaku yang
diinginkan, dan sudah dicatat sebagai komentar di `src/format.c`.

**Jejak di CI.** Menambah `src/format.c` berarti menyentuh `Makefile` **dan
4 daftar sumber mentah** di `.github/workflows/test.yml` (ASan, debug,
coverage, fuzz). Dua Dockerfile aman: satu memakai `make`, satu memakai image
rilis.

## 8. Urutan pengerjaan

1. Tahap 1: `TOKEN_KOMENTAR` + flag opt-in, dengan tes yang membuktikan
   compiler tidak berubah (seluruh suite tetap hijau, bytecode identik).
2. Tes properti bytecode lebih dulu, memakai formatter identitas (keluaran =
   masukan) supaya jaringnya siap sebelum ada yang bisa jatuh.
3. Formatter: indentasi dan spasi, belum komentar.
4. Penempatan komentar sesuai tabel §3.
5. `--tulis` dan `--periksa`.
