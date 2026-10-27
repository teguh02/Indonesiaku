/* =====================================================================
   Definisi bahasa Indonesiaku untuk highlight.js.
   Dipakai oleh playground (editor) dan docs.html (blok kode panduan).

   Daftar kata kunci & builtin WAJIB sinkron dengan
   editors/vscode-indonesiaku/extension.js dan src/scanner.c.
   Lihat checklist "Feature workflow" di AGENTS.md: kata kunci baru
   harus ditambahkan di sini juga, bukan hanya di ekstensi VS Code.
   ===================================================================== */
(function () {
  "use strict";

  var KATA_KUNCI = [
    "adalah", "akhirnya", "atau", "benar", "buang", "bukan", "coba", "dalam",
    "dan", "dari", "dengan", "diri", "fungsi", "global", "hentikan", "impor",
    "jika", "jika_lain", "kecuali", "kelas", "kembalikan", "kosong", "lanjut",
    "lewati", "naikkan", "nonlokal", "pastikan", "salah", "sebagai", "selagi",
    "selain", "super", "tidak", "untuk", "variabel"
  ];

  var BAWAAN = [
    "abs", "acak", "acak_bulat", "ada_berkas", "ada_yang",
    "adalah_instansi", "akar", "atap", "baca", "baca_baris", "baca_semua",
    "bagi_sisa", "biner", "bisa_dipanggil", "buka_berkas", "bulat",
    "bulatkan", "cetak", "chr", "daftar", "db_ambil_satu", "db_ambil_semua",
    "db_baris_terpengaruh", "db_batal", "db_buka", "db_eksekusi",
    "db_id_terakhir", "db_komit", "db_mulai", "db_tutup", "enumerasi",
    "format", "format_tanggal", "ganti", "hapus", "hapus_berkas",
    "hapus_kunci", "hapus_pada", "hash", "heksa", "henti", "himpunan",
    "himpunan_beku", "huruf_besar", "huruf_kecil", "id", "input", "iter",
    "jam", "jenis", "jumlah", "kamus", "karakter", "ke_angka", "ke_bulat",
    "ke_desimal", "ke_logika", "ke_teks", "kunci", "lanjutkan", "lantai",
    "max", "min", "oktal", "ord", "pangkat", "panjang", "pasangkan", "peta",
    "potong", "punya", "rentang", "repr", "saring", "semua", "sisip",
    "soket_baca", "soket_dengar", "soket_terima", "soket_tulis",
    "soket_tutup", "tambah", "tambah_berkas", "tanggal", "terbalik",
    "terurut", "tulis", "tulis_semua", "tupel", "turunan_dari", "tutup",
    "waktu"
  ];

  function definisi(hljs) {
    return {
      name: "Indonesiaku",
      aliases: ["idk"],
      case_insensitive: false,
      keywords: {
        keyword: KATA_KUNCI.join(" "),
        built_in: BAWAAN.join(" "),
        literal: "benar salah kosong"
      },
      contains: [
        hljs.HASH_COMMENT_MODE,
        // Teks tiga kutip lebih dulu, agar tidak dipotong aturan kutip tunggal.
        { className: "string", begin: '"""', end: '"""' },
        {
          className: "string",
          variants: [
            { begin: 'f"', end: '"', illegal: "\\n",
              contains: [
                { begin: "\\\\." },
                { className: "subst", begin: "\\{", end: "\\}" }
              ] },
            { begin: '"', end: '"', illegal: "\\n",
              contains: [{ begin: "\\\\." }] }
          ]
        },
        {
          className: "number",
          variants: [
            { begin: "\\b0[xX][0-9a-fA-F_]+\\b" },
            { begin: "\\b0[bB][01_]+\\b" },
            { begin: "\\b0[oO][0-7_]+\\b" },
            { begin: "\\b\\d[\\d_]*(\\.[\\d_]+)?([eE][+-]?\\d+)?\\b" }
          ],
          relevance: 0
        },
        // Nama fungsi setelah 'fungsi' / nama kelas setelah 'kelas'.
        {
          className: "title.function",
          begin: /\bfungsi\s+/, end: /(?=\()/, excludeBegin: true,
          contains: [{ className: "title.function", begin: /[A-Za-z_][A-Za-z0-9_]*/ }]
        },
        {
          className: "title.class",
          begin: /\bkelas\s+/, end: /(?=[{<\s])/, excludeBegin: true,
          contains: [{ className: "title.class", begin: /[A-Za-z_][A-Za-z0-9_]*/ }]
        }
      ]
    };
  }

  function daftarkan() {
    if (!window.hljs || window.hljs.getLanguage("indonesiaku")) return;
    window.hljs.registerLanguage("indonesiaku", definisi);
  }

  // Dipanggil langsung (hljs sudah ada) atau ditunda sampai hljs termuat.
  window.indkDaftarkanBahasa = daftarkan;
  daftarkan();
})();
