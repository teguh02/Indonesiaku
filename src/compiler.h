#ifndef indonesiaku_compiler_h
#define indonesiaku_compiler_h

#include "object.h"
#include "vm.h"

ObjFunction* compile(const char* source);
ObjFunction* compileInModule(const char* source, ObjModule* module);
void markCompilerRoots();

#endif
