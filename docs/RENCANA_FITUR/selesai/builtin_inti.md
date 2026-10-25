# Rencana Fitur: Builtin inti yang masih hilang

- **Status:** selesai (sebagian — sisa menunggu tipe baru di Fase 2)
- **Target versi:** 0.7.0 (Fase 1–2)
- **Pemilik:** Teguh
- **Keputusan terkait:** 02 §D, K10, calon K17

## 1. Masalah

Dari 71 fungsi bawaan Python di `docs/ANALISIS/02_PETA_FITUR_PYTHON.md` §D,
Indonesiaku baru punya sebagian. Yang hilang bukan fungsi eksotis — ini yang
dipakai di halaman pertama tutorial Python mana pun:

```indonesiaku
variabel nilai = [80, 92, 75]
cetak(jumlah(nilai))              # jumlah() tidak terdefinisi
cetak(terurut(nilai))             # terurut() tidak terdefinisi
cetak(semua([benar, salah]))      # semua() tidak terdefinisi
untuk pasangan dalam pasangkan(nama, nilai) { }   # pasangkan() tidak terdefinisi
```

Hari ini semuanya gagal dengan `Variabel tidak terdefinisi`. Mahasiswa yang
menyalin contoh Python langsung mentok di baris pertama.

Ada dua masalah tambahan yang lebih buruk daripada sekadar "belum ada",
karena fungsinya **ada tapi jawabannya beda dari Python**:

| Ekspresi | Indonesiaku | Python | Status |
|---|---|---|---|
| `bulat(2.5)` | `3` | `2` | ❌ salah diam-diam |
| `bulat(3.5)` | `4` | `4` | ✅ kebetulan sama |
| `bulat(-2.5)` | `-3` | `-2` | ❌ salah diam-diam |
| `max([3,1,2])` | `GalatTipe` | `3` | ❌ tidak menerima iterable |

`bulat` memakai pembulatan "half away from zero", Python memakai **banker's
rounding** (half to even). Ini kategori "jawaban salah tanpa error" — persis
yang Fase 0 seharusnya hapus, tapi lolos.

## 2. Sintaks usulan

Tidak ada sintaks baru. Semua adalah fungsi biasa.

```indonesiaku
# Agregasi
cetak(jumlah([1, 2, 3]))              # 6
cetak(jumlah([1, 2], 10))             # 12 (nilai awal)
cetak(max([3, 1, 2]), min([3, 1, 2])) # 3 1   — sekarang menerima daftar
cetak(max(3, 1, 2))                   # 3     — bentuk lama tetap jalan

# Logika koleksi
cetak(semua([benar, benar]))          # benar
cetak(ada_yang([salah, benar]))       # benar
cetak(semua([]))                      # benar  (seperti Python)
cetak(ada_yang([]))                   # salah

# Urutan
cetak(terurut([3, 1, 2]))             # [1, 2, 3]
cetak(terurut([3, 1, 2], benar))      # [3, 2, 1]  (menurun)
cetak(terbalik([1, 2, 3]))            # [3, 2, 1]

# Pasangan & transformasi
cetak(pasangkan([1, 2], ["a", "b"]))  # [[1, "a"], [2, "b"]]
cetak(peta(kuadrat, [1, 2, 3]))       # [1, 4, 9]
cetak(saring(genap, [1, 2, 3, 4]))    # [2, 4]

# Angka
cetak(bulatkan(2.5), bulatkan(3.5))   # 2 4   (banker's, seperti Python)
cetak(bulatkan(3.14159, 2))           # 3.14  (digit opsional)
cetak(bagi_sisa(17, 5))               # [3, 2]
cetak(biner(10), oktal(10), heksa(255))  # 0b1010 0o12 0xff

# Tipe & objek
cetak(adalah_instansi(a, A))          # benar
cetak(bisa_dipanggil(kuadrat))        # benar
cetak(repr("halo"))                   # "halo"  (dengan kutip)
cetak(id(a))                          # identitas objek
```

## 3. Semantik

### Daftar fungsi (21 baru + 2 perbaikan)

