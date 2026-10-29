# Roadmap — Bahasa Indonesiaku

**Satu-satunya sumber kebenaran untuk roadmap.** Versi rilis terbaru: `1.3.2`
(`INDK_VERSION` di `src/common.h`). Riwayat detail per rilis ada di
[`CHANGELOG.md`](../CHANGELOG.md).

**Visi:** bahasa bergaya Python dengan kata kunci Bahasa Indonesia —
semantik selaras dengan Python 3 sehingga apa yang dipelajari di
Indonesiaku langsung berlaku di Python, dan sebaliknya. Dasar analisis dan
alasan setiap fase ada di [`ANALISIS/`](ANALISIS/README.md).

## Sudah selesai

| Versi | Isi | Bukti di repo |
|---|---|---|
| 0.1.0 | Scanner, compiler Pratt, VM stack, REPL, fungsi, `jika`/`selagi`, `cetak` | `src/` |
| 0.2.0 | Closure/upvalue, list, kamus, `untuk...dalam`, `hentikan`/`lanjut`, kelas + warisan, `naikkan`/`coba`/`kecuali`, `impor`, GC mark-sweep | `examples/closure.idk`, `examples/oop/` |
| 0.3.0 | Pustaka standar `pustaka/*.idk`, file I/O, waktu/tanggal, konversi, escape string, `argumen`, REPL multi-baris | `pustaka/`, `examples/berkas.idk` |
| 0.4.0 | JSON, HTTP, soket TCP, server REST contoh, `ord`/`chr`/`sisip`/`hapus_pada`; pesan error dengan nomor baris + jejak panggilan | `examples/todo_server.idk`, `src/vm.c` `runtimeError` |
| 1.0.0 | Fase 0–3 tuntas: stabilisasi, inti mirip Python, tipe data lengkap (`bulat`/`desimal`, tupel, himpunan, himpunan beku), modul bernamespace + paket + `__utama__`, SQLite; `SPESIFIKASI.md` normatif; contoh panduan diuji otomatis | `docs/SPESIFIKASI.md`, `tests/test_panduan.sh` |
| 1.1.0 | Fase 4 tuntas: metode ajaib lengkap (30), atribut kelas, `dengan ... sebagai`, dekornator (`@properti`, `@metode_statis`, `@metode_kelas`, `@abstrak`), pewarisan berganda + MRO C3, kelas abstrak | `examples/oop/`, `docs/RENCANA_FITUR/selesai/` |

## Fase pengembangan

Status: ⬜ belum · 🟨 dikerjakan · ✅ selesai. Setiap butir besar mendapat
berkas `RENCANA_FITUR/<nama>.md` saat mulai dikerjakan. Kode B*/K* merujuk
ke [`ANALISIS/01_KONDISI_SAAT_INI.md`](ANALISIS/01_KONDISI_SAAT_INI.md) (bug)
dan [`ANALISIS/03_KEPUTUSAN_DESAIN.md`](ANALISIS/03_KEPUTUSAN_DESAIN.md)
(keputusan desain).

## Definisi "selesai" (berlaku untuk SETIAP fase dan fitur)

Sebuah butir di bawah **tidak boleh** ditandai ✅ hanya karena kodenya jalan.
Satu butir dinyatakan selesai bila seluruhnya terpenuhi:

1. **Rencana lebih dulu di `docs/`** — `RENCANA_FITUR/<nama>.md` dari
   `TEMPLATE.md`, ditulis **sebelum** menyentuh kode, lalu dipindah ke
   `RENCANA_FITUR/selesai/` saat tuntas. Perencanaan tidak pernah ditulis di
   `README.md`, `CHANGELOG.md`, atau `website/`.
2. **Uji wajib** — contoh golden di `examples/` + `tests/expected/`, pasangan
   `.idk`/`.err` di `tests/galat/` untuk yang harus gagal, seluruh
   `tests/test_all.sh` hijau, dan build sanitizer
   (`-DDEBUG_STRESS_GC -fsanitize=address,undefined`) bila menyentuh
   alokasi/stack/koleksi/objek. Berkas golden tidak pernah diregenerasi buta.
3. **Dokumentasi pengguna ikut diperbarui** — `website/panduan/`
   (`03_PANDUAN_SINTAKS.md` untuk sintaks, `07_PUSTAKA_STANDAR.md` untuk
   builtin), plus `CHANGELOG.md` dan `README.md`. Potongan kode di panduan
   wajib benar-benar dijalankan, bukan ditulis dari ingatan.
4. **Ekstensi VS Code ikut diperbarui** — `editors/vscode-indonesiaku/`:
   keyword baru ke `extension.js` KEYWORDS + `tmLanguage.json`, builtin baru
   ke BUILTINS, sintaks baru yang layak ke `snippets/`.
5. **Status dibalik di sini dan di `ANALISIS/02_PETA_FITUR_PYTHON.md`**, dan
   pitfall baru dicatat di `AGENTS.md`.

Interpreter, uji, dokumentasi, dan tooling editor adalah **satu kesatuan** —
fitur yang menyala di terminal tapi tidak di editor, atau tidak ada di
panduan, dihitung belum selesai. Checklist lengkapnya ada di
[`AGENTS.md`](../AGENTS.md) bagian *Feature workflow*; bagian itu yang
mengikat, daftar ini ringkasannya.

### Fase 0 — Stabilisasi (target 0.5.0) — *"tidak ada jawaban salah"*

Tujuan: program pemula (halo dunia, percabangan, perulangan, list, kamus,
fungsi) selalu benar atau gagal dengan pesan jelas. Tanpa fitur besar baru.

