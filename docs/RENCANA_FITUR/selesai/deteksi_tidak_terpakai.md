# Rencana Fitur: Deteksi variabel dan fungsi tidak terpakai (`indk periksa`)

- **Status:** selesai (tingkat 2 dirilis; tingkat 1 dan 3 sengaja tidak dikerjakan)
- **Target versi:** Fase 7 (perkakas) — dikerjakan setelah Fase 1 tuntas
- **Pengusul:** Teguh
- **Keputusan terkait:** K3 (lingkup assignment), calon K15

## 1. Masalah

Tidak ada cara mengetahui bahwa sebuah variabel atau fungsi ditulis lalu tidak
pernah dipakai. Kode sisa eksperimen menumpuk tanpa peringatan:

```idk
fungsi hitung(a, b) {
    hasil = a + b
    sementara = a * b       # tidak pernah dibaca — tidak ada tanda apa pun
    kembalikan hasil
}

fungsi tidak_dipanggil() {  # mati, tapi diam saja
    cetak("halo")
}
```

Go menolak kompilasi untuk kasus pertama (`declared and not used`). Usulan ini
mengambil ide itu, tetapi **tidak** mengambil bentuk galatnya — lihat §7.

## 2. Bentuk yang diusulkan

Tiga tingkat, diurutkan dari yang paling murah. Rencana ini memilih **tingkat 2**
sebagai target utama dan tingkat 1 sebagai bonus murah di sepanjang jalan.

### Tingkat 1 — peringatan saat menjalankan (lokal saja)

```text
$ indk program.idk
peringatan: [baris 3] variabel lokal 'sementara' tidak pernah dipakai
10
```

Keluar lewat **stderr**, exit code tetap 0, program tetap jalan.

### Tingkat 2 — perintah linter `indk periksa` (target utama)

```text
$ indk periksa program.idk
program.idk:3:5  variabel lokal 'sementara' tidak pernah dipakai
program.idk:7:1  fungsi 'tidak_dipanggil' tidak pernah dipanggil di berkas ini
2 temuan
```

Exit code: `0` bersih, `1` ada temuan. Linter yang diminta eksplisit boleh
galak karena penggunanya yang memanggil.

### Tingkat 3 — diagnostics di editor

Garis bawah kuning di VS Code. Butuh LSP yang belum ada; di luar lingkup
rencana ini, dicatat sebagai lanjutan.

## 3. Semantik

### Apa yang dilaporkan

| Jenis | Tingkat 1 | Tingkat 2 | Alasan |
|---|---|---|---|
| Variabel lokal di dalam fungsi | ✅ | ✅ | Tidak mungkin dipakai dari luar |
| Parameter fungsi | ❌ | ⚠️ opsional | Parameter tak terpakai sering wajib demi tanda tangan |
| Variabel tingkat skrip (global) | ❌ | ✅ | Bisa dipakai berkas lain lewat `impor` — hanya aman di linter satu berkas |
| Fungsi tingkat atas | ❌ | ✅ | Idem |
| Metode kelas | ❌ | ❌ | Dipanggil dinamis; terlalu banyak positif palsu |
| Atribut `diri.x` | ❌ | ❌ | Idem |

### Aturan

- "Terpakai" = namanya pernah **dibaca**. Assignment saja tidak dihitung:
  `x = 1` lalu `x = 2` tanpa pembacaan tetap tidak terpakai.
- Nama berawalan garis bawah (`_`, `_sisa`) **dikecualikan** — konvensi
  "sengaja tidak dipakai", seperti Python dan Go.
- `global x` / `nonlokal x` menandai `x` terpakai di fungsi tersebut (niat
  eksplisit).
- Variabel yang ditangkap closure (`isCaptured`) terhitung terpakai.
- `coba ... kecuali G sebagai e` dengan `e` tak terpakai: dilaporkan di
  tingkat 2 saja.
- Di **REPL** fitur ini mati total — setiap baris adalah ruang lingkupnya
  sendiri dan hampir semuanya akan terlihat tidak terpakai.

## 4. Rencana implementasi

