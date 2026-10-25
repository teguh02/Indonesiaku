const vscode = require("vscode");

const KEYWORDS = [
  "adalah", "akhirnya", "atau", "benar", "buang", "bukan", "coba", "dalam", "dan", "dari",
  "dengan", "diri", "fungsi", "global", "hentikan", "impor", "jika", "jika_lain", "kecuali",
  "kelas", "kembalikan", "kosong", "lanjut", "lewati", "naikkan", "nonlokal", "pastikan", "salah",
  "sebagai", "selagi", "selain", "super", "tidak", "untuk", "variabel"
];

const BUILTINS = [
  "abs", "acak", "acak_bulat", "ada_berkas", "ada_yang", "adalah_instansi", "akar", "atap", "baca",
  "baca_baris", "baca_semua", "bagi_sisa", "biner", "bisa_dipanggil", "buka_berkas", "bulat", "bulatkan",
  "cetak", "chr", "db_ambil_satu", "db_ambil_semua", "db_baris_terpengaruh", "db_batal", "db_buka", "db_eksekusi",
  "db_id_terakhir", "db_komit", "db_mulai", "db_tutup",
  "enumerasi", "format", "format_tanggal", "ganti", "hapus", "hapus_berkas", "hapus_kunci",
  "hapus_pada", "heksa", "henti", "himpunan", "huruf_besar", "huruf_kecil", "id", "input", "jam", "jenis", "jumlah",
  "kamus", "karakter", "ke_angka", "ke_bulat", "ke_logika", "ke_teks", "kunci", "lantai", "max", "min",
  "oktal", "ord", "pangkat", "panjang", "pasangkan", "peta", "potong", "punya", "rentang", "repr",
  "saring", "semua", "sisip", "soket_baca", "soket_dengar", "soket_terima", "soket_tulis", "soket_tutup",
  "tambah", "tambah_berkas", "tanggal", "terbalik", "terurut", "tulis", "tulis_semua", "tupel", "tutup", "waktu"
];

// Magic methods (K19): written inside `kelas`, called by the interpreter.
const MAGIC_METHODS = [
  "_bagi_", "_bagi_bulat_", "_kali_", "_kurang_dari_", "_kurang_sama_", "_kurangi_",
  "_lebih_dari_", "_lebih_sama_", "_negatif_", "_pangkat_", "_repr_", "_sama_",
  "_sisa_", "_tambah_", "_teks_", "_tidak_sama_"
];

function item(label, kind, detail, insertText) {
  const result = new vscode.CompletionItem(label, kind);
  result.detail = detail;
  result.insertText = insertText || label;
  return result;
}

function activate(context) {
  const provider = vscode.languages.registerCompletionItemProvider("indonesiaku", {
    provideCompletionItems() {
      return [
        ...KEYWORDS.map((word) => item(word, vscode.CompletionItemKind.Keyword, "kata kunci Indonesiaku")),
        ...BUILTINS.map((name) => item(name, vscode.CompletionItemKind.Function, "fungsi bawaan Indonesiaku", `${name}($0)`)),
        ...MAGIC_METHODS.map((name) => item(name, vscode.CompletionItemKind.Method, "metode ajaib Indonesiaku (K19)"))
      ];
    }
  });

  context.subscriptions.push(provider);
}

function deactivate() {}

module.exports = { activate, deactivate };
