#ifndef indonesiaku_scanner_h
#define indonesiaku_scanner_h

typedef enum {
    // Single-character tokens.
    TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
    TOKEN_LEFT_BRACKET, TOKEN_RIGHT_BRACKET,
    TOKEN_COMMA, TOKEN_DOT, TOKEN_MINUS, TOKEN_PLUS,
    TOKEN_COLON,      // dicadangkan: irisan a[1:3] & literal kamus (Fase 1)
    TOKEN_SEMICOLON, TOKEN_NEWLINE, TOKEN_SLASH, TOKEN_STAR,
    TOKEN_PERCENT, TOKEN_POWER,

    // One or two character tokens.
    TOKEN_BANG, TOKEN_BANG_EQUAL,
    TOKEN_EQUAL, TOKEN_EQUAL_EQUAL,
    TOKEN_GREATER, TOKEN_GREATER_EQUAL,
    TOKEN_LESS, TOKEN_LESS_EQUAL,
    TOKEN_SLASH_SLASH,  // Pembagian bulat
    TOKEN_SLASH_SLASH_EQUAL,
    TOKEN_PLUS_EQUAL, TOKEN_MINUS_EQUAL,
    TOKEN_STAR_EQUAL, TOKEN_SLASH_EQUAL,
    TOKEN_PERCENT_EQUAL, TOKEN_POWER_EQUAL,

    // Literals.
    TOKEN_IDENTIFIER, TOKEN_STRING, TOKEN_NUMBER,

    // Keywords dalam Bahasa Indonesia
    TOKEN_DAN, TOKEN_KELAS, TOKEN_SELAIN, TOKEN_SALAH,
    TOKEN_UNTUK, TOKEN_FUNGSI, TOKEN_JIKA, TOKEN_KOSONG,
    TOKEN_ATAU, TOKEN_KEMBALIKAN, TOKEN_SUPER,
    TOKEN_DIRI, TOKEN_BENAR, TOKEN_VARIABEL, TOKEN_SELAGI,
    TOKEN_JIKA_LAIN, TOKEN_DALAM, TOKEN_HENTIKAN, TOKEN_LANJUT,
    TOKEN_LEWATI, TOKEN_IMPOR, TOKEN_DARI, TOKEN_SEBAGAI, TOKEN_PASTIKAN,
    TOKEN_FSTRING,
    TOKEN_DENGAN, TOKEN_COBA, TOKEN_KECUALI, TOKEN_AKHIRNYA,
    TOKEN_GLOBAL, TOKEN_NONLOKAL,
    TOKEN_NAIKKAN, TOKEN_TIDAK, TOKEN_ADALAH, TOKEN_BUKAN,

    TOKEN_ERROR, TOKEN_EOF
} TokenType;

typedef struct {
    TokenType type;
    const char* start;
    int length;
    int line;
} Token;

// Opaque snapshot of the scanner position, for bounded lookahead.
typedef struct {
    const char* start;
    const char* current;
    int line;
    int groupingDepth;
} ScannerState;

void initScanner(const char* source);
Token scanToken();
ScannerState saveScanner();
void restoreScanner(ScannerState state);
// Re-point the scanner at a NUL-terminated buffer, keeping the reported line
// number (used to compile f-string expressions without losing their line).
void initScannerAt(const char* source, int line);

// Start of the buffer currently being scanned. Error reporting uses it to
// find the beginning of a line; NULL before the first initScanner call.
const char* scannerSourceStart(void);

#endif