| Butir | Ref | Status |
|---|---|---|
| Perbaiki `//` (tidak memancarkan opcode) + tambah `//=` | B1, B13 | ✅ |
| Tolak `f(x=1)` dengan error kompilasi sampai argumen bernama ada | B2 | ✅ |
| Batas rekursi 1000 (`FRAMES_MAX`); jejak error merangkum frame berulang | B3 | ✅ |
| `**` asosiatif kanan; `-2 ** 2 == -4` | B4, B5 | ✅ |
| `%` dan `//` mengikuti tanda pembagi (Python) | B6 | ✅ |
| Penugasan majemuk memakai seluruh ekspresi kanan (`x *= 2 + 3`) | B14 | ✅ |
| `tidak` lebih lemah dari perbandingan | B15 | ✅ |
| Harness tes galat `tests/galat/` (program yang harus gagal) | — | ✅ |
| `lewati` dan `akhirnya` diimplementasikan | B7 | ✅ |
| Semua galat runtime bisa ditangkap `kecuali`; handler `coba` dilepas saat `kembalikan`/`hentikan`/`lanjut` | B8 | ✅ |
| Bagi nol menjadi error | B9 | ✅ |
| Cetak desimal *shortest round-trip* | B10 | ✅ |
| `panjang`/indeks/`potong`/`ord`/`chr` per karakter Unicode | B11 | ✅ |
| `cetak(a, b)` satu baris dipisah spasi; `cetak()` baris kosong | B12 | ✅ |
| `kembalikan`/`hentikan`/`lanjut` keluar dari `coba` ber-`akhirnya` menjalankan blok akhirnya | B7 lanjutan | ✅ |
| Hapus token mati (`INDENT`/`DEDENT`/`NEWLINE`); `;` jadi pemisah pernyataan; `:` dicadangkan untuk Fase 1 | 01 §6 | ✅ |
| Selaraskan README/panduan dengan kenyataan (semua blok kode di dokumentasi diuji dengan `indk`) | 01 §6 | ✅ |
| Tes golden untuk setiap bug di atas | — | ✅ |
| Putuskan K1–K14 (keputusan desain) — semua rekomendasi disetujui | 03 | ✅ |

### Fase 1 — Inti mirip Python (0.6–0.7) — *"tutorial Python bab 1–5 bisa ditiru"*

| Butir | Ref | Status |
|---|---|---|
| Truthiness Python ([rencana](RENCANA_FITUR/selesai/kebenaran_python.md)) | K1 | ✅ |
| `rentang()` ([rencana](RENCANA_FITUR/selesai/rentang.md)); `untuk` atas teks dan rentang | K9 | ✅ |
| Operator `dalam` / `tidak dalam` ([rencana](RENCANA_FITUR/selesai/operator_dalam.md)) | 02 §A | ✅ |
| `enumerasi()`, `untuk` atas kamus | K9 | ✅ |
| `adalah` / `bukan`, rantai perbandingan (`1 < x < 3`, `a dalam b == c`) | 02 §A–B | ✅ |
| Hierarki kelas `Galat*` dan `kecuali Tipe sebagai e` (semua galat runtime sudah bisa ditangkap sejak Fase 0) | K5, 06 §9 | ✅ |
| Akhir baris mengakhiri pernyataan (di luar kurung), seperti Python | B16 | ✅ |
| Assignment di fungsi = lokal; `global`, `nonlokal` | K3 | ✅ |
| Literal kamus `{ }`, indeks negatif, irisan `[a:b:c]` | 02 §B | ✅ |
| Argumen bawaan (nilai default parameter) | K15 | ✅ |
| Argumen bernama `f(b=2)` | K16 | ✅ |
| `cetak` jadi fungsi biasa (`pisah=`, `akhir=`) ([rencana](RENCANA_FITUR/selesai/cetak_fungsi.md)) | K12 | ✅ |
| `repr` untuk elemen koleksi ([rencana](RENCANA_FITUR/selesai/repr_koleksi.md)) | K12 sisa | ✅ |
| Fungsi anonim (ekspresi `fungsi(x) { ... }`), ekspresi kondisional ([rencana](RENCANA_FITUR/selesai/fungsi_anonim_kondisional.md)) | 02 §B | ✅ |
| f-string, string `"""` multi-baris, literal `1e3`, `0x`, `0b`, `1_000` ([rencana](RENCANA_FITUR/selesai/literal_fstring.md)) | 02 §B | ✅ |
| Metode pada `teks`/`daftar`/`kamus` (P1 dari katalog 06) ([rencana](RENCANA_FITUR/selesai/metode_tipe.md)) | K10, 06 §1–3 | ✅ |
| `+`/`*` untuk teks & daftar, `==` struktural | K7 | ✅ |
| `pastikan` (assert) ([rencana](RENCANA_FITUR/selesai/pastikan.md)) | 02 §A | ✅ |
| Traceback dengan kolom + potongan baris; saran "mungkin maksudmu…" ([rencana](RENCANA_FITUR/selesai/traceback_kolom.md)) | 04 §5, 05 §5 | ✅ |
| Kolom pada galat *runtime* ([rencana](RENCANA_FITUR/selesai/kolom_runtime.md)) | 04 §5 | ✅ |

### Fase 2 — Tipe data lengkap (0.8)

✅ ``bulat`` int64 ([rencana](RENCANA_FITUR/selesai/tipe_angka_bulat.md));
✅ penugasan berganda & unpacking ([rencana](RENCANA_FITUR/selesai/penugasan_berganda.md));
✅ `buang` (del) ([rencana](RENCANA_FITUR/selesai/buang.md));
✅ kunci kamus bukan cuma teks — `Value`-based ([rencana](RENCANA_FITUR/selesai/kamus_kunci_hash.md));
✅ operator bitwise `& | ^ ~ << >>` + `&= |= ^= <<= >>=` ([rencana](RENCANA_FITUR/selesai/operator_bitwise.md));
✅ `tupel` `(1, 2)`/`(1,)`/`()` + `tupel()` ([rencana](RENCANA_FITUR/selesai/tupel.md), K17);
✅ `himpunan([...])` + `| & - ^` ([rencana](RENCANA_FITUR/selesai/himpunan.md), K17);
✅ tupel tanpa kurung — `x = 1, 2`, `kembalikan a, b` ([rencana](RENCANA_FITUR/selesai/sisa_fase2.md), K17);
✅ `himpunan_beku` — himpunan yang dapat di-hash ([rencana](RENCANA_FITUR/selesai/sisa_fase2.md));
✅ builtin `hash`, `iter`/`lanjutkan`, `turunan_dari`, `ke_desimal`, `daftar()` ([rencana](RENCANA_FITUR/selesai/sisa_fase2.md)).

