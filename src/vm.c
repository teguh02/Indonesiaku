#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

//#define DEBUG_TRACE_EXECUTION

#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "object.h"
#include "memory.h"
#include "vm.h"
#include "native.h"

VM vm;

static void closeUpvalues(Value* last);
static ObjInstance* newError(ObjClass* klass, const char* message);

static Value clockNative(int argCount, Value* args) {
    return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

static void resetStack() {
    vm.stackTop = vm.stack;
    vm.frameCount = 0;
    vm.openUpvalues = NULL;
    vm.tryCount = 0;
    vm.exitFrame = 0;
    vm.nativeCallDepth = 0;
}

// Print an uncaught error with its traceback and reset the VM.
static void reportError(const char* message) {
    vm.hadError = true;
    fprintf(stderr, "%s\n", message);

    // Identical consecutive frames (deep recursion) are collapsed into one
    // summary line, like Python's "[Previous line repeated N more times]".
    ObjFunction* lastFunction = NULL;
    int lastLine = -1;
    int repeated = 0;
    for (int i = vm.frameCount - 1; i >= 0; i--) {
        CallFrame* frame = &vm.frames[i];
        ObjFunction* function = frame->closure->function;
        size_t instruction = frame->ip - function->chunk.code - 1;
        int line = function->chunk.lines[instruction];
        if (function == lastFunction && line == lastLine) {
            repeated++;
            continue;
        }
        if (repeated > 0) {
            fprintf(stderr, "[baris sebelumnya diulang %d kali lagi]\n", repeated);
            repeated = 0;
        }
        lastFunction = function;
        lastLine = line;
        fprintf(stderr, "[baris %d] dalam ", line);
        if (function->name == NULL) {
            fprintf(stderr, "skrip\n");
        } else {
            fprintf(stderr, "%s()\n", function->name->chars);
        }
    }
    if (repeated > 0) {
        fprintf(stderr, "[baris sebelumnya diulang %d kali lagi]\n", repeated);
    }

    resetStack();
}

// Raise a runtime error. Inside a 'coba' block it is caught (the message
// becomes the 'kecuali' value); otherwise it is printed and aborts the run.
// Callers must still return INTERPRET_RUNTIME_ERROR so run() unwinds.
void runtimeError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(vm.runtimeErrorMessage, sizeof(vm.runtimeErrorMessage), format, args);
    va_end(args);

    if (vm.tryCount > 0 || vm.nativeCallDepth > 0) {
        // No allocation here: the error string is created in interpret()
        // once the stack has been unwound to the handler. With
        // nativeCallDepth > 0 the pending error is picked up by
        // callFromNative() and re-raised as a native error instead.
        vm.hadError = true;
        vm.runtimeErrorPending = true;
        vm.runtimeErrorClass = vm.runtimeErrorClass != NULL ? vm.runtimeErrorClass : vm.galat;
        return;
    }
    reportError(vm.runtimeErrorMessage);
}

// An internal VM failure (corrupted stack, bad jump). Never catchable.
void fatalError(const char* format, ...) {
    char message[512];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    vm.runtimeErrorPending = false;
    reportError(message);
}

// Raise a *catchable* error from a native function. Formats a message string
// and stashes it; callValue will convert it into a throwValue() unwind after
// the native returns, so `coba/kecuali` can catch it. Unlike runtimeError,
// this does not tear down the VM.
void nativeRaise(const char* format, ...) {
    char buffer[512];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    vm.nativeErrorValue = OBJ_VAL(newError(vm.galatNilai, buffer));
    vm.nativeErrorPending = true;
}

// Same as nativeRaise, but picks the error class (GalatTipe, GalatBagiNol, ...)
// so `kecuali GalatTipe` can select it.
void nativeRaiseKelas(ObjClass* klass, const char* format, ...) {
    char buffer[512];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    vm.nativeErrorValue = OBJ_VAL(newError(klass, buffer));
    vm.nativeErrorPending = true;
}

// The one instance check in the VM: used by OP_IS_INSTANCE (`kecuali GalatTipe`)
// and by the builtin adalah_instansi(). Walks the superclass chain.
bool valueIsInstanceOf(Value value, Value klass) {
    if (!IS_INSTANCE(value) || !IS_CLASS(klass)) return false;
    ObjClass* expected = AS_CLASS(klass);
    for (ObjClass* actual = AS_INSTANCE(value)->klass; actual != NULL;
         actual = actual->superclass) {
        if (actual == expected) return true;
    }
    return false;
}

static ObjClass* defineErrorClass(const char* name, ObjClass* superclass) {
    ObjString* className = copyString(name, (int)strlen(name));
    push(OBJ_VAL(className));
    ObjClass* klass = newClass(className);
    klass->superclass = superclass;
    push(OBJ_VAL(klass));
    tableSet(&vm.globals, className, OBJ_VAL(klass));
    pop();
    pop();
    return klass;
}

static ObjInstance* newError(ObjClass* klass, const char* message) {
    ObjInstance* error = newInstance(klass);
    push(OBJ_VAL(error));
    ObjString* key = copyString("pesan", 5);
    push(OBJ_VAL(key));
    ObjString* text = copyString(message, (int)strlen(message));
    push(OBJ_VAL(text));
    tableSet(&error->fields, key, OBJ_VAL(text));
    pop();
    pop();
    pop();
    return error;
}

static void defineNative(const char* name, NativeFn function) {
    push(OBJ_VAL(copyString(name, (int)strlen(name))));
    push(OBJ_VAL(newNative(function)));
    tableSet(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[1]);
    pop();
    pop();
}

