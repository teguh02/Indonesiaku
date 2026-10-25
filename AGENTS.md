# AGENTS.md

Indonesiaku: a Python-inspired language with Indonesian keywords, implemented as a bytecode VM in C11. Architecture follows *Crafting Interpreters* (clox): scanner → single-pass Pratt compiler → chunk (bytecode) → stack VM, with a mark-sweep garbage collector. Version constant: `INDK_VERSION` in `src/common.h` (currently `0.4.0`).

## Identity: Python semantics, PHP-style braces (NON-NEGOTIABLE)

**Python is our reference for SEMANTICS, never for block syntax.** Copy
Python's behavior — truthiness, operator rules, scoping, error model, string
and collection semantics — but blocks are delimited by `{ }` like PHP/C, never
by indentation.

## Tata Kelola Proyek, Nama, dan Disclaimer (NON-NEGOTIABLE untuk dokumentasi publik)

Repo ini sudah memiliki identitas kebijakan, nama proyek ("Indonesiaku"), dan
versi lokal. Jangan menambahkan disclaimer, lisensi, atau pernyataan afiliasi
baru di luar file yang ada (`README.md`, `website/panduan/README.md`, dll)
tanpa menyelaraskannya dengan isi yang sudah ada dan tidak memperlakukan proyek
ini sebagai bahasa baru yang belum ada dokumentasinya.

Poin yang wajib dijaga setiap kali dokumen publik disentuh:

- **Nama proyek tetap "Indonesiaku"** (bukan "Indonesiaku Lang" atau variasi
  lain). Jika sebuah draft eksternal menyebut nama lain, sesuaikan sebelum
  masuk ke repo.
- **Jangan membuat asumsi bahwa proyek ini belum punya roadmap/versi.** Proyek
  ini sudah menempuh v0.4.0 dengan roadmap di `docs/ROADMAP.md` dan analisis
  desain di `docs/ANALISIS/`. Dokumen publik harus tetap selaras dengan status
  tersebut.
- **Jangan menimbulkan kesan afiliasi resmi** dengan pemerintah atau lembaga
  negara (lambang negara, identitas kementerian, atribut resmi). Pernyataan
  independensi boleh ditulis ulang/disederhanakan selama tetap netral dan tidak
  bertentangan dengan isi yang sudah ada.
- **Jangan copy-paste disclaimer dari luar mentah-mentahnya.** Jika ada teks
  disclaimer/identitas dari diskusi eksternal, adaptasi dulu: ganti nama proyek,
  sesuaikan konteks repo, dan pastikan tidak mengubah makna teknis atau fitur
  yang sudah tercantum di dokumen lain.
- **Update bersamaan.** Jika README atau panduan pengguna disentuh untuk bagian
  independensi/disclaimer, cek apakah bagian lain (identitas proyek, roadmap,
  daftar fitur) masih konsisten. Tidak ada dokumen publik yang boleh berdiri
  sendiri dengan klaim yang berbeda.

Singkatnya: nominal proyek, pendekatan nonkomersial, dan larangan kesan afiliasi
resmi boleh ditambahkan/ diperbarui sebagai langkah dokumentasi, tapi tetap
mengikuti gaya dan isi repo yang sudah ada — bukan menimpa atau menambah asumsi
baru tentang "status bahasa baru".

```indonesiaku
fungsi sapa(nama) {        # PHP-style braces — no ':' , no indentation rule
    jika nama == "" {
        kembalikan "Halo, dunia!"
    }
    kembalikan "Halo, " + nama
}
```

Consequences you must respect in every change:

- **There is NO indentation rule at all.** Leading whitespace is never
  significant — not for blocks, not for errors, not for the REPL. Code written
  entirely flush-left is fully valid Indonesiaku and must keep working:

  ```indonesiaku
  fungsi faktorial(n) {
  jika n > 1 {
  kembalikan n * faktorial(n - 1)
  }
  kembalikan 1
  }
  cetak(faktorial(5))        # 120 — rata kiri, tetap sah
  ```

  Inconsistent indentation (2 spaces here, 8 there, a tab in between) is
  likewise never an error. Indentation is **only** a readability convention in
  our examples and docs; the compiler must never reject or warn about it.
- The scanner has **no INDENT/DEDENT tokens and must never grow them** (K4).
  Whitespace at line start carries no meaning. Statements end at a newline
  outside `()`/`[]` (B16), but a block only ends at `}`.
- No `:` to open a block. `fungsi f() {`, not `fungsi f():`.
- `{` and `}` are *structural*. Any future Python construct that reuses braces
  (dict literals `{"a": 1}`, set literals) is a genuine grammar conflict and
  needs an explicit disambiguation rule plus a decision recorded in
  `docs/ANALISIS/03_KEPUTUSAN_DESAIN.md` — it also has to be taught to
  `prescanFunctionBody` and `skipBalancedBraces` in `compiler.c`, which both
  count raw braces.
- When a doc, example, or error message explains syntax, show braces. Never
  write Python-indented pseudo-code as if it were Indonesiaku.

This is deliberate, not a shortcut: braces survive copy-paste from WhatsApp and
the web, keep the REPL's multi-line buffering simple, and give beginners a
clearer error (`Harapkan '}'`) than tab-vs-space mistakes. See K4 in
`docs/ANALISIS/03_KEPUTUSAN_DESAIN.md`.

## Feature workflow (MANDATORY — read before writing code)

The repo is ONE unit: interpreter, tests, user docs, and editor tooling must
never drift apart. A feature is not done when it compiles; it is done when all
four are in sync.

