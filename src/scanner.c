#include <stdio.h>
#include <string.h>

#include "common.h"
#include "scanner.h"

typedef struct {
    const char* start;
    const char* current;
    int line;
} Scanner;

Scanner scanner;

void initScanner(const char* source) {
    scanner.start = source;
    scanner.current = source;
    scanner.line = 1;
}

static bool isAlpha(char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
            c == '_';
}

static bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

static bool isAtEnd() {
    return *scanner.current == '\0';
}

static char advance() {
    scanner.current++;
    return scanner.current[-1];
}

static char peek() {
    return *scanner.current;
}

static char peekNext() {
    if (isAtEnd()) return '\0';
    return scanner.current[1];
}

static bool match(char expected) {
    if (isAtEnd()) return false;
    if (*scanner.current != expected) return false;
    scanner.current++;
    return true;
}

static Token makeToken(TokenType type) {
    Token token;
    token.type = type;
    token.start = scanner.start;
    token.length = (int)(scanner.current - scanner.start);
    token.line = scanner.line;
    return token;
}

static Token errorToken(const char* message) {
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = scanner.line;
    return token;
}

static void skipWhitespace() {
    for (;;) {
        char c = peek();
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance();
                break;
            case '\n':
                scanner.line++;
                advance();
                break;
            case '#':
                // A comment goes until the end of the line.
                while (peek() != '\n' && !isAtEnd()) advance();
                break;
            default:
                return;
        }
    }
}

static TokenType checkKeyword(int start, int length, const char* rest, TokenType type) {
    if (scanner.current - scanner.start == start + length &&
        memcmp(scanner.start + start, rest, length) == 0) {
        return type;
    }

    return TOKEN_IDENTIFIER;
}

static TokenType identifierType() {
    switch (scanner.start[0]) {
        case 'a':
            if (scanner.current - scanner.start > 1) {
                switch (scanner.start[1]) {
                    case 'k': return checkKeyword(2, 7, "hirnya", TOKEN_AKHIRNYA);
                }
            }
            break;
        case 'b':
            return checkKeyword(1, 4, "enar", TOKEN_BENAR);
        case 'c':
            if (scanner.current - scanner.start > 1) {
                switch (scanner.start[1]) {
                    case 'e': return checkKeyword(2, 3, "tak", TOKEN_CETAK);
                    case 'o': return checkKeyword(2, 2, "ba", TOKEN_COBA);
                }
            }
            break;
        case 'd':
            if (scanner.current - scanner.start > 1) {
                switch (scanner.start[1]) {
                    case 'a':
                        if (scanner.current - scanner.start > 2) {
                            switch (scanner.start[2]) {
                                case 'l': return checkKeyword(3, 2, "am", TOKEN_DALAM);
                                case 'n': return TOKEN_DAN;
                                case 'r': return checkKeyword(3, 1, "i", TOKEN_DARI);
                            }
                        }
                        break;
                    case 'e': return checkKeyword(2, 4, "ngan", TOKEN_DENGAN);
                    case 'i': return checkKeyword(2, 2, "ri", TOKEN_DIRI);
                }
            }
            break;
        case 'f':
            return checkKeyword(1, 5, "ungsi", TOKEN_FUNGSI);
        case 'h':
            return checkKeyword(1, 7, "entikan", TOKEN_HENTIKAN);
        case 'i':
            return checkKeyword(1, 4, "mpor", TOKEN_IMPOR);
        case 'j':
            if (scanner.current - scanner.start > 1) {
                switch (scanner.start[1]) {
                    case 'i':
                        if (scanner.current - scanner.start == 4) {
                            return TOKEN_JIKA;
                        }
                        return checkKeyword(2, 7, "ka_lain", TOKEN_JIKA_LAIN);
                }
            }
            break;
        case 'k':
            if (scanner.current - scanner.start > 1) {
                switch (scanner.start[1]) {
                    case 'e':
                        if (scanner.current - scanner.start > 2) {
                            switch (scanner.start[2]) {
                                case 'c': return checkKeyword(3, 4, "uali", TOKEN_KECUALI);
                                case 'l': return checkKeyword(3, 2, "as", TOKEN_KELAS);
                                case 'm': return checkKeyword(3, 7, "balikan", TOKEN_KEMBALIKAN);
                            }
                        }
                        break;
                    case 'o': return checkKeyword(2, 4, "song", TOKEN_KOSONG);
                }
            }
            break;
        case 'l':
            if (scanner.current - scanner.start > 1) {
                switch (scanner.start[1]) {
                    case 'a': return checkKeyword(2, 4, "njut", TOKEN_LANJUT);
                    case 'e': return checkKeyword(2, 4, "wati", TOKEN_LEWATI);
                }
            }
            break;
        case 'n':
            return checkKeyword(1, 6, "aikkan", TOKEN_NAIKKAN);
        case 'o':
            return checkKeyword(1, 3, "tau", TOKEN_ATAU);
        case 's':
            if (scanner.current - scanner.start > 1) {
                switch (scanner.start[1]) {
                    case 'a': return checkKeyword(2, 3, "lah", TOKEN_SALAH);
                    case 'e':
                        if (scanner.current - scanner.start > 2) {
                            switch (scanner.start[2]) {
                                case 'b': return checkKeyword(3, 4, "agai", TOKEN_SEBAGAI);
                                case 'l':
                                    if (scanner.current - scanner.start == 6) {
                                        // Disambiguate 'selagi' vs 'selain'
                                        if (scanner.start[4] == 'g' && scanner.start[5] == 'i') return TOKEN_SELAGI;
                                        if (scanner.start[4] == 'i' && scanner.start[5] == 'n') return TOKEN_SELAIN;
                                    }
                                    break;
                            }
                        }
                        break;
                    case 'u': return checkKeyword(2, 3, "per", TOKEN_SUPER);
                }
            }
            break;
        case 't':
            return checkKeyword(1, 4, "idak", TOKEN_TIDAK);
        case 'u':
            return checkKeyword(1, 4, "ntuk", TOKEN_UNTUK);
        case 'v':
            return checkKeyword(1, 7, "ariabel", TOKEN_VARIABEL);
    }

    return TOKEN_IDENTIFIER;
}

