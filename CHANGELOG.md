# Changelog - Bahasa Pemrograman Indonesiaku

Semua perubahan penting pada proyek ini akan didokumentasikan dalam file ini.

Format berdasarkan [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
dan proyek ini mengikuti [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added — Fase 2 tuntas
- ✨ **Tupel tanpa kurung** — `x = 1, 2` dan `kembalikan a, b` membentuk tupel
  tanpa tanda kurung, termasuk `y = 1,` untuk satu elemen. Hanya berlaku di
  sisi kanan penugasan dan setelah `kembalikan`; di dalam kurung mana pun koma
  tetap berarti pemisah, jadi `f(1, 2)`, `[1, 2]`, dan `{"a": 1, "b": 2}` tidak
  berubah arti.
- ✨ **`himpunan_beku([...])`** — himpunan yang tidak dapat diubah dan **dapat
  di-hash**, sehingga boleh menjadi kunci kamus atau anggota himpunan lain
  (`frozenset` Python). Operator `| & - ^` menghasilkan himpunan beku hanya
  bila kedua operand beku; `tambah`/`hapus` menolaknya dengan `GalatTipe`.
  `himpunan([1]) == himpunan_beku([1])` tetap `benar`, seperti Python.
- ✨ **`hash(x)`** — nilai hash sebuah nilai; `GalatTipe` bila tidak dapat
  di-hash.
- ✨ **`iter(urutan)` + `lanjutkan(it[, bawaan])`** — iterator eksplisit.
  Dinamai `lanjutkan` karena `lanjut` sudah menjadi kata kunci perulangan.
  Tanpa `bawaan`, iterator yang habis menaikkan `HentiIterasi` (kelas galat
  yang selama ini terdaftar tetapi belum pernah dipakai).
- ✨ **`turunan_dari(kelas, induk)`** — `issubclass`; sebuah kelas adalah
  turunan dari dirinya sendiri.
- ✨ **`ke_desimal(x)`** — `float()`; dibutuhkan sejak K2 membuat
  `ke_angka("3")` menghasilkan `bulat`.
- ✨ **`daftar([urutan])`** — `list()`; selalu salinan baru, termasuk saat
  argumennya sudah berupa daftar.
- `jenis()` kini membedakan `"himpunan_beku"` dan `"iterator"`.

### Fixed — ekstensi VS Code
- 🐛 35 builtin tidak pernah ter-highlight di tmLanguage (termasuk `rentang`,
  `terurut`, `himpunan`, `tupel`, `repr`, dan seluruh fungsi `db_*`). Daftar
  pola kini dibangkitkan dari `defineNative()` di `src/vm.c`, diurutkan dari
  nama terpanjang supaya `himpunan_beku` tidak kalah oleh `himpunan`.

### Fixed — pesan galat
- 🐛 Galat bertipe yang melewati `coba` tanpa `kecuali` yang cocok (mis.
  `GalatNama` melewati `kecuali GalatTipe`) dulu hanya tercetak
  "Kesalahan tak tertangani." — kini pesan aslinya ditampilkan
  (`tests/galat/galat_bertipe_diteruskan`).
- 🐛 Pengindeksan pada nilai yang tidak berindeks kini `GalatTipe` (dulu kelas
  umum), dengan pesan yang menyebut daftar/tupel/teks/kamus.

### Fixed — compiler hang (ditemukan fuzzer CI)
- 🐛 `akhirnya` bersarang yang berisi `kembalikan`/`hentikan`/`lanjut` membuat
  compiler menyalin blok luar secara eksponensial: kedalaman 6 butuh 24 detik,
  input fuzzer CI menggantung > 5 menit di bawah ASan. Salinan kini dibatasi
  64 per fungsi; melewatinya adalah satu galat kompilasi yang jelas
  (`tests/galat/akhirnya_salinan_berlebih`), tanpa rentetan galat susulan.

### Fixed — penggabungan Fase 1 + Fase 2
- 🐛 List comprehension dengan filter (`[x untuk x dalam a jika syarat]`)
  gagal kompilasi setelah ekspresi kondisional (`a jika c selain b`) masuk:
  `jika` pada filter terbaca sebagai awal ekspresi kondisional. Urutan dan
  filter kini diurai seperti `or_test` di Python; ekspresi kondisional di
  urutan perlu tanda kurung.
- 🐛 Berkas sumber ber-CRLF (checkout git di Windows) membocorkan `\r` ke
  string tiga kutip, `panjang()`, dan potongan baris pada pesan galat. Sumber
  kini dinormalkan saat dibaca (`\r\n` → `\n`), seperti Python.
- 🐛 `pastikan salah, 404` menyimpan pesan `404.0`; pesan `bulat` kini tetap
  `404`.

### Changed — Fase 2 ⚠️ MERUSAK
- 🔢 **Dua tipe angka: `bulat` (int64) dan `desimal` (double)** — K2.
  Literal tanpa titik menjadi `bulat`; hasil aritmetika bulat tetap bulat dan
  **tepat** sampai 2⁶³-1 (`9007199254740993 + 1` benar, dulu salah karena
  double). Pelanggaran jangkauan menaikkan `GalatLuapan` (subkelas
  `GalatAritmetika`, bisa ditangkap) alih-alih berubah diam-diam menjadi
  double; literal yang terlalu besar adalah galat kompilasi. Bignum menyusul
  di Fase 2b.

  Perilaku yang **berubah** (semuanya mengikuti Python 3):
  - `/` selalu `desimal`: `6 / 2` sekarang `3.0`, dulu `3`.
  - `desimal` selalu menampilkan `.0`: `cetak(3.0)` sekarang `3.0`, dulu `3`.
  - `//`, `%`, `**` (pangkat ≥ 0) pada dua `bulat` menghasilkan `bulat`.
  - `1 == 1.0` → `benar`; `1 adalah 1.0` → `salah`.
  - `bulat`, `bulatkan` (tanpa `digit`), `lantai`, `atap`, `abs`, `panjang`,
    `ord`, `ke_bulat`, `ke_angka`, `jumlah` (semua bulat), `biner`/`oktal`/
    `heksa`, `acak_bulat`, `id`, `hash`, elemen `rentang()`, dan indeks
    `enumerasi()` kini menghasilkan `bulat` seperti padanan Python-nya.
  - `bulatkan(x, digit)` tetap `desimal` (seperti `round(x, n)` di Python).
  - `biner`/`oktal`/`heksa` menerima `bulat` 64-bit penuh (batas 2⁵³ hilang).
  - SQLite: kolom `INTEGER` menjadi `bulat`; batas ±2⁵³ untuk `INTEGER` hilang.
  - `pustaka/matematika.idk`: `mat_kpk` memakai `//`, jadi hasilnya `bulat`.

  - Literal `0x`/`0b`/`0o` dan `1_000` (dari Fase 1) juga menjadi `bulat`;
    sebelum K2 semuanya `double`.

  Semua golden diperiksa satu per satu; yang berubah hanya baris yang memang
  berbeda di Python (mis. `akar(16)` → `4.0`, `7.5 // 2` → `3.0`).

### Changed — Fase 2 (K8) ⚠️ MERUSAK
- 🗝️ **Kunci kamus tidak lagi hanya teks.** Teks, `bulat`, `desimal`,
  `benar`/`salah`, dan `kosong` boleh menjadi kunci; daftar/kamus/rentang
  ditolak dengan `GalatTipe`. `1` dan `1.0` adalah kunci yang sama (seperti
  Python). Ini membuat **`kamus[1]` berarti kunci `1`**, bukan "kunci pertama
  menurut urutan penyisipan" — untuk posisi pakai `kunci(kamus)[0]`.
  Iterasi `untuk k dalam kamus` tetap urutan penyisipan (opcode `OP_SEQ_GET`).

### Added — Fase 2
- 🧺 **Himpunan** (K17) lewat `himpunan([urutan])` saja, tanpa literal
  (`examples/himpunan.idk`). Anggota unik dan harus hashable (aturan K8);
  `dalam`, `panjang`, `untuk`, comprehension, `==` tanpa peduli urutan,
  `| & - ^` (himpunan baru, kedua operand harus himpunan), `.tambah`,
  `.hapus` (`GalatKunci` bila tidak ada). Seperti `set` Python, himpunan
  tidak hashable dan tidak berindeks. Dicetak `himpunan([1, 2])`.
- 📦 **Tupel** (K17) `(1, 2)`, `(1,)` (koma wajib), `()`; `(1)` tetap
  pengelompokan (`examples/tupel.idk`). Seperti Python: indeks, irisan
  (hasilnya tupel), unpacking, `untuk`, `dalam`, `+`, `*`, `==` struktural
  (`(1,) == [1]` salah), dan bisa menjadi **kunci kamus** bila elemennya
  hashable. Tidak bisa diubah: `t[i] = x`, `buang t[i]`, `tambah`/`hapus`/
  `sisip`/`hapus_pada` → `GalatTipe`. Builtin `tupel([urutan])`; metode
  hanya yang tidak mengubah. Parameter SQL `db_*` kini boleh tupel.
- 🔣 **Operator bitwise** `&`, `|`, `^`, `~`, `<<`, `>>` dan
  `&= |= ^= <<= >>=` (`examples/operator_bitwise.idk`), presedensi seperti
  Python (di atas perbandingan, di bawah `+ -`). Hanya untuk `bulat`;
  `desimal` → `GalatTipe`, geser negatif → `GalatNilai`, `<<` di luar int64 →
  `GalatLuapan`. `benar & salah` tetap logika seperti `bool` Python; `~benar`
  ditolak (pakai `tidak`). `^` adalah XOR, bukan pangkat.
- 🔁 **List comprehension** `[ekspresi untuk x dalam urutan jika syarat]`
  (`examples/komprehensi.idk`). Satu `untuk` dan satu `jika` opsional; boleh
  bersarang. Variabel loop hanya hidup di dalam comprehension, dan nama luar
  tetap terlihat.
- 🧩 **Penugasan berganda & unpacking** `a, b = 1, 2`, `a, b = b, a`,
  `x, y, z = daftar`, teks per karakter, dan `rentang()`
  (`examples/penugasan_berganda.idk`). Jumlah yang tidak cocok menimbulkan
  `GalatNilai` yang bisa ditangkap. Di dalam fungsi, setiap nama tetap menjadi
  lokal (K3), termasuk yang berada di dalam blok `jika`.
- 🗑️ **`buang` (padanan `del`)** untuk variabel global/lokal, elemen daftar,
  kunci kamus, dan atribut objek (`examples/buang.idk`). Membaca nama lokal
  yang sudah di-buang menimbulkan galat; indeks/kunci/atribut yang tidak ada
  menimbulkan `GalatIndeks`/`GalatKunci`/`GalatAtribut`.

### Added — Fase 1
- ✅ **Kolom dan potongan baris pada galat runtime** — traceback kini
  `[baris N, kolom M]` dengan baris sumber dan caret pada bingkai terdalam.
  Sumber disimpan sebagai objek GC di dalam chunk, sehingga sumber modul
  `impor` tetap tersedia saat galat terjadi.
- ✅ **Teks di dalam koleksi dicetak dengan kutip (`repr`)** — `cetak(["a"])`
  kini `["a"]`, bukan `[a]`. Aturan escape tunggal dipakai bersama oleh
  `cetak`, `ke_teks`, `format`, f-string, dan `repr`.
- ✅ **Pesan galat kompilasi dengan kolom, potongan baris, dan caret** —
  `[baris 2, kolom 11] ...` diikuti baris sumber dan `^` di bawah token.
  Baris panjang dipotong dengan `…`, tab disalin apa adanya agar caret lurus.
- ✅ **Saran "Mungkin maksudmu ...?"** untuk nama global yang salah ketik,
  memakai jarak edit terbatas (≤ 2 dan ≤ sepertiga panjang nama).
- ✅ **`cetak` menjadi fungsi biasa (K12)** — bukan lagi pernyataan/opcode.
  Mendukung `pisah=` dan `akhir=`, serta dapat diperlakukan sebagai nilai
  (disimpan, dioper, `bisa_dipanggil(cetak)`). `OP_PRINT`, `TOKEN_CETAK`, dan
  `printStatement()` dihapus. Konsekuensi seperti Python: `cetak` kini nama
  biasa yang bisa ditimpa.
- ✅ **Metode pada tipe bawaan (K10)** — `"budi".huruf_besar()`,
  `[3,1,2].terurut()`, `{"a":1}.punya("a")`, dan dapat dirangkai.
  Setiap metode memanggil native yang sudah ada (semuanya sudah menerima
  penerima sebagai argumen pertama), jadi tidak ada logika teks/daftar/kamus
  yang ditulis ulang dan fungsi bebas lama tetap berfungsi.
  Metode bawaan harus langsung dipanggil; menyimpannya sebagai nilai ditolak
  dengan pesan yang menjelaskan caranya.
- ✅ **Literal angka baru** — eksponen (`1e3`, `1.5e-3`), heksadesimal
  (`0xFF`), biner (`0b1010`), oktal (`0o17`), dan pemisah ribuan
  (`1_000_000`). Garis bawah wajib di antara digit; `1__0` dan `0b2` ditolak
  dengan pesan sendiri. Semuanya tetap `double` — bilangan bulat presisi
  penuh masih menunggu Fase 2.
- ✅ **String tiga kutip** — `"""..."""` mempertahankan baris baru apa adanya.
- ✅ **f-string** — `f"Halo {nama}"`. Didesugar menjadi `+` dan `ke_teks()`
  yang sudah ada, jadi tanpa opcode baru. Mendukung ekspresi apa pun,
  `{{`/`}}` harfiah, serta kutip di dalam `{ }` (`f"{d["a"]}"`).
  Belum ada format spec (`{x:.2f}`) — menunggu metode tipe teks.
- ✅ **Fungsi anonim** — `fungsi(x) { kembalikan x * x }` sebagai ekspresi,
  closure penuh, mendukung nilai bawaan + argumen bernama. Badannya tetap
  `{ }` dan `kembalikan`; tidak ada bentuk satu-ekspresi gaya `lambda` supaya
  cara menulis fungsi tetap satu. Di traceback muncul sebagai `<anonim>`.
- ✅ **Ekspresi kondisional** — `"lulus" jika nilai >= 60 selain "gagal"`.
  Kondisi dinilai lebih dulu dan hanya cabang terpilih yang berjalan, jadi
  `x[0] jika panjang(x) > 0 selain "kosong"` aman pada daftar kosong.
- ✅ **`pastikan` (assert)** — `pastikan kondisi` atau
  `pastikan kondisi, "pesan"`. Gagal menaikkan `GalatPenegasan`, kelas yang
  sudah terdaftar di hierarki galat sejak Fase 1 tetapi belum pernah dipakai.
  Seperti Python, **ekspresi pesan hanya dinilai saat penegasannya gagal**,
  sehingga `pastikan benar, hitung_mahal()` tidak memanggil apa pun.
  Kebenaran kondisinya memakai aturan `isFalsey` yang sama dengan `jika`.

  Tidak ada mode `-O` untuk mematikan penegasan: belum ada satu pun flag
  runtime di `indk`, dan menambahnya demi fitur ini tidak sepadan.

### Added — Fase 3
- 🗄️ **Basis data SQLite tertanam** — 10 fungsi bawaan (`db_buka`,
  `db_eksekusi`, `db_ambil_semua`, `db_ambil_satu`, `db_baris_terpengaruh`,
  `db_id_terakhir`, `db_mulai`, `db_komit`, `db_batal`, `db_tutup`).
  SQLite **3.50.4** di-*vendor* sebagai amalgamation resmi (public domain,
  SHA256 tercatat di `src/SQLITE_PROVENANCE.md`), jadi `make` tetap satu
  langkah tanpa dependensi dan tanpa menunggu FFI. Handle `OBJ_DB` menutup
  diri saat GC seperti `OBJ_FILE`/`OBJ_SOCKET`; `db_buka(":memory:")` bekerja
  di lingkungan read-only seperti playground.

  Keamanan bukan tambahan belakangan: parameter `?` ada sejak versi pertama,
  satu panggilan hanya menerima **satu** pernyataan — ditolak *sebelum*
  pernyataan pertama dijalankan, sehingga `"INSERT ...; DROP TABLE ..."`
  tidak pernah mengubah apa pun — jumlah `?` wajib sama dengan panjang
  parameter, dan hanya `?` anonim yang diterima. `BLOB` serta `INTEGER` di
  luar jangkauan tepat ±2⁵³ **bergalat** daripada merusak data diam-diam.
  Ukuran biner naik ~140 KB → ~1,2 MB.

### Fixed
- 🐛 **Use-after-free saat mengompilasi f-string** — `restoreScanner` tidak
  mengembalikan penunjuk sumber, sehingga ia tertinggal menunjuk buffer
  sementara f-string yang sudah dibebaskan. Tidak terlihat sebelumnya karena
  penunjuk itu belum pernah dibaca setelah restore.
- 🐛 **Kamus dicetak sesuai urutan penambahan**, bukan urutan tabel hash.
  Sebelumnya `cetak(d)` bisa berbeda urutan dari `kunci(d)` dan dari
  perulangan `untuk ... dalam d` pada kamus yang sama.
- 🐛 **Kompilator menggantung selamanya pada `}` yang tidak punya pasangan.**
  Berkas yang hanya berisi `}` pun membuat `indk` berputar tanpa henti
  (exit 124) alih-alih melaporkan galat. `declaration()` sengaja kembali
  tanpa mengonsumsi saat bertemu `}` (itulah cara `block()` tahu blok sudah
  habis), tetapi di tingkat atas tidak ada blok yang ditutup sehingga
  perulangan tidak pernah maju. Sekarang menghasilkan
  `'}' tidak punya '{' yang cocok.` dengan exit 65.

### Added — Fase 1
- ✨ **Beberapa klausa `kecuali` untuk satu `coba`**, dicoba berurutan seperti
  Python: `} kecuali GalatAritmetika sebagai e { … } kecuali GalatPencarian
  sebagai e { … }`. Sebelumnya hanya satu klausa yang dikompilasi dan klausa
  kedua memicu galat parser (lalu menggantung). Bila tidak ada yang cocok,
  galat dilempar ulang ke `coba` di luarnya. Berfungsi juga bersama
  `akhirnya` dan bentuk tuple `kecuali (A, B) sebagai e`. `kecuali` tanpa
  tipe menangkap semuanya sehingga wajib menjadi klausa terakhir.
- ✨ **19 fungsi bawaan baru (padanan builtin Python):**
  - Agregat & urutan: `jumlah()` (sum), `semua()` (all), `ada_yang()` (any),
    `terurut()` (sorted), `terbalik()` (reversed), `pasangkan()` (zip),
    `peta()` (map), `saring()` (filter). `peta`/`saring`/`pasangkan`/`terbalik`
    mengembalikan daftar, belum iterator malas.
  - Angka: `bulatkan()` (round, *banker's rounding*), `bagi_sisa()` (divmod),
    `biner()` / `oktal()` / `heksa()` (bin/oct/hex, int64).
  - Konversi & introspeksi: `ke_bulat()` (int), `ke_logika()` (bool),
    `adalah_instansi()` (isinstance), `bisa_dipanggil()` (callable),
    `repr()`, `id()`.
  - **Catatan:** `bulat` yang lama tetap ada dan sengaja berbeda dari
    `round()` Python; `bulatkan` adalah padanan `round()` yang benar.
- ✨ **`max()` / `min()` menerima satu argumen daftar:** `max([3, 1, 2])`
  selain bentuk dua argumen yang sudah ada.
- ✨ **Argumen bernama:** `gambar("Judul", tengah=benar)` mengirim nilai
  berdasarkan nama parameter, sehingga parameter di tengah boleh dilewati dan
  urutannya bebas. Berlaku untuk fungsi, `init`, metode, dan `super`. Argumen
  posisional wajib mendahului yang bernama; nama tak dikenal, parameter yang
  diisi dua kali, dan parameter wajib yang kosong menaikkan `GalatTipe` yang
  menyebut nama parameternya. Fungsi bawaan belum menerimanya.
- ✨ **Parameter dengan nilai bawaan:** `fungsi sapa(nama, salam="Halo") { }`,
  juga untuk `init` dan metode kelas. Nilai bawaan boleh ekspresi apa pun dan
  boleh merujuk parameter di kirinya (`fungsi f(a, b=a)`). Parameter wajib
  setelah parameter berdefault ditolak saat kompilasi. **Berbeda dari Python
  (K15):** nilai bawaan dievaluasi setiap kali fungsi dipanggil, jadi
  `fungsi f(x, daftar=[])` tidak pernah membawa isi panggilan sebelumnya.
- ✨ **Gabung dan ulang untuk teks & daftar:** `[1, 2] + [3]` menggabungkan dua
  daftar (hasil selalu objek baru); `[0] * 3`, `3 * [0]`, `"-" * 20`, dan
  `2 * "ab"` mengulang. Pengali harus bilangan bulat — nol/negatif memberi
  hasil kosong seperti Python, pecahan → `GalatTipe`. Hasil dibatasi 65535
  elemen/karakter → `GalatNilai`.
- ✨ **`rentang()`** (range): `rentang(akhir)`, `rentang(awal, akhir)`,
  `rentang(awal, akhir, langkah)`. Objek rentang malas (tidak membuat list):
  bisa diiterasi `untuk`, diukur `panjang()`, diindeks `r[i]`, dan dicetak
  `rentang(0, 5)`. Argumen bukan bilangan bulat atau langkah nol → galat.
- ✨ **Operator `dalam` / `tidak dalam`** (in / not in) untuk list (elemen),
  teks (substring), kamus (kunci), dan rentang; presedensi sama dengan
  perbandingan.
- ✨ `untuk huruf dalam "teks"` mengiterasi per karakter.
- ✨ **Rantai perbandingan dan identitas:** `1 < x < 3`, `a dalam b == c`,
  `adalah`, dan `bukan adalah`. Operand tengah dievaluasi sekali dan rantai
  berhenti saat perbandingan pertama salah.
- ✨ **`enumerasi()` dan kamus iterable:** `enumerasi(urutan, awal=0)` membuat
  pasangan `[indeks, nilai]`; `untuk k dalam kamus` mengiterasi kunci menurut
  urutan penyisipan.

- ✨ **Hierarki `Galat*` dan handler bertipe:** galat runtime/native sekarang berupa
  instansi galat dengan `.pesan`; `kecuali GalatTipe sebagai e` dan
  `kecuali (GalatTipe, GalatNilai) sebagai e` didukung. `kecuali e` lama tetap
  kompatibel dan mencetak pesannya.

- ✨ **Akhir baris sebagai pemisah pernyataan:** newline di luar `()`/`[]` kini
  mengakhiri pernyataan; newline di dalam keduanya boleh untuk pemanggilan dan
  list multiline. Titik koma tetap dapat memisahkan pernyataan pada satu baris.

- ✨ **Literal kamus, indeks negatif, dan irisan:** `{"nama": "Budi"}` (kunci
  harus teks, urutan penyisipan dipertahankan, koma akhir & multi-baris boleh),
  `a[-1]` untuk daftar/teks/rentang termasuk pada penugasan, serta irisan
  `a[awal:akhir:langkah]` untuk daftar dan teks dengan aturan Python (batas
  di-*clamp*, langkah negatif membalik, `a[:]` menyalin). `{` di posisi ekspresi
  adalah literal kamus; di posisi pernyataan tetap blok.

### Changed — Fase 1 (merusak kompatibilitas)
- ⚠️ **`==` membandingkan isi, bukan identitas (K7).** `[1, 2] == [1, 2]`,
  `{"a": 1} == {"a": 1}`, dan `rentang(3) == rentang(3)` kini bernilai
  **benar**; perbandingan daftar/kamus bersifat rekursif dan urutan penyisipan
  kamus tidak berpengaruh. Operator `dalam` ikut berubah, jadi
  `[1, 2] dalam [[1, 2]]` kini benar. Untuk membandingkan **objek yang sama**
  gunakan `adalah` / `bukan adalah` — keduanya tetap identitas. Instansi kelas,
  fungsi, berkas, dan soket tetap dibandingkan berdasarkan identitas.
- ⚠️ **Assignment di dalam fungsi membuat variabel lokal (K3)**, seperti Python.
  Dulu `x = 5` di dalam fungsi menulis ke global. Untuk menulis ke luar fungsi,
  deklarasikan `global nama` (variabel tingkat skrip) atau `nonlokal nama`
  (variabel fungsi pembungkus); keduanya menerima beberapa nama sekaligus dan
  hanya boleh di dalam fungsi. Variabel hasil assignment di dalam blok
  `jika`/`selagi`/`untuk` hidup sampai akhir fungsi. `variabel x = ...` tetap sah.
  Closure yang **mengubah** variabel tangkapannya kini wajib `nonlokal`.
- ⚠️ **Truthiness mengikuti Python (K1)**: `0`, `""`, `[]`, kamus kosong, dan
  rentang kosong kini bernilai **salah** dalam `jika`/`selagi`/`dan`/`atau`/
  `tidak` (dulu hanya `salah` dan `kosong`). `selagi antrian { ... }` dan
  `nama atau "bawaan"` kini bekerja seperti di Python.
- ⚠️ `tidak` yang muncul tepat setelah sebuah ekspresi (di baris berikutnya)
  kini dibaca sebagai awal `tidak dalam` (lihat B16).

### Fixed — CI
- 🐛 Build gagal di GCC 14 (Linux CI): `usleep` tidak dideklarasikan di bawah
  `-std=c11`; `native.c` kini mendefinisikan `_DEFAULT_SOURCE`.
- 🐛 Tes `http_pakai` gagal di Linux/macOS: keluarannya memuat `\r` mentah
  sehingga git menyimpan golden-nya sebagai biner berisi CRLF Windows. Contoh
  kini menampilkan `\r` sebagai teks.
- 🐛 Ditemukan fuzzer (ASan): blok `akhirnya` yang berisi `coba ... akhirnya`
  sendiri menimpa data blok luar saat dikompilasi ulang inline, sehingga
  `kembalikan` berikutnya menjalankan blok yang salah atau membaca memori basi.

### Fixed — Fase 0 (stabilisasi)
- 🐛 **`//` (pembagian bulat) rusak**: compiler tidak menghasilkan opcode,
  sehingga `1 + 10 // 3` menjadi `13`. Kini `OP_FLOOR_DIVIDE` membulatkan ke
  bawah seperti Python (`-7 // 2 == -4`). `//=` juga ditambahkan.
- 🐛 **Argumen bernama diam-diam salah**: `f(b=1, a=5)` dulu dikompilasi sebagai
  assignment global dan nilainya dikirim berurutan (`-4`). Kini ditolak saat
  kompilasi dengan pesan "Argumen bernama (nama=nilai) belum didukung".
- 🐛 **Batas rekursi** naik dari 64 menjadi 1000 panggilan (sama dengan Python).
  Jejak error merangkum frame yang berulang
  (`[baris sebelumnya diulang N kali lagi]`).
- 🐛 `**` kini asosiatif kanan (`2 ** 3 ** 2 == 512`) dan lebih kuat dari minus
  unary (`-2 ** 2 == -4`).
- 🐛 `%` mengikuti tanda pembagi seperti Python (`-7 % 3 == 2`).
- 🐛 Penugasan majemuk memakai seluruh ekspresi di sisi kanan: `x *= 2 + 3`
  dulu hanya mengalikan dengan `2`.
- 🐛 `tidak` kini lebih lemah dari perbandingan: `tidak 1 == 2` bernilai `benar`
  (dulu `(tidak 1) == 2`).
- 🐛 **Semua galat runtime bisa ditangkap** `coba/kecuali`: indeks di luar
  jangkauan, variabel tak terdefinisi, tipe operand salah, memanggil
  bukan-fungsi, rekursi terlalu dalam, dll. Nilai `e` adalah pesan galatnya.
  Hanya kerusakan internal VM yang tetap fatal.
- 🐛 `kembalikan`/`hentikan`/`lanjut` dari dalam `coba` kini melepas handler-nya
  (dulu handler basi bisa "menangkap" galat setelah fungsinya selesai).
- 🐛 **Bagi nol** (`/`, `//`, `%`, `0 ** -n`) kini galat "Pembagian dengan nol."
  (dulu menghasilkan `takhingga`/`nan`).
- 🐛 **Presisi cetak angka**: representasi terpendek yang tepat, seperti Python
  (`0.1 + 0.2` → `0.30000000000000004`, `2 ** 0.5` → `1.4142135623730951`).
- 🐛 **Teks Unicode**: `panjang`, `teks[i]`, `karakter`, dan `potong` menghitung
  karakter, bukan byte (`panjang("héllo") == 5`). `ord`/`chr` memakai kode
  Unicode (`ord("é") == 233`, `chr(128512)` → 😀).
- 🐛 Kata kunci `akhirnya` tidak pernah dikenali scanner (salah hitung panjang),
  sehingga terbaca sebagai nama variabel.

### Added — Fase 0
- ✨ `lewati` (pass): pernyataan kosong, termasuk `kelas Kosong { lewati }`.
- ✨ `akhirnya` (finally): `coba { } kecuali e { } akhirnya { }` dan
  `coba { } akhirnya { }`; blok akhirnya selalu jalan — termasuk saat
  `kembalikan`/`hentikan`/`lanjut` keluar dari `coba` — lalu galat yang belum
  ditangkap diteruskan. Nilai `kembalikan` dihitung sebelum akhirnya berjalan.
- ✨ `;` sebagai pemisah pernyataan dalam satu baris (`x = 1; cetak(x)`), dan
  `kembalikan` tanpa nilai sebelum `}` (`jika x { kembalikan }`).

### Removed — Fase 0
- 🧹 Token mati `INDENT`/`DEDENT`/`NEWLINE` dari scanner (sisa desain lama
  berbasis indentasi; tidak pernah dihasilkan).

### Changed — Fase 0 (merusak kompatibilitas)
- ⚠️ `cetak(a, b, c)` kini mencetak semua argumen dalam **satu baris** dipisah
  spasi (seperti Python `print`); dulu tiap argumen di baris sendiri.
  `cetak()` mencetak baris kosong.
- ⚠️ `chr()` menerima kode Unicode hingga `0x10FFFF` dan menghasilkan UTF-8;
  `chr(200)` kini `È`, bukan satu byte mentah.

### Changed
- 📁 **Pemisahan dokumentasi**: isi `docs/` dipindah ke `website/` — situs statis
  di `website/`, panduan pengguna di `website/panduan/`. Folder `docs/` kini
  khusus **perencanaan bahasa**.
- 🌐 Situs tidak lagi memakai Netlify (`netlify.toml` + `.netlify/` dihapus);
  akan di-host sendiri sebagai satu bundel Docker bersama playground.

### Added
- 🧪 **Tes galat** `tests/galat/*.idk` + `*.err`: menguji program yang *harus*
  gagal (exit ≠ 0 dan stderr persis sama). Dijalankan oleh kedua runner.
- 🧪 Tes golden `operator_aritmetika`, `rekursi_dalam`, `coba_akhirnya`,
  `galat_tertangkap`, `teks_unicode`, `cetak_angka`, dan tes galat
  `handler_basi`, `bagi_nol`, `akhirnya_teruskan`; golden `coba_akhirnya_lompat`
  dan `titik_koma`.
- 📚 `docs/ANALISIS/`: audit v0.4.0, peta fitur Python → Indonesiaku, keputusan
  desain, teknik lanjutan, dan katalog fungsi/kelas yang akan diduplikasi.
  `docs/ROADMAP.md` disusun ulang menjadi Fase 0–8 menuju 1.0.
- ⌨️ **stdin di playground** — panel Input terpisah di halaman playground;
  `POST /api/jalankan` menerima `{kode, input}` dan menyalurkan `input` sebagai
  stdin, sehingga `input("Nama: ")` membaca satu baris per pemanggilan.
- ⚙️ **API playground** — `docker/playground/server.py` menjalankan `indk` untuk
  `POST /api/jalankan` (`{kode, input}` → `{stdout, stderr, kode_keluar, ms}`)
  dengan batas 3 detik, 128 MB, 64 KB input/output, dan rate limit 60/menit per
  IP.
- 🐳 **Bundel Docker situs + playground** — satu image (nginx + `indk` + python),
  dijalankan sebagai dua layanan: `web` (publik) dan `runner` (jaringan
  internal, tanpa akses internet, read-only, non-root).
- 🎨 **Desain ulang situs** — Tailwind CSS (Play CDN) + token di
  `website/assets/tailwind-config.js`, Alpine.js untuk tema/bahasa, tipografi
  Plus Jakarta Sans + JetBrains Mono, mode terang & gelap berbasis kelas.
- 🌐 **Dua bahasa (ID/EN)** — semua teks UI di `website/assets/i18n.json`;
  pemilih bahasa tersimpan di `localStorage`, halaman Tentang & Penulis punya
  artikel penuh dalam dua bahasa.
- ⚡ **Senna.js (SPA)** — navigasi antar halaman tanpa reload lewat surface
  `#konten`, sementara setiap halaman tetap HTML utuh dengan URL sendiri
  (ramah mesin pencari). Tombol back/forward tetap bekerja.
- 🧪 **Halaman playground** — `website/playground/index.html`: editor kode +
  panel keluaran, `POST` AJAX ke `/api/jalankan` (`{ kode }` →
  `{ stdout, stderr, kode_keluar, ms }`), pintasan Ctrl+Enter. Backend API
  menyusul (halaman menampilkan status "backend tidak tersedia" bila belum ada).
- 🗺️ `docs/ROADMAP.md` sebagai satu-satunya sumber kebenaran roadmap, plus
  `docs/README.md` dan `docs/RENCANA_FITUR/TEMPLATE.md` untuk rencana per fitur.

### Fixed
- 🔗 Tautan antar-dokumen yang masih menunjuk `docs/…` (termasuk nama berkas
  yang salah sejak lama, mis. `02_PANDUAN_SINTAKS.md` → `03_PANDUAN_SINTAKS.md`).

## [0.4.0] - 2026-08-07

Rilis JSON + REST: aplikasi contoh Todo-list dalam dua bentuk — CRUD CLI
dan REST API HTTP (soket TCP), keduanya diuji di CI.

### Added
- 🌐 **Soket TCP** — tipe objek `OBJ_SOCKET` + native `soket_dengar(port)`,
  `soket_terima`, `soket_baca`, `soket_tulis`, `soket_tutup` (Winsock di
  Windows, BSD di POSIX; listen di 127.0.0.1). Auto-close saat GC.
- 🔌 **`pustaka/http.idk`** — `http_urai_request` (parse request HTTP),
  `http_response`/`http_response_json` (bangun response).
- 🚀 **Server REST HTTP** — `examples/todo_server.idk`: REST API Todo-list
  (`GET/POST/PUT/DELETE /todos`) dengan penyimpanan JSON. Diuji CI (job
  `todo-http` + `tests/test_todo_server.{sh,ps1}`).
- 📇 **JSON** — `pustaka/json.idk` (`json_teks`, `json_urai`), ditulis dalam
  Indonesiaku sendiri.
- ✅ **Todo-list JSON DB (CLI)** — `examples/todo_cli.idk` (CRUD command-line).
  Diuji CI (job `todo-crud`).
- 🔡 **Native tambahan** — `ord`, `chr`, `hapus_pada(list, i)`,
  `sisip(list, i, nilai)`.
- 🧪 **Uji sintaks dasar** — `examples/sintaks_dasar.idk` (smoke test golden).

### Fixed
- 🐛 **Regresi assignment implisit di dalam blok** — `x = 10` di dalam
  `jika`/`selagi`/`untuk` (setelah scoping v0.3.0). Kini memakai semantik
  definisi di kedalaman scope mana pun.
- 🐛 **Korupsi nilai kamus saat GC** — `OP_INDEX_SET` dict mem-pop operand
  sebelum `tableSet`; kini ditahan di stack. Ditemukan lewat parser JSON di
  bawah stress-GC.

### Changed
- `jenis()` mengembalikan `"fungsi"` untuk fungsi/closure/native/metode
  terikat, dan `"soket"` untuk soket.

## [0.3.0] - 2026-08-07

Rilis pustaka standar: fungsi bawaan untuk berkas, waktu, konversi, plus
pustaka teks/daftar/matematika/kamus yang ditulis dalam bahasa Indonesiaku
sendiri (dogfooding).

### Added
- 📚 **Pustaka standar dalam bahasa sendiri** (`pustaka/`, di-`impor`):
  `teks.idk` (pisah/gabung/cari/pangkas/balik/...), `daftar.idk`
  (peta/saring/lipat/urutkan/iris/...), `matematika.idk`
  (faktorial/fpb/kpk/prima/rata_rata/...), `kamus.idk`
  (nilai/gabung/dari_pasangan/...).
- 🔡 **Primitif & pengindeksan teks** — `karakter(teks, i)`,
  `potong(teks, mulai, jumlah)`, dan pengindeksan `teks[i]` (baca).
- 🧯 **Error native yang dapat ditangkap** — fungsi bawaan memicu kesalahan
  via `nativeRaise` yang bisa ditangkap `coba/kecuali` (dulu semua fatal).
- 🔤 **Konversi & format** — `ke_angka`, `ke_teks`, `format("... {} ...", ...)`,
  `acak_bulat(min, maks)`. `acak()` kini di-seed.
- 📄 **Berkas (File I/O)** — `buka_berkas`, `tutup`, `baca`, `baca_baris`,
  `tulis`, `baca_semua`, `tulis_semua`, `tambah_berkas`, `ada_berkas`,
  `hapus_berkas`. Tipe objek baru `OBJ_FILE` (auto-close saat GC).
- 🕐 **Waktu & Tanggal** — `waktu()`, `tanggal()` (kamus komponen),
  `format_tanggal(pola)`.
- 🧵 **Urutan escape string** — `\n \t \r \" \\ \'` kini diproses di literal
  string (scanner + compiler).
- ⌨️ **Argumen baris perintah** — variabel global `argumen` (list string);
  `indk skrip.idk arg1 arg2 ...`.
- 🖥️ **REPL multi-baris** — blok (`fungsi`/`kelas`/`jika`/...) dapat ditulis
  lintas baris; prompt lanjutan `...` sampai kurung kurawal seimbang.
- 📖 **Referensi pustaka standar** terstruktur di `docs/07_PUSTAKA_STANDAR.md`.

### Fixed
- 🐛 **Variabel bocor dari badan blok**: `variabel` yang dideklarasikan di
  dalam `jika`/`selagi`/`untuk` tidak di-scope, sehingga bocor ke variabel
  berikutnya (terutama saat `hentikan`). Kini setiap badan blok memakai
  scope tersendiri. Regressi dijaga oleh `examples/lingkup_variabel.idk`.

### Changed
- `<` dan `>` kini juga membandingkan string (via `strcmp`), memungkinkan
  pengurutan string.
- Pencetakan angka lebih presisi (15 digit, tanpa notasi ilmiah tak sengaja,
  integer tanpa `.0`).

## [0.2.0] - 2026-08-07

Rilis fitur besar: bahasa kini mencakup closure, struktur data, OOP penuh,
penanganan kesalahan, dan sistem modul — semua dengan garbage collector
mark-sweep dan cakupan pengujian golden-output.

### Added
- 🧠 **Closures & upvalues** — fungsi bersarang menangkap variabel lokal
  (gunakan `variabel` untuk lokal yang di-capture); upvalue tertutup persist
  setelah fungsi induk kembali.
- 📚 **List** — literal `[..]`, pengindeksan baca/tulis, list bersarang;
  builtin `panjang`, `tambah`, `hapus`.
- 🔑 **Kamus (dictionary)** — `kamus()`, pengindeksan kunci string; builtin
  `punya`, `kunci`, `hapus_kunci`.
- 🔁 **`untuk ... dalam`** — iterasi list (for-in).
- ⏭️ **`hentikan` / `lanjut`** — break & continue di `selagi` dan `untuk`.
- 🏛️ **Kelas & OOP** — `kelas`, `init` (konstruktor), `diri` (this), metode,
  field, pewarisan (`kelas Anak < Induk`), `super`, polimorfisme.
- 🧯 **Penanganan kesalahan** — `naikkan` (raise) dan `coba { } kecuali e { }`
  (try/catch) dengan unwinding call-stack dan nesting.
- 📦 **Sistem modul** — `impor "file.idk"` memuat file lain ke namespace global
  dengan cache anti-duplikasi.
- ♻️ **Garbage Collector mark-sweep** — koleksi otomatis berbasis ambang
  alokasi; flag `DEBUG_STRESS_GC` / `DEBUG_LOG_GC`.
- 🧰 Builtin baru: `abs`, `pangkat`, `bulat`, `lantai`, `atap`, `acak`,
  `huruf_besar`, `huruf_kecil`, `ganti`, `jenis`, `input`, `henti`, dan builtin
  list/kamus di atas.

### Fixed
- 🐛 **Build link error**: `src/native.c` tidak ada di `Makefile SOURCES`
  (dan one-liner gcc README) — menyebabkan `undefined reference` saat link.
- 🐛 **Stack underflow** pada setiap `jika`/`selagi`: `OP_JUMP_IF_FALSE`
  melakukan pop ganda. Kini tidak pop (sesuai desain clox).
- 🐛 **Operator `atau` (OR) rusak total** — salah kelas huruf di scanner
  (`case 'o'` alih-alih `'a'`) → "Variabel tidak terdefinisi 'atau'".
- 🐛 **Propagasi error**: `push`/`pop` yang gagal kini menghentikan eksekusi
  (dulu lanjut dengan stack korup).
- 🐛 **`OP_DEFINE_GLOBAL` tidak pop** → deklarasi meninggalkan nilai di stack,
  merusak slot lokal `untuk...dalam`. Deklarasi kini stack-neutral.
- 🐛 **`make clean` gagal di Windows** (`del /Q src/main.o` — `/` diartikan
  switch). Diperbaiki dengan path backslash.
- 🐛 **Makefile tanpa dependensi header** → perubahan `.h` tidak memicu
  rekompilasi (miscompile senyap). Kini `%.o: %.c $(HEADERS)`.
- 🐛 Hardening fungsi native: validasi tipe semua argumen sebelum konversi.

### Changed
- 🔒 CI (`test.yml`) tidak lagi `continue-on-error` pada langkah build & test;
  ditambah job **ASan/UBSan** (stress GC), **Valgrind**, dan **coverage (lcov)**.
- ✅ Suite pengujian beralih ke **golden-output** (bandingkan stdout + cek
  stderr/exit-code), bukan hanya exit-code. Ditambah benchmark performa.
- 📖 Dokumentasi sintaks diselaraskan ke sintaks kurung kurawal `{ }` yang
  sebenarnya di-parse (README sebelumnya salah mengajarkan gaya Python).
- `cetak` bekerja untuk semua tipe baru (list, kamus, objek).

## [0.1.1] - 2025-10-28

### Changed

- Bumped version to `0.1.1`.
- Added CLI flags `-v/--version` and `-h/--help`.
- Disabled compiler debug bytecode printing by default for release builds.
- Updated installer metadata and documentation to reference `v0.1.1`.

## [0.1.0] - 2025-10-27

### Added
- ✨ **Fitur Inti**
  - Scanner/Lexer dengan kata kunci Bahasa Indonesia
  - Parser dengan Pratt parsing untuk ekspresi
  - Compiler bytecode dari AST
  - Virtual Machine stack-based untuk eksekusi
  - REPL (Read-Eval-Print Loop) interaktif

- 📊 **Tipe Data**
  - Number (integer dan float)
  - Boolean (`benar`, `salah`)
  - Kosong (None/null)
  - String (basic, dengan quote single/double)
  - Object (internal: String, Function)

- 🔤 **Kata Kunci**
  - `benar`, `salah`, `kosong` - Literal values
  - `dan`, `atau`, `tidak` - Operator logika
  - `jika`, `jika_lain`, `selain` - Percabangan
  - `selagi` - Perulangan while
  - `fungsi` - Definisi fungsi
  - `kembalikan` - Return statement
  - `cetak` - Print/output

- ➕ **Operator**
  - Aritmatika: `+`, `-`, `*`, `/`, `%`, `**`
  - Perbandingan: `==`, `!=`, `<`, `<=`, `>`, `>=`
  - Logika: `dan`, `atau`, `tidak`
  - Unary: `-` (negasi), `tidak` (NOT)

- 🔧 **Fitur Bahasa**
  - Variabel dinamis (tanpa deklarasi tipe)
  - Fungsi dengan parameter
  - Return values dari fungsi
  - Rekursi (tail recursion dan nested recursion)
  - Scope lokal dan global
  - First-class functions

- 🏗️ **Infrastruktur**
  - Hash table untuk globals dan string interning
  - Memory management (allocator)
  - Value system dengan tagged union
  - Call frames untuk function calls
  - Stack-based VM dengan 64 frame depth

- 📝 **Dokumentasi**
  - README.md lengkap dengan tutorial
  - QUICKSTART.md untuk memulai cepat
  - SYNTAX_REFERENCE.md untuk referensi sintaks
  - Contoh program di folder examples/

- 🔨 **Build System**
  - Makefile untuk Linux/Mac/Windows
  - VS Code tasks.json untuk integrasi editor
  - .gitignore untuk version control

- 📦 **Contoh Program**
  - hello.idk - Hello World
  - variabel.idk - Variabel dan operasi aritmatika
  - percabangan.idk - If-elif-else statements
  - perulangan_selagi.idk - While loops
  - fungsi.idk - Function definitions dan calls
  - kalkulator.idk - Program kalkulator sederhana
  - fibonacci.idk - Rekursi Fibonacci
  - prima.idk - Cek bilangan prima

- 🐛 **Debug Tools**
  - Disassembler bytecode
  - Trace execution (dengan flag DEBUG_TRACE_EXECUTION)
  - Print bytecode (dengan flag DEBUG_PRINT_CODE)

### Technical Details
- **Architecture**: Bytecode VM dengan stack-based execution
- **Language**: C11 standard
- **Compiler**: GCC/Clang compatible
- **VM Opcodes**: 28 opcodes dasar
- **Max Frames**: 64 nested function calls
- **Max Stack**: 16384 values (FRAMES_MAX * 256)
- **Max Constants**: 256 per chunk
- **Max Locals**: 256 per function

### Known Limitations
- ❌ Belum ada for...in loop (gunakan while)
- ❌ Belum ada list/array operations
- ❌ Belum ada dictionary/object literals
- ❌ Belum ada string concatenation dengan +
- ❌ Belum ada break/continue statements
- ❌ Belum ada class/OOP
- ❌ Belum ada module system
- ❌ Belum ada exception handling
- ❌ Belum ada file I/O
- ❌ Error messages masih basic

### Performance Notes
- Function calls: Stack-based, relatif cepat
- Recursion: Mendukung hingga 64 level
- String interning: Otomatis untuk efisiensi memori
- Hash table: FNV-1a hash untuk globals lookup

## Development Roadmap

### v0.2 (Next)
- [ ] For...in loops dengan iterables
- [ ] List operations (create, index, append)
- [ ] Break dan continue statements
- [ ] String concatenation dengan operator +
- [ ] Compound assignment operators (+=, -=, dll)
- [ ] Better error messages dengan line numbers

### v0.3 (Future)
- [ ] Classes dan objects
- [ ] Methods dan properties
- [ ] `diri` (self) keyword
- [ ] Inheritance
- [ ] Garbage Collection (mark-sweep)
- [ ] String methods (upper, lower, split, dll)

### v0.4 (Future)
- [ ] Module system (`impor`, `dari`, `sebagai`)
- [ ] Standard library (math, string, file)
- [ ] File I/O operations
- [ ] Exception handling (`coba`, `kecuali`, `akhirnya`)
- [ ] Better REPL (multiline, history)

### v1.0 (Goal)
- [ ] Stable API
- [ ] Comprehensive standard library
- [ ] Package manager
- [ ] Documentation generator
- [ ] Debugger integration
- [ ] Performance optimizations
- [ ] Cross-platform binary releases

## Contributors
- Initial development: Proyek Indonesiaku Team

## License
Educational and open-source project.

---

**Format Tanggal**: YYYY-MM-DD
**Format Versi**: MAJOR.MINOR.PATCH
