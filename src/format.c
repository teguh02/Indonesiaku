#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "format.h"
#include "scanner.h"

// Growable output buffer.
typedef struct {
    char* chars;
    size_t count;
    size_t capacity;
} Out;

static void outInit(Out* o) {
    o->capacity = 1024;
    o->count = 0;
    o->chars = (char*)malloc(o->capacity);
    if (o->chars != NULL) o->chars[0] = '\0';
}

static void outAppend(Out* o, const char* text, size_t length) {
    if (o->chars == NULL) return;
    if (o->count + length + 1 > o->capacity) {
        while (o->count + length + 1 > o->capacity) o->capacity *= 2;
        char* grown = (char*)realloc(o->chars, o->capacity);
        if (grown == NULL) { free(o->chars); o->chars = NULL; return; }
        o->chars = grown;
    }
    memcpy(o->chars + o->count, text, length);
    o->count += length;
    o->chars[o->count] = '\0';
}

// Note: CRLF input comes out as LF, because readFile() normalises newlines
// before we ever see it. That is the desired formatter behaviour (one line
// ending style), not an accident -- `rapikan --tulis` on a CRLF file
// converts it to LF.
//
// ponytail: identity formatter. It only proves the token stream can rebuild
// the source byte for byte, including comments -- the safety net the plan
// asks for BEFORE any real reformatting exists. Replace the body of the loop
// with real layout rules once the bytecode-identity test is wired up.
char* formatSource(const char* source) {
    Out out;
    outInit(&out);
    if (out.chars == NULL) return NULL;

    scannerKeepComments(true);
    initScanner(source);

    // Tokens carry their own start/length into `source`, so copying the gap
    // between the previous token's end and this token's start preserves every
    // space, newline and blank line exactly.
    const char* cursor = source;
    for (;;) {
        Token token = scanToken();
        if (token.type == TOKEN_ERROR) {
            scannerKeepComments(false);
            free(out.chars);
            return NULL;
        }
        if (token.type == TOKEN_EOF) break;

        if (token.start > cursor) {
            outAppend(&out, cursor, (size_t)(token.start - cursor));
        }
        outAppend(&out, token.start, (size_t)token.length);
        cursor = token.start + token.length;
        if (out.chars == NULL) { scannerKeepComments(false); return NULL; }
    }

    // Trailing whitespace after the last token.
    size_t tailLen = strlen(cursor);
    if (tailLen > 0) outAppend(&out, cursor, tailLen);

    scannerKeepComments(false);
    return out.chars;
}