void initVM() {
    resetStack();
    vm.objects = NULL;
    vm.hadError = false;
    vm.nativeErrorPending = false;
    vm.nativeErrorValue = KOSONG_VAL;
    vm.runtimeErrorPending = false;

    srand((unsigned int)time(NULL));

    vm.bytesAllocated = 0;
    vm.nextGC = 1024 * 1024;
    vm.grayCount = 0;
    vm.grayCapacity = 0;
    vm.grayStack = NULL;

    initTable(&vm.globals);
    initTable(&vm.strings);
    initTable(&vm.loadedModules);

    vm.initString = NULL;
    vm.initString = copyString("init", 4);
    vm.runtimeErrorClass = NULL;
    vm.galat = defineErrorClass("Galat", NULL);
    vm.galatAritmetika = defineErrorClass("GalatAritmetika", vm.galat);
    vm.galatBagiNol = defineErrorClass("GalatBagiNol", vm.galatAritmetika);
    ObjClass* galatPencarian = defineErrorClass("GalatPencarian", vm.galat);
    vm.galatIndeks = defineErrorClass("GalatIndeks", galatPencarian);
    defineErrorClass("GalatKunci", galatPencarian);
    vm.galatTipe = defineErrorClass("GalatTipe", vm.galat);
    vm.galatNilai = defineErrorClass("GalatNilai", vm.galat);
    vm.galatNama = defineErrorClass("GalatNama", vm.galat);
    defineErrorClass("GalatAtribut", vm.galat);
    defineErrorClass("GalatBerkas", vm.galat);
    defineErrorClass("GalatOS", vm.galat);
    defineErrorClass("GalatImpor", vm.galat);
    defineErrorClass("GalatRekursi", vm.galat);
    defineErrorClass("GalatPenegasan", vm.galat);
    defineErrorClass("HentiIterasi", vm.galat);
    defineErrorClass("GalatSintaks", vm.galat);

    defineNative("db_buka", nativeFnDbBuka);
    defineNative("db_tutup", nativeFnDbTutup);
    defineNative("db_eksekusi", nativeFnDbEksekusi);
    defineNative("db_ambil_semua", nativeFnDbAmbilSemua);
    defineNative("db_ambil_satu", nativeFnDbAmbilSatu);
    defineNative("db_baris_terpengaruh", nativeFnDbBarisTerpengaruh);
    defineNative("db_id_terakhir", nativeFnDbIdTerakhir);
    defineNative("db_mulai", nativeFnDbMulai);
    defineNative("db_komit", nativeFnDbKomit);
    defineNative("db_batal", nativeFnDbBatal);

    defineNative("jam", clockNative);
    defineNative("min", nativeFnMin);
    defineNative("max", nativeFnMax);
    defineNative("abs", nativeFnAbs);
    defineNative("akar", nativeFnAkar);
    defineNative("pangkat", nativeFnPangkat);
    defineNative("bulat", nativeFnBulat);
    defineNative("lantai", nativeFnLantai);
    defineNative("atap", nativeFnAtap);
    defineNative("acak", nativeFnAcak);

    defineNative("bulatkan", nativeFnBulatkan);
    defineNative("bagi_sisa", nativeFnBagiSisa);
    defineNative("biner", nativeFnBiner);
    defineNative("oktal", nativeFnOktal);
    defineNative("heksa", nativeFnHeksa);
    defineNative("ke_bulat", nativeFnKeBulat);
    defineNative("ke_logika", nativeFnKeLogika);
    defineNative("adalah_instansi", nativeFnAdalahInstansi);
    defineNative("bisa_dipanggil", nativeFnBisaDipanggil);
    defineNative("repr", nativeFnRepr);
    defineNative("id", nativeFnId);

    defineNative("panjang", nativeFnPanjang);
    defineNative("huruf_besar", nativeFnHurofBesar);
    defineNative("huruf_kecil", nativeFnHurofKecil);
    defineNative("ganti", nativeFnGanti);

    defineNative("tambah", nativeFnTambah);
    defineNative("hapus", nativeFnHapus);

    defineNative("kamus", nativeFnKamus);
    defineNative("punya", nativeFnPunya);
    defineNative("hapus_kunci", nativeFnHapusKunci);
    defineNative("enumerasi", nativeFnEnumerasi);
    defineNative("kunci", nativeFnKunci);

    defineNative("jenis", nativeFnJenis);
    defineNative("input", nativeFnInput);
    defineNative("henti", nativeFnHenti);

    defineNative("ke_angka", nativeFnKeAngka);
    defineNative("ke_teks", nativeFnKeTeks);
    defineNative("format", nativeFnFormat);
    defineNative("acak_bulat", nativeFnAcakBulat);

    defineNative("buka_berkas", nativeFnBukaBerkas);
    defineNative("tutup", nativeFnTutup);
    defineNative("baca", nativeFnBaca);
    defineNative("baca_baris", nativeFnBacaBaris);
    defineNative("tulis", nativeFnTulis);
    defineNative("baca_semua", nativeFnBacaSemua);
    defineNative("tulis_semua", nativeFnTulisSemua);
    defineNative("tambah_berkas", nativeFnTambahBerkas);
    defineNative("ada_berkas", nativeFnAdaBerkas);
    defineNative("hapus_berkas", nativeFnHapusBerkas);

    defineNative("waktu", nativeFnWaktu);
    defineNative("tanggal", nativeFnTanggal);
    defineNative("format_tanggal", nativeFnFormatTanggal);

    defineNative("karakter", nativeFnKarakter);
    defineNative("potong", nativeFnPotong);
    defineNative("ord", nativeFnOrd);
    defineNative("chr", nativeFnChr);
    defineNative("hapus_pada", nativeFnHapusPada);
    defineNative("sisip", nativeFnSisip);
    defineNative("rentang", nativeFnRentang);

    defineNative("jumlah", nativeFnJumlah);
    defineNative("semua", nativeFnSemua);
    defineNative("ada_yang", nativeFnAdaYang);
    defineNative("terurut", nativeFnTerurut);
    defineNative("terbalik", nativeFnTerbalik);
    defineNative("pasangkan", nativeFnPasangkan);
    defineNative("peta", nativeFnPeta);
    defineNative("saring", nativeFnSaring);

    defineNative("soket_dengar", nativeFnSoketDengar);
    defineNative("soket_terima", nativeFnSoketTerima);
    defineNative("soket_baca", nativeFnSoketBaca);
    defineNative("soket_tulis", nativeFnSoketTulis);
    defineNative("soket_tutup", nativeFnSoketTutup);
}

void freeVM() {
    freeTable(&vm.globals);
    freeTable(&vm.strings);
    freeTable(&vm.loadedModules);
    vm.initString = NULL;
    freeObjects();
}

// Expose command-line arguments to the program as a global list `argumen`.
// argv[start..argc) become string elements (start skips the script path).
void defineArgs(int argc, const char* argv[], int start) {
    ObjList* list = newList();
    push(OBJ_VAL(list));  // keep reachable during allocation
    for (int i = start; i < argc; i++) {
        Value s = OBJ_VAL(copyString(argv[i], (int)strlen(argv[i])));
        push(s);
        writeValueArray(&list->items, s);
        pop();
    }
    ObjString* name = copyString("argumen", 7);
    push(OBJ_VAL(name));
    tableSet(&vm.globals, name, OBJ_VAL(list));
    pop();  // name
    pop();  // list
}

void push(Value value) {
    if (vm.stackTop >= vm.stack + STACK_MAX) {
        runtimeError("Stack overflow: maksimum kedalaman stack %d terlampaui", STACK_MAX);
        return;
    }
    *vm.stackTop = value;
    vm.stackTop++;
}

Value pop() {
    if (vm.stackTop <= vm.stack) {
        fatalError("Stack underflow: mencoba mengambil nilai dari stack kosong");
        return KOSONG_VAL;
    }
    vm.stackTop--;
    Value result = *vm.stackTop;
    return result;
}

static Value peek(int distance) {
    return vm.stackTop[-1 - distance];
}

// names != NULL: the last names->items.count arguments on the stack were
// passed by name and must be moved to the slots their names select.
static bool call(ObjClosure* closure, int argCount, ObjList* names) {
    ObjFunction* fn = closure->function;
    int namedCount = names == NULL ? 0 : names->items.count;
    int positional = argCount - namedCount;

    if (argCount > fn->arity) {
        runtimeError("Harapkan paling banyak %d argumen tetapi mendapat %d.",
                     fn->arity, argCount);
        return false;
    }
    if (positional > fn->arity) {
        runtimeError("Terlalu banyak argumen posisional.");
        return false;
    }

    // Rebuild the parameter slots: positional args keep their place, keyword
    // args move to the slot their name selects, the rest get the marker that
    // tells the prologue to evaluate that parameter's default.
    Value* base = vm.stackTop - argCount;
    Value slots[UINT8_COUNT];
    for (int i = 0; i < fn->arity; i++) {
        slots[i] = i < positional ? base[i] : TAKTERISI_VAL;
    }

    for (int i = 0; i < namedCount; i++) {
        ObjString* wanted = AS_STRING(names->items.values[i]);
        int target = -1;
        for (int p = 0; p < fn->arity; p++) {
            if (fn->paramNames[p] == wanted) { target = p; break; }
        }
        if (target < 0) {
            vm.runtimeErrorClass = vm.galatTipe;
            runtimeError("Fungsi tidak punya parameter bernama '%s'.", wanted->chars);
            return false;
        }
        if (target < positional || !IS_TAKTERISI(slots[target])) {
            vm.runtimeErrorClass = vm.galatTipe;
            runtimeError("Parameter '%s' diisi lebih dari sekali.", wanted->chars);
            return false;
        }
        slots[target] = base[positional + i];
    }

    for (int i = 0; i < fn->requiredArity; i++) {
        if (IS_TAKTERISI(slots[i])) {
            vm.runtimeErrorClass = vm.galatTipe;
            runtimeError("Parameter wajib '%s' tidak diisi.", fn->paramNames[i]->chars);
            return false;
        }
    }

    if (vm.frameCount >= FRAMES_MAX) {
        runtimeError("Call stack overflow: kedalaman maksimal panggilan fungsi %d terlampaui. Kemungkinan rekursi tak terbatas?", FRAMES_MAX);
        return false;
    }

    for (int i = 0; i < fn->arity; i++) base[i] = slots[i];
    vm.stackTop = base + fn->arity;

    CallFrame* frame = &vm.frames[vm.frameCount++];
    frame->closure = closure;
    frame->ip = fn->chunk.code;
    frame->slots = base - 1;   // slot 0 is the callee itself
    return true;
}