**1. Plan in `docs/` FIRST.** Every language change — new syntax, new builtin,
changed semantics, breaking change — starts as a plan file in
`docs/RENCANA_FITUR/<nama>.md`, copied from `docs/RENCANA_FITUR/TEMPLATE.md`
(masalah, sintaks, semantik, berkas tersentuh, rencana uji). No planning,
roadmap, or design notes anywhere else: not in `README.md`, not in
`CHANGELOG.md`, not in `website/`. `docs/ROADMAP.md` is the single source of
truth for status.

**2. When the feature works, update ALL of these in the same change** — this
list is a checklist, not a suggestion. Documentation and editor tooling are
part of the feature, not follow-up work: a keyword that highlights in the
terminal but not in VS Code is a bug, and a feature absent from
`website/panduan/` does not exist for users. A PR missing any row is
incomplete, however good the C code is.

| Target | What to update |
|---|---|
| `examples/` + `tests/expected/` | one golden example exercising the feature (stdout only, stderr empty) |
| `tests/galat/` | `.idk` + `.err` pair for anything that must fail |
| `website/panduan/` | `03_PANDUAN_SINTAKS.md` for syntax, `07_PUSTAKA_STANDAR.md` for new builtins — these ARE the live site |
| `editors/vscode-indonesiaku/` | new keywords → `extension.js` KEYWORDS + `syntaxes/indonesiaku.tmLanguage.json`; new builtins → BUILTINS; new syntax worth a snippet → `snippets/indonesiaku.code-snippets` |
| `CHANGELOG.md` | entry, marked ⚠️ when it breaks compatibility |
| `docs/ROADMAP.md`, `docs/ANALISIS/02_PETA_FITUR_PYTHON.md` | flip status to ✅ |
| `README.md`, this `AGENTS.md` | user-visible syntax and new pitfalls |
| plan file | status → `selesai`, then `git mv` into `docs/RENCANA_FITUR/selesai/` |

**3. A breaking change must also fix every consumer in-repo:** `examples/`,
`pustaka/`, and the integration test scripts. Grep, don't guess — the golden
suite catches most of it, but `pustaka/*.idk` is imported by examples and fails
far from the edit.

**4. Testing is NOT optional, and "the suite passed" is not the whole bar.**
Before committing:

- `bash tests/test_all.sh` → every test green, no exceptions.
- Touched allocation, the stack, collections, closures, objects, or anything
  holding a raw pointer? Rebuild with
  `-DDEBUG_STRESS_GC -fsanitize=address,undefined` and run the goldens AND
  `tests/galat/` against that binary — `tests/test_all.sh` rebuilds normally,
  so it will NOT use your instrumented binary. Grepping stderr alone silently
  accepts crashes: assert the expected exit code and output too. Nearly every
  use-after-free in this repo's history was invisible to the normal suite.
- **Never regenerate `tests/expected/*.out` or `tests/galat/*.err` blindly.**
  When a formatting change invalidates many fixtures, read each diff, or
  normalise the new output back to the old shape and assert the remainder is
  byte-identical. Reading them is how a real bug (a newline token printed
  literally, splitting an error message in half) was caught instead of frozen
  into a golden.
- Verify the claims you write in docs by actually running the snippets. An
  example in `website/panduan/` that was never executed is a future bug report.
- A background job exiting 0 does not prove the tests ran — read its output.

**5. Only then commit**, code+tests separate from docs (see Conventions),
`git fetch` + rebase, re-test, push.

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
- **Error tests** (`tests/galat/`): for behavior that must FAIL (compile errors, uncatchable runtime errors), add `tests/galat/foo.idk` + `tests/galat/foo.err`. Passes when the exit code is non-zero and stderr equals the `.err` file exactly; stdout is ignored. Generate with `indk.exe tests/galat/foo.idk 2>tests/galat/foo.err 1>NUL`. Both runners run these after the goldens.
- A golden must never contain a raw `\r` (e.g. printing an HTTP response): Windows output then has mixed `\r\r\n`, git classifies the file as binary (`git ls-files --eol` shows `i/-text`) and stores the Windows bytes, so Linux/macOS CI fails. Print such text with `ganti(t, "\r", "\\r")` (see `examples/http_pakai.idk`). Check new fixtures with `git ls-files --eol tests/expected/<name>.out` → must be `i/lf`.
- `src/native.c` defines `_DEFAULT_SOURCE` before its includes: `usleep` is POSIX, hidden by glibc under `-std=c11`, and GCC 14 rejects implicit declarations. Any new POSIX call needs the same care.
- Line endings: on Windows `indk.exe` writes CRLF and git (`autocrlf=true`) checks fixtures out as CRLF, so locally generated `.out`/`.err` files are CRLF; git normalizes them on commit and Linux CI compares LF to LF.
- Non-obvious: some `examples/` files are libraries meant to be `impor`-ed (e.g. `pustaka_matematika.idk`) — they define only, so their golden is an empty file. Don't add top-level `cetak` to them.

## GC / debugging the VM

- Mark-sweep GC is always on. To verify GC correctness after touching allocation/stack code, build with `-DDEBUG_STRESS_GC` (collects on every allocation) and run all examples — output must be unchanged. This is the single most valuable check for VM changes.
- Other flags in `src/common.h`: `DEBUG_LOG_GC`, `DEBUG_PRINT_CODE`, `DEBUG_TRACE_EXECUTION`.
- GC-safety rule: any C code that allocates while holding an unrooted object must keep it reachable (push it on the VM stack, or use `peek` instead of `pop`). See `makeConstant`, `concatenate`, `allocateString`, and the `kunci`/`OP_BUILD_LIST` handlers for the pattern.

## Language reality (differs from older docs)

