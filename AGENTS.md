# AGENTS.md

Indonesiaku: a Python-inspired language with Indonesian keywords, implemented as a bytecode VM in C11. Architecture follows *Crafting Interpreters* (clox): scanner → single-pass Pratt compiler → chunk (bytecode) → stack VM, with a mark-sweep garbage collector. Version constant: `INDK_VERSION` in `src/common.h` (currently `0.3.0`).

## Build

- `make` → produces `indk` (Linux/macOS) or `indk.exe` (Windows). C11, `-Isrc`, links `-lm`.
- `make clean` removes objects + binary (Windows and POSIX both handled; the recipe uses backslash paths on Windows because `del` mis-parses forward slashes).
- All 11 `src/*.c` are in the Makefile `SOURCES` (including `src/native.c` — it was historically missing, causing `undefined reference to nativeFn*` at link; do not remove it).
- Objects depend on all headers (`%.o: %.c $(HEADERS)`). Editing a `.h` triggers a full recompile — important because this codebase is tightly header-coupled and stale partial builds silently miscompile. If behavior seems impossible, do a clean rebuild before debugging.
- Local dev here: no `make` on PATH; use `mingw32-make` (MSYS2 UCRT64 at `C:\msys64\ucrt64\bin`). Raw gcc one-liner also works — see README.

## Run

- `./indk file.idk` — run a script; `./indk` — REPL; `./indk -v` — version; `./indk -h` — help.

## Test

- Windows: `powershell -ExecutionPolicy Bypass -File tests/test_all.ps1`
- Linux/macOS: `bash tests/test_all.sh` (also builds first)
- **Golden-output** harness: each `tests/expected/<name>.out` is compared against the stdout of the matching `examples/**/<name>.idk`. A test fails on stdout mismatch, non-zero exit, OR any stderr. Runners discover tests by iterating `tests/expected/*.out` and finding the source recursively under `examples/`.
- Add a test by dropping `examples/foo.idk` and generating its golden: run the binary and redirect stdout only (stderr must stay empty for a passing example), e.g. `indk.exe examples/foo.idk 1>tests/expected/foo.out 2>NUL`.
- `.gitignore` ignores `*.out` globally but re-includes `tests/expected/*.out` — keep that negation or goldens vanish.
- Non-obvious: some `examples/` files are libraries meant to be `impor`-ed (e.g. `pustaka_matematika.idk`) — they define only, so their golden is an empty file. Don't add top-level `cetak` to them.

## GC / debugging the VM

- Mark-sweep GC is always on. To verify GC correctness after touching allocation/stack code, build with `-DDEBUG_STRESS_GC` (collects on every allocation) and run all examples — output must be unchanged. This is the single most valuable check for VM changes.
- Other flags in `src/common.h`: `DEBUG_LOG_GC`, `DEBUG_PRINT_CODE`, `DEBUG_TRACE_EXECUTION`.
- GC-safety rule: any C code that allocates while holding an unrooted object must keep it reachable (push it on the VM stack, or use `peek` instead of `pop`). See `makeConstant`, `concatenate`, `allocateString`, and the `kunci`/`OP_BUILD_LIST` handlers for the pattern.

## Language reality (differs from older docs)