static Token identifier() {
    while (isAlpha(peek()) || isDigit(peek())) advance();
    return makeToken(identifierType());
}

static Token number() {
    while (isDigit(peek())) advance();

    // Look for a fractional part.
    if (peek() == '.' && isDigit(peekNext())) {
        // Consume the ".".
        advance();

        while (isDigit(peek())) advance();
    }

    return makeToken(TOKEN_NUMBER);
}

static Token string() {
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n') scanner.line++;
        advance();
    }

    if (isAtEnd()) return errorToken("String tidak ditutup.");

    // The closing quote.
    advance();
    return makeToken(TOKEN_STRING);
}

Token scanToken() {
    skipWhitespace();
    scanner.start = scanner.current;

    if (isAtEnd()) return makeToken(TOKEN_EOF);

    char c = advance();
    if (isAlpha(c)) return identifier();
    if (isDigit(c)) return number();

    switch (c) {
        case '(': return makeToken(TOKEN_LEFT_PAREN);
        case ')': return makeToken(TOKEN_RIGHT_PAREN);
        case '{': return makeToken(TOKEN_LEFT_BRACE);
        case '}': return makeToken(TOKEN_RIGHT_BRACE);
        case '[': return makeToken(TOKEN_LEFT_BRACKET);
        case ']': return makeToken(TOKEN_RIGHT_BRACKET);
        case ':': return makeToken(TOKEN_COLON);
        case ';': return makeToken(TOKEN_SEMICOLON);
        case ',': return makeToken(TOKEN_COMMA);
        case '.': return makeToken(TOKEN_DOT);
        case '-':
            return makeToken(
                match('=') ? TOKEN_MINUS_EQUAL : TOKEN_MINUS);
        case '+':
            return makeToken(
                match('=') ? TOKEN_PLUS_EQUAL : TOKEN_PLUS);
        case '/':
            if (match('/')) {
                return makeToken(TOKEN_SLASH_SLASH);
            } else if (match('=')) {
                return makeToken(TOKEN_SLASH_EQUAL);
            }
            return makeToken(TOKEN_SLASH);
        case '*':
            if (match('*')) {
                return makeToken(TOKEN_POWER);
            } else if (match('=')) {
                return makeToken(TOKEN_STAR_EQUAL);
            }
            return makeToken(TOKEN_STAR);
        case '%': return makeToken(TOKEN_PERCENT);
        case '!':
            return makeToken(
                match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
        case '=':
            return makeToken(
                match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
        case '<':
            return makeToken(
                match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
        case '>':
            return makeToken(
                match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
        case '"': return string();
        case '\'': {
            // Single quote strings
            while (peek() != '\'' && !isAtEnd()) {
                if (peek() == '\n') scanner.line++;
                advance();
            }
            if (isAtEnd()) return errorToken("String tidak ditutup.");
            advance();
            return makeToken(TOKEN_STRING);
        }
        case '\n':
            return makeToken(TOKEN_NEWLINE);
    }

    return errorToken("Karakter tidak dikenal.");
}