- **Blocks use braces `{ }`, not Python indentation.** The scanner has no INDENT/DEDENT. `jika c { ... } jika_lain c2 { ... } selain { ... }`, `selagi c { ... }`, `fungsi f() { ... }`.
- Implicit assignment `x = 10` at top level defines a global. **Inside a function, assignment creates a function-local (K3)** — use `global x` to write the script-level variable and `nonlokal x` to write an enclosing function's. `variabel x = ...` still works and names the same local. `prescanFunctionBody` (compiler.c) peeks over the body once before compiling it and pre-declares a slot (`OP_KOSONG`) for every assigned bare name, so a name first assigned inside a `jika`/`selagi`/`untuk` body survives that block's `endScope`. It skips nested `fungsi`/`kelas` bodies (they get their own prescan) and names already taken by `variabel` at the body's own depth. It walks raw tokens, so a future construct whose header can contain `{` (dict literals, anonymous functions) must be taught to it.
- **Numbers come in two types (K2): `bulat` (int64, `VAL_INT`) and `desimal`
  (double, `VAL_NUMBER`).** `IS_NUMBER`/`AS_NUMBER` are inline functions that
  cover both (`asNumber` widens an int to double) — never rewrite them as
  macros, or `IS_NUMBER(pop())` starts evaluating its argument twice. Use
  `IS_INT`/`AS_INT` to pick the exact int64 path. Arithmetic opcodes keep
  int+int in int64 and raise `GalatLuapan` on overflow (`intAddOverflow` etc.
  in `value.c`); `/` always produces a desimal, `//`/`%`/`**` stay bulat when
  both operands are bulat. `valuesEqual` compares numerically across the two
  types (`1 == 1.0` is benar) but `valuesIdentical` does not (`1 adalah 1.0`
  is salah). `numberToString` prints desimal with a mandatory `.0`, and
  `rangeNumberToString` prints rentang bounds without it. `rangeElement`
  (`object.c`) is the only correct way to read a rentang element if you want
  bulat elements. Truthiness: `0` and `0.0` are false.
