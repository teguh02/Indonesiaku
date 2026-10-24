# Rencana: Pisahkan `website/` (dokumentasi penggunaan + playground) dari `docs/` (perencanaan bahasa)

Status: **fase 0–2 sudah dikerjakan**, playground & deploy menyusul.
Netlify sudah dihapus dari repo (`netlify.toml` + `.netlify/`).

Keputusan yang sudah diambil:

- Pemindahan `docs/` → `website/` dieksekusi sekarang (bukan sekadar usulan).
- Playground + situs dibundel sebagai **satu paket Docker** (folder `website/`
  ikut di dalam image), bukan layanan terpisah.
- Akses cukup **LAN/VPN**; tidak perlu IP publik, tunnel, atau TLS publik.

---

## 1. Analisis kondisi saat ini

### Isi `docs/` (4.967 baris, dua peran tercampur)

| File | Baris | Peran sebenarnya |
|---|---:|---|
| `README.md` | 150 | Indeks dokumentasi (hub) |
| `01_PERKENALAN.md` | 239 | Visi/misi/fitur/roadmap — **setengah perencanaan** |
| `02_INSTALASI.md` | 1.092 | Panduan pengguna |
| `03_PANDUAN_SINTAKS.md` | 916 | Referensi pengguna (**paling penting**) |
| `04_KONTRIBUSI.md` | 714 | Panduan kontributor |
| `05_CREATOR.md` | 421 | Profil penulis |
| `06_PENUTUP.md` | 663 | FAQ + penutup |
| `07_PUSTAKA_STANDAR.md` | 263 | Referensi pustaka standar |
| `index/about/author/docs.html` + `assets/site.css` | 509 | Situs statis |

### Masalah yang terlihat

1. **Dua tujuan dalam satu folder.** `docs/` sekaligus menjadi (a) dokumentasi
   produk, (b) root publikasi situs, dan nanti (c) perencanaan bahasa. Setiap
   perubahan salah satu peran menyentuh folder yang sama.
2. **Situs bergantung pada isi folder itu.** `docs/docs.html` mengambil `*.md`
   secara relatif (`fetch(file)`), dan sidebar-nya hard-code di array `DOCS`.
   Jadi pemindahan file `.md` = pemindahan situs + edit `DOCS`.
3. **Referensi silang tersebar** ke `docs/` di: `README.md` (tabel baris 9–20,
   baris 659, 689), `AGENTS.md` (2 bagian), CI `test.yml:348`
   (`test -d docs`), `CHANGELOG.md` (entri historis, biarkan),
   `compiler/setup_prompt.txt:18`, `compiler/SETUP_WINDOWS_INSTALLER.md:50`.
4. **Konten sudah mulai basi**: `README.md` masih menulis "Saat Ini (v0.2)" dan
   roadmap v0.1–v0.4, padahal `INDK_VERSION` = `0.4.0` dan semua fitur roadmap
   v0.4 sudah selesai. `01_PERKENALAN.md` kemungkinan sama. Ini justru bahan
   bagus untuk folder perencanaan yang baru.
5. **Tidak ada sumber kebenaran untuk roadmap.** Roadmap ada di `README.md`,
   `CHANGELOG.md`, `docs/01_PERKENALAN.md`, `docs/06_PENUTUP.md` — empat tempat,
   mudah saling bertentangan.
6. **Belum ada apa pun soal playground** (tidak ada HTML/JS/wasm target), dan
   repo belum punya jalur build WebAssembly.

### Temuan penting soal deployment

- `172.31.10.81` adalah **alamat privat (RFC 1918)**. Dari internet tidak bisa
  diakses; hanya dari LAN/VPN yang sama. Kalau tujuannya "orang bisa mencoba
  langsung" dari luar, perlu salah satu: IP publik, reverse proxy di host
  publik, Cloudflare Tunnel, atau Tailscale Funnel.
- Password root yang dikirim lewat chat **jangan** dimasukkan ke repo, script,
  atau CI. Sebaiknya dibuatkan SSH key khusus deploy dan password diganti.
- Karena situs kini di-host sendiri, header yang dulu diurus Netlify harus
  diatur server: `*.md` → `text/markdown; charset=utf-8`, `*.idk` →
  `text/plain; charset=utf-8` (tanpa ini `fetch()` di reader gagal).

---

## 2. Struktur target

