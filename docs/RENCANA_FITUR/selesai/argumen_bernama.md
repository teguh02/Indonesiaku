# Rencana Fitur: Argumen bernama (`f(b=2)`)

- **Status:** selesai
- **Target versi:** 0.6.0 (Fase 1)
- **Keputusan terkait:** K12, K15, calon K16

## 1. Masalah

Argumen hanya bisa dikirim menurut urutan. Fungsi dengan banyak parameter
opsional jadi sulit dibaca dan mustahil melewati satu parameter di tengah:

```idk
fungsi gambar(teks, lebar=80, garis="-", tengah=salah) { ... }

gambar("Judul", 80, "-", benar)   # apa arti 80, "-", benar?
gambar("Judul", tengah=benar)     # Argumen bernama (nama=nilai) belum didukung.
```

Ini juga prasyarat K12: `cetak("a", "b", pisah="-", akhir="")`.

## 2. Sintaks

```idk
gambar("Judul", tengah=benar)            # lewati lebar dan garis
gambar("Judul", garis="=", lebar=40)     # urutan bebas
gambar(teks="Judul")                     # parameter wajib pun boleh dinamai

kelas Kotak {
    init(lebar, tinggi=10) { ... }
}
Kotak(lebar=3)                           # init juga
```

Aturan sintaks (sama seperti Python):

- Argumen posisional harus mendahului argumen bernama.
  `f(a=1, 2)` → galat kompilasi.
- Nama argumen adalah identifier polos, bukan ekspresi: `f("x"=1)` tidak sah.

## 3. Semantik

- Nama harus cocok dengan nama parameter fungsi yang dipanggil. Tidak cocok →
  `GalatTipe`: `f() tidak punya parameter bernama 'warna'.`
- Satu parameter tidak boleh diisi dua kali (posisional + bernama, atau dua
  kali bernama) → `GalatTipe`.
- Parameter wajib yang tidak terisi (posisional maupun bernama) → `GalatTipe`
  yang menyebut nama parameternya, bukan sekadar jumlah.
- Parameter berdefault yang tidak terisi menjalankan nilai bawaannya (K15:
  tiap panggilan).
- **Fungsi native tidak menerima argumen bernama** untuk saat ini → `GalatTipe`
  dengan pesan jelas. (`cetak(pisah=)` ditangani rencana K12 tersendiri.)

## 4. Rencana implementasi

### Masalah inti: prolog bertingkat tidak bisa berlubang

Argumen bawaan (K15) memakai **prolog bertingkat**: `call()` melompat ke
`entryPoints[argCount - requiredArity]` dan kode default jatuh berurutan
sampai badan fungsi. Mekanisme itu hanya bisa mengisi **ekor** parameter.

Argumen bernama membolehkan lubang di tengah:

```idk
fungsi f(a, b=2, c=3) { }
f(1, c=9)      # b kosong, c terisi — tidak ada satu titik masuk yang benar
```

Jadi prolog bertingkat **diganti**, bukan ditambahi.

### Mekanisme pengganti: penanda + isi bersyarat

Tambah tipe nilai internal `VAL_TAKTERISI` yang tidak pernah bisa dibuat dari
kode pengguna. `call()` mengisi setiap slot parameter yang tidak diberikan
dengan penanda ini, lalu prolog memeriksa tiap parameter berdefault:

```text
; untuk tiap parameter berdefault, berurutan:
OP_ISI_BAWAAN slot, lompat   ; jika slot != penanda, lompat ke berikutnya
<kode ekspresi default>
OP_SET_LOCAL slot
OP_POP
berikutnya:
...
<badan fungsi>
```

Ini **lebih sederhana** dari yang digantikan: `ObjFunction.entryPoints` hilang
sepenuhnya (berikut pembebasannya di `freeObject`), diganti satu opcode.

Penanda tidak pernah bocor ke pengguna: parameter wajib yang kosong sudah
ditolak `call()` sebelum eksekusi, dan setiap parameter berdefault pasti
ditimpa oleh prolognya.

### Nama parameter saat runtime

Pemanggil tidak tahu fungsi mana yang dipanggil saat kompilasi (callee adalah
nilai runtime), jadi pencocokan nama terjadi di VM. `ObjFunction` menyimpan
`ObjString** paramNames` — wajib ditandai GC (`markObject` tiap nama) karena
ini `ObjString*`, berbeda dari `entryPoints` yang hanya `int`.