- Truthiness is Python's (K1, `isFalsey` in `vm.c`): `salah`, `kosong`, `0`, `""`, empty list/kamus/rentang are false; every instance is true. Collection emptiness uses live counts (`dictLength` skips tombstones).
- `rentang()` returns an `OBJ_RANGE` (start/stop/step doubles, no list). `untuk` is desugared to `OP_LIST_LEN` + `OP_INDEX_GET`, so any sequence those two opcodes accept is iterable — currently list, rentang, and teks (per character). Add a type there to make it iterable.
- `x dalam y` / `x tidak dalam y` compile to `OP_IN` (+ `OP_NOT`) at `PREC_COMPARISON` (`containsValue` in `vm.c`). `TOKEN_TIDAK` now has an infix rule (`notIn`), so because of B16 a line starting with `tidak` right after an expression is a compile error ("Harapkan 'dalam'"). Comparison chaining is not implemented yet.
- **`{` is position-dependent.** `statement()` consumes `{` as a block; everywhere else the Pratt prefix rule `dictLiteral` reads it as a dict literal (`OP_BUILD_DICT`). A bare `{"a": 1}` statement is therefore a block, not a dict — the guides say to write `x = {"a": 1}`. `{` does NOT raise the scanner's `groupingDepth` (blocks need newline tokens), so `dictLiteral` skips `TOKEN_NEWLINE` by hand for multi-line literals. Dict keys must evaluate to strings.
- Negative indices are resolved in the VM (`OP_INDEX_GET`/`OP_INDEX_SET`) for list, teks, and rentang — NOT for `kamus[angka]`, which means "nth key in insertion order". Error messages report the index the user wrote, not the normalised one.
- Slicing `a[awal:akhir:langkah]` emits `OP_SLICE` with four stack slots (target + 3 bounds); omitted parts are compiled as `OP_KOSONG` so the VM distinguishes "not given" from a real value. `resolveSlice` in `vm.c` holds the Python clamping rules; a step of 0 raises `GalatNilai`. Slices are read-only — assignment is rejected at compile time.
- **`==` is structural, `adalah` is identity (K7) — do not conflate them.** `valuesEqual` in `value.c` recurses into lists, dicts, and ranges (dict insertion order is irrelevant; depth capped at `EQUAL_MAX_DEPTH` so self-referential containers can't hang the VM). `valuesIdentical` is the identity version. `OP_EQUAL` uses the first, `OP_IDENTICAL` the second. Two call sites must stay on identity: `chainCompare`'s `CHAIN_IDENTITY`/`CHAIN_NOT_IDENTITY` AND `emitComparison` (the non-chained path) — forgetting the second one silently makes `adalah` structural, which the `gabung_sama` golden exists to catch. `containsValue` (`dalam`) deliberately inherits the structural version.
- **tupel (K17)** is `OBJ_TUPLE`, deliberately laid out exactly like `ObjList` (`Obj` + `ValueArray items`). Read paths (index, slice, unpack, iterate, `dalam`, `==`, hash, `+`, `*`, native sequence helpers) go through `sequenceItems(v)` in `object.h`, which returns the items of a daftar OR tupel; never write through it. Write paths must reject tupel explicitly: `OP_INDEX_SET`/`OP_INDEX_DELETE` and `rejectTuple` in the mutating natives. A new list-handling site therefore needs a decision: read → `sequenceItems`, write → reject. Literal parsing is in `grouping()`: a comma after the first expression makes a tupel, `()` is empty, `(x)` stays grouping. `hashValue` combines element hashes (structural, so equal tupel are one dict key); an unhashable element makes the whole tupel unhashable.
- **himpunan (K17)** is `OBJ_SET`, laid out exactly like `ObjDict` (`table` + insertion-order `keys`, guarded by `_Static_assert`), so it reuses `dictSet`/`dictDelete`/`dictLength`/`dictKeyAt` via `setAsDict()`. No literal syntax — only `himpunan(...)`. Members follow the K8 hash rules; a set itself is unhashable (Python's `set`), so it cannot be a dict key — a hashable `himpunan_beku` is future work. `| & ^` go through `bitwiseBinary` → `setBinary`, `-` through the set branch at the top of `OP_SUBTRACT`. Order is insertion order; goldens compare via `terurut`. Regression: `examples/himpunan.idk`.
- An uncaught typed galat rethrown by `OP_THROW` (no `kecuali` matched) prints its `.pesan` via `errorMessage`; it used to lose the message. Regression: `tests/galat/galat_bertipe_diteruskan`.
- A single chunk holds at most **256 constants** (one-byte index, as in clox). A long top-level script full of literals hits "Terlalu banyak konstanta dalam satu chunk"; goldens split sections into functions (each has its own table, see `examples/tupel.idk`). Lifting this needs an `OP_CONSTANT_LONG`.
- `+` accepts two lists (`concatenateLists`, both operands stay on the stack while the result grows); `*` accepts list/teks × angka in either order. `repeatCount` rejects non-integers (`GalatTipe`) and caps results at `REPEAT_MAX` 65535 (`GalatNilai`) so `[0] * 1e9` can't exhaust memory. Negative/zero factors give an empty result, as in Python.
- Implemented: closures/upvalues, lists `[..]` + indexing, dicts via `kamus()` + string indexing, `untuk x dalam list { }`, `hentikan`/`lanjut`, classes (`kelas`, `init`, `diri`, `<` inheritance, `super`), `naikkan` + `coba/kecuali`, `impor "file.idk"` (shared global namespace, cached).
- Catchable vs fatal: **every** `runtimeError` is catchable by `kecuali` (the caught value is the message string). When a handler is active, `runtimeError` only records the message (`vm.runtimeErrorPending`, no allocation) and `run()` returns; `interpret()` then unwinds via `throwValue`, pushes the message, and calls `run()` again. Only `fatalError` (stack underflow, bad jump offset — VM bugs) is never catchable. Natives still use `nativeRaise`. Uncaught errors print and exit 70.
- `coba` handlers: `OP_RETURN` drops handlers registered by the returning frame; `hentikan`/`lanjut` emit `OP_TRY_END` for each `coba` they leave (`Compiler.tryDepth` vs `LoopCtx.tryDepth`).
- **Several `kecuali` clauses chain by jump, and `catchClause()` does NOT emit its own rethrow.** It returns the unpatched "no match" jump; `tryStatement` patches it onto the next clause and only the last one falls through to `OP_POP` + `OP_THROW`. An untyped `kecuali e` returns -1 (always matches) and must be last — `semanticError`, not `error`, so there is no panic-mode cascade. `scanTryClauses` must skip EVERY `kecuali` clause in its lookahead loop, otherwise a trailing `akhirnya` goes undetected and its outer handler is never registered.
- **`declaration()` returns WITHOUT consuming when it sees `}`** — that is how `block()` detects the end of a block. Any loop calling it at top level must therefore guard against `}` itself, or it spins forever: a file containing only `}` used to hang the compiler (exit 124). `compile()` has that guard; add one to any new top-level loop.
- `akhirnya` (finally): the single-pass compiler peeks ahead with `saveScanner`/`restoreScanner` (`scanTryClauses`) to learn whether `kecuali`/`akhirnya` follow, because a finally needs an OUTER handler registered before the body. The finally block runs with two hidden locals (raised value, rethrow flag) and rethrows via `OP_THROW`. `kembalikan`/`hentikan`/`lanjut` that leave such a `coba` re-compile the finally block inline at the jump (`exitTryBlocks` → `inlineFinallyBlock`, rewinding the scanner to the position saved in `Compiler.finallies[]`), like CPython's duplicated finally code. **Each copy counts against `FINALLY_INLINE_MAX` (64) per function**: a jump inside an `akhirnya` re-inlines the enclosing ones, so nesting grows the code exponentially — the CI fuzzer hung the compiler with it. Past the cap `reportFinallyBudget` prints one error (lifting both inline suppression and panic mode) and sets `suppressFurtherErrors`, because the copies already made overflow the constant table and cascade. Never remove the cap. Errors during that re-compile are silenced (`inlineFinallyDepth`) because the in-place copy reports them. A returned value is evaluated first and parked in a hidden local. `semanticError` reports well-formed-but-forbidden code without panic-mode cascades.
- **List comprehension `[expr untuk x dalam seq jika cond]` is compiled into
  an implicit one-parameter function**, called once per element by
  `OP_APPEND`/`OP_CALL` (`listComprehension` in `compiler.c`). Two reasons, both
  structural: the element tokens come BEFORE the header that declares `x`, so
  the compiler peeks ahead (`scanComprehension`) and re-reads them after the
  header; and the loop state cannot live in hidden locals of the enclosing
  frame, because an enclosing call's callee/arguments already occupy those
  slots (slots are stack positions). Only one `untuk` plus one optional `jika`
  are supported. The **iterable and the `jika` filter are parsed with
  `parsePrecedence(PREC_OR)`, not `expression()`** — Python's `or_test` —
  because `expression()` also handles the conditional expression
  `a jika c selain b`, which would swallow the filter's `jika` and demand a
  `selain`. The element itself is a full `expression()`. A conditional
  iterable needs parentheses: `[x untuk x dalam (a jika c selain b)]`.
- **Source files are newline-normalised on load** (`normalizeNewlines` in
  `common.h`, called by `readFile` in `main.c` and `readModuleSource` in
  `vm.c`): `\r\n` and lone `\r` become `\n`, like Python's universal newlines.
  Without it a CRLF checkout (git on Windows) leaked `\r` into triple-quoted
  strings, `panjang()`, and error snippets. Any new path that reads source
  must call it too.
- `number()` in `compiler.c` merges Fase 1 literals with K2: `_` separators,
  `0b`/`0o`/`0x` and every literal without `.`/exponent are `bulat` (int64,
  overflow is a compile error); only `.`/`e` literals are `desimal`.
- **`buang` (del)** has its own opcodes: `OP_DELETE_GLOBAL` removes the name
  from the table, while `OP_DELETE_LOCAL`/`OP_DELETE_UPVALUE` write
  `VAL_TAKTERISI` into the slot — `OP_GET_LOCAL`/`OP_GET_UPVALUE` therefore
  check for it and raise "sudah di-buang". `OP_INDEX_DELETE` handles list and
  dict, `OP_DELETE_PROPERTY` an instance field. Any new read path for locals
  must keep that check.
- **Dictionaries are keyed by `Value` (K8), not by string.** `ObjDict.table`
  is a `ValueTable` (`table.c`), separate from the `ObjString*` tables used for
  globals, string interning, class methods and instance fields. Key equality is
  `valuesEqual`, so `1` and `1.0` are the same key; `hashValue` in `value.c`
  must stay consistent with that (it hashes whole doubles as int64).
  Unhashable keys (list/kamus/rentang) are rejected *before* insertion.
  `ObjDict.keys` is now a contiguous live-key list in insertion order (delete
  shifts, no tombstones). Dict iteration cannot use `OP_INDEX_GET` any more —
  `kamus[i]` is a key lookup — so `untuk`/comprehension use `OP_SEQ_GET`, which
  maps a kamus position to `dictKeyAt`.
- **`a, b = ...` is a statement-level construct** (`looksLikeUnpack` peeks for
  `IDENT (, IDENT)+ =`), compiled as the RHS sequence + `OP_UNPACK n` + one
  assignment per name. `prescanFunctionBody` tracks comma-chains so every name
  in the list becomes a function-local (K3), not just the last one before `=`.
- Statements are newline-terminated outside `()`/`[]`; the scanner suppresses newline tokens while its grouping depth is positive. Preserve `ScannerState.groupingDepth` across parser lookahead, and do not add INDENT/DEDENT: `{}` remains the block syntax.
- Strings are UTF-8 bytes but the language counts **characters**: `ObjString.charCount` is computed at allocation; `panjang`, `teks[i]`, `karakter`, `potong` go through `utf8ByteOffset` (O(1) when `charCount == length`, i.e. ASCII). `ord`/`chr` use code points.
- `cetak(a, b)` compiles to `OP_PRINT <count>`: one line, space-separated. Numbers print via `numberToString` as the shortest round-trip repr (integral values < 1e16 without `.0`).
- String literals process escape sequences (`\n \t \r \" \\ \'`) in the compiler's `string()` (compiler.c); the scanner skips escaped chars so `\"` doesn't end a string. Adding new escapes = edit both.
- Builtins live in `src/native.c`, registered in `initVM` (`src/vm.c`). Families: math, string, list, dict, conversion (`ke_angka`/`ke_teks`/`format`), file I/O (`buka_berkas`/`baca`/`tulis`/...), date/time (`waktu`/`tanggal`), TCP sockets (`soket_dengar`/`soket_terima`/`soket_baca`/`soket_tulis`/`soket_tutup`), util. `OBJ_FILE` and `OBJ_SOCKET` handles auto-close on GC.
- Sockets: Winsock on Windows (Makefile adds `-lws2_32`; native.c lazy-inits `WSAStartup`), BSD sockets on POSIX; servers listen on `127.0.0.1` only. `examples/todo_server.idk` is a REST HTTP server built on `pustaka/http.idk` (request parse + response build) + `pustaka/json.idk`. Integration-tested in CI (`tests/test_todo_server.{sh,ps1}`, job `todo-http`) — a blocking server can't be a golden test, so it's driven by `curl` and shut down via the `/berhenti` endpoint.
- Command-line args after the script path are exposed as the global list `argumen`.
- Standard library written **in the language itself** lives in `pustaka/` (`teks.idk`, `daftar.idk`, `matematika.idk`, `kamus.idk`, `json.idk`, `http.idk`), imported via `impor "pustaka/..."`. Functions are prefixed (`teks_`, `daftar_`, `mat_`, `kamus_`, `json_`, `http_`) because globals share one namespace. They build on the C primitives `karakter`/`potong` (string char/substring), `ord`/`chr`, and first-class functions.
- Implicit assignment `x = 10` to a global-target name uses define-semantics (create-or-update) at ANY scope depth — so it works inside `jika`/`selagi`/`untuk` bodies (which are their own scopes). At script level that means globals; inside a function only a name declared `global` reaches a global this way. Only compound assignment (`+=` etc.) requires the global to already exist.
- `ObjSet` carries a `frozen` flag instead of a separate `OBJ_FROZENSET` type, so every
  `ObjType` switch stays untouched. Consequences: `hashValue` accepts a frozen set (XOR of
  member hashes, so insertion order does not matter), `valuesEqual` deliberately ignores
  `frozen` (`himpunan([1]) == himpunan_beku([1])` is benar, as in Python), and every write
  path goes through `rejectFrozenSet` — there are exactly two (`tambah`, `hapus`).
- The editor's builtin highlight list is GENERATED from `defineNative()` in `src/vm.c`,
  sorted longest-name-first so `himpunan_beku` wins the alternation against `himpunan`.
  Do not hand-edit it: 35 builtins silently went unhighlighted for months that way.
- `allowTupleRHS` in `compiler.c` gates the bare-comma tupel (`x = 1, 2`). It is enabled by
  exactly four callers — `expressionStatement`, `varDeclaration`, `returnStatement` (both
  branches), and the `OP_INDEX_SET` path — and `maybeTupleTail` additionally requires
  `scannerGroupingDepth() == 0`. Both guards are needed: without the depth check a comma
  inside `f(x = 1, y)` or `[a, b]` would start building a tupel.
- GC-safety when mutating collections: keep operands on the VM stack (`peek`, not `pop`) until after `tableSet`/`writeValueArray`, which can grow and trigger a GC. `OP_INDEX_SET` for dicts and the `sisip` native follow this. `ObjDict.keys` is also a GC-marked insertion-order list: update it only through `dictSet`/`dictDelete`.
- **Value formatting lives in THREE paths and they must agree:** `printObject` (`object.c`, used by `cetak`), `appendValueText` (`native.c`, used by `ke_teks`/`format`/f-strings), and `nativeFnRepr`. The escape table is shared via `stringEscape(char)` in `object.c`, called by both `printStringRepr` (object.c) and `appendStringRepr` (native.c) — patching only `printObject` makes `cetak(x)` and `ke_teks(x)` disagree. Strings print bare on their own but are quoted as **collection elements** (`cetak(["a"])` → `["a"]`), Python's str/repr split; double quotes, not Python's single, because dict keys already use `"`.
- **Iterate `ObjDict`/`ObjSet` through `keys`, never `table.entries`** — the table is hash order, `keys` is insertion order, and printing that disagrees with `kunci()` or `untuk ... dalam` on the same dict is a bug users see immediately.
- **Built-in methods (`"a".huruf_besar()`) cost zero new opcodes** because every relevant native already takes its receiver as `args[0]` and `OP_INVOKE` leaves the stack as `receiver, arg1, …`; `invoke()` just maps name → native. Verify that invariant before adding an entry to the string/list/dict method tables — a native with a different argument order needs a wrapper. Built-in methods can be *called* but not taken as values (no `ObjBoundMethod`); the error message says so explicitly.
- **Natives reject keyword arguments by default.** `vm.nativeArgNames` is filled only for natives listed in `nativeTakesKeywords()` (currently just `cetak`), so an unsupported `pisah=` is a clear error instead of being silently ignored. `cetak` is a plain native function, **not a keyword** — `OP_PRINT` was removed, and overwriting `cetak` behaves exactly like Python.
- **Compile AND runtime errors carry a column**, a source snippet and a caret. The chunk stores a column per bytecode plus its source text as an `ObjString`, so the GC keeps a module's source alive for as long as its functions — `readModuleSource` frees its own buffer right after compiling, which is why the source cannot be borrowed. Only the innermost frame prints a snippet. The column points at the **end** of the offending expression (the last token parsed), not at the operator.
- **f-strings compile their embedded expression from a temporary copy that is then freed** — so never let a token outlive it. `parser.previous` is reset to the f-string token before the free; `ScannerState` saves/restores `source` **and** `sourceEnd`; `lineStartOf` checks **both** buffer bounds (a token from a freed slice can sit at a higher address and pass a start-only check). Columns inside an f-string are pre-computed from the real line before the sub-scanner starts: coarse but never misleading. All three were latent use-after-frees that only ASan caught.
- `examples/todo_cli.idk` is the Todo-list JSON DB demo (CRUD over a JSON file via `pustaka/json.idk`). Integration-tested in CI by `tests/test_todo_cli.{sh,ps1}` (job `todo-crud`) plus golden tests for `json_pakai`/`todo_demo`.
- Operators follow Python: `//` floors (`OP_FLOOR_DIVIDE`), `%` takes the divisor's sign, `**` is right-associative and binds tighter than unary minus (`PREC_POWER` > `PREC_UNARY`), `tidak` binds looser than comparisons (`PREC_NOT`). Compound assignment (`+=`, `//=`, ...) is table-driven via `matchCompoundAssign` and its right side is a full `expression()`. **A new compound operator must be added to BOTH `matchCompoundAssign` and `isCompoundAssign`** — the latter feeds `prescanFunctionBody`, so forgetting it makes `x op= 1` inside a function skip K3 local detection. Regression: `examples/operator_aritmetika.idk`.
- Bitwise `& | ^ << >>` share one handler, `bitwiseBinary` in `vm.c` (operands via `bitwiseOperand`: bulat, or logika as 0/1); `~` is `OP_BIT_NOT`. Precedence levels `PREC_BIT_OR < PREC_BIT_XOR < PREC_BIT_AND < PREC_SHIFT` sit between comparisons and `PREC_TERM`, exactly Python's order. Shifts go through `intShiftLeft`/`intShiftRight` in `value.c`, written without UB (no shift by ≥ 64, no signed left shift, no implementation-defined right shift of negatives) — keep it that way, UBSan in CI checks. `bitwiseBinary` dispatches on operand type on purpose: set union/intersection (`himpunan | himpunan`) and `kamus | kamus` go there as new branches, not new opcodes. Regression: `examples/operator_bitwise.idk`.
- **Default parameters and keyword arguments share one mechanism — do not reintroduce the old one.** Parameters may have defaults (`fungsi f(a, b=2)`) and calls may name arguments (`f(b=2)`, also for `init`, methods, and `super`). `call()` fills every slot the caller did not supply with the internal marker `VAL_TAKTERISI`, and each default is guarded by `OP_ISI_BAWAAN slot, lompat` so its expression runs only when the slot is still unfilled. An earlier design used a staged prologue (`ObjFunction.entryPoints`, jumping straight to an entry point chosen by `argCount`); it was removed because keyword arguments can leave a gap in the MIDDLE (`f(1, c=9)`) that a single entry point cannot express. Per K15 defaults are evaluated on **every call**, not once at definition — deliberately unlike Python, so `fungsi f(x, daftar=[])` never shares a list; never "fix" this. Golden `argumen_bawaan` exists to catch a regression here.
- Keyword names are matched in the VM, not the compiler: the callee is a runtime value. `ObjFunction.paramNames` holds the parameter names and `OP_CALL_BERNAMA` carries the caller's name list as a chunk constant. Methods called with keywords fall back to `OP_GET_PROPERTY`/`OP_GET_SUPER` + a normal call, so `OP_INVOKE`/`OP_SUPER_INVOKE` need no keyword variants. Native functions reject keyword arguments with a clear message (`cetak(pisah=, akhir=)` waits on K12).
- Two GC traps when touching this code, both invisible to the normal suite and only caught by `-DDEBUG_STRESS_GC` + ASan: (1) `paramNames` is `ObjString**`, must be marked in `blackenObject`, and must be NULL-filled immediately after `ALLOCATE` — the `copyString` calls that populate it allocate, and a GC mid-loop would walk uninitialised slots; (2) the keyword-name list in `argumentList` is built on the VM stack as it goes, never in a local C array, or names collected earlier are freed while later arguments allocate.
- `callArgStart` in `compiler.c` no longer rejects `f(nama=nilai)`; it only tracks argument start positions. Parenthesized `f((x = 1))` remains an assignment expression.
- Max call depth `FRAMES_MAX` is 1000 (Python's default); runtime tracebacks collapse identical consecutive frames into `[baris sebelumnya diulang N kali lagi]`.
- Non-obvious VM facts: `<`/`>` compare strings via `strcmp` (not just numbers). String indexing `teks[i]` (read-only; strings immutable) shares the `OP_INDEX_GET` path with list/dict. `variabel` declared inside a `jika`/`selagi`/`untuk` body is properly scoped (each body gets `beginScope`/`endScope`); a regression once leaked such locals past the loop — see `examples/lingkup_variabel.idk`.
- The REPL (`main.c`) buffers input across lines until braces balance, so multi-line `fungsi`/`kelas`/`jika` blocks work; `braceDelta` ignores braces in strings/comments.

## CI

- `.github/workflows/test.yml` jobs: `test-linux`/`test-macos`/`test-windows` (build + golden tests, gating), **`todo-crud`** (CLI CRUD integration via `tests/test_todo_cli.sh`), **`todo-http`** (REST server integration via `tests/test_todo_server.sh`, driven by `curl`), **`sanitizers`** (ASan+UBSan with `-DDEBUG_STRESS_GC`), **`valgrind`**, **`coverage`** (lcov artifact), **`fuzz`** (short libFuzzer run on `tests/fuzz_compile.c` — compiles the front end only, does NOT execute, so infinite-loop programs can't cause timeouts). `continue-on-error` was removed from build/test steps — CI red now means a real failure.
- Non-Linux `sanitizers`/`valgrind`/`fuzz` jobs build via raw gcc/clang (no `-lws2_32`; on Linux sockets are in libc). The gating Windows job builds via `make` and DOES link `-lws2_32`.
- `.github/workflows/release.yml`: on `v*.*.*` tags; runs the test suite before publishing artifacts.

## Layout gotchas

- `compiler/` is **not** the language compiler — it's the Windows installer (Inno Setup `.iss` + assets). `dist/` holds prebuilt `Indonesiaku-Setup-*.exe`. All language implementation lives in `src/`.
- Root-level `test_*.idk` files are ad-hoc scratch scripts (git-ignored), not the test suite.
- Typed errors use `kecuali GalatTipe sebagai e` or `kecuali (GalatTipe, GalatNilai) sebagai e`; instances carry `.pesan`. Keep the legacy `kecuali e` form working when changing VM error paths, and root newly created error instances/message strings until their fields own them.
- `src/` map: `main.c` CLI/REPL · `scanner.c` lexer · `compiler.c` single-pass parser+codegen (Pratt) · `chunk.c` bytecode · `vm.c` interpreter loop + native registration + import · `native.c` builtins · `value.c`/`object.c` values & heap objects · `table.c` hash table · `memory.c` allocation + GC · `debug.c` disassembler.
- `docker/images/Dockerfile` is multi-stage (build → slim Alpine runtime, ~12.6 MB); it bundles the `indk` binary + `pustaka/` + `examples/`. Image is `teguh02/indonesiaku:<version>` and `:latest` on Docker Hub. Keep the `LABEL version` in sync with `INDK_VERSION`.

## Website (`website/`) vs language planning (`docs/`)

- **Two separate folders, two separate jobs.** `website/` = everything users see (usage docs, site, playground). `docs/` = internal language planning (roadmap, spec, per-feature plans) — **all development planning goes here, nowhere else**. Never put user guides in `docs/` and never put roadmap/planning in `website/panduan/`. Both must be updated when a feature lands: `docs/` records the decision, `website/panduan/` teaches the user.
- `website/` pages: `index.html` (landing), `about.html`, `author.html`, `docs.html` (markdown reader), `playground/index.html`, shared `assets/`, and the user guides in `website/panduan/*.md`. There is no Netlify anymore — the site is self-hosted.
- **Frontend stack (no build step):** Tailwind Play CDN + tokens in `assets/tailwind-config.js`, Alpine.js components in `assets/app.js`, and **Senna.js 2.7.3** (globals build) as the SPA engine. Every page is still a complete HTML document with its own URL (crawlable/SEO); Senna only swaps the `#konten` surface. Navigation must never rely on `#konten` being re-parsed by hand — Senna evaluates inline scripts inside the surface, so page scripts MUST be wrapped in an IIFE.
- **Bilingual:** all UI strings live in `assets/i18n.json` (`id` + `en`, identical keys). `app.js` fetches it **before** loading Alpine and only then calls `Alpine.data('shell', …)`; never add an Alpine `<script>` tag to a page. Long-form prose on `/about` and `/author` is duplicated as two `<article>` blocks toggled by `x-show="lang === 'id'"`.
- **Theme:** `darkMode: 'class'`; the inline pre-paint script in each `<head>` applies `dark` from `localStorage['indk-theme']` or `prefers-color-scheme`. Never move that script below the stylesheet or the page flashes light.
- **SPA gotchas:** new pages must also load `marked` + `highlight.js` in the `<head>` (Senna does not swap `<head>`, so the docs reader would break when reached by SPA navigation); docs code uses `history.replaceState(history.state, …)` because plain `null` wipes Senna's history entry and breaks the back button.
- `website/playground/index.html` is the playground client (plain HTML+JS, no build step). It `POST`s `{ kode }` to `/api/jalankan` and expects `{ stdout, stderr, kode_keluar, ms }`. The API is served by the same Docker bundle; until it exists the page shows a "backend tidak tersedia" state.
- Local preview must be over HTTP (fetch + CDN + Senna): `python -m http.server 8000 -d website`, then open `http://localhost:8000/`. `file://` will not work.

## Playground API (`docker/playground/`)

- `server.py` — stdlib HTTP service. `POST /api/jalankan {kode, input}` → `{stdout, stderr, kode_keluar, ms}`. It writes the code to a temp dir, symlinks `pustaka/` next to it (so `impor "pustaka/..."` works), writes `input` to a stdin file (one line per `input()` call), runs `indk main.idk`, and returns the result. Limits: 128 KB body, 64 KB each for code/input/output, 3 s wall + 3 s CPU, 128 MB address space, 1 MB file size, 32 fds, 60 req/min per IP. Pass `--static <dir>` to also serve the site (dev mode, no nginx needed).
- `nginx.conf` — serves `/usr/share/nginx/html` (the `website/` copy), forces `text/markdown` for `*.md` and `text/plain` for `*.idk`, and proxies `/api/` to `http://runner:8099`.
- `Dockerfile` — one image: builds `indk`, bundles `website/`, `pustaka/`, nginx, python3. `docker-compose.yml` runs it as two services: `web` (published port, attached to `publik` + `internal`) and `runner` (`internal` only, so no internet egress; read-only rootfs, tmpfs `/tmp`, non-root uid 10001, cap_drop ALL).
- Run: `docker compose -f docker/playground/docker-compose.yml up -d --build` (site on `:8080`).
- Dev without Docker: build `indk`, then
  `INDK_BIN=$PWD/indk PUSTAKA_DIR=$PWD/pustaka python3 docker/playground/server.py --port 8099 --static website`.
- The page hard-codes `const API_URL = "/api/jalankan"`; keep nginx and dev server on that path.
- `.dockerignore` at the repo root keeps build contexts small; it applies to `docker/images/` too (that image only needs `src/` + `Makefile`).
- `website/docs.html` renders the `.md` files at runtime with marked.js (fetched relative to `website/`, so entries are `panduan/<file>.md`) and syntax-highlights with highlight.js. To add a doc to the reader's sidebar, edit the `DOCS` array in `docs.html`; the `mdMatch` regex in `fixLinks()` must keep accepting `/` in paths.
- **Embedding example code in docs:** put a marker `{{embed:examples/foo.idk}}` on its own line in a `.md`. The reader fetches that file from `raw.githubusercontent.com/teguh02/Indonesiaku/main/...` so it always mirrors the repo — `examples/` is NOT copied into `docs/`. This means an embedded example must be committed+pushed to `main` before it shows on the live site.
- Deployment target is our own server, as ONE Docker bundle: the `website/` static files (docs + site) and the playground run together in a container; no Netlify, no `.netlify/`. Whatever serves the static files MUST send `text/markdown; charset=utf-8` for `*.md` and `text/plain; charset=utf-8` for `*.idk`, otherwise `fetch()` in the reader fails. The reader requires HTTP — it does not work over `file://`. For local preview: `python -m http.server 8000 -d website`.
- When language features change, keep these in sync — see the mandatory checklist in **Feature workflow** at the top: `README.md`, `CHANGELOG.md`, `website/panduan/*.md` (esp. `03_PANDUAN_SINTAKS.md` and `07_PUSTAKA_STANDAR.md`), `editors/vscode-indonesiaku/` (keywords, builtins, snippets), `docs/ROADMAP.md`, this `AGENTS.md`, and the Dockerfile `LABEL version`.

Bagian **Tata Kelola Proyek, Nama, dan Disclaimer** di atas tetap berlaku untuk
setiap perubahan pada `README.md`, `website/panduan/`, atau dokumen publik lain
yang menyentuh identitas proyek, nama, afiliasi, atau pernyataan independensi.

## Conventions

- C style: 4-space indent, types PascalCase, constants UPPER_CASE, variables snake_case, braces same-line. `.editorconfig` enforces LF + 4-space (tabs for Makefile).
- Commit messages use Indonesian prefixes: `Tambah:` (add), `Perbaiki:` (fix), `Ubah:` (change), `Docs:`, `Refactor:`, `Test:`, `Chore:`.

## Docs content

- User guides (Indonesian) in `website/panduan/` (`README` + `01_PERKENALAN` … `07_PUSTAKA_STANDAR`). Syntax reference: `website/panduan/03_PANDUAN_SINTAKS.md`; builtin/stdlib reference: `website/panduan/07_PUSTAKA_STANDAR.md`. These `.md` files are what the live site renders.
- Language planning lives in `docs/`: `docs/README.md` (index + how to write a plan), `docs/ROADMAP.md` (SINGLE source of truth for roadmap — do not duplicate a roadmap in README/CHANGELOG), `docs/RENCANA_FITUR/TEMPLATE.md` (per-feature plan template).
- Repo/author facts to keep consistent: GitHub `teguh02/Indonesiaku`, email `teguhrijanandi02@gmail.com` (older docs used a wrong `teguhriyan` handle — fixed).