static bool callValue(Value callee, int argCount, ObjList* names) {
    if (IS_OBJ(callee)) {
        switch (OBJ_TYPE(callee)) {
            case OBJ_BOUND_METHOD: {
                ObjBoundMethod* bound = AS_BOUND_METHOD(callee);
                vm.stackTop[-argCount - 1] = bound->receiver;
                return call(bound->method, argCount, names);
            }
            case OBJ_CLASS: {
                ObjClass* klass = AS_CLASS(callee);
                vm.stackTop[-argCount - 1] = OBJ_VAL(newInstance(klass));
                Value initializer;
                if (tableGet(&klass->methods, vm.initString, &initializer)) {
                    return call(AS_CLOSURE(initializer), argCount, names);
                } else if (argCount != 0) {
                    runtimeError("Harapkan 0 argumen tetapi mendapat %d.", argCount);
                    return false;
                }
                return true;
            }
            case OBJ_CLOSURE:
                return call(AS_CLOSURE(callee), argCount, names);
            case OBJ_NATIVE: {
                if (names != NULL) {
                    vm.runtimeErrorClass = vm.galatTipe;
                    runtimeError("Fungsi bawaan belum menerima argumen bernama.");
                    return false;
                }
                NativeFn native = AS_NATIVE(callee);
                Value result = native(argCount, vm.stackTop - argCount);
                vm.stackTop -= argCount + 1;
                push(result);
                return true;
            }
            default:
                break; // Non-callable object type.
        }
    }
    runtimeError("Hanya dapat memanggil fungsi dan kelas.");
    return false;
}

static bool bindMethod(ObjClass* klass, ObjString* name) {
    Value method;
    if (!tableGet(&klass->methods, name, &method)) {
        runtimeError("Properti tidak terdefinisi '%s'.", name->chars);
        return false;
    }

    ObjBoundMethod* bound = newBoundMethod(peek(0), AS_CLOSURE(method));
    pop();
    push(OBJ_VAL(bound));
    return true;
}

static bool invokeFromClass(ObjClass* klass, ObjString* name, int argCount) {
    Value method;
    if (!tableGet(&klass->methods, name, &method)) {
        runtimeError("Metode tidak terdefinisi '%s'.", name->chars);
        return false;
    }
    return call(AS_CLOSURE(method), argCount, NULL);
}

static bool invoke(ObjString* name, int argCount) {
    Value receiver = peek(argCount);

    if (!IS_INSTANCE(receiver)) {
        runtimeError("Hanya objek yang memiliki metode.");
        return false;
    }

    ObjInstance* instance = AS_INSTANCE(receiver);

    // A field holding a callable takes precedence over a method.
    Value value;
    if (tableGet(&instance->fields, name, &value)) {
        vm.stackTop[-argCount - 1] = value;
        return callValue(value, argCount, NULL);
    }

    return invokeFromClass(instance->klass, name, argCount);
}

static void defineMethod(ObjString* name) {
    Value method = peek(0);
    ObjClass* klass = AS_CLASS(peek(1));
    tableSet(&klass->methods, name, method);
    pop();
}

// Read an entire file into a NUL-terminated heap buffer, or NULL on failure.
static char* readModuleSource(const char* path) {
    FILE* file = fopen(path, "rb");
    if (file == NULL) return NULL;

    fseek(file, 0L, SEEK_END);
    long size = ftell(file);
    if (size < 0) { fclose(file); return NULL; }
    rewind(file);

    char* buffer = (char*)malloc((size_t)size + 1);
    if (buffer == NULL) { fclose(file); return NULL; }

    size_t read = fread(buffer, sizeof(char), (size_t)size, file);
    buffer[read] = '\0';
    fclose(file);
    return buffer;
}

// Compile and execute another .idk file in the current global namespace.
// Modules are cached by path so re-importing is a no-op. Returns false on
// a load/compile failure (a runtime error is raised).
static bool importModule(ObjString* path) {
    // Already loaded? Skip.
    Value dummy;
    if (tableGet(&vm.loadedModules, path, &dummy)) {
        return true;
    }

    char* source = readModuleSource(path->chars);
    if (source == NULL) {
        runtimeError("Tidak dapat mengimpor file '%s'.", path->chars);
        return false;
    }

    // Mark as loaded before running to break import cycles.
    tableSet(&vm.loadedModules, path, BOOL_VAL(true));

    ObjFunction* function = compile(source);
    free(source);
    if (function == NULL) {
        runtimeError("Gagal mengompilasi modul '%s'.", path->chars);
        return false;
    }

    push(OBJ_VAL(function));
    ObjClosure* closure = newClosure(function);
    pop();
    push(OBJ_VAL(closure));
    return call(closure, 0, NULL);
}

// Unwind to the nearest active 'coba' handler. Restores frame count and
// stack top to the try-block's entry state and points the current frame's
// ip at the 'kecuali' handler. Returns false if there is no handler.
// Handlers registered *below* vm.exitFrame belong to a frame that is waiting
// on a C stack frame (a native running a callback) and must not be unwound
// to directly — the native converts the error and re-raises it instead.
static bool throwValue(Value error) {
    (void)error;
    if (vm.tryCount == 0) return false;
    if (vm.tryHandlers[vm.tryCount - 1].frameCount <= vm.exitFrame) return false;

    TryHandler* handler = &vm.tryHandlers[--vm.tryCount];
    // Close any upvalues that live above the try block's stack top.
    closeUpvalues(handler->stackTop);
    vm.frameCount = handler->frameCount;
    vm.stackTop = handler->stackTop;

    CallFrame* frame = &vm.frames[vm.frameCount - 1];
    frame->ip = handler->handlerIp;
    return true;
}

// After a call, a native may have flagged a catchable error via nativeRaise.
// Convert it into a throwValue() unwind. Returns:
//   0 = no pending error (continue normally)
//   1 = error raised and caught (a handler is now active; caller must refresh
//       its frame pointer and push the error value for 'kecuali')
//  -1 = error raised but uncaught (caller should abort with RUNTIME_ERROR)
static int resolveNativeError() {
    if (!vm.nativeErrorPending) return 0;
    vm.nativeErrorPending = false;
    Value error = vm.nativeErrorValue;
    vm.nativeErrorValue = KOSONG_VAL;

    if (!throwValue(error)) {
        if (vm.nativeCallDepth > 0) {
            // Inside a callback invoked from C: let the outer native see it.
            vm.nativeErrorValue = error;
            vm.nativeErrorPending = true;
            return -1;
        }
        if (IS_STRING(error)) {
            runtimeError("%s", AS_CSTRING(error));
        } else if (IS_INSTANCE(error)) {
            ObjInstance* instance = AS_INSTANCE(error);
            for (int i = 0; i < instance->fields.capacity; i++) {
                Entry* entry = &instance->fields.entries[i];
                if (entry->key != NULL && strcmp(entry->key->chars, "pesan") == 0 &&
                    IS_STRING(entry->value)) {
                    runtimeError("%s", AS_CSTRING(entry->value));
                    return -1;
                }
            }
            runtimeError("Kesalahan tak tertangani dari fungsi bawaan.");
        } else {
            runtimeError("Kesalahan tak tertangani dari fungsi bawaan.");
        }
        return -1;
    }
    push(error);  // make the raised value available to 'kecuali'
    return 1;
}

static ObjUpvalue* captureUpvalue(Value* local) {
    ObjUpvalue* prevUpvalue = NULL;
    ObjUpvalue* upvalue = vm.openUpvalues;
    while (upvalue != NULL && upvalue->location > local) {
        prevUpvalue = upvalue;
        upvalue = upvalue->next;
    }

    if (upvalue != NULL && upvalue->location == local) {
        return upvalue;
    }

    ObjUpvalue* createdUpvalue = newUpvalue(local);
    createdUpvalue->next = upvalue;

    if (prevUpvalue == NULL) {
        vm.openUpvalues = createdUpvalue;
    } else {
        prevUpvalue->next = createdUpvalue;
    }

    return createdUpvalue;
}

