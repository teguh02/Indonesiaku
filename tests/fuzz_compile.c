// libFuzzer target for the Indonesiaku scanner + compiler (+ VM).
//
// Feeds arbitrary bytes as source code into the interpreter to shake out
// crashes, buffer overruns, and UB in the front end. Build with clang:
//
//   clang -g -O1 -fsanitize=fuzzer,address,undefined -Isrc \
//     tests/fuzz_compile.c src/chunk.c src/compiler.c src/debug.c \
//     src/memory.c src/native.c src/object.c src/scanner.c src/table.c \
//     src/value.c src/vm.c -lm -o fuzz_compile
//   ./fuzz_compile -max_len=4096 -timeout=5 -rss_limit_mb=2048
//
// Note: this links the real VM (not main.c). The VM is reinitialized per
// input so global state does not leak between runs.

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "vm.h"

int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    // NUL-terminate the input as a C string (the scanner expects that).
    char* source = (char*)malloc(size + 1);
    if (source == NULL) return 0;
    memcpy(source, data, size);
    source[size] = '\0';

    initVM();
    interpret(source);
    freeVM();

    free(source);
    return 0;
}