> Fase 2 **selesai**. Catatan penamaan: `next` dinamai **`lanjutkan`** karena
> `lanjut` sudah menjadi kata kunci perulangan. Koma pembentuk tupel hanya
> berlaku di sisi kanan penugasan dan setelah `kembalikan`; di dalam kurung
> koma tetap berarti pemisah. `variabel a, b = ...` tetap tidak didukung —
> tulis `a, b = ...` tanpa `variabel`.

Ref: K2, K8, K11, K13, 06 §4–6.

### Fase 3 — Modul & pustaka inti (0.9) — ✅ **selesai**

✅ `ObjModule` dengan namespace sendiri per berkas;
✅ `impor x`, `impor x sebagai y`, `dari x impor y`, `dari x impor y sebagai z`;
✅ paket (`impor a.b` → `a/b.idk`);
✅ jalur pencarian (direktori pengimpor → `INDK_PATH` → `pustaka/`);
✅ `__nama__` / `__utama__`
([rencana](RENCANA_FITUR/selesai/modul_namespace.md)).
✅ **Basis data SQLite**, lihat
[`RENCANA_FITUR/selesai/basis_data_sqlite.md`](RENCANA_FITUR/selesai/basis_data_sqlite.md)
(dimajukan dari Fase 8: amalgamation adalah satu berkas `.c`, tidak butuh FFI).
SQLite 3.50.4 di-*vendor*; 10 fungsi `db_*` dengan parameter `?` wajib dan
satu pernyataan per panggilan.

Bentuk lama `impor "file.idk"` sengaja dipertahankan apa adanya (namespace
bersama) demi kompatibilitas K14 — itu yang dipakai `pustaka/*.idk`.

**Sisa yang disengaja:** modul P1 katalog 07 (`matematika`, `acak`, `teks`,
`sistem`, `waktu`, `tanggal_waktu`, `json`) belum ditulis ulang sebagai modul
**native C**. Isinya sudah tersedia sebagai `pustaka/*.idk` dan kini dapat
diimpor bermodul (`impor matematika`), sehingga penulisan ulang ke C adalah
optimasi, bukan fitur yang hilang.
Ref: 04 §6, 07 §1–5.

### ★ Rilis 1.0 — "Stabil untuk umum" — ✅ **syarat terpenuhi**

Syarat: ✅ Fase 0–3 selesai; ✅ [`SPESIFIKASI.md`](SPESIFIKASI.md) ditulis;
✅ semua contoh di panduan diuji otomatis (`tests/test_panduan.sh`, ikut
dijalankan `tests/test_all.sh`); ✅ kebijakan stabilitas K14 berlaku. Setelah
1.0, tidak ada breaking change sampai 2.0.

### Fase 4 — OOP lengkap (1.1–1.2)

Metode ajaib (06 §10), atribut kelas, `@properti`/`@metode_statis`/
`@metode_kelas`, dekorator, pewarisan berganda + MRO C3, `dengan ... sebagai`
(manajer konteks), `adalah_instansi`, kelas abstrak. Ref: 04 §4.

✅ metode ajaib tahap 1 — representasi, perbandingan, aritmetika
([rencana](RENCANA_FITUR/selesai/metode_ajaib.md), K19).

✅ metode ajaib tahap 2a — protokol objek: `_panjang_`, `_logika_`,
`_ambil_`/`_atur_`, `_hash_` + builtin `hash()`
([rencana](RENCANA_FITUR/selesai/protokol_objek.md), K19).

✅ metode ajaib tahap 2b — cerminan aritmetika `_ktambah_` dkk
([rencana](RENCANA_FITUR/selesai/cerminan_aritmetika.md), K19). Sentinel
`TidakDiterapkan` sengaja ditunda; alasannya ada di rencana itu.

✅ metode ajaib tahap 2c — `terurut`/`min`/`max` atas objek
([rencana](RENCANA_FITUR/selesai/urut_objek.md), K19).

✅ metode ajaib tahap 2d — `_iter_` (objek di `untuk ... dalam`) dan
`_panggil_` (objek sebagai fungsi)
([rencana](RENCANA_FITUR/selesai/iter_panggil.md), K19). Metode ajaib kini
**lengkap**: 30 metode. `adalah_instansi` sudah ada sejak Fase 2.

✅ atribut kelas — nilai bersama di badan kelas
([rencana](RENCANA_FITUR/selesai/atribut_kelas.md)).

✅ `dengan ... sebagai` — manajer konteks lewat `_masuk_`/`_keluar_`
([rencana](RENCANA_FITUR/selesai/dengan_sebagai.md)).

✅ Dekorator `@nama` pada fungsi, plus `@properti`, `@metode_statis`, dan
`@metode_kelas` ([rencana](RENCANA_FITUR/selesai/dekorator.md)).

✅ Pewarisan berganda + MRO C3
([rencana](RENCANA_FITUR/selesai/pewarisan_berganda.md)).

✅ Kelas abstrak `@abstrak`
([rencana](RENCANA_FITUR/selesai/kelas_abstrak.md)).

> **Fase 4 SELESAI.** Seluruh item OOP lengkap sudah diterapkan: metode ajaib
> (30), `adalah_instansi`, atribut kelas, `dengan ... sebagai`, dekorator
> (`@properti`, `@metode_statis`, `@metode_kelas`, `@abstrak`), pewarisan
> berganda + MRO C3, dan kelas abstrak.

### Fase 5 — Iterator & generator (1.3)

✅ Protokol iterator malas `_berikutnya_` (melengkapi `_iter_` dari Fase 4)
([rencana](RENCANA_FITUR/selesai/iterator_malas.md)).

✅ `hasilkan` (yield) — fungsi generator dengan frame yang ditangguhkan.

✅ Generator expression `(x untuk x dalam s)`.

✅ Comprehension kamus `{k: v untuk ...}` dan himpunan `{x untuk ...}`.

✅ `cocokkan`/`kasus` (pattern matching): literal, `_`, pengikatan nama,
`Kelas()`, alternatif `atau`, dan penjaga `jika`.

Ref: 04 §7.