static void closeUpvalues(Value* last) {
    while (vm.openUpvalues != NULL &&
           vm.openUpvalues->location >= last) {
        ObjUpvalue* upvalue = vm.openUpvalues;
        upvalue->closed = *upvalue->location;
        upvalue->location = &upvalue->closed;
        vm.openUpvalues = upvalue->next;
    }
}

// Does `haystack` contain `needle` as a substring? Byte comparison is exact
// for UTF-8: a valid sequence can only match at character boundaries.
static bool stringContains(ObjString* haystack, ObjString* needle) {
    if (needle->length == 0) return true;
    for (int i = 0; i + needle->length <= haystack->length; i++) {
        if (memcmp(haystack->chars + i, needle->chars, needle->length) == 0) {
            return true;
        }
    }
    return false;
}

// `item dalam container` (Python 'in'). Returns false after raising a
// runtime error when the container type does not support membership.
static bool containsValue(Value container, Value item, bool* found) {
    if (IS_LIST(container)) {
        ObjList* list = AS_LIST(container);
        *found = false;
        for (int i = 0; i < list->items.count && !*found; i++) {
            *found = valuesEqual(list->items.values[i], item);
        }
        return true;
    }
    if (IS_STRING(container)) {
        if (!IS_STRING(item)) {
            runtimeError("'dalam' pada teks memerlukan teks di sebelah kiri.");
            return false;
        }
        *found = stringContains(AS_STRING(container), AS_STRING(item));
        return true;
    }
    if (IS_DICT(container)) {
        Value ignored;
        *found = IS_STRING(item) &&
                 tableGet(&AS_DICT(container)->table, AS_STRING(item), &ignored);
        return true;
    }
    if (IS_RANGE(container)) {
        *found = IS_NUMBER(item) && rangeContains(AS_RANGE(container), AS_NUMBER(item));
        return true;
    }
    runtimeError("Operand kanan 'dalam' harus list, teks, kamus, atau rentang.");
    return false;
}

static bool chainCompare(ChainCompare op, Value left, Value right, bool* result) {
    switch (op) {
        case CHAIN_EQUAL:
            *result = valuesEqual(left, right);
            return true;
        case CHAIN_IDENTITY:
            *result = valuesIdentical(left, right);   // 'adalah' stays identity
            return true;
        case CHAIN_NOT_EQUAL:
            *result = !valuesEqual(left, right);
            return true;
        case CHAIN_NOT_IDENTITY:
            *result = !valuesIdentical(left, right);
            return true;
        case CHAIN_IN:
        case CHAIN_NOT_IN:
            if (!containsValue(right, left, result)) return false;
            if (op == CHAIN_NOT_IN) *result = !*result;
            return true;
        case CHAIN_GREATER:
        case CHAIN_GREATER_EQUAL:
        case CHAIN_LESS:
        case CHAIN_LESS_EQUAL:
            if (IS_STRING(left) && IS_STRING(right)) {
                int cmp = strcmp(AS_STRING(left)->chars, AS_STRING(right)->chars);
                *result = op == CHAIN_GREATER ? cmp > 0 :
                          op == CHAIN_GREATER_EQUAL ? cmp >= 0 :
                          op == CHAIN_LESS ? cmp < 0 : cmp <= 0;
                return true;
            }
            if (!IS_NUMBER(left) || !IS_NUMBER(right)) {
                runtimeError("Operand harus berupa angka atau dua string.");
                return false;
            }
            *result = op == CHAIN_GREATER ? AS_NUMBER(left) > AS_NUMBER(right) :
                      op == CHAIN_GREATER_EQUAL ? AS_NUMBER(left) >= AS_NUMBER(right) :
                      op == CHAIN_LESS ? AS_NUMBER(left) < AS_NUMBER(right) :
                      AS_NUMBER(left) <= AS_NUMBER(right);
            return true;
    }
    fatalError("Operator rantai perbandingan tidak valid.");
    return false;
}

// Python truthiness (K1): salah, kosong, 0, "" and empty list/kamus/rentang
// are false; everything else (including nan and every instance) is true.
bool isFalsey(Value value) {
    switch (value.type) {
        case VAL_KOSONG: return true;
        case VAL_TAKTERISI: return true;   // unreachable; be safe, not crashy
        case VAL_BOOL:   return !AS_BOOL(value);
        case VAL_NUMBER: return AS_NUMBER(value) == 0;
        case VAL_OBJ:
            switch (OBJ_TYPE(value)) {
                case OBJ_STRING: return AS_STRING(value)->length == 0;
                case OBJ_LIST:   return AS_LIST(value)->items.count == 0;
                case OBJ_DICT:   return dictLength(AS_DICT(value)) == 0;
                case OBJ_RANGE:  return rangeLength(AS_RANGE(value)) == 0;
                default:         return false;
            }
    }
    return false;
}

// Python slice bounds: resolve negative indices, clamp to the sequence, and
// honour the step's direction. Returns false when a bound is not a number or
// the step is 0; *msg then holds the error text.
static bool resolveSlice(Value startV, Value stopV, Value stepV, int length,
                         int* start, int* stop, int* step, const char** msg) {
    *step = 1;
    if (!IS_KOSONG(stepV)) {
        if (!IS_NUMBER(stepV)) { *msg = "Langkah irisan harus berupa angka."; return false; }
        *step = (int)AS_NUMBER(stepV);
        if (*step == 0) { *msg = "Langkah irisan tidak boleh nol."; return false; }
    }

    if (IS_KOSONG(startV)) {
        *start = *step > 0 ? 0 : length - 1;
    } else {
        if (!IS_NUMBER(startV)) { *msg = "Batas irisan harus berupa angka."; return false; }
        *start = (int)AS_NUMBER(startV);
        if (*start < 0) *start += length;
        if (*step > 0) {
            if (*start < 0) *start = 0;
            if (*start > length) *start = length;
        } else {
            if (*start < -1) *start = -1;
            if (*start > length - 1) *start = length - 1;
        }
    }

    if (IS_KOSONG(stopV)) {
        *stop = *step > 0 ? length : -1;
    } else {
        if (!IS_NUMBER(stopV)) { *msg = "Batas irisan harus berupa angka."; return false; }
        *stop = (int)AS_NUMBER(stopV);
        if (*stop < 0) *stop += length;
        if (*step > 0) {
            if (*stop < 0) *stop = 0;
            if (*stop > length) *stop = length;
        } else {
            if (*stop < -1) *stop = -1;
            if (*stop > length - 1) *stop = length - 1;
        }
    }
    return true;
}

static void concatenate() {
    // Keep operands on the stack (via peek) so a GC triggered by the
    // allocations below cannot free them mid-concatenation.
    ObjString* b = AS_STRING(peek(0));
    ObjString* a = AS_STRING(peek(1));

    int length = a->length + b->length;
    char* chars = ALLOCATE(char, length + 1);
    memcpy(chars, a->chars, a->length);
    memcpy(chars + a->length, b->chars, b->length);
    chars[length] = '\0';

    ObjString* result = takeString(chars, length);
    pop();
    pop();
    push(OBJ_VAL(result));
}

// daftar + daftar: a fresh list holding shallow copies of both sides.
// Both operands stay on the stack while the result grows (writeValueArray
// can allocate and trigger a GC).
static void concatenateLists() {
    ObjList* b = AS_LIST(peek(0));
    ObjList* a = AS_LIST(peek(1));
    ObjList* result = newList();
    push(OBJ_VAL(result));
    for (int i = 0; i < a->items.count; i++) writeValueArray(&result->items, a->items.values[i]);
    for (int i = 0; i < b->items.count; i++) writeValueArray(&result->items, b->items.values[i]);
    pop();              // result
    pop(); pop();       // operands
    push(OBJ_VAL(result));
}

// Shared guard for 'daftar * n' and 'teks * n'. Returns false (after raising)
// when the count is not a whole number or the result would be absurdly large.
#define REPEAT_MAX 65535

