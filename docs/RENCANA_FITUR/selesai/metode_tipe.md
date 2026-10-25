# Rencana Fitur: Metode pada tipe bawaan

- **Status:** selesai
- **Target versi:** 0.7.0 (Fase 1)
- **Pemilik:** Teguh
- **Keputusan terkait:** K10 (sudah menetapkan desainnya)

## 1. Masalah

Operasi pada teks/daftar/kamus hanya tersedia sebagai fungsi bebas:

```indonesiaku
cetak(huruf_besar("budi"))
tambah(daftar, 4)
cetak(punya(kamus, "a"))
```

Gaya ini membaca terbalik dibanding Python dan membuat rantai operasi jadi
bersarang ke dalam:

```indonesiaku
cetak(huruf_besar(ganti(potong(teks, 0, 5), "a", "b")))
```

## 2. Yang sudah ada (dan tidak perlu ditulis ulang)

Pemeriksaan sebelum menulis kode: **seluruh native yang relevan sudah
menerima penerimanya sebagai argumen pertama.**

| Native | Tanda tangan |
|---|---|
| `huruf_besar(teks)` | penerima pertama |
| `ganti(teks, lama, baru)` | penerima pertama |
| `potong(teks, mulai, jumlah)` | penerima pertama |
| `tambah(daftar, nilai)` | penerima pertama |
| `sisip(daftar, indeks, nilai)` | penerima pertama |
| `punya(kamus, kunci)` | penerima pertama |

Sementara itu tata letak stack saat `OP_INVOKE` adalah persis
`penerima, arg1, arg2, ...`.

Artinya `"budi".huruf_besar()` dapat memanggil **native `huruf_besar` yang
sudah ada** dengan `argCount + 1` dan penunjuk ke slot penerima — tanpa satu
baris pun logika teks/daftar/kamus ditulis ulang. Yang dibutuhkan hanya tabel
pemetaan nama → native per tipe.

`peta(fungsi, iterable)` dan `saring(fungsi, iterable)` **tidak** ikut, karena
penerimanya bukan argumen pertama. Memaksanya masuk berarti menulis pembungkus
baru; ditunda sampai ada kebutuhan nyata.

## 3. Sintaks & semantik

```indonesiaku
cetak("budi".huruf_besar())               # BUDI
cetak("a,b,c".ganti(",", "-"))            # a-b-c
daftar.tambah(4)
cetak(kamus.punya("a"))
cetak([3, 1, 2].terurut())                # [1, 2, 3]
```

- Fungsi bebas **tetap ada** (K10): `huruf_besar(x)` dan `x.huruf_besar()`
  keduanya sah dan memanggil native yang sama. Tidak ada yang rusak.
- Metode mengubah di tempat tetap mengubah di tempat: `daftar.tambah(4)`
  sama persis dengan `tambah(daftar, 4)`.
- Galat tetap lewat jalur native yang ada, jadi `coba`/`kecuali` bekerja.
- Nama metode yang tidak dikenal menghasilkan pesan yang menyebut tipenya,
  bukan "Hanya objek yang memiliki metode."

**Batas yang diambil sadar:** hanya bentuk **panggilan** `x.metode()` yang
didukung. Mengambil metode sebagai nilai (`variabel f = "a".huruf_besar`)
ditolak dengan pesan jelas, karena `ObjBoundMethod` saat ini hanya dapat
memuat `ObjClosure`, bukan native. Menambah varian objek baru demi itu tidak
sepadan sekarang — ditandai `ponytail:` di kode.

## 4. Rencana implementasi

| Bagian | Perubahan |
|---|---|
| `src/vm.c` | tabel `NativeMethod` untuk teks, daftar, kamus |
| `src/vm.c` | `invoke()` mencari tabel saat penerima bukan instansi |
| `src/vm.c` | `OP_GET_PROPERTY` memberi pesan khusus untuk tipe bawaan |

Tidak ada opcode baru, tidak ada token baru, tidak ada perubahan compiler —
`dot()` sudah memancarkan `OP_INVOKE` untuk `x.m()`.

Pemanggilannya meniru cabang `OBJ_NATIVE` di `callValue()` yang sudah ada:

