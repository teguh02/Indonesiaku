#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"

static void repl() {
    char line[1024];
    for (;;) {
        printf("> ");

        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }

        interpret(line);
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
