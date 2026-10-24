# Indonesiaku for Visual Studio Code

Dukungan dasar untuk berkas `.idk`:

- syntax highlighting untuk komentar, string, angka, operator, kata kunci, dan fungsi bawaan;
- auto-closing braces, brackets, parentheses, dan tanda kutip;
- snippets untuk fungsi, kondisi, loop, kelas, impor, dan `coba/kecuali`;
- autocomplete untuk kata kunci dan fungsi bawaan yang terdaftar di `src/vm.c`.

## Instalasi lokal

```bash
cd editors/vscode-indonesiaku
npm run package
code --install-extension indonesiaku-0.1.0.vsix
```

Atau buka folder ini di VS Code lalu tekan `F5` untuk menjalankan Extension Development Host.

## Cakupan

Extension ini bersifat deklaratif ditambah provider completion ringan. Ia belum mencakup diagnostics, go-to-definition, formatter, atau language server; fitur tersebut memerlukan parser/editor protocol tersendiri.
