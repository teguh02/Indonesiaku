# Indonesiaku for Visual Studio Code

Dukungan dasar untuk berkas `.idk`:

- syntax highlighting untuk komentar, string, angka, operator, kata kunci, dan fungsi bawaan;
- auto-closing braces, brackets, parentheses, dan tanda kutip;
- snippets untuk fungsi, kondisi, loop, kelas, impor, dan `coba/kecuali`;
- autocomplete untuk kata kunci dan fungsi bawaan yang terdaftar di `src/vm.c`;
- **Format Document** yang memanggil `indk rapikan` (Fase 7).

## Formatter

`Format Document` (`Shift+Alt+F`) menjalankan `indk rapikan <berkas>` dan
mengganti isi editor dengan hasilnya. Interpreter dipanggil **tanpa**
`--tulis`, jadi berkas di disk tidak disentuh — VS Code sendiri yang
menerapkan edit, sehingga undo tetap berfungsi.

Berkas yang belum disimpan (`untitled` atau masih kotor) dilewati karena
`indk rapikan` membaca dari disk.

Jika `indk` tidak ada di `PATH`, atur jalurnya:

```json
{ "indonesiaku.jalurIndk": "/jalur/ke/indk" }
```

Uji formatter tanpa membuka editor:

```bash
node editors/vscode-indonesiaku/test-format.js
```

## Instalasi lokal

```bash
cd editors/vscode-indonesiaku
npm run package
code --install-extension indonesiaku-0.1.0.vsix
```

Atau buka folder ini di VS Code lalu tekan `F5` untuk menjalankan Extension Development Host.

## Cakupan

Extension ini bersifat deklaratif, ditambah provider completion ringan dan formatter yang mendelegasikan ke `indk rapikan`. Ia belum mencakup diagnostics, go-to-definition, atau language server; fitur tersebut memerlukan parser/editor protocol tersendiri.
