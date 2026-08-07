#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"

// Count net brace depth in a line, ignoring braces inside strings and after
// a '#' comment. Used by the REPL to keep reading until a block is complete.
static int braceDelta(const char* s) {
    int depth = 0;
    bool inString = false;
    char quote = 0;
    for (const char* p = s; *p; p++) {
        char c = *p;
        if (inString) {
            if (c == '\\' && p[1] != '\0') { p++; continue; }
            if (c == quote) inString = false;
        } else if (c == '"' || c == '\'') {
            inString = true;
            quote = c;
        } else if (c == '#') {
            break;  // rest of line is a comment
        } else if (c == '{') {
            depth++;
        } else if (c == '}') {
            depth--;
        }
    }
    return depth;
}

static void repl() {
    char line[1024];
    char buffer[8192];

    for (;;) {
        buffer[0] = '\0';
        int depth = 0;
        bool first = true;

        // Read one logical unit: keep reading continuation lines while there
        // are unclosed braces so multi-line blocks (fungsi/kelas/jika/...) work.
        for (;;) {
            printf(first ? "> " : "... ");
            if (!fgets(line, sizeof(line), stdin)) {
                printf("\n");
                if (buffer[0] != '\0') interpret(buffer);
                return;
            }
            first = false;

            // Append the line to the accumulation buffer (bounded).
            if (strlen(buffer) + strlen(line) < sizeof(buffer)) {
                strcat(buffer, line);
            }

            depth += braceDelta(line);
            if (depth <= 0) break;  // block complete (or no block opened)
        }

        interpret(buffer);
    }
}

static char* readFile(const char* path) {
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Tidak dapat membuka file \"%s\".\n", path);
        exit(74);
    }

    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    char* buffer = (char*)malloc(fileSize + 1);
    if (buffer == NULL) {
        fprintf(stderr, "Tidak cukup memori untuk membaca \"%s\".\n", path);
        exit(74);
    }

    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    if (bytesRead < fileSize) {
        fprintf(stderr, "Tidak dapat membaca file \"%s\".\n", path);
        exit(74);
    }

    buffer[bytesRead] = '\0';

    fclose(file);
    return buffer;
}

static int runFile(const char* path) {
    char* source = readFile(path);
    InterpretResult result = interpret(source);
    free(source);

    if (result == INTERPRET_COMPILE_ERROR) return 65;
    if (result == INTERPRET_RUNTIME_ERROR) return 70;
    return 0;
}

int main(int argc, const char* argv[]) {
    // Handle simple CLI flags only when they are the first argument, so a
    // script can receive its own "-v"/"-h" arguments without interception.
    if (argc >= 2) {
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0) {
            printf("%s\n", INDK_VERSION);
            return 0;
        }
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            printf("Indonesiaku %s\n", INDK_VERSION);
            printf("Penggunaan: indk [path] [argumen...]\n");
            printf("Opsi:\n  -v, --version   Tampilkan versi\n  -h, --help      Tampilkan bantuan\n");
            return 0;
        }
    }

    initVM();

    int exitCode = 0;
    if (argc == 1) {
        repl();
    } else {
        // indk script.idk [arg1 arg2 ...]
        defineArgs(argc, argv, 2);
        exitCode = runFile(argv[1]);
    }

    freeVM();
    return exitCode;
}