```
website/                     # semua yang dilihat pengguna
├── index.html               # landing
├── about.html
├── author.html
├── docs.html                # reader markdown (DOCS array di sini)
├── playground.html          # BARU: editor + output
├── assets/
│   ├── site.css
│   └── playground.js        # BARU
└── panduan/                 # markdown yang dibaca docs.html
    ├── README.md
    ├── 01_PERKENALAN.md     # dirapikan: fokus "apa & kenapa", tanpa roadmap
    ├── 02_INSTALASI.md
    ├── 03_PANDUAN_SINTAKS.md
    ├── 04_KONTRIBUSI.md
    ├── 05_CREATOR.md
    ├── 06_PENUTUP.md
    └── 07_PUSTAKA_STANDAR.md

docs/                        # perencanaan bahasa (internal, bukan situs)
├── README.md                # indeks + cara pakai folder ini
├── ROADMAP.md               # SATU sumber kebenaran roadmap
├── SPESIFIKASI_BAHASA.md    # grammar, semantik, daftar kata kunci
├── KEPUTUSAN_DESAIN.md      # catatan keputusan (ADR ringkas)
└── RENCANA_FITUR/           # satu file per fitur yang direncanakan
    └── TEMPLATE.md
```

Prinsip: **`website/` = keadaan sekarang** (cara pakai), **`docs/` = masa depan**
(apa yang mau dibangun dan kenapa). Roadmap tidak lagi diduplikasi di README.

---

## 3. Fase kerja

### Fase 0 — Bersihkan Netlify ✅ (sudah dikerjakan)
- Hapus `netlify.toml` dan `.netlify/`.
- Perbaiki penyebutan Netlify di `AGENTS.md` + `knowledge.md`.
- Di sisi Netlify: hapus site `b20b3ac0-…` (dilakukan user).

### Fase 1 — Pindahkan situs & dokumen pengguna ke `website/` ✅
1. `git mv docs website` (riwayat terjaga), lalu `mkdir website/panduan` dan
   `git mv website/*.md website/panduan/`.
2. `website/docs.html`:
   - `DOCS[].file` → `"panduan/README.md"`, dst.
   - Regex `mdMatch` di `fixLinks()` perlu menerima `/`
     (`^(\w[\w/.-]*?\.md)(#.*)?$`) agar tautan antar-dokumen tetap jalan.
   - Sidebar: tambah entri "Playground".
3. `README.md`: tabel baris 9–20 → tautan `website/panduan/...`; baris 659 &
   689 → `website/panduan/04_KONTRIBUSI.md`; tambah baris "Coba langsung:
   `https://<host>/playground.html`".
4. `AGENTS.md`: tulis ulang bagian "Docs website" → "Website (`website/`)" dan
   "Docs content" → "Docs (`docs/` = perencanaan)".
5. `.github/workflows/test.yml` job `build-documentation`: `test -d docs` →
   `test -d website && test -f website/docs.html && test -d docs`.
6. `compiler/setup_prompt.txt` & `compiler/SETUP_WINDOWS_INSTALLER.md`: samakan
   diagram struktur (atau tandai sebagai dokumen installer lama).
7. Verifikasi: `python -m http.server 8000 -d website` → buka
   `http://localhost:8000/docs.html`, klik **semua** entri sidebar, pastikan
   tidak ada 404 dan embed `{{embed:...}}` termuat.

### Fase 2 — Isi `docs/` sebagai perencanaan ✅ (kerangka)
1. Tulis `docs/README.md` (indeks + aturan: setiap fitur punya file rencana).
2. Satukan roadmap dari 4 tempat ke `docs/ROADMAP.md`; `README.md` cukup
   menautkannya.
3. `SPESIFIKASI_BAHASA.md`: inventaris kata kunci nyata, operator, aturan
   scope, semantik `impor`, batasan (mis. `akhirnya` belum ada, literal kamus
   `{}` belum ada, set belum ada).
4. `RENCANA_FITUR/TEMPLATE.md`: motivasi, sintaks usulan, perubahan
   `scanner/compiler/vm/native`, rencana uji golden, dampak dokumentasi.
5. Calon isi awal: `akhirnya`, literal kamus & set, `lewati`, `dari ... impor`,
   `akhirnya`, string method, package manager, debugger.

### Fase 3 — Playground
Lihat bagian 4 (pilihan arsitektur).

### Fase 4 — Deploy ke server sendiri
Lihat bagian 5.

---

## 4. Playground: pilihan arsitektur

| | **A. WASM di browser** | **B. Sandbox di server** | **C. Hibrida** |
|---|---|---|---|
| Cara kerja | VM C dikompilasi ke WebAssembly (emscripten), jalan di tab pengguna | Browser `POST /api/jalankan`, server menjalankan `indk` | WASM dulu, server sebagai fallback |
| Perubahan repo | Target build baru + `main` dipisah dari CLI, stub `baca/berkas` & `soket_*`, `input()` diganti | Nol perubahan pada VM; cukup layanan kecil + Docker | Dua-duanya |
| Risiko | Rendering error JS, ukuran unduhan (±200–400 KB) | **Eksekusi kode sembarang di server** — wajib sandbox ketat | Sama seperti A + B |
| Skalabilitas | Gratis, tanpa beban CPU server | 1 proses per request; rawan penyalahgunaan | Baik |
| Waktu | 2–4 hari | ±1 hari | 4–5 hari |