```text
Value* slot = vm.stackTop - argCount - 1;   // penerima
Value hasil = fn(argCount + 1, slot);
vm.stackTop = slot;
push(hasil);
```

`resolveNativeError()` sudah dipanggil oleh penangan `OP_INVOKE`, jadi galat
native tetap menjadi galat bahasa yang bisa ditangkap.

Pencarian memakai perbandingan linear atas tabel kecil (±12 entri per tipe).
Ditandai `ponytail:` — bila kelak terasa lambat, naikkan ke tabel hash yang
sudah ada di `src/table.c`.

## 5. Metode yang dipetakan

**teks:** `panjang`, `huruf_besar`, `huruf_kecil`, `ganti`, `potong`,
`karakter`, `format`, `ord`

**daftar:** `panjang`, `tambah`, `hapus`, `hapus_pada`, `sisip`, `terurut`,
`terbalik`, `jumlah`, `semua`, `ada_yang`, `enumerasi`

**kamus:** `panjang`, `punya`, `hapus_kunci`, `kunci`, `enumerasi`

## 6. Pengujian

Golden `examples/metode_tipe.idk` + expected:
- tiap metode di atas, dibandingkan hasilnya dengan bentuk fungsi bebas
- perubahan di tempat terbukti (`daftar.tambah` mengubah daftar aslinya)
- rantai metode: `"a,b".ganti(",", "-").huruf_besar()`
- metode pada literal langsung: `[3,1,2].terurut()`
- galat metode tertangkap `coba`/`kecuali`
- fungsi bebas lama tetap jalan (bukti tidak ada regresi)

Tes galat: metode tak dikenal pada teks, metode diambil tanpa dipanggil,
jumlah argumen salah lewat bentuk metode.

Sanitizer ASan+UBSan+`DEBUG_STRESS_GC`.

## 7. Dampak dokumentasi

- [x] `website/panduan/03_PANDUAN_SINTAKS.md` — seksi Metode pada Tipe Bawaan (4 blok diuji)
- [x] `README.md` + `CHANGELOG.md`
- [x] `docs/ROADMAP.md` — butir Fase 1 → ✅
- [x] `docs/ANALISIS/02_PETA_FITUR_PYTHON.md` — status `str`/`list` + C.1
- [x] `docs/ANALISIS/03_KEPUTUSAN_DESAIN.md` — K10 ditandai terlaksana 0.7.0
- [x] `editors/vscode-indonesiaku/` — tidak diperlukan: metode bukan kata kunci, dan completion-nya ikut `BUILTINS` yang sudah ada
- [ ] `AGENTS.md` — tertunda: berkas diproteksi Hermes, perlu persetujuan interaktif pemilik repo

## 8. Risiko

- **Penerima dinilai dua kali?** Tidak — `OP_INVOKE` menilai penerima sekali
  dan meninggalkannya di stack. Tetap diuji dengan penerima berefek samping.
- **Native yang mengubah stack.** Native dipanggil dengan penunjuk ke slot
  penerima; bila native memicu GC, penerima masih di stack sehingga ter-root.
  Wajib diuji di bawah `DEBUG_STRESS_GC`.
- **Bentrok dengan properti instansi.** Pencarian tabel hanya dijalankan
  ketika penerima **bukan** `ObjInstance`, jadi kelas buatan pengguna tidak
  terpengaruh.

## 9. Hasil (2026-10-25)

```text
tests/test_all.sh                  117/117 PASSED
ASan+UBSan+DEBUG_STRESS_GC         bersih (300 iterasi metode + rantai)
```

Catatan dari pengerjaan:

- Asumsi §2 diperiksa lebih dulu dengan memindai `IS_*(args[0])` di seluruh
  native terkait: **semuanya** memakai `args[0]` sebagai penerima. Karena itu
  tabel pemetaan cukup, tanpa pembungkus.
- `vm.galatAtribut` ternyata belum ada meski kelas `GalatAtribut` sudah
  didaftarkan; ditambahkan seperti `vm.galatPenegasan`.
- Penerima terbukti dinilai **sekali** per panggilan (diuji dengan fungsi
  berefek samping), jadi `sekali().terurut()` aman.
- Kelas buatan pengguna tidak terpengaruh: pencarian tabel hanya berjalan
  ketika penerima bukan `ObjInstance`.