### Sisi pemanggil

`argumentList()` mengumpulkan argumen bernama, memancarkan nilainya setelah
yang posisional, lalu memancarkan `OP_CALL_BERNAMA argCount, konstanta` di
mana konstanta adalah `ObjList` berisi nama-nama (urutan sama dengan nilai).

| Bagian | Perubahan |
|---|---|
| `src/value.h` | `VAL_TAKTERISI` + makro `IS_TAKTERISI` / `TAKTERISI_VAL` |
| `src/object.h` / `object.c` | `ObjFunction.paramNames`; hapus `entryPoints` |
| `src/memory.c` | tandai `paramNames` di `blackenObject`, bebaskan di `freeObject`; hapus pembebasan `entryPoints` |
| `src/chunk.h` / `debug.c` | `OP_ISI_BAWAAN`, `OP_CALL_BERNAMA` |
| `src/compiler.c` | prolog bersyarat; `argumentList()` memahami `nama=nilai`; hapus pesan "belum didukung" |
| `src/vm.c` | `call()` menerima nama + mengisi penanda; handler dua opcode baru |

**GC-safety:** daftar nama di sisi pemanggil adalah konstanta chunk (sudah
ditandai lewat `markArray(&chunk.constants)`). `paramNames` butuh penandaan
baru — ini titik paling mungkin salah, dan wajib diuji dengan
`-DDEBUG_STRESS_GC`.

## 5. Pengujian

Golden `examples/argumen_bernama.idk` + `tests/expected/argumen_bernama.out`:

- bernama untuk parameter opsional, urutan dibalik;
- **melewati parameter di tengah** (`f(1, c=9)`) — kasus yang mematahkan
  prolog bertingkat;
- parameter wajib dikirim dengan nama;
- campuran posisional + bernama;
- `init` dan metode kelas;
- nilai bawaan tetap dievaluasi tiap panggilan (regresi K15);
- fungsi berdefault dioper sebagai nilai lalu dipanggil dengan nama.

Tes galat:
- `tests/galat/bernama_tidak_dikenal.idk` — nama parameter tidak ada;
- `tests/galat/bernama_ganda.idk` — satu parameter diisi dua kali;
- `tests/galat/bernama_sebelum_posisional.idk` — `f(a=1, 2)`;
- `tests/galat/bernama_wajib_kurang.idk` — parameter wajib tidak terisi.

Plus suite penuh, `-DDEBUG_STRESS_GC`, ASan+UBSan, build C11.

## 6. Dampak dokumentasi

- [ ] `website/panduan/03_PANDUAN_SINTAKS.md` (seksi Fungsi)
- [ ] `README.md` + `CHANGELOG.md`
- [ ] `docs/ROADMAP.md` + `docs/ANALISIS/02_PETA_FITUR_PYTHON.md`
- [ ] `docs/ANALISIS/03_KEPUTUSAN_DESAIN.md` — K15 diperbarui (mekanisme
      berubah, semantik tetap), K16 untuk argumen bernama
- [ ] `AGENTS.md` — prolog bertingkat sudah tidak ada; ganti dengan penanda
- [ ] `editors/vscode-indonesiaku/` — tidak ada keyword baru

## 7. Risiko & alternatif

**Risiko utama: mengganti mekanisme yang baru dikirim.** Argumen bawaan baru
selesai di commit sebelumnya. Mengganti prolog bertingkat berarti seluruh
golden `argumen_bawaan` harus tetap lulus tanpa perubahan — itu justru
jaringan pengaman terbaik yang kita punya, jadi golden itu **tidak boleh
disunting** selama pekerjaan ini.

**Alternatif ditolak:** mempertahankan prolog bertingkat dan melarang lubang
di tengah (`f(1, c=9)` ditolak). Ditolak karena justru melewati parameter di
tengah adalah alasan utama orang memakai argumen bernama.

**Alternatif ditolak:** mencocokkan nama saat kompilasi bila callee kebetulan
fungsi global yang sudah dikenal. Ditolak: hasilnya bergantung pada apakah
fungsi sudah didefinisikan di atas titik panggil — aturan yang mengejutkan dan
dua jalur kode untuk satu fitur.

**Di luar lingkup:** `*args` / `**kwargs`, argumen bernama untuk fungsi
native, dan `cetak(pisah=, akhir=)` (K12, rencana tersendiri setelah ini).