> **Fase 5 SELESAI.** Metode ajaib kini 31 (`_berikutnya_`), generator dan
> generator expression berjalan malas, comprehension tersedia untuk daftar,
> kamus, dan himpunan, serta semuanya dapat membaca generator maupun iterator
> kustom. Batas yang disengaja: pola belum bersarang (`[a, b]` destructuring
> belum ada), dan `hasilkan` belum didukung di dalam blok `coba ... akhirnya`.

### Fase 6 — Pustaka standar luas (1.x berkelanjutan) — ✅ selesai (kecuali yang ditunda)

Modul P2 dari katalog 07. Modul baru berbentuk **modul bernamespace**
(`impor koleksi`, `dari koleksi impor Penghitung`), ditulis dalam Indonesiaku
di `pustaka/` kecuali yang ditandai C. Sebagian besar cocok untuk kontributor
baru.

**Gelombang A — modul murni Indonesiaku (selesai):**

| Modul | Status | Bukti |
|---|---|---|
| `salin` (copy) | ✅ | `examples/pustaka_salin.idk` |
| `statistik` (statistics) | ✅ | `examples/pustaka_statistik.idk` |
| `koleksi` (collections) | ✅ | `examples/pustaka_koleksi.idk` |
| `iterasi` (itertools, eager) | ✅ | `examples/pustaka_iterasi.idk` |
| `fungsional` (functools) | ✅ | `examples/pustaka_fungsional.idk` |

**Gelombang B & C** ([B](RENCANA_FITUR/selesai/pustaka_fase6_batch_b.md),
[C](RENCANA_FITUR/selesai/pustaka_fase6_gelombang_c.md),
[kura](RENCANA_FITUR/selesai/pustaka_kura.md)):

| Modul | Status | Bukti |
|---|---|---|
| `jalur` (os.path) | ✅ | `examples/pustaka_jalur.idk`, `examples/sistem_dasar.idk` |
| `opsi` (argparse) | ✅ | `examples/pustaka_opsi.idk` |
| `log` (logging) | ✅ | `examples/pustaka_log.idk` |
| `proses` (subprocess) + `indk --sandbox` | ✅ | `examples/pustaka_proses.idk` |
| builtin sistem operasi (`src/sistem.c`) | ✅ | `examples/sistem_dasar.idk` |
| `pola` (re, mesin C linear) | ✅ | `examples/pustaka_pola.idk` |
| `bita`, `base64`, `csv` | ✅ | `examples/pustaka_{bita,base64,csv}.idk` |
| `md5`/`sha1`/`sha256`/`sha512` | ✅ | `examples/pustaka_hash.idk` |
| klien HTTP (`http_ambil`) + `soket_sambung` | ✅ | `examples/pustaka_http_klien.idk`, `tests/test_klien_http.sh` |
| trigonometri (`sin` … `atan2`, `radian`, `derajat`) | ✅ | `examples/trigonometri.idk` |
| `kura` (turtle → SVG) | ✅ | `examples/pustaka_kura.idk` |

Lanjutan yang sengaja di luar Fase 6: HTTPS (butuh TLS, Fase 8), kanvas
`kura` beranimasi di playground, dan modul P3 katalog 07.

**Ditunda karena keterbatasan bahasa** (bukan lupa): `tupel_bernama`,
`@cache_lru`, dan `urutan_lengkap` menunggu fungsi variadik (`*args`) dan
penyuntikan metode saat runtime. Alasannya dicatat di
[`RENCANA_FITUR/selesai/pustaka_fase6_modul_dasar.md`](RENCANA_FITUR/selesai/pustaka_fase6_modul_dasar.md).

### Fase 7 — Perkakas & kinerja

✅ `indk --dis` — tampilkan bytecode tanpa menjalankan, termasuk fungsi
bersarang.

✅ `indk periksa` (linter) — deteksi variabel lokal tidak terpakai
([rencana](RENCANA_FITUR/selesai/deteksi_tidak_terpakai.md)). Hanya tingkat 2
(perintah opt-in); tingkat 1 dibatalkan karena menulis ke stderr di setiap
kali menjalankan program bentrok dengan syarat stderr kosong pada tes golden.

✅ `indk uji` — jalankan setiap fungsi `uji_*` di sebuah berkas, laporkan
lulus/gagal, exit 1 bila ada yang gagal. Asersi memakai `pastikan` yang sudah
ada, jadi tidak ada sintaks tes baru.

- [x] **`indk rapikan [--tulis|--periksa]`** — formatter berbasis token.
      Indentasi 4 spasi, spasi operator, `{` kamus vs blok, komentar bertahan
      utuh. Dijamin tidak mengubah arti program: 117/117 bytecode identik
      sebelum/sesudah format, dan hasilnya idempoten.
      Batas sadar: tidak memecah/menyambung baris — mengikuti baris asli
      penulis. K4 tetap aman: `examples/tanpa_indentasi.idk` dirapikan dan
      keluarannya tetap cocok golden.

✅ **Formatter VS Code** — `Format Document` memanggil `indk rapikan`
([`editors/vscode-indonesiaku/`](../editors/vscode-indonesiaku/)). Mendelegasikan
ke interpreter, bukan mengimplementasi ulang aturan format di JavaScript.

Belum: LSP + diagnostics editor, debugger,
playground via **WebAssembly**, cache `.idkc`, computed goto, superinstruksi,
inline cache, NaN-boxing, GC generasional, utas/asinkron.
Ref: 04 §1, §8–10, §12.

> **Catatan computed goto.** Diukur dulu, belum dikerjakan. `run()` punya 85
> opcode dengan 93 `case`, dan **19 di antaranya mengandung `for`/`while`** —
> di sana `break` berarti keluar loop, bukan keluar switch. Transformasi
> mekanis ke tabel lompat akan salah kompilasi secara diam-diam di 19 titik
> itu. Build sudah `-O2`. Kerjakan hanya dengan profil yang menunjukkan
> dispatch benar-benar jadi hambatan, dan konversi satu per satu disertai
> pemeriksaan tiap `break`.