| Nama | Python | Perilaku | Tempat |
|---|---|---|---|
| `jumlah(d, awal=0)` | `sum` | hanya angka; non-angka → `GalatTipe` | C |
| `terurut(d, menurun=salah)` | `sorted` | daftar **baru**; angka atau teks, tidak campur | C |
| `terbalik(d)` | `reversed` | daftar baru (bukan iterator — belum ada protokol) | C |
| `semua(d)` | `all` | truthiness K1; `[]` → `benar` | C |
| `ada_yang(d)` | `any` | truthiness K1; `[]` → `salah` | C |
| `pasangkan(a, b, ...)` | `zip` | daftar pasangan, panjang = yang terpendek | C |
| `peta(f, d)` | `map` | daftar baru (bukan lazy) | C |
| `saring(f, d)` | `filter` | daftar baru | C |
| `bulatkan(x, digit=0)` | `round` | **banker's rounding** | C |
| `bagi_sisa(a, b)` | `divmod` | `[hasil_bagi, sisa]`; `b == 0` → `GalatBagiNol` | C |
| `biner/oktal/heksa(n)` | `bin/oct/hex` | teks berawalan `0b`/`0o`/`0x`; wajib bulat | C |
| `adalah_instansi(o, K)` | `isinstance` | pakai `OP_IS_INSTANCE` yang sudah ada | C |
| `bisa_dipanggil(x)` | `callable` | fungsi, closure, kelas, native → `benar` | C |
| `repr(x)` | `repr` | teks diberi kutip; sisanya seperti `ke_teks` | C |
| `id(x)` | `id` | alamat heap objek; primitif → nilai hash-nya | C |
| `hash(x)` | `hash` | konsisten dengan `==` struktural (K7) | C |
| `ke_bulat(x)` | `int` | potong ke arah nol; teks `"12"` → `12` | C |
| `ke_logika(x)` | `bool` | truthiness K1 | C |
| `daftar(x)` | `list` | salin daftar / teks jadi daftar karakter | C |
| `indeks_dari(d, x)` | `.index` | `-1` bila tidak ada (bukan galat) | pustaka |
| `hitung(d, x)` | `.count` | jumlah kemunculan | pustaka |

### Dua perbaikan pada yang sudah ada

1. **`bulat` dibiarkan apa adanya, `bulatkan` yang baru memakai banker's
   rounding.** `bulat` sudah dipakai di `examples/` dan `pustaka/`;
   mengubah semantiknya diam-diam adalah breaking change yang tidak terlihat.
   `bulat` tetap "half away from zero" dan **didokumentasikan sebagai
   berbeda dari Python**, `bulatkan` adalah padanan `round` yang benar.
   Alternatif (mengubah `bulat`) dibahas di §7.
2. **`max`/`min` menerima satu daftar** selain bentuk varargs. Satu argumen
   daftar → agregasi isinya; ≥2 argumen → bandingkan argumennya. Satu
   argumen non-daftar → `GalatTipe` (Python juga menolak `max(5)`).
   Daftar kosong → `GalatNilai`, seperti Python.

### Aturan umum

- Semua menerima daftar; `rentang` dan `teks` ikut diterima di tempat yang
  masuk akal (`jumlah(rentang(5))`, `terurut("halo")`), karena keduanya sudah
  iterable lewat `OP_LIST_LEN`/`OP_INDEX_GET`.
- Semua galat memakai `nativeRaise` → **bisa ditangkap** `coba/kecuali`.
- Tidak ada yang mengubah argumennya. `terurut` dan `terbalik` mengembalikan
  daftar baru; yang in-place menyusul sebagai metode di Fase 1 (`.urutkan()`).
- `peta`/`saring` mengembalikan daftar, bukan iterator lazy — protokol
  iterator baru ada di Fase 5. Namanya sengaja sama dengan Python supaya
  tidak perlu diganti saat lazy-nya masuk.

## 4. Rencana implementasi

| Bagian | Perubahan |
|---|---|
| `src/native.c` | ±19 fungsi baru; `nativeMax`/`nativeMin` menerima daftar |
| `src/vm.c` | `defineNative` untuk tiap nama baru di `initVM` |
| `pustaka/daftar.idk` | `indeks_dari`, `hitung` (dogfooding, bukan C) |
| `src/object.h` | tidak ada tipe baru |

Tidak ada opcode, token, atau aturan parsing baru — ini murni penambahan
builtin. `adalah_instansi` memakai logika `OP_IS_INSTANCE` yang sudah dipakai
`kecuali GalatTipe`; faktorkan jadi satu helper C agar tidak ada dua
implementasi pengecekan tipe yang bisa berbeda.

