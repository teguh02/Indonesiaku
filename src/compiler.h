#ifndef indonesiaku_compiler_h
#define indonesiaku_compiler_h

#include "object.h"
#include "vm.h"

ObjFunction* compile(const char* source);
ObjFunction* compileInModule(const char* source, ObjModule* module);
void markCompilerRoots();

// Linter (indk periksa). Off by default so normal runs are unaffected.
void compilerSetLint(bool enabled);
int  compilerLintCount(void);
void compilerLintPrint(const char* path);

#endif