> **Catatan `indk rapikan`.** Penghalangnya sudah dibereskan, bukan dihindari.
> Dulu `skipWhitespace()` di `src/scanner.c` membuang komentar sepenuhnya —
> tidak ada `TOKEN_KOMENTAR`, jadi komentar tidak pernah sampai ke token
> stream, dan formatter berbasis token akan **menghapus seluruh komentar**
> pengguna. Sekarang scanner punya mode penyimpan komentar yang **opt-in**
> (`scannerKeepComments()`, default mati) sehingga compiler tidak terpengaruh:
> dibuktikan dengan 117/117 berkas menghasilkan bytecode identik saat flag
> mati. Formatter memakai mode itu dan komentar bertahan utuh.
> Rencana: [`RENCANA_FITUR/selesai/komentar_dan_rapikan.md`](RENCANA_FITUR/selesai/komentar_dan_rapikan.md).

> **Catatan cache `.idkc`.** Diukur, lalu ditunda sebagai YAGNI. Berkas
> sumber terbesar di repo (`metode_ajaib.idk`, 8 KB) butuh **3,1 ms** total,
> dan **1,3 ms** di antaranya adalah ongkos start proses — jadi kompilasi
> sesungguhnya sekitar **1,8 ms**. Cache bytecode berarti menambah format
> serialisasi, penomoran versi, invalidasi berdasarkan mtime/hash, dan
> penulisan graf objek GC (`ObjString`, `ObjFunction` bersarang, `Chunk`
> dengan `lines`/`columns`/`source`) demi menghemat < 2 ms. Kerjakan kalau
> ada program yang kompilasinya benar-benar terasa, bukan sebelum itu.

> **Catatan playground WebAssembly.** Terhalang perkakas: `emcc` tidak
> terpasang di mesin ini (`which emcc` kosong, tidak ada
> `/usr/lib/emscripten`). Playground yang ada sekarang berjalan lewat Docker
> dan sudah berfungsi, jadi WASM adalah penggantian jalur eksekusi, bukan
> fitur baru untuk pengguna. Butuh pemasangan Emscripten lebih dulu.

> **Catatan LSP, debugger, inline cache, superinstruksi, NaN-boxing, GC
> generasional, utas/asinkron.** Belum dikerjakan. Masing-masing berukuran
> setara satu fase tersendiri — NaN-boxing menyentuh setiap `Value` di
> seluruh basis kode, GC generasional menulis ulang `memory.c`, dan LSP
> adalah program kedua di samping interpreter. Jangan dikerjakan sebagai
> tambahan di sela pekerjaan lain; masing-masing butuh berkas rencana
> sendiri di `docs/RENCANA_FITUR/` lebih dulu.

### Fase 8 — Ekosistem

Manajer paket + registri, API embedding C, FFI, MySQL/PostgreSQL (klien
jaringan — `sqlite` sendiri sudah dimajukan ke Fase 3), TLS/HTTPS, GUI,
AOT/bundel executable, ~~anotasi tipe opsional~~ ✅ ([rencana](RENCANA_FITUR/selesai/anotasi_tipe.md)), mode sandbox untuk
sekolah.
Ref: 04 §11–13, 05.

## Roadmap implementasi per fase — setelah v1.3.2

Bagian ini menyatukan rencana P1–P7, WDB-01–05, WEB-01–07, DX-01–02, dan SSR-01–10 yang sebelumnya tersebar dalam beberapa tabel. **Fase 0–6 sudah selesai, Fase 7 baru sebagian, dan Fase 8 baru sebagian (anotasi tipe opsional).** Fase 9 dan seterusnya adalah urutan kerja yang **diusulkan**, bukan penanda implementasi saat ini. Nomor ID lama tetap dipakai sebagai jejak rencana.

**Cara membaca:** setiap fase memiliki tujuan, kebutuhan awal, langkah implementasi berurutan, contoh API target, pengujian wajib, serta definisi selesai. Tanda [ ] berarti belum. Implementasi dapat dibuat dalam pull request kecil; jangan memaksa seluruh fase dalam satu PR. Semua perubahan tetap tunduk pada "Definisi selesai" dan "Feature workflow" di AGENTS.md, termasuk rencana detail sebelum kode, golden test, negative test, dokumentasi dan editor.

### Fase 9 — Language Server Protocol (LSP) dan pengalaman editor

**Tujuan:** file .idk mendapatkan diagnostics, autocomplete, hover, dan go-to-definition di VS Code tanpa menjalankan program pengguna. **Status:** ⬜ belum. **Pemetaan:** P1 (bagian Fase 7 yang masih tertunda).

**Langkah pengembangan:**

1. [ ] Buat dokumen desain di docs/RENCANA_FITUR/lsp.md. Catat batas parser single-pass, kebutuhan analisis file tidak lengkap, dan keputusan apakah menggunakan scanner/compiler dalam mode analisis terpisah atau komponen parser toleran error.
2. [ ] Spesifikasikan proses language server mandiri (misalnya executable indk-lsp) dengan JSON-RPC 2.0 lewat stdio, initialize/shutdown, sinkronisasi dokumen, dan pembatasan input.
3. [ ] Implementasikan diagnostics untuk kesalahan sintaks dan simbol yang benar-benar bisa dipastikan salah; **jangan mengeksekusi kode** untuk memeriksa file.
4. [ ] Implementasikan completion kata kunci, builtin dan simbol scope; hover dokumentasi sederhana; go-to-definition untuk variabel/fungsi lokal, lalu impor lintas file.
5. [ ] Hubungkan language server ke editors/vscode-indonesiaku tanpa merusak fitur syntax highlight dan formatter yang sudah ada.
6. [ ] Tambahkan fixtures LSP, uji request/response JSON-RPC, posisi UTF-16 editor vs offset UTF-8 scanner, dokumen belum lengkap, Unicode, multiple files, pembatalan, dan pemulihan setelah input salah.
7. [ ] Perbarui panduan VS Code, contoh penggunaan, proses build/release LSP lintas platform, serta dokumen keputusan desain.

**Target API/CLI (usulan):** indk-lsp --stdio; integrasi otomatis saat membuka berkas .idk.

**Selesai jika:** diagnostics, completion, hover dan definisi diuji; kode tidak dieksekusi; editor tetap stabil saat sumber belum valid; CI Linux/Windows/macOS lulus.