static bool repeatCount(Value value, int* out) {
    if (!IS_NUMBER(value)) {
        vm.runtimeErrorClass = vm.galatTipe;
        runtimeError("Pengali harus berupa angka bulat.");
        return false;
    }
    double n = AS_NUMBER(value);
    if (n != floor(n)) {
        vm.runtimeErrorClass = vm.galatTipe;
        runtimeError("Pengali harus berupa angka bulat.");
        return false;
    }
    if (n > REPEAT_MAX) {
        vm.runtimeErrorClass = vm.galatNilai;
        runtimeError("Hasil pengulangan terlalu besar (maksimum %d).", REPEAT_MAX);
        return false;
    }
    *out = n < 0 ? 0 : (int)n;   // negative repeats give an empty result, as in Python
    return true;
}

static InterpretResult run() {
    CallFrame* frame = &vm.frames[vm.frameCount - 1];

#define READ_BYTE() (*frame->ip++)

#define READ_SHORT() \
    (frame->ip += 2, \
    (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))

#define READ_CONSTANT() \
    (frame->closure->function->chunk.constants.values[READ_BYTE()])

#define READ_STRING() AS_STRING(READ_CONSTANT())

#define BINARY_OP(valueType, op) \
    do { \
      if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
        runtimeError("Operand harus berupa angka."); \
        return INTERPRET_RUNTIME_ERROR; \
      } \
      double b = AS_NUMBER(pop()); \
      double a = AS_NUMBER(pop()); \
      push(valueType(a op b)); \
    } while (false)

    for (;;) {
        // A runtime error may have been raised by push()/pop() or a native
        // function without a direct return path. Abort cleanly instead of
        // continuing to execute with a corrupted stack.
        if (vm.hadError) return INTERPRET_RUNTIME_ERROR;

#ifdef DEBUG_TRACE_EXECUTION
        int ipOffset = (int)(frame->ip - frame->closure->function->chunk.code);
        fprintf(stderr, "[LOOP START] ip=%d, frameCount=%d\n", ipOffset, vm.frameCount);
        printf("          ");
        for (Value* slot = vm.stack; slot < vm.stackTop; slot++) {
            printf("[ ");
            printValue(*slot);
            printf(" ]");
        }
        printf("\n");
        disassembleInstruction(&frame->closure->function->chunk, ipOffset);
#endif

        uint8_t instruction;
        switch (instruction = READ_BYTE()) {
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                push(constant);
                break;
            }
            case OP_KOSONG: push(KOSONG_VAL); break;
            case OP_BENAR: push(BOOL_VAL(true)); break;
            case OP_SALAH: push(BOOL_VAL(false)); break;
            case OP_POP: pop(); break;
            case OP_GET_LOCAL: {
                uint8_t slot = READ_BYTE();
                push(frame->slots[slot]);
                break;
            }
            case OP_SET_LOCAL: {
                uint8_t slot = READ_BYTE();
                frame->slots[slot] = peek(0);
                // Don't pop - leave value on stack (assignment is an expression)
                break;
            }
            case OP_GET_GLOBAL: {
                ObjString* name = READ_STRING();
                Value value;
                if (!tableGet(&vm.globals, name, &value)) {
                    runtimeError("Variabel tidak terdefinisi '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                push(value);
                break;
            }
            case OP_DEFINE_GLOBAL: {
                ObjString* name = READ_STRING();
                tableSet(&vm.globals, name, peek(0));
                pop();
                break;
            }
            case OP_SET_GLOBAL: {
                ObjString* name = READ_STRING();
                if (tableSet(&vm.globals, name, peek(0))) {
                    tableDelete(&vm.globals, name);
                    runtimeError("Variabel tidak terdefinisi '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                // Don't pop - leave value on stack (assignment is an expression)
                break;
            }
            case OP_EQUAL: {
                Value b = pop();
                Value a = pop();
                push(BOOL_VAL(valuesEqual(a, b)));
                break;
            }
            case OP_IDENTICAL: {
                Value b = pop();
                Value a = pop();
                push(BOOL_VAL(valuesIdentical(a, b)));
                break;
            }
            case OP_GREATER: {
                if (IS_STRING(peek(0)) && IS_STRING(peek(1))) {
                    ObjString* b = AS_STRING(pop());
                    ObjString* a = AS_STRING(pop());
                    push(BOOL_VAL(strcmp(a->chars, b->chars) > 0));
                } else {
                    BINARY_OP(BOOL_VAL, >);
                }
                break;
            }
            case OP_LESS: {
                if (IS_STRING(peek(0)) && IS_STRING(peek(1))) {
                    ObjString* b = AS_STRING(pop());
                    ObjString* a = AS_STRING(pop());
                    push(BOOL_VAL(strcmp(a->chars, b->chars) < 0));
                } else {
                    BINARY_OP(BOOL_VAL, <);
                }
                break;
            }
            case OP_ADD: {
                if (IS_STRING(peek(0)) && IS_STRING(peek(1))) {
                    concatenate();
                } else if (IS_LIST(peek(0)) && IS_LIST(peek(1))) {
                    concatenateLists();
                } else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
                    double b = AS_NUMBER(pop());
                    double a = AS_NUMBER(pop());
                    push(NUMBER_VAL(a + b));
                } else {
                    vm.runtimeErrorClass = vm.galatTipe;
                    runtimeError("Operand '+' harus dua angka, dua teks, atau dua daftar.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_SUBTRACT: BINARY_OP(NUMBER_VAL, -); break;
            case OP_MULTIPLY: {
                // daftar * n, n * daftar, teks * n, n * teks, angka * angka
                Value b = peek(0);
                Value a = peek(1);
                if (IS_NUMBER(a) && IS_NUMBER(b)) {
                    double y = AS_NUMBER(pop());
                    double x = AS_NUMBER(pop());
                    push(NUMBER_VAL(x * y));
                    break;
                }

                Value seq = IS_NUMBER(b) ? a : b;    // the repeated operand
                Value times = IS_NUMBER(b) ? b : a;
                if (!IS_LIST(seq) && !IS_STRING(seq)) {
                    vm.runtimeErrorClass = vm.galatTipe;
                    runtimeError("Operand '*' harus dua angka, atau daftar/teks dengan angka.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                int n;
                if (!repeatCount(times, &n)) return INTERPRET_RUNTIME_ERROR;

                if (IS_LIST(seq)) {
                    ObjList* src = AS_LIST(seq);
                    if ((long)src->items.count * n > REPEAT_MAX) {
                        vm.runtimeErrorClass = vm.galatNilai;
                        runtimeError("Hasil pengulangan terlalu besar (maksimum %d).", REPEAT_MAX);
                        return INTERPRET_RUNTIME_ERROR;
                    }
                    ObjList* result = newList();
                    push(OBJ_VAL(result));   // root while writeValueArray allocates
                    for (int r = 0; r < n; r++) {
                        for (int i = 0; i < src->items.count; i++) {
                            writeValueArray(&result->items, src->items.values[i]);
                        }
                    }
                    pop();
                    vm.stackTop -= 2;
                    push(OBJ_VAL(result));
                } else {
                    ObjString* src = AS_STRING(seq);
                    if ((long)src->length * n > REPEAT_MAX) {
                        vm.runtimeErrorClass = vm.galatNilai;
                        runtimeError("Hasil pengulangan terlalu besar (maksimum %d).", REPEAT_MAX);
                        return INTERPRET_RUNTIME_ERROR;
                    }
                    int length = src->length * n;
                    char* chars = ALLOCATE(char, length + 1);
                    for (int r = 0; r < n; r++) {
                        memcpy(chars + (size_t)r * src->length, src->chars, src->length);
                    }
                    chars[length] = '\0';
                    ObjString* result = takeString(chars, length);
                    vm.stackTop -= 2;
                    push(OBJ_VAL(result));
                }
                break;
            }
            case OP_DIVIDE: {
                if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) {
                    runtimeError("Operand harus berupa angka.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                if (AS_NUMBER(peek(0)) == 0) {
                    vm.runtimeErrorClass = vm.galatBagiNol;
                    runtimeError("Pembagian dengan nol.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                double b = AS_NUMBER(pop());
                double a = AS_NUMBER(pop());
                push(NUMBER_VAL(a / b));
                break;
            }
            case OP_MODULO: {
                if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) {
                    runtimeError("Operand harus berupa angka.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                if (AS_NUMBER(peek(0)) == 0) {
                    vm.runtimeErrorClass = vm.galatBagiNol;
                    runtimeError("Pembagian dengan nol.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                double b = AS_NUMBER(pop());
                double a = AS_NUMBER(pop());
                // Python semantics: the result takes the sign of the divisor,
                // so -7 % 3 == 2 (C's fmod would give -1).
                double r = fmod(a, b);
                if (r == 0) {
                    r = 0;  // avoid printing "-0" for -6 % 3
                } else if ((r < 0) != (b < 0)) {
                    r += b;
                }
                push(NUMBER_VAL(r));
                break;
            }
            case OP_IN: {
                bool found;
                if (!containsValue(peek(0), peek(1), &found)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                vm.stackTop -= 2;
                push(BOOL_VAL(found));
                break;
            }
            case OP_CHAIN_COMPARE: {
                ChainCompare op = (ChainCompare)READ_BYTE();
                Value right = peek(0);
                bool result;
                if (!chainCompare(op, peek(1), right, &result)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                vm.stackTop -= 2;
                push(right);
                push(BOOL_VAL(result));
                break;
            }
            case OP_CHAIN_CLEANUP: {
                Value result = pop();
                pop();
                push(result);
                break;
            }
            case OP_FLOOR_DIVIDE: {
                if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) {
                    runtimeError("Operand harus berupa angka.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                if (AS_NUMBER(peek(0)) == 0) {
                    vm.runtimeErrorClass = vm.galatBagiNol;
                    runtimeError("Pembagian dengan nol.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                double b = AS_NUMBER(pop());
                double a = AS_NUMBER(pop());
                // Rounds toward negative infinity, like Python: -7 // 2 == -4.
                push(NUMBER_VAL(floor(a / b)));
                break;
            }
            case OP_POWER: {
                if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) {
                    runtimeError("Operand harus berupa angka.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                if (AS_NUMBER(peek(1)) == 0 && AS_NUMBER(peek(0)) < 0) {
                    runtimeError("0 tidak dapat dipangkatkan dengan bilangan negatif.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                double b = AS_NUMBER(pop());
                double a = AS_NUMBER(pop());
                push(NUMBER_VAL(pow(a, b)));
                break;
            }
            case OP_NOT:
                push(BOOL_VAL(isFalsey(pop())));
                break;
            case OP_NEGATE:
                if (!IS_NUMBER(peek(0))) {
                    runtimeError("Operand harus berupa angka.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                push(NUMBER_VAL(-AS_NUMBER(pop())));
                break;
            case OP_PRINT: {
                int count = READ_BYTE();
                for (int i = count - 1; i >= 0; i--) {
                    printValue(peek(i));
                    if (i > 0) printf(" ");
                }
                printf("\n");
                vm.stackTop -= count;
                break;
            }
            case OP_LOOP: {
                uint16_t offset = READ_SHORT();
                // Ensure we don't jump to invalid bytecode
                uint8_t* newIp = frame->ip - offset;
                if (newIp < frame->closure->function->chunk.code ||
                    newIp >= frame->closure->function->chunk.code + frame->closure->function->chunk.count) {
                    fatalError("Invalid loop jump offset: offset=%d, ip was %td",
                                 offset, (frame->ip - frame->closure->function->chunk.code));
                    return INTERPRET_RUNTIME_ERROR;
                }
                frame->ip = newIp;
                break;
            }
            case OP_JUMP: {
                uint16_t offset = READ_SHORT();
                frame->ip += offset;
                break;
            }
            case OP_JUMP_IF_FALSE: {
                uint16_t offset = READ_SHORT();
                if (isFalsey(peek(0))) {
                    frame->ip += offset;
                }
                break;
            }
            case OP_CALL: {
                int argCount = READ_BYTE();
                if (!callValue(peek(argCount), argCount, NULL)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                int ne = resolveNativeError();
                if (ne == -1) return INTERPRET_RUNTIME_ERROR;
                frame = &vm.frames[vm.frameCount - 1];
                break;
            }
            case OP_CALL_BERNAMA: {
                int argCount = READ_BYTE();
                ObjList* names = AS_LIST(READ_CONSTANT());
                if (!callValue(peek(argCount), argCount, names)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                int ne = resolveNativeError();
                if (ne == -1) return INTERPRET_RUNTIME_ERROR;
                frame = &vm.frames[vm.frameCount - 1];
                break;
            }
            case OP_ISI_BAWAAN: {
                // Skip this parameter's default expression when the caller
                // already supplied a value for the slot.
                uint8_t slot = READ_BYTE();
                uint16_t offset = READ_SHORT();
                if (!IS_TAKTERISI(frame->slots[slot])) {
                    frame->ip += offset;
                }
                break;
            }
            case OP_CLOSURE: {
                ObjFunction* function = AS_FUNCTION(READ_CONSTANT());
                ObjClosure* closure = newClosure(function);
                push(OBJ_VAL(closure));
                for (int i = 0; i < closure->upvalueCount; i++) {
                    uint8_t isLocal = READ_BYTE();
                    uint8_t index = READ_BYTE();
                    if (isLocal) {
                        closure->upvalues[i] = captureUpvalue(frame->slots + index);
                    } else {
                        closure->upvalues[i] = frame->closure->upvalues[index];
                    }
                }
                break;
            }
            case OP_GET_UPVALUE: {
                uint8_t slot = READ_BYTE();
                push(*frame->closure->upvalues[slot]->location);
                break;
            }
            case OP_SET_UPVALUE: {
                uint8_t slot = READ_BYTE();
                *frame->closure->upvalues[slot]->location = peek(0);
                break;
            }
            case OP_CLOSE_UPVALUE:
                closeUpvalues(vm.stackTop - 1);
                pop();
                break;
            case OP_BUILD_LIST: {
                int count = READ_BYTE();
                ObjList* list = newList();
                // Keep the list reachable while appending (append may
                // grow the item array and trigger a GC).
                push(OBJ_VAL(list));
                for (int i = count; i >= 1; i--) {
                    writeValueArray(&list->items, peek(i));
                }
                pop();                       // the list
                vm.stackTop -= count;        // the elements
                push(OBJ_VAL(list));
                break;
            }
            case OP_BUILD_DICT: {
                int count = READ_BYTE();
                // Stack: key1, val1, ... keyN, valN
                ObjDict* dict = newDict();
                // Keep the dict reachable: dictSet can grow both the table and
                // the insertion-order key array, either of which may GC.
                push(OBJ_VAL(dict));
                for (int i = count; i >= 1; i--) {
                    Value key = peek(i * 2);        // +1 for the dict itself
                    Value value = peek(i * 2 - 1);
                    if (!IS_STRING(key)) {
                        vm.runtimeErrorClass = vm.galatTipe;
                        runtimeError("Kunci kamus harus berupa teks.");
                        return INTERPRET_RUNTIME_ERROR;
                    }
                    dictSet(dict, AS_STRING(key), value);
                }
                pop();                          // the dict
                vm.stackTop -= count * 2;       // the pairs
                push(OBJ_VAL(dict));
                break;
            }
            case OP_INDEX_GET: {
                if (IS_LIST(peek(1)) && IS_NUMBER(peek(0))) {
                    int given = (int)AS_NUMBER(pop());
                    ObjList* list = AS_LIST(pop());
                    int index = given < 0 ? given + list->items.count : given;
                    if (index < 0 || index >= list->items.count) {
                        vm.runtimeErrorClass = vm.galatIndeks;
                        runtimeError("Indeks list di luar jangkauan (indeks %d, panjang %d).",
                                     given, list->items.count);
                        return INTERPRET_RUNTIME_ERROR;
                    }
                    push(list->items.values[index]);
                } else if (IS_RANGE(peek(1)) && IS_NUMBER(peek(0))) {
                    double index = AS_NUMBER(pop());
                    ObjRange* range = AS_RANGE(pop());
                    double length = rangeLength(range);
                    if (index < 0) index += length;
                    if (index != floor(index) || index < 0 || index >= length) {
                        runtimeError("Indeks rentang di luar jangkauan (indeks %.15g, panjang %.15g).",
                                     index, length);
                        return INTERPRET_RUNTIME_ERROR;
                    }
                    push(NUMBER_VAL(range->start + index * range->step));
                } else if (IS_DICT(peek(1)) && IS_NUMBER(peek(0))) {
                    int index = (int)AS_NUMBER(pop());
                    ObjDict* dict = AS_DICT(pop());
                    if (index < 0 || index >= dictLength(dict)) {
                        runtimeError("Indeks kamus di luar jangkauan.");
                        return INTERPRET_RUNTIME_ERROR;
                    }
                    push(dictKeyAt(dict, index));
                } else if (IS_DICT(peek(1)) && IS_STRING(peek(0))) {
                    ObjString* key = AS_STRING(pop());
                    ObjDict* dict = AS_DICT(pop());
                    Value value;
                    if (!tableGet(&dict->table, key, &value)) {
                        push(KOSONG_VAL);
                    } else {
                        push(value);
                    }
                } else if (IS_STRING(peek(1)) && IS_NUMBER(peek(0))) {
                    // Index by character; keep the string on the stack while
                    // copyString allocates (a GC could otherwise free it).
                    int index = (int)AS_NUMBER(peek(0));
                    ObjString* str = AS_STRING(peek(1));
                    int given = index;
                    if (index < 0) index += str->charCount;
                    if (index < 0 || index >= str->charCount) {
                        runtimeError("Indeks string di luar jangkauan (indeks %d, panjang %d).",
                                     given, str->charCount);
                        return INTERPRET_RUNTIME_ERROR;
                    }
                    int offset = utf8ByteOffset(str, index);
                    ObjString* ch = copyString(str->chars + offset, utf8CharBytes(str, offset));
                    vm.stackTop -= 2;
                    push(OBJ_VAL(ch));
                } else {
                    runtimeError("Pengindeksan tidak valid: gunakan list[angka], kamus[string], atau teks[angka].");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_INDEX_SET: {
                // Stack: target, index/key, value
                if (IS_LIST(peek(2)) && IS_NUMBER(peek(1))) {
                    Value value = pop();
                    int given = (int)AS_NUMBER(pop());
                    ObjList* list = AS_LIST(pop());
                    int index = given < 0 ? given + list->items.count : given;
                    if (index < 0 || index >= list->items.count) {
                        vm.runtimeErrorClass = vm.galatIndeks;
                        runtimeError("Indeks list di luar jangkauan (indeks %d, panjang %d).",
                                     given, list->items.count);
                        return INTERPRET_RUNTIME_ERROR;
                    }
                    list->items.values[index] = value;
                    push(value);
                } else if (IS_DICT(peek(2)) && IS_STRING(peek(1))) {
                    // Keep operands on the stack while tableSet runs — it can
                    // grow the table and trigger a GC that would otherwise
                    // free the dict/key/value.
                    ObjDict* dict = AS_DICT(peek(2));
                    ObjString* key = AS_STRING(peek(1));
                    Value value = peek(0);
                    dictSet(dict, key, value);
                    pop();  // value
                    pop();  // key
                    pop();  // dict
                    push(value);
                } else {
                    if (IS_STRING(peek(2))) {
                        runtimeError("String tidak dapat diubah; gunakan potong()/gabungan untuk membuat string baru.");
                    } else {
                        runtimeError("Pengindeksan tidak valid: gunakan list[angka] atau kamus[string].");
                    }
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_SLICE: {
                // Stack: target, start, stop, step (omitted parts are kosong).
                // Operands stay on the stack while the result allocates.
                Value target = peek(3);
                int start, stop, step;
                const char* msg = NULL;

                if (IS_LIST(target)) {
                    ObjList* src = AS_LIST(target);
                    if (!resolveSlice(peek(2), peek(1), peek(0), src->items.count,
                                      &start, &stop, &step, &msg)) {
                        vm.runtimeErrorClass = vm.galatNilai;
                        runtimeError("%s", msg);
                        return INTERPRET_RUNTIME_ERROR;
                    }
                    ObjList* out = newList();
                    push(OBJ_VAL(out));   // root: writeValueArray may GC
                    for (int i = start; step > 0 ? i < stop : i > stop; i += step) {
                        writeValueArray(&out->items, src->items.values[i]);
                    }
                    pop();
                    vm.stackTop -= 4;
                    push(OBJ_VAL(out));
                } else if (IS_STRING(target)) {
                    ObjString* src = AS_STRING(target);
                    if (!resolveSlice(peek(2), peek(1), peek(0), src->charCount,
                                      &start, &stop, &step, &msg)) {
                        vm.runtimeErrorClass = vm.galatNilai;
                        runtimeError("%s", msg);
                        return INTERPRET_RUNTIME_ERROR;
                    }
                    // Build the byte buffer first; src stays rooted on the stack.
                    int count = 0;
                    for (int i = start; step > 0 ? i < stop : i > stop; i += step) count++;
                    // Worst case: every character is 4 UTF-8 bytes.
                    char* buffer = ALLOCATE(char, (size_t)count * 4 + 1);
                    int len = 0;
                    for (int i = start; step > 0 ? i < stop : i > stop; i += step) {
                        int offset = utf8ByteOffset(src, i);
                        int bytes = utf8CharBytes(src, offset);
                        memcpy(buffer + len, src->chars + offset, bytes);
                        len += bytes;
                    }
                    ObjString* out = copyString(buffer, len);
                    FREE_ARRAY(char, buffer, (size_t)count * 4 + 1);
                    vm.stackTop -= 4;
                    push(OBJ_VAL(out));
                } else {
                    vm.runtimeErrorClass = vm.galatTipe;
                    runtimeError("Irisan hanya untuk daftar dan teks.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_LIST_LEN: {
                // Length of what 'untuk' iterates; elements are then read
                // with OP_INDEX_GET, so any indexable sequence works.
                Value seq = pop();
                if (IS_LIST(seq)) {
                    push(NUMBER_VAL(AS_LIST(seq)->items.count));
                } else if (IS_RANGE(seq)) {
                    push(NUMBER_VAL(rangeLength(AS_RANGE(seq))));
                } else if (IS_DICT(seq)) {
                    push(NUMBER_VAL(dictLength(AS_DICT(seq))));
                } else if (IS_STRING(seq)) {
                    push(NUMBER_VAL(AS_STRING(seq)->charCount));
                } else {
                    runtimeError("'untuk ... dalam' hanya dapat mengiterasi list, teks, atau rentang.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_CLASS:
                push(OBJ_VAL(newClass(READ_STRING())));
                break;
            case OP_INHERIT: {
                Value superclass = peek(1);
                if (!IS_CLASS(superclass)) {
                    runtimeError("Kelas induk harus berupa kelas.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                ObjClass* subclass = AS_CLASS(peek(0));
                subclass->superclass = AS_CLASS(superclass);
                tableAddAll(&AS_CLASS(superclass)->methods, &subclass->methods);
                pop();  // subclass
                break;
            }
            case OP_METHOD:
                defineMethod(READ_STRING());
                break;
            case OP_GET_PROPERTY: {
                if (!IS_INSTANCE(peek(0))) {
                    runtimeError("Hanya objek yang memiliki properti.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                ObjInstance* instance = AS_INSTANCE(peek(0));
                ObjString* name = READ_STRING();

                Value value;
                if (tableGet(&instance->fields, name, &value)) {
                    pop();  // instance
                    push(value);
                    break;
                }

                if (!bindMethod(instance->klass, name)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_SET_PROPERTY: {
                if (!IS_INSTANCE(peek(1))) {
                    runtimeError("Hanya objek yang memiliki properti.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                ObjInstance* instance = AS_INSTANCE(peek(1));
                tableSet(&instance->fields, READ_STRING(), peek(0));
                Value value = pop();
                pop();  // instance
                push(value);
                break;
            }
            case OP_INVOKE: {
                ObjString* method = READ_STRING();
                int argCount = READ_BYTE();
                if (!invoke(method, argCount)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                int ne = resolveNativeError();
                if (ne == -1) return INTERPRET_RUNTIME_ERROR;
                frame = &vm.frames[vm.frameCount - 1];
                break;
            }
            case OP_GET_SUPER: {
                ObjString* name = READ_STRING();
                ObjClass* superclass = AS_CLASS(pop());
                if (!bindMethod(superclass, name)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_SUPER_INVOKE: {
                ObjString* method = READ_STRING();
                int argCount = READ_BYTE();
                ObjClass* superclass = AS_CLASS(pop());
                if (!invokeFromClass(superclass, method, argCount)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                frame = &vm.frames[vm.frameCount - 1];
                break;
            }
            case OP_TRY_BEGIN: {
                uint16_t offset = READ_SHORT();
                if (vm.tryCount == TRY_MAX) {
                    runtimeError("Terlalu banyak blok 'coba' bersarang.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                TryHandler* handler = &vm.tryHandlers[vm.tryCount++];
                handler->handlerIp = frame->ip + offset;
                handler->frameCount = vm.frameCount;
                handler->stackTop = vm.stackTop;
                break;
            }
            case OP_TRY_END:
                // Normal completion of a 'coba' block: unregister its handler.
                if (vm.tryCount > 0) vm.tryCount--;
                break;
            case OP_THROW: {
                Value error = pop();
                if (!throwValue(error)) {
                    if (vm.nativeCallDepth > 0) {
                        // Inside a callback invoked from C: hand the value
                        // back to the native, which re-raises it.
                        vm.nativeErrorValue = error;
                        vm.nativeErrorPending = true;
                        return INTERPRET_RUNTIME_ERROR;
                    }
                    // No handler: report and abort.
                    if (IS_STRING(error)) {
                        runtimeError("Kesalahan tak tertangani: %s", AS_CSTRING(error));
                    } else {
                        runtimeError("Kesalahan tak tertangani.");
                    }
                    return INTERPRET_RUNTIME_ERROR;
                }
                frame = &vm.frames[vm.frameCount - 1];
                push(error);  // make the raised value available to 'kecuali'
                break;
            }
            case OP_IS_INSTANCE: {
                Value target = pop();
                push(BOOL_VAL(valueIsInstanceOf(peek(0), target)));
                break;
            }
            case OP_IMPORT: {
                ObjString* path = AS_STRING(pop());
                bool alreadyLoaded = false;
                Value dummy;
                if (tableGet(&vm.loadedModules, path, &dummy)) {
                    alreadyLoaded = true;
                }
                if (!importModule(path)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                if (alreadyLoaded) {
                    // No module frame was pushed; supply the value that the
                    // trailing OP_POP expects.
                    push(KOSONG_VAL);
                } else {
                    // A new frame is now executing the module.
                    frame = &vm.frames[vm.frameCount - 1];
                }
                break;
            }
            case OP_RETURN: {
                Value result = pop();
                closeUpvalues(frame->slots);
                // 'kembalikan' from inside 'coba' skips OP_TRY_END; drop the
                // handlers registered by this frame so a later error cannot
                // jump back into a function that has already returned.
                while (vm.tryCount > 0 &&
                       vm.tryHandlers[vm.tryCount - 1].frameCount >= vm.frameCount) {
                    vm.tryCount--;
                }
                vm.frameCount--;
                if (vm.frameCount == vm.exitFrame) {
                    if (vm.exitFrame == 0) {
                        pop();
                        return INTERPRET_OK;
                    }
                    // Returning to a native that invoked this callable from C:
                    // leave just the result where the callee sat.
                    vm.stackTop = frame->slots;
                    push(result);
                    return INTERPRET_OK;
                }

                vm.stackTop = frame->slots;
                push(result);
                frame = &vm.frames[vm.frameCount - 1];
                break;
            }
        }
    }

#undef READ_BYTE
#undef READ_SHORT
#undef READ_CONSTANT
#undef READ_STRING
#undef BINARY_OP
}

// Call an Indonesiaku callable from inside a native. The callee and its
// arguments must already be on the VM stack. On success they are replaced by
// the result (also stored in *result). Any error is converted into a pending
// native error, so the native's own caller unwinds normally.
bool callFromNative(int argCount, Value* result) {
    int frameBefore = vm.frameCount;
    Value* calleeSlot = vm.stackTop - argCount - 1;
    Value callee = *calleeSlot;

    // Raised while this flag is up, runtimeError only records the message;
    // we convert it to a catchable native error below.
    vm.nativeCallDepth++;

    if (!callValue(callee, argCount, NULL)) {
        vm.nativeCallDepth--;
        // callValue already raised via runtimeError; it was only recorded
        // (nativeCallDepth/tryCount), convert it to a native error below.
        if (vm.runtimeErrorPending) {
            vm.runtimeErrorPending = false;
            vm.hadError = false;
            ObjClass* klass = vm.runtimeErrorClass != NULL ? vm.runtimeErrorClass : vm.galat;
            vm.runtimeErrorClass = NULL;
            vm.nativeErrorValue = OBJ_VAL(newError(klass, vm.runtimeErrorMessage));
            vm.nativeErrorPending = true;
        }
        vm.stackTop = calleeSlot;
        return false;
    }

    if (vm.frameCount == frameBefore) {
        // A native callee: callValue already left the result on the stack.
        vm.nativeCallDepth--;
        if (vm.nativeErrorPending) { vm.stackTop = calleeSlot; return false; }
        *result = pop();
        return true;
    }

    int savedExit = vm.exitFrame;
    vm.exitFrame = frameBefore;
    InterpretResult r = run();
    vm.nativeCallDepth--;
    vm.exitFrame = savedExit;

    if (r != INTERPRET_OK) {
        if (vm.runtimeErrorPending) {
            vm.runtimeErrorPending = false;
            vm.hadError = false;
            ObjClass* klass = vm.runtimeErrorClass != NULL ? vm.runtimeErrorClass : vm.galat;
            vm.runtimeErrorClass = NULL;
            vm.nativeErrorValue = OBJ_VAL(newError(klass, vm.runtimeErrorMessage));
            vm.nativeErrorPending = true;
        }
        // Drop any frames and handlers the aborted callback left behind.
        vm.frameCount = frameBefore;
        while (vm.tryCount > 0 && vm.tryHandlers[vm.tryCount - 1].frameCount > frameBefore) {
            vm.tryCount--;
        }
        vm.stackTop = calleeSlot;
        return false;
    }

    *result = pop();
    return true;
}

InterpretResult interpret(const char* source) {
    vm.hadError = false;
    ObjFunction* function = compile(source);
    if (function == NULL) return INTERPRET_COMPILE_ERROR;

    push(OBJ_VAL(function));
    ObjClosure* closure = newClosure(function);
    pop();
    push(OBJ_VAL(closure));
    call(closure, 0, NULL);

    InterpretResult result = run();
    // A runtimeError() inside 'coba' stops run() with a pending error:
    // unwind to the handler, hand it the message, and keep running.
    while (result == INTERPRET_RUNTIME_ERROR && vm.runtimeErrorPending) {
        vm.runtimeErrorPending = false;
        vm.hadError = false;
        if (!throwValue(KOSONG_VAL)) {  // unreachable: a handler existed
            reportError(vm.runtimeErrorMessage);
            break;
        }
        const char* message = vm.runtimeErrorMessage;
        ObjClass* klass = vm.runtimeErrorClass != NULL ? vm.runtimeErrorClass : vm.galat;
        vm.runtimeErrorClass = NULL;
        push(OBJ_VAL(newError(klass, message)));
        result = run();
    }
    return result;
}