- **Blocks use braces `{ }`, not Python indentation.** The scanner has no INDENT/DEDENT. `jika c { ... } jika_lain c2 { ... } selain { ... }`, `selagi c { ... }`, `fungsi f() { ... }`.
- Implicit assignment `x = 10` at top level defines a global. Inside a function, assignment to an unresolved name still targets a global — use `variabel x = ...` to create a function-local (required for closures to capture).
- Implemented: closures/upvalues, lists `[..]` + indexing, dicts via `kamus()` + string indexing, `untuk x dalam list { }`, `hentikan`/`lanjut`, classes (`kelas`, `init`, `diri`, `<` inheritance, `super`), `naikkan` + `coba/kecuali`, `impor "file.idk"` (shared global namespace, cached).
- Catchable vs fatal: only user `naikkan` **and native-function errors** (raised via `nativeRaise`) are caught by `kecuali`. Truly internal runtime errors (`runtimeError`: undefined var, bad opcode) print and abort (exit 70). Native builtins use `nativeRaise`, so I/O/argument failures are catchable.
- String literals process escape sequences (`\n \t \r \" \\ \'`) in the compiler's `string()` (compiler.c); the scanner skips escaped chars so `\"` doesn't end a string. Adding new escapes = edit both.
- Builtins live in `src/native.c`, registered in `initVM` (`src/vm.c`). Families: math, string, list, dict, conversion (`ke_angka`/`ke_teks`/`format`), file I/O (`buka_berkas`/`baca`/`tulis`/...), date/time (`waktu`/`tanggal`), util. `OBJ_FILE` handles auto-close on GC.
- Command-line args after the script path are exposed as the global list `argumen`.
- Standard library written **in the language itself** lives in `pustaka/` (`teks.idk`, `daftar.idk`, `matematika.idk`, `kamus.idk`, `json.idk`), imported via `impor "pustaka/..."`. Functions are prefixed (`teks_`, `daftar_`, `mat_`, `kamus_`, `json_`) because globals share one namespace. They build on the C primitives `karakter`/`potong` (string char/substring), `ord`/`chr`, and first-class functions.
- Implicit assignment `x = 10` to a global-target name uses define-semantics (create-or-update) at ANY scope depth — so it works inside `jika`/`selagi`/`untuk` bodies (which are their own scopes). Only compound assignment (`+=` etc.) requires the global to already exist.
- GC-safety when mutating collections: keep operands on the VM stack (`peek`, not `pop`) until after `tableSet`/`writeValueArray`, which can grow and trigger a GC. `OP_INDEX_SET` for dicts and the `sisip` native follow this.
- `examples/todo_cli.idk` is the Todo-list JSON DB demo (CRUD over a JSON file via `pustaka/json.idk`). Integration-tested in CI by `tests/test_todo_cli.{sh,ps1}` (job `todo-crud`) plus golden tests for `json_pakai`/`todo_demo`.
- Non-obvious VM facts: `<`/`>` compare strings via `strcmp` (not just numbers). String indexing `teks[i]` (read-only; strings immutable) shares the `OP_INDEX_GET` path with list/dict. `variabel` declared inside a `jika`/`selagi`/`untuk` body is properly scoped (each body gets `beginScope`/`endScope`); a regression once leaked such locals past the loop — see `examples/lingkup_variabel.idk`.
- The REPL (`main.c`) buffers input across lines until braces balance, so multi-line `fungsi`/`kelas`/`jika` blocks work; `braceDelta` ignores braces in strings/comments.

## CI

- `.github/workflows/test.yml`: gating jobs on Linux/macOS/Windows (build + golden tests), plus **ASan+UBSan (stress GC)**, **Valgrind**, **coverage (lcov artifact)**, and a short **libFuzzer** run (`tests/fuzz_compile.c`). `continue-on-error` was removed from build/test steps — CI red now means a real failure.
- `.github/workflows/release.yml`: on `v*.*.*` tags; now runs the test suite before publishing artifacts.

## Layout gotchas

- `compiler/` is **not** the language compiler — it's the Windows installer (Inno Setup `.iss` + assets). `dist/` holds prebuilt `Indonesiaku-Setup-*.exe`. All language implementation lives in `src/`.
- Root-level `test_*.idk` files are ad-hoc scratch scripts (git-ignored), not the test suite.
- `src/` map: `main.c` CLI/REPL · `scanner.c` lexer · `compiler.c` single-pass parser+codegen (Pratt) · `chunk.c` bytecode · `vm.c` interpreter loop + native registration + import · `native.c` builtins · `value.c`/`object.c` values & heap objects · `table.c` hash table · `memory.c` allocation + GC · `debug.c` disassembler.

## Conventions

- C style: 4-space indent, types PascalCase, constants UPPER_CASE, variables snake_case, braces same-line. `.editorconfig` enforces LF + 4-space (tabs for Makefile).
- Commit messages use Indonesian prefixes: `Tambah:` (add), `Perbaiki:` (fix), `Ubah:` (change), `Docs:`, `Refactor:`, `Test:`, `Chore:`.

## Docs

- Structured docs (Indonesian) in `docs/`; syntax reference: `docs/03_PANDUAN_SINTAKS.md`.