### Fase 10 — Driver dan fondasi database

**Tujuan:** SQLite dipertahankan, lalu MySQL/MariaDB dan PostgreSQL dapat dipakai dengan query aman. **Status:** ⬜ belum. **Pemetaan:** WDB-01–05, P6.

**Langkah pengembangan:**

1. [ ] Buat docs/RENCANA_FITUR/database_multi_driver.md; tetapkan API, semantik transaksi, tipe nilai/NULL/BLOB, error, koneksi dan lifecycle GC. Jangan mengubah builtin db_* tanpa kebijakan kompatibilitas.
2. [ ] Audit src/db.c, src/object.c/h, src/native.c/h, dan examples/db_sqlite.idk. Lengkapi SQLite: dukungan data biner/BLOB masuk-keluar, busy timeout, pengujian transaksi dan penutupan handle idempoten (**WDB-01**).
3. [ ] Buat driver MySQL/MariaDB (**WDB-02**) menggunakan client library teruji dan prepared statements; dukung host/port, opsi TLS dengan verifikasi, timeout, penanganan koneksi terputus, query satu/banyak baris, perubahan baris dan ID terakhir sesuai dialek.
4. [ ] Buat driver PostgreSQL (**WDB-03**) memakai libpq atau pustaka kompatibel yang terawat; pastikan parameter binding, hasil data bertipe, transaksi, timeout dan validasi TLS/hostname.
5. [ ] Rancang API abstraksi bersama (**WDB-04**), misalnya db.koneksi(konfigurasi), koneksi.eksekusi(sql, parameter), koneksi.ambil_satu, koneksi.ambil_semua, koneksi.mulai/komit/batal/tutup. Nama final diputuskan lewat dokumen rencana; API SQLite lama tetap berjalan.
6. [ ] Tambahkan lapisan migrasi skema dan query builder parameterized ringan (**WDB-05**); simpan versi migrasi dan catat aturan rollback. ORM penuh tidak wajib.
7. [ ] Siapkan database uji MySQL, MariaDB, PostgreSQL dalam container CI yang terisolasi; jalankan suite bersama untuk koneksi, CRUD, transaksi, rollback, NULL, Unicode, BLOB, kegagalan TLS, kredensial salah, timeout, SQL injection dan cleanup.
8. [ ] Perbarui docs/SPESIFIKASI.md bila kontrak bahasa berubah, website/panduan/07_PUSTAKA_STANDAR.md, examples/, CHANGELOG.md dan instalasi dependensi per OS.

**Contoh API target (belum tersedia):** koneksi = basisdata.buka({"driver":"postgresql","host":"localhost","nama":"sekolah"}); koneksi.ambil_semua("SELECT * FROM siswa WHERE id = ?", [1]). Placeholder disesuaikan internal oleh driver, bukan dirangkai ke SQL secara manual.

**Selesai jika:** tiga keluarga database (SQLite, MySQL/MariaDB, PostgreSQL) berhasil CRUD/transaksi dengan parameter, error dapat ditangkap, tidak ada password masuk log, dependensi dan cara build terdokumentasi. TLS driver boleh berbagi komponen dari Fase 11 dan harus lulus sebelum driver koneksi luar dianggap siap.

### Fase 11 — HTTP, HTTPS/TLS dan server backend modular

**Tujuan:** aplikasi Indonesiaku dapat melayani REST API secara terstruktur; contoh todo_server.idk yang single-thread/blocking bukan lagi satu-satunya pola. **Status:** ⬜ belum. **Pemetaan:** P2, WEB-01, WEB-02, WEB-03, WEB-07.

**Langkah pengembangan:**

1. [ ] Buat docs/RENCANA_FITUR/http_web_server.md dan https_tls.md. Tentukan kontrak Request/Response, router, middleware, batas request, model konkurensi dan pemakaian reverse proxy.
2. [ ] Perluas klien pustaka/http.idk sehingga http_ambil dapat menerima https:// (**P2**). Pilih TLS library yang terawat; aktifkan verifikasi CA dan hostname secara default; timeout dan redirect dibatasi; insecure-by-default dilarang.
3. [ ] Rancang modul server HTTP dengan routing metode/path, parameter URL, query string, header, status, error handler dan middleware (**WEB-01**). Hindari ekspose detail socket mentah pada API yang digunakan pemula.
4. [ ] Tambahkan parsing application/json, application/x-www-form-urlencoded, multipart/form-data, file upload streaming dan batas ukuran/bentuk input (**WEB-02**).
5. [ ] Implementasikan static file handler untuk CSS, JavaScript, gambar dan aset lain, MIME yang benar, cache header, penolakan traversal serta symlink berbahaya (**WEB-03**). Ini **bukan** template HTML.
6. [ ] Rancang concurrency yang terukur, batas koneksi/thread/worker, timeout, backpressure, shutdown aman dan log terstruktur (**WEB-07**). Mulai dengan model sederhana yang dapat diuji dan ditingkatkan; jangan langsung mengklaim setara server produksi.
7. [ ] Buat contoh CRUD API memakai database Fase 10, uji dengan curl, serta CI untuk slow client, request berukuran besar, malformed HTTP, upload, redirect, sertifikat kedaluwarsa/tidak cocok, race/resource leak dan graceful shutdown.
8. [ ] Dokumentasikan deployment dengan Docker/reverse proxy TLS, isolasi proses, pembatasan resource dan risiko membuka port secara publik.

**Target API (usulan):** app = web.Aplikasi(); app.rute("GET", "/api/siswa", fungsi_handler); app.jalankan(port=8080). Sintaks final harus sesuai kemampuan compiler, bukan diasumsikan hanya karena mirip Python.

**Selesai jika:** server API bisa dipakai tanpa HTML, klien HTTPS memvalidasi sertifikat, parsing request aman dan tes integrasi lintas platform lulus.

### Fase 12 — HTML template engine dan Server-Side Rendering (SSR)

**Tujuan:** file .idk menjalankan logika backend, file .html berisi tampilan standar; server merender HTML dinamis tanpa SPA/React. Template engine dibuat independen dari server agar dapat diuji sendiri. **Status:** ⬜ belum. **Pemetaan:** SSR-01–10.

