# Rencana Fitur: modul server HTTP (routing + middleware)

- **Status:** selesai
- **Target versi:** 1.4.0
- **Pemilik:** Teguh Rijanandi

## 1. Masalah

Menulis server berarti merangkai `jika` secara manual. Dari
`examples/todo_server.idk`:

```indonesiaku
jika metode == "GET" dan path == "/todos" { ... }
jika metode == "POST" dan path == "/todos" { ... }
variabel id = path_id(path)        # penguraian "/todos/<id>" ditulis tangan
jika metode == "GET" dan id != 0 - 1 { ... }
```

Tiga masalah nyata:

1. **Parameter URL ditulis tangan.** `path_id()` di contoh itu hanya bisa
   `/todos/<id>`; tiap pola baru berarti fungsi baru.
2. **Query string tidak pernah diurai.** `/cari?q=buku` membuat `path`
   menjadi `"/cari?q=buku"`, sehingga tidak cocok dengan rute mana pun —
   diam-diam 404.
3. **Nama header case-sensitive di sisi server.** `http_urai_request()`
   menyimpan nama header apa adanya, padahal RFC 9110 §5.1 menyatakan nama
   header **case-insensitive**. Klien yang mengirim `host:` tidak akan
   terbaca oleh kode yang mencari `"Host"`. Klien HTTP di berkas yang sama
   sudah huruf-kecilkan — sisi server tidak. Ini **bug**, bukan selera.

Roadmap Fase 11 langkah 3 (**WEB-01**) meminta routing metode/path,
parameter URL, query string, header, status, error handler, dan middleware.

## 2. Sintaks usulan

```indonesiaku
impor web

variabel app = web.Aplikasi()

app.rute("GET", "/todos", fungsi(req) {
    kembalikan web.json(200, "[]")
})

# Parameter URL dengan titik dua:
app.rute("GET", "/todos/:id", fungsi(req) {
    kembalikan web.teks(200, "id = " + req["params"]["id"])
})

# Middleware berjalan sebelum handler, urut pendaftaran:
app.gunakan(fungsi(req, lanjut) {
    cetak(req["metode"] + " " + req["path"])
    kembalikan lanjut(req)
})

cetak(app.tangani(mentah))     # mentah = teks request -> teks respons
```

`app.tangani()` sengaja menerima **teks** dan mengembalikan **teks**,
bukan menyentuh soket. Dengan begitu server dapat diuji tanpa jaringan, dan
gelung soket yang sudah ada di `todo_server.idk` tetap dipakai apa adanya.

## 3. Semantik

**Pencocokan rute**

- Dicocokkan berurutan sesuai pendaftaran; yang pertama cocok dipakai.
  Sederhana dan dapat diramalkan. (`ponytail:` linear, cukup untuk puluhan
  rute; pakai pohon kalau nanti ratusan.)
- Segmen `:nama` cocok dengan **satu** segmen dan masuk ke `req["params"]`.
- Jumlah segmen harus sama persis — `/a/:b` tidak cocok dengan `/a/b/c`.
- Path cocok tapi metode tidak → **405** dengan header `Allow`, bukan 404.
  Membedakan "tidak ada" dari "metode salah" sangat membantu saat debug.
- Tidak ada yang cocok → **404**.

**Query string**

- `?a=1&b=2` diurai ke `req["query"]`; `path` menjadi bersih tanpa query.
- `%20` dan `+` didekode. `%` tidak sah dibiarkan apa adanya — menolak
  seluruh permintaan karena satu karakter aneh lebih merugikan.
- Kunci berulang (`?a=1&a=2`): yang **terakhir** menang. Daftar-nilai bisa
  menyusul kalau ada kebutuhan nyata.

**Header**

- Nama header di-**huruf-kecil**-kan, menyamai perilaku klien.
- **Perubahan perilaku:** `req["header"]["Host"]` menjadi
  `req["header"]["host"]`. `examples/http_pakai.idk` ikut diperbarui.
  Ini memperbaiki bug: sebelumnya klien yang mengirim `host:` tidak terbaca.

**Middleware**

- Dipanggil `middleware(req, lanjut)`; memanggil `lanjut(req)` meneruskan,
  mengembalikan respons tanpa memanggil `lanjut` menghentikan rantai
  (berguna untuk autentikasi).
- Berjalan urut pendaftaran, sebelum handler.

**Galat**

- Handler yang menaikkan galat → **500**, dan **detail galat tidak bocor ke
  klien**; pesan aslinya dicetak ke `tulis_galat()` untuk operator.
  Membocorkan jejak tumpukan ke publik adalah kebocoran informasi.
- `app.galat(fungsi(req, e) { ... })` menimpa penanganan bawaan.

## 4. Rencana implementasi

Berkas baru `pustaka/web.idk`. **Tidak ada perubahan C.**
`pustaka/http.idk` hanya disentuh untuk perbaikan huruf-kecil header.

| Bagian | Isi |
|---|---|
| `_web_urai_query` | `?a=1&b=2` → kamus, dengan dekode persen |
| `_web_dekode_persen` | `%20`/`+` → spasi |
| `_web_cocok` | cocokkan pola `:param` dengan path |
| `kelas Aplikasi` | `rute()`, `gunakan()`, `galat()`, `tangani()` |
| `json()`, `teks()` | pembungkus tipis di atas `http_response*` |

## 5. Pengujian

- Golden `examples/web_router.idk` + `.out` — seluruhnya tanpa jaringan,
  karena `tangani()` bekerja pada teks.
- Kasus wajib: rute persis; `:param` tunggal dan ganda; jumlah segmen tidak
  sama; 404; **405 dengan `Allow`**; query diurai; `%20` dan `+`; `%` tidak
  sah; kunci query berulang; middleware meneruskan; middleware menghentikan;
  urutan middleware; handler menaikkan galat → 500 **tanpa bocor**;
  `app.galat()` kustom; header huruf-kecil.
- Integrasi: `tests/web_router_server.sh` menjalankan server sungguhan
  memakai `pustaka/web.idk` di atas gelung soket, lalu `curl` menembaknya —
  membuktikan modul ini benar-benar bisa melayani HTTP, bukan hanya lulus
  uji teks.
- Jalankan `-DDEBUG_STRESS_GC`.

## 6. Dampak dokumentasi & tooling

- [x] `website/panduan/07_PUSTAKA_STANDAR.md` — bagian modul `web`
- [x] `examples/http_pakai.idk` + golden-nya (header huruf-kecil)
- [x] `CHANGELOG.md` (termasuk perubahan perilaku) + `docs/ROADMAP.md`
      langkah 3 Fase 11 **dicentang**
- [ ] editor — tidak ada builtin VM baru
- [x] Berkas rencana ini → `selesai/`

## 7. Yang TIDAK dikerjakan

- **Parsing badan** (JSON/form/multipart) — langkah 4, branch terpisah.
- **Aset statis** — langkah 5, branch terpisah.
- **Concurrency** — langkah 6; butuh thread/proses yang belum ada di VM.
- **Wildcard `*`, rute bersarang, grup** — belum ada kebutuhan nyata.
