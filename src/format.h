#ifndef indonesiaku_format_h
#define indonesiaku_format_h

// indk rapikan. Returns a malloc'd string the caller must free, or NULL on a
// scan error. Works from the token stream only -- no parser, no AST.
char* formatSource(const char* source);

#endif