**Langkah pengembangan:**

1. [ ] Rancang docs/RENCANA_FITUR/ssr_template_engine.md. Definisikan modul terpisah (mis. pustaka/templat.idk atau engine native C jika hasil profil membutuhkannya), template root, render(nama,data), render_teks(sumber,data), error dan scope (**SSR-01**).
2. [ ] Bangun loader dan parser yang memisahkan literal HTML, interpolasi {{ nama }} dan tag {% ... %}, dengan penanda path/baris/kolom, batas ukuran dan kedalaman (**SSR-02**).
3. [ ] Implementasikan interpolasi variabel, akses kamus dan atribut yang dibolehkan, serta error untuk nilai yang tidak tersedia; **jangan evaluasi string menjadi kode .idk arbitrer** (**SSR-03**).
4. [ ] Tambahkan kondisi dan loop bertingkat dengan sintaks yang konsisten, misalnya {% jika ... %} / {% selain %} / {% akhirjika %}, {% untuk ... %} / {% akhiruntuk %}, dan uji scoping (**SSR-04**).
5. [ ] Tambahkan HTML auto-escaping **sejak awal interpolasi** (**SSR-06**); tangani &, <, >, kutip, dan aturan khusus atribut/URL; hindari menyatakan aman untuk ekspresi di JS/CSS inline. Konten HTML tepercaya hanya melalui API eksplisit yang diaudit.
6. [ ] Tambahkan include komponen, template inheritance dan block override; batasi path dalam root template dan cegah siklus (**SSR-05**).
7. [ ] Integrasikan render ke response web Fase 11 dengan Content-Type text/html; charset=utf-8, status/headers yang tepat dan halaman error yang tidak membocorkan rahasia (**SSR-07**).
8. [ ] Tambahkan cache parsing, invalidasi dalam mode development, pembatasan memori dan kesetaraan hasil dengan mode tanpa cache (**SSR-08**).
9. [ ] Buat tes diagnosis path/baris/kolom, input XSS, Unicode, nesting, template tidak ada, include cyclic, traversal, beban input besar dan sanitizer/GC (**SSR-09**).
10. [ ] Tulis contoh website dinamis menggunakan SQLite (listing dan detail mahasiswa), dokumentasi template di website/panduan/, serta panduan deployment dan troubleshooting (**SSR-10**).

**Contoh target (usulan):** kembalikan templat.render("mahasiswa.html", {"nama":"Budi"}); isi template: <h1>{{ nama }}</h1>. Sintaks ini belum implementasi.

**Selesai jika:** halaman server-side menampilkan data dinamis dari database, auto-escaping aktif, include/layout bekerja, diagnostics jelas dan seluruh tes aman lulus.

### Fase 13 — Keamanan dan fitur aplikasi web

**Tujuan:** membangun fondasi aplikasi bisnis yang membutuhkan input pengguna, login, session dan otorisasi. **Status:** ⬜ belum. **Pemetaan:** WEB-04, WEB-05, WEB-06.

**Langkah pengembangan:**

1. [ ] Buat rencana docs/RENCANA_FITUR/web_security_auth.md, threat model (browser, cookie, CSRF, upload, input, session, proxy) dan matriks fitur minimum.
2. [ ] Implementasikan cookie HttpOnly, Secure, SameSite, expiry dan session storage yang dapat dikonfigurasi; rotasi ID saat login dan invalidasi logout (**WEB-04**).
3. [ ] Sediakan validasi input, output escaping sesuai konteks, proteksi CSRF pada endpoint state-changing dengan session-cookie, CORS yang dibatasi dan trusted-proxy yang eksplisit (**WEB-05**).
4. [ ] Implementasikan autentikasi, password hashing adaptif yang teruji (utamakan Argon2id jika dependensi tersedia), login/logout, role/permission, dan perlindungan brute force/rate limit (**WEB-06**).
5. [ ] Siapkan contoh aplikasi CRUD dengan login dan role (admin/pengguna), memanfaatkan database Fase 10, HTTP Fase 11, SSR Fase 12.
6. [ ] Tambahkan tes terhadap session fixation, CSRF, cookie flags, file upload berbahaya, validasi izin lintas pengguna, timing/lockout, secret leakage serta regresi keamanan.

**Selesai jika:** jalur login-logout, kontrol akses, session, CSRF dan validasi input diuji otomatis; tidak ada default credential atau penyimpanan password plaintext.

### Fase 14 — Package manager, perkakas proyek dan integrasi library

**Tujuan:** developer dapat memulai dan membagikan proyek backend tanpa menyalin modul secara manual. **Status:** ⬜ belum. **Pemetaan:** P3, DX-01, DX-02.

**Langkah pengembangan:**

1. [ ] Buat rencana docs/RENCANA_FITUR/manajer_paket.md: manifest, nama paket/namespace, versi semantik, lockfile, cache, URL registri, checksum/signature dan lokasi instalasi non-root.
2. [ ] Implementasikan perintah CLI inisialisasi proyek, tambah, hapus, daftar, perbarui dan pemulihan lockfile; pisahkan dependency resolution dari unduhan.
3. [ ] Rancang format paket yang dapat dipublikasikan dan registry/API dengan otorisasi, integritas artefak, kebijakan takedown dan mitigasi package-name squatting. **Registri publik belum perlu diluncurkan sebelum desain keamanan selesai.**
4. [ ] Tambahkan scaffolding aplikasi backend (**DX-01**): direktori modul, file konfigurasi, contoh HTTP dan database, tes awal, penggunaan environment serta perintah menjalankan/menguji.
5. [ ] Integrasikan paket backend dan SSR dengan manajer paket serta lockfile (**DX-02**) tanpa menghilangkan opsi memakai standard library bawaan.
6. [ ] Tambahkan tes konflik dependensi, paket berbahaya, checksum tidak cocok, install offline/cache, downgrade, path traversal arsip, reproducible install, serta Windows/Linux/macOS.
7. [ ] Dokumentasikan cara membuat, mempublikasikan, menginstal dan mengaudit dependensi paket serta lisensi pihak ketiga.