Hook-nya sudah ada; yang kurang hanya satu bit per slot.

| Bagian | Perubahan |
|---|---|
| `src/compiler.c` | `Local` dapat `bool used`; `resolveLocal`/`resolveUpvalue` menyetel `used = true`; `endScope` melaporkan slot dengan `used == false` |
| `src/compiler.c` | `Local` juga perlu menyimpan baris & kolom deklarasi untuk pesan |
| `src/main.c` | subperintah `periksa` — kompilasi saja, jangan jalankan, cetak temuan |
| `src/vm.c` | flag `vm.lintOnly` agar `interpret()` berhenti setelah `compile()` |

Global dan fungsi tingkat atas tidak punya slot lokal, jadi tingkat 2 butuh
satu tabel tambahan di `Compiler` tingkat skrip: nama → (baris, pernah dibaca).
`OP_GET_GLOBAL` di-emit dari `namedVariable`, jadi titik catatnya tunggal.

**Catatan penting (K3):** `prescanFunctionBody()` sudah mendeklarasikan slot
lokal untuk setiap nama yang di-assign di badan fungsi. Berarti posisi token
yang tercatat adalah posisi **prescan**, bukan tempat tulis pertama. Pesan
galat harus memakai baris yang disimpan saat prescan, dan itu harus diuji —
salah di sini akan membuat seluruh pesan menunjuk baris yang membingungkan.

**Tidak ada perubahan runtime.** Fitur ini murni waktu-kompilasi, jadi tidak
ada opcode baru dan tidak ada pertimbangan GC.

## 5. Pengujian

Tingkat 1:
- `examples/` tidak dipakai (golden mensyaratkan stderr kosong). Sebagai
  gantinya: `tests/galat/` tidak cocok juga karena exit 0.
- Perlu mekanisme tes baru yang kecil: `tests/lint/<nama>.idk` +
  `<nama>.lint` berisi stderr yang diharapkan. Runner `tests/test_all.sh`
  ditambah satu loop.

Kasus wajib:
- lokal tak terpakai di fungsi; lokal terpakai hanya di `jika` bersarang;
- nama berawalan `_` tidak dilaporkan;
- variabel yang ditangkap closure tidak dilaporkan;
- `global`/`nonlokal` tidak dilaporkan;
- fungsi tingkat atas tak terpanggil (tingkat 2);
- berkas yang di-`impor` tidak membuat temuan palsu;
- `indk periksa` exit 1 saat ada temuan, 0 saat bersih;
- seluruh `examples/` dan `pustaka/` harus **bersih** menurut tingkat 1 —
  kalau tidak, perbaiki sumbernya, jangan longgarkan aturannya.

## 6. Dampak dokumentasi

- [ ] `website/panduan/03_PANDUAN_SINTAKS.md` — seksi singkat
- [ ] `website/panduan/` — halaman perkakas (`indk periksa`) kalau tingkat 2 jadi
- [ ] `README.md` + `CHANGELOG.md` + `docs/ROADMAP.md` (Fase 7)
- [ ] `AGENTS.md` — bahwa `used` wajib disetel di **semua** jalur resolve
- [ ] `editors/vscode-indonesiaku/` — tidak ada keyword baru; task `indk periksa` opsional

## 7. Risiko & alternatif

**Mengapa bukan galat seperti Go.** Tiga alasan:

1. Go menolak kompilasi hanya untuk **lokal**, bukan global dan bukan fungsi,
   justru karena keduanya bisa dipakai dari berkas lain. Kita punya `impor`
   dengan namespace global bersama: menerapkan aturan itu ke fungsi akan
   membuat seluruh `pustaka/*.idk` gagal kompilasi, karena sebuah pustaka
   memang penuh fungsi yang tidak dipakai oleh dirinya sendiri.
2. Aturan Go paling menyakitkan saat **debugging**: komentari satu `cetak(x)`
   untuk menguji sesuatu, program langsung menolak jalan. Pengguna kita adalah
   mahasiswa yang sedang belajar; menghentikan program karena variabel sisa
   eksperimen adalah hukuman di saat yang salah.
