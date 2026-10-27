# Roadmap — Bahasa Indonesiaku

**Satu-satunya sumber kebenaran untuk roadmap.** Versi saat ini: `1.0.0`
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

> Fase 4 **sedang dikerjakan** — yang tersisa: pewarisan berganda + MRO C3,
> kelas abstrak.

### Fase 5 — Iterator & generator (1.3)

Protokol iterator malas `_berikutnya_` (melengkapi `_iter_` dari Fase 4),
`hasilkan` (yield), generator expression,
dict/set comprehension, `cocokkan`/`kasus` (pattern matching).
Ref: 04 §7.

### Fase 6 — Pustaka standar luas (1.x berkelanjutan)

Modul P2 dari katalog 07: `os`/`jalur`, `koleksi`, `iterasi`,
`fungsional`, `pola` (regex), `csv`, `hash`, `base64`, `log`, `opsi`,
`proses`, `salin`, `statistik`, `kura` (turtle), klien HTTP, `bita`.
Sebagian besar cocok untuk kontributor baru.

### Fase 7 — Perkakas & kinerja

`indk uji`, `indk rapikan` (formatter), `indk periksa` (linter — lihat
[`RENCANA_FITUR/deteksi_tidak_terpakai.md`](RENCANA_FITUR/deteksi_tidak_terpakai.md)
untuk deteksi variabel/fungsi tidak terpakai), LSP +
ekstensi VS Code, debugger, `indk --dis`, playground via **WebAssembly**,
cache `.idkc`, computed goto, superinstruksi, inline cache, NaN-boxing, GC
generasional, utas/asinkron. Ref: 04 §1, §8–10, §12.

### Fase 8 — Ekosistem

Manajer paket + registri, API embedding C, FFI, MySQL/PostgreSQL (klien
jaringan — `sqlite` sendiri sudah dimajukan ke Fase 3), TLS/HTTPS, GUI,
AOT/bundel executable, anotasi tipe opsional, mode sandbox untuk sekolah.
Ref: 04 §11–13, 05.

## Cara memakai berkas ini

1. Untuk mengusulkan fitur: salin `RENCANA_FITUR/TEMPLATE.md`, isi, lalu
   tautkan dari butir fase yang relevan.
2. Saat fitur selesai: ubah status di sini + catat di `CHANGELOG.md`, lalu
   hapus/arsipkan berkas rencananya.
3. Jangan menyalin roadmap ini ke `README.md` atau berkas panduan.