**Selesai jika:** proyek bersih dapat dibuat dan dependensi dipasang secara reproducible dengan lockfile, validasi integritas aktif dan tutorial telah diuji.

### Fase 15 — Debugger, embedding API dan FFI

**Tujuan:** memperluas ekosistem tooling dan integrasi native secara aman. **Status:** ⬜ belum. **Pemetaan:** P4, P5.

**Langkah pengembangan:**

1. [ ] Rancang debugger dalam docs/RENCANA_FITUR/debugger.md: metadata baris/kolom, breakpoint, pause/step, inspeksi frame/variabel, protokol komunikasi editor dan overhead saat debug nonaktif.
2. [ ] Implementasikan eksekusi debugger terkontrol dan tes step in/over/out, exception, closure, generator, nested call, serta terminasi program.
3. [ ] Buat docs/RENCANA_FITUR/embedding_ffi.md: ABI C publik, lifecycle VM, registrasi fungsi native, konversi Value, error, multi-instance dan rooting GC.
4. [ ] Implementasikan API embedding terlebih dahulu; FFI umum menyusul hanya setelah batas keamanan, ownership memori dan sandbox disepakati.
5. [ ] Sediakan header publik, contoh host C, dokumentasi API, tes sanitizer, kompatibilitas compiler serta prosedur pemeliharaan ABI.

**Selesai jika:** debugging teruji tanpa merusak mode biasa, aplikasi C dapat memanggil Indonesiaku melalui API stabil yang terdokumentasi dan interaksi GC aman.

### Fase 16 — WebAssembly, performa dan perluasan ekosistem

**Tujuan:** memperkuat jangkauan platform dan kinerja **berdasarkan pengukuran**, bukan menambah kompleksitas sebelum perlu. **Status:** ⬜ belum. **Pemetaan:** P7 serta sisa Fase 7–8.

**Langkah pengembangan:**

1. [ ] Buat rencana docs/RENCANA_FITUR/playground_wasm.md; siapkan Emscripten, bangun interpreter ke WebAssembly dan tentukan pembatasan fitur OS, file, network dan resource.
2. [ ] Jalankan playground .idk langsung dalam browser untuk kode lokal tanpa bergantung pada eksekusi server (**P7**); pertahankan Docker playground yang sudah ada sebagai alternatif.
3. [ ] Tambahkan tes kesetaraan hasil interpreter native vs WASM, timeout/memory guard, penanganan error dan perbedaan runtime browser.
4. [ ] Profilkan program nyata lebih dulu sebelum menilai cache .idkc, computed goto, superinstruksi, inline cache, NaN-boxing dan GC generasional. Tuliskan benchmark sebelum/sesudah dan rollback plan; optimasi yang tidak terbukti bermanfaat tetap ditunda.
5. [ ] Rencanakan threading/asinkron, GUI, AOT/bundel executable dan penguatan sandbox sebagai subproyek terpisah; jangan klaim selesai tanpa spesifikasi, tes isolasi dan deployment yang aman.

**Selesai jika:** playground WASM bekerja pada contoh yang didukung, perbedaan fitur terdokumentasi, serta optimasi hanya digabung setelah benchmark dan regresi membuktikan manfaat.

### Checklist wajib untuk setiap fase (panduan untuk AI coding agent)

1. **Audit sebelum coding:** baca AGENTS.md, docs/SPESIFIKASI.md, docs/ANALISIS/, README, contoh dan source terkait; verifikasi fitur existing supaya tidak menduplikasi fungsi atau merusak kompatibilitas sejak 1.0.
2. **Rencanakan dahulu:** buat satu atau beberapa dokumen docs/RENCANA_FITUR/<fitur>.md memakai TEMPLATE.md; tuliskan kebutuhan, batas scope, alternatif desain, API, semantik error, keamanan, dampak VM/GC, daftar berkas, tes dan kriteria selesai.
3. **Kerjakan bertahap:** gunakan branch fitur/PR kecil; mulai dari API dan tes perilaku, lanjut implementasi, lalu integrasi. Jangan mencampur fitur antar-fase tanpa dependensi yang jelas.
4. **Perbarui konsumen:** contoh .idk, pasangan tests/expected atau tests/galat, tes integrasi, panduan website, editor VS Code, spesifikasi jika relevan, CHANGELOG, dan file rencana.
5. **Uji wajib:** jalankan semua golden/negative tests, sanitizer/DEBUG_STRESS_GC jika memodifikasi memori/objek/stack, Valgrind bila relevan, pengujian lintas OS, dan tes keamanan fitur network/database/template.
6. **Tinjau dan rilis:** ukur performa bila klaim optimasi, pastikan CI hijau, review kode dan keamanan, cantumkan perubahan API/breaking changes. Sejak v1.0, jangan melakukan breaking change sebelum kebijakan versi mayor memperbolehkan.
7. **Tandai status secara jujur:** [ ] belum; [x] selesai hanya setelah implementasi + pengujian + dokumentasi. Pindahkan dokumen rencana ke docs/RENCANA_FITUR/selesai/ sesuai AGENTS.md dan perbarui tabel status fase.

### Ringkasan status fase baru

| Fase | Fokus | Status |
|---|---|---|
| 9 | LSP dan dukungan editor | ⬜ belum |
| 10 | SQLite, MySQL/MariaDB, PostgreSQL | ⬜ belum |
| 11 | HTTP/HTTPS dan backend server | ⬜ belum |
| 12 | Server-Side Rendering / HTML template | ⬜ belum |
| 13 | Session, keamanan, login dan otorisasi | ⬜ belum |
| 14 | Package manager dan developer tooling | ⬜ belum |
| 15 | Debugger, embedding C, FFI | ⬜ belum |
| 16 | WASM playground dan optimasi lanjut | ⬜ belum |

## Cara memakai berkas ini

1. Untuk mengusulkan fitur: salin `RENCANA_FITUR/TEMPLATE.md`, isi, lalu
   tautkan dari butir fase yang relevan.
2. Saat fitur selesai: ubah status di sini + catat di `CHANGELOG.md`, lalu
   hapus/arsipkan berkas rencananya.
3. Jangan menyalin roadmap ini ke `README.md` atau berkas panduan.