3. Compiler kita **single-pass**. Saat kita tahu sebuah nama tak terpakai,
   bytecode-nya sudah dipancarkan. Untuk lokal ini masih murah (cek di
   `endScope`), tetapi untuk global/fungsi butuh sapuan tambahan — itulah
   sebabnya tingkat 2 dipisah menjadi perintah tersendiri, bukan beban di
   setiap kali menjalankan program.

**Risiko positif palsu.** Metode kelas dan atribut sengaja tidak diperiksa.
Kalau nanti ada permintaan, itu butuh analisis alias yang jauh lebih besar
daripada nilai yang diberikannya.

**Alternatif ditolak:** menjadikannya galat dengan opsi `--izinkan-tak-terpakai`.
Ditolak karena flag yang hampir selalu dinyalakan adalah tanda aturannya
memang salah.

## 8b. Catatan pelaksanaan (diisi saat implementasi)

**Tingkat 1 dibatalkan, bukan ditunda.** Peringatan di setiap kali menjalankan
program menulis ke stderr, sedangkan seluruh tes golden mensyaratkan stderr
kosong. Memasangnya berarti menyentuh ~90 fixture demi fitur yang nilainya
sudah diberikan tingkat 2 secara opt-in. Tingkat 2 (`indk periksa`) saja.

**Yang diperiksa dipersempit ke deklarasi eksplisit.** Percobaan pertama
memeriksa semua slot lokal dan menghasilkan **30 berkas** temuan di
`examples/` + `pustaka/` — hampir semuanya positif palsu:

- parameter fungsi (`_tambah_(lain)` wajib ada demi tanda tangan),
- `kecuali ... sebagai e` yang memang tidak membaca `e`,
- variabel loop `untuk`, `dengan ... sebagai`, dan slot `super`,
- slot hasil `prescanFunctionBody` yang kembar dengan `variabel` di blok
  bersarang: pembacaan menyelesaikan ke slot dalam, sehingga salinan prescan
  tampak tak terpakai.

Sekarang hanya `variabel x` yang eksplisit yang dilinting (`lintCheck`),
sehingga temuan tinggal **2 berkas**, keduanya benar-benar mati.

**Dua berkas itu diperbaiki di sumbernya**, bukan dengan melonggarkan aturan:
`lingkup_variabel.idk` dan `metode_tipe.idk` memang sengaja menyimpan nilai
yang tak dibaca (fixture regresi scope dan probe jalur galat), jadi namanya
diberi awalan `_` sesuai konvensi di §3. Keluaran golden-nya byte-identik.

**Jejak di jalur normal: nol.** Diff `src/compiler.c` tidak menambah satu pun
pemanggilan `emitByte`/`emitBytes`/`emitJump`/`emitConstant`, jadi bytecode
untuk `indk program.idk` tidak berubah sama sekali.

**Jebakan yang ditemukan saat implementasi:**

1. `endCompiler()` **tidak** memanggil `endScope()` — lokal tingkat atas sebuah
   fungsi ikut hilang bersama `Compiler`-nya. Tanpa hook kedua di
   `endCompiler`, variabel seperti `sementara` di contoh §1 tidak pernah
   terlihat linter.
2. `compilerSetLint(false)` sempat me-reset `lintCount`, sehingga temuan
   terhapus tepat sebelum dicetak dan hasilnya selalu "0 temuan". Reset hanya
   dilakukan saat menyalakan.
3. Slot 0 setiap fungsi adalah slot penerima dengan nama kosong; harus
   dikecualikan atau ia dilaporkan sebagai variabel tak terpakai.

## 8. Urutan pengerjaan

Dikerjakan **setelah 6 butir Fase 1 tersisa selesai**. Urutan di dalam rencana:

1. Tingkat 1 (lokal, peringatan) — kecil, memakai `endScope` yang sudah ada.
2. Infrastruktur tes `tests/lint/`.
3. Tingkat 2 (`indk periksa`, global + fungsi).
4. Tingkat 3 (LSP) — hanya bila LSP sudah ada karena alasan lain.