**Keputusan: opsi B, dibundel satu image Docker dengan situs.** Satu container
berisi nginx (menyajikan `website/`) + endpoint playground yang menjalankan
`indk`. Karena itu pagar berikut wajib — tanpa ini jangan dinyalakan, karena
`indk` tidak punya sandbox sendiri (program bisa `tulis_semua`, `hapus_berkas`,
bahkan `soket_dengar`):

- jalan di container, user non-root, root filesystem read-only, `tmpfs` 1 MB
  untuk `/tmp`;
- `--network none`, `--memory 64m`, `--pids-limit 64`, `--cpus 0.5`;
- batas waktu keras 3 detik (SIGKILL), ukuran sumber maks 64 KB;
- output dipotong 64 KB; `argumen` dikosongkan;
- rate limit per IP (mis. 30 request/menit) dan antrean global;
- `pustaka/` di-bundle read-only agar `impor "pustaka/..."` tetap bekerja;
- tidak ada akses ke host secrets; container terpisah dari container situs.

API yang diusulkan (satu endpoint, cukup simple):

```http
POST /api/jalankan
Content-Type: application/json

{ "kode": "cetak(\"halo\")" }

→ 200 { "stdout": "halo\n", "stderr": "", "kode_keluar": 0, "ms": 7 }
```

Status saat ini:

- ✅ Klien selesai: `website/playground/index.html` (editor + panel keluaran,
  `POST` JSON ke `/api/jalankan`, Ctrl+Enter, penanganan error/timeout).
  Tautan "Playground" sudah ada di navbar semua halaman.
- ❌ Belum ada: endpoint API, image Docker bundel, dan integrasi nginx.

---

## 5. Deploy

Karena Netlify dihapus, deploy = salin folder statis + satu layanan playground.

```
/var/www/indonesiaku/            # hasil salin website/ (read-only untuk server)
├── index.html, docs.html, playground.html, assets/, panduan/
/etc/nginx/sites-available/indonesiaku.conf
```

Kebutuhan server: **satu** container (nginx + playground) yang menyajikan
`website/`, plus `content-type` benar untuk `.md`/`.idk`. Karena akses hanya
LAN/VPN, TLS tidak wajib.

Rancangan bundel Docker yang disepakati:

```
docker/website/Dockerfile
├── stage build : kompilasi indk (sama seperti docker/images/Dockerfile)
└── stage run   : nginx + indk + website/ + pustaka/
    ├── /usr/share/nginx/html  ← isi website/
    └── /usr/local/bin/indk    ← dipakai endpoint playground
```

Satu image, satu `docker compose up -d`; playground tidak diekspos ke host,
hanya lewat nginx (`location /api/`).

Langkah minimum:
1. Buat SSH key deploy (`ssh-copy-id`), nonaktifkan `PermitRootLogin`/password
   auth, ganti password yang sudah terekspos (password root pernah dikirim
   lewat chat — anggap bocor).
2. `rsync -az --delete website/ root@172.31.10.81:/var/www/indonesiaku/`.
3. Konfigurasi nginx + `nginx -t` + reload.
4. Untuk playground: `docker compose up -d` (image `indk` sudah ada di
   `docker/images/Dockerfile`), nginx `proxy_pass /api/`.
5. Verifikasi: `curl -I .../docs.html` (200), `curl -I .../panduan/01_....md`
   (harus `text/markdown`), playground mengembalikan stdout untuk
   `cetak("halo")`, dan container playground tidak bisa menulis ke host.

Catatan: jangan commit skrip deploy yang berisi password. Simpan target host di
variabel lokal/`~/.ssh/config`.

---

## 6. Risiko

| Risiko | Dampak | Mitigasi |
|---|---|---|
| `.md` di-`fetch()` tapi server salah content-type | Reader kosong | Set header di nginx, uji via `curl -I` |
| Tautan `README.md` & `AGENTS.md` menunjuk `docs/` lama | Dokumentasi 404 | Update di commit yang sama; uji dengan link checker |
| CI masih cek `docs/` | Job merah | Update `test.yml` di commit yang sama |
| IP privat → pengunjung luar tidak bisa akses | Playground "tidak jalan" di internet | Putuskan akses: LAN saja, tunnel, atau IP publik |
| Playground = RCE | Server bisa dikuasai | Sandbox container + timeout, atau pilih opsi WASM |
| Password root bocor | Server disusupi | Rotasi password, SSH key, matikan login root |

## 7. Urutan commit yang disarankan

1. `Chore: hapus konfigurasi Netlify` (sudah).
2. `Refactor: pindahkan situs dan panduan pengguna ke website/` + update semua
   referensi + CI.
3. `Docs: jadikan docs/ folder perencanaan bahasa` (roadmap tunggal + spec).
4. `Tambah: playground` (satu PR sendiri).
5. `Docs: perbarui AGENTS.md, knowledge.md, CHANGELOG.md`.
