# Rencana Fitur: protokol objek (metode ajaib tahap 2A)

- **Status:** selesai
- **Target versi:** 1.1.0
- **Pemilik:** Teguh Rijanandi

## 1. Masalah

Setelah K19 tahap 1, kelas buatan sudah bisa dicetak (`_teks_`), dibandingkan
(`_sama_`, `_kurang_dari_`), dan berhitung (`_tambah_`). Tapi ia masih belum
bisa berperilaku seperti tipe bawaan pada empat hal paling dasar:

```python
kelas Keranjang {
    init() { diri.isi = ["apel", "jeruk"] }
}
k = Keranjang()

cetak(panjang(k))      # GalatTipe: panjang() hanya menerima string, list, kamus, atau rentang
jika k { cetak("ada") }  # selalu benar — keranjang kosong pun dianggap benar
cetak(k["apel"])       # GalatTipe: Tipe tidak mendukung indeks
h = {k: 1}             # objek bisa jadi kunci, tapi _sama_ membuatnya ditolak tanpa jalan keluar
```

Akibatnya `panjang()` dan `jika` berbohong untuk wadah buatan sendiri, dan
kelas yang mendefinisikan `_sama_` **tidak punya cara apa pun** untuk kembali
bisa dipakai sebagai kunci kamus — K19 sengaja menolaknya karena `_hash_`
belum ada. Rencana ini yang membuka jalan itu.

## 2. Sintaks usulan

```python
kelas Keranjang {
    init(isi) { diri.isi = isi }

    _panjang_()        { kembalikan panjang(diri.isi) }
    _logika_()         { kembalikan panjang(diri.isi) > 0 }
    _ambil_(kunci)     { kembalikan diri.isi[kunci] }
    _atur_(kunci, nilai) { diri.isi[kunci] = nilai }
    _hash_()           { kembalikan hash(ke_teks(diri.isi)) }
    _sama_(lain)       { kembalikan diri.isi == lain.isi }
}

k = Keranjang(["apel", "jeruk"])
cetak(panjang(k))              # 2
jika k { cetak("ada isinya") } # ada isinya
jika bukan Keranjang([]) { cetak("kosong itu salah") }
cetak(k[0])                    # apel
k[0] = "mangga"
cetak({Keranjang([1]): "satu"})
```

Tidak ada kata kunci baru. Empat nama metode baru: `_panjang_`, `_logika_`,
`_ambil_`, `_atur_`, `_hash_`. Satu builtin baru: `hash(nilai)`.

## 3. Semantik

**`_panjang_`** — dipakai `panjang(objek)`. Harus mengembalikan `bulat` >= 0;
selain itu `GalatTipe`. `desimal` ditolak walaupun bulat nilainya (`2.0`),
sama seperti Python menolak `__len__` non-`int`.

**`_logika_`** — dipakai di mana pun kebenaran dibutuhkan: `jika`, `selagi`,
`bukan`, `dan`/`atau`, `semua()`/`sebagian()`, operator ternary. Nilai
kembaliannya diambil kebenarannya apa adanya (tidak wajib `logika`), mengikuti
aturan kebenaran Python yang sudah dipakai `isFalsey`.

Urutan mundur mengikuti Python: ada `_logika_` → pakai itu; tidak ada tapi ada
`_panjang_` → benar bila panjang != 0; tidak ada keduanya → objek selalu
benar (perilaku sekarang, kompatibel mundur).

**`_ambil_` / `_atur_`** — dipakai `objek[kunci]` dan `objek[kunci] = nilai`.
Kunci diteruskan apa adanya, tipe apa pun (angka, teks, tupel). Instance tanpa
`_ambil_` tetap `GalatTipe` seperti sekarang. `buang objek[k]` **tidak**
termasuk rencana ini (butuh `_hapus_`, ditunda).

**`_hash_`** — dipakai saat objek jadi kunci kamus/anggota himpunan. Harus
mengembalikan `bulat`; selain itu `GalatTipe`. Aturan K19 yang ada diperlunak,
bukan dibuang:

| `_sama_` | `_hash_` | hasil |
|---|---|---|
| tidak | tidak | hashable, identitas (seperti sekarang) |
| ya | tidak | **tidak** hashable (seperti sekarang — K19) |
| ya | ya | hashable memakai `_hash_` |
| tidak | ya | hashable memakai `_hash_` |

**`hash(nilai)`** — builtin baru, membuka `hashValue` yang sudah ada ke
pengguna supaya `_hash_` bisa ditulis tanpa mengarang fungsi hash sendiri.
Mengembalikan `bulat`; `GalatTipe` untuk nilai yang tidak dapat di-hash.

**Invarian yang dijaga:** objek yang `_sama_`-nya benar **wajib** punya
`_hash_` yang sama, persis kontrak Python. Bahasa tidak bisa memaksakan ini;
dilanggar berarti kamus berperilaku aneh. Dicatat di panduan, bukan dijaga
runtime.

## 4. Rencana implementasi