**GC-safety (wajib):** `terurut`, `terbalik`, `pasangkan`, `peta`, `saring`,
dan `daftar` semuanya mengalokasi daftar hasil sambil memegang argumen.
Daftar hasil harus di-`push` ke stack VM sebelum `writeValueArray` dipanggil
berulang. `peta`/`saring` lebih berbahaya lagi: keduanya **memanggil fungsi
Indonesiaku dari dalam C**, yang bisa memicu GC kapan saja — ikuti pola
`callValue` + `run()` bersarang, dan jaga daftar hasil tetap di stack selama
seluruh loop.

## 5. Pengujian

- Golden `examples/builtin_inti.idk` + `tests/expected/builtin_inti.out`.
- Kasus batas wajib:
  - `jumlah([])` → `0`; `jumlah(["a"])` → `GalatTipe`
  - `semua([])` → `benar`, `ada_yang([])` → `salah`
  - `bulatkan(0.5)`, `bulatkan(1.5)`, `bulatkan(2.5)`, `bulatkan(-0.5)`
    → `0 2 2 0` (banker's; ini inti ujinya)
  - `bulatkan(2.675, 2)` → `2.67` (batas presisi float, sama seperti Python)
  - `max([])` → `GalatNilai`; `max(5)` → `GalatTipe`; `max(3,1,2)` → `3`
  - `terurut(["b","a"])` → urut teks; `terurut([1,"a"])` → `GalatTipe`
  - `pasangkan([1,2,3], ["a"])` → `[[1,"a"]]` (terpendek menang)
  - `bagi_sisa(-7, 2)` → `[-4, 1]` (mengikuti `//` dan `%` Python)
  - `peta` dengan fungsi yang menaikkan galat → galat merambat keluar
- Tes galat: `jumlah_tipe_salah`, `max_kosong`, `bagi_sisa_nol`,
  `terurut_campur`.
- **Wajib** `-DDEBUG_STRESS_GC` + ASan: `peta`/`saring` memanggil balik ke VM,
  ini titik paling rawan di seluruh paket.
- Bandingkan tiap hasil dengan `python3 -c` sebelum menulis golden.

## 6. Dampak dokumentasi

- [x] `website/panduan/07_PUSTAKA_STANDAR.md` — seluruh fungsi baru
- [x] `website/panduan/03_PANDUAN_SINTAKS.md` — contoh `terurut`/`pasangkan`
- [x] `README.md` + `CHANGELOG.md`
- [x] `docs/ROADMAP.md` + `docs/ANALISIS/02_PETA_FITUR_PYTHON.md` §D → ✅
- [x] `docs/ANALISIS/03_KEPUTUSAN_DESAIN.md` — K17: `bulat` vs `bulatkan`
- [x] `editors/vscode-indonesiaku/extension.js` — BUILTINS (53 → ±72)
- [x] `AGENTS.md` — jebakan: `peta`/`saring` memanggil balik ke VM

## 7. Risiko & alternatif

**Mengubah `bulat` jadi banker's rounding (ditolak).** Lebih bersih secara
semantik, tapi diam-diam mengubah hasil program yang sudah ada tanpa ada
error — persis jenis kerusakan yang paling sulit dilacak pengguna. Dua nama
dengan dokumentasi yang jujur lebih aman. Ditinjau ulang di 1.0, di mana
breaking change masih boleh (K14).

**`peta`/`saring` eager, bukan lazy.** Boros memori untuk data besar, tapi
iterator belum ada (Fase 5). Nama sudah benar, jadi perubahan nanti tidak
mengubah kode pengguna — hanya alokasinya.

**Menulis sebagian di `pustaka/` saja, bukan C.** Dicoba untuk `jumlah` dan
`terurut`: keduanya terlalu panas (dipakai di loop) dan `terurut` dalam
Indonesiaku berarti sort O(n²) atau implementasi merge sort manual. C lebih
sedikit kode total. `indeks_dari`/`hitung` tetap di `pustaka/` karena
sepele.

**Jumlah builtin global membengkak** (53 → ±72) di satu namespace datar.
Ini argumen untuk mempercepat Fase 3 (`ObjModule`), bukan alasan menunda
fungsi yang dipakai sehari-hari.