Mengikuti pola tahap 1 — bitmask `ObjClass.magic` + `instanceMagic()`, jadi
kelas tanpa metode ajaib tidak membayar apa pun (satu AND).

1. `src/object.h` — tambah `MAGIC_PANJANG`, `MAGIC_LOGIKA`, `MAGIC_AMBIL`,
   `MAGIC_ATUR`, `MAGIC_HASH` ke `MagicKind` (menjadi 21 slot, masih jauh di
   bawah 32 bit).
2. `src/vm.c` — tambah nama ke tabel `magicNames`.
3. `src/vm.c` `OP_INDEX_GET` / `OP_INDEX_SET` — sebelum galat tipe, coba
   `_ambil_`/`_atur_` lewat `call()` biasa supaya nilai kembaliannya mendarat
   di tempat hasil operator (pola yang sama dengan `binaryMagic`).
4. Kebenaran — `isFalsey` ada di `value.c` dan tidak boleh menjalankan kode
   pengguna; pakai pola **hook** yang sudah dipakai `instanceEqualHook`:
   `instanceTruthyHook` dipasang VM di `initVM`.
5. `panjang()` di `src/native.c` — coba `_panjang_` lewat `callFromNative`
   sebelum menolak.
6. `hashValue` di `src/value.c` — pakai hook juga (`instanceHashHook`), dan
   longgarkan aturan penolakan `_sama_` sesuai tabel di atas.
7. Builtin `hash()` baru di `src/native.c`.

## 5. Rencana uji

- Golden: `examples/oop/protokol_objek.idk` + `tests/expected/protokol_objek.out`.
- Galat: `tests/galat/protokol_objek_salah.idk` + `.err` —
  `_panjang_` mengembalikan teks, `_panjang_` negatif, `_hash_` non-bulat,
  `_sama_` tanpa `_hash_` dipakai sebagai kunci (harus tetap ditolak).
- Kasus batas wajib: `_logika_` mundur ke `_panjang_`; objek tanpa keduanya
  tetap benar; `_ambil_` dengan kunci tupel; `_ambil_` yang melempar galat di
  tengah ekspresi; `_hash_` yang melempar galat saat jadi kunci; rekursi
  `_panjang_` yang memanggil `panjang(diri)`.
- Menyentuh objek/alokasi/stack → **wajib** build sanitizer
  (`-DDEBUG_STRESS_GC -fsanitize=address,undefined`) dan jalankan seluruh
  golden + `tests/galat/` terhadap biner itu. `tests/test_all.sh` membangun
  ulang secara normal sehingga tidak memakai biner instrumentasi.

## 6. Dampak dokumentasi & tooling

- [x] `website/panduan/03_PANDUAN_SINTAKS.md`
- [x] `website/panduan/07_PUSTAKA_STANDAR.md` (builtin `hash()`)
- [x] `editors/vscode-indonesiaku/extension.js` — BUILTINS (`hash`)
- [x] `editors/vscode-indonesiaku/syntaxes/indonesiaku.tmLanguage.json`
- [x] `editors/vscode-indonesiaku/snippets/indonesiaku.code-snippets`
- [x] `README.md`
- [x] `CHANGELOG.md` + `docs/ROADMAP.md` + `docs/ANALISIS/02_PETA_FITUR_PYTHON.md`
- [x] `AGENTS.md` (jebakan: `isFalsey`/`hashValue` tidak boleh menjalankan kode pengguna langsung)
- [x] Berkas rencana ini: status → `selesai`, lalu `git mv` ke `selesai/`

## 7. Risiko & alternatif

**Kompatibilitas.** Semua perilaku baru hanya menyala bila kelas
mendefinisikan metodenya; program lama tidak berubah. Satu-satunya pelonggaran
adalah `_sama_` + `_hash_` yang tadinya mustahil — menambah yang diterima,
tidak mengurangi.

**Kode pengguna di jalur C yang dalam.** `isFalsey` dan `hashValue` dipanggil
dari dalam loop C (pencarian tabel, perbandingan koleksi). Menjalankan metode
pengguna di sana bisa memicu GC atau melempar galat di titik yang tidak siap.
Karena itu dipakai pola hook + `callFromNative` yang sudah terbukti untuk
`_sama_` di tahap 1, bukan panggilan langsung. Ini risiko terbesar rencana ini
dan alasan uji sanitizer diwajibkan.

**Hash yang berubah-ubah.** `_hash_` pengguna boleh mengembalikan nilai
berbeda untuk objek yang sama bila atributnya diubah; kamus lalu kehilangan
kuncinya. Sama persis dengan Python, dan tidak dijaga runtime — dicatat di
panduan.

**Alternatif yang ditolak:** menjadikan `panjang()` memanggil metode bernama
`panjang()` biasa (tanpa garis bawah). Ditolak karena mencampur ruang nama
metode biasa dengan protokol, dan tidak konsisten dengan `_teks_`/`_sama_`
yang sudah dirilis di 1.0.
