# Why oop_test_01_basic.idk Fails - Detailed Explanation

## Quick Answer

**oop_test_01_basic.idk FAILS INTENTIONALLY** ✅

The test demonstrates features that are **NOT YET IMPLEMENTED** in v0.1 but are planned for v0.3.

---

## The Error

When running `oop_test_01_basic.idk`, you see:

```
[baris 6] Kesalahan di 'kelas': Harapkan ekspresi.
[baris 11] Kesalahan di 'diri': Harapkan ekspresi.
[baris 16] Kesalahan di 'diri': Harapkan ekspresi.
[baris 23] Kesalahan di akhir: Harapkan '}' setelah blok.
```

---

## Why It Fails

### Root Cause: Missing Language Features

#### 1. The `kelas` Keyword
```indonesiaku
kelas Manusia {  # Line 6 - ERROR HERE
    ...
}
```

**Issue**: 
- ✅ Scanner RECOGNIZES the `kelas` keyword
- ❌ Compiler DOES NOT IMPLEMENT class syntax
- Result: Compiler expects an expression, not a class definition

**Status**: Not implemented in v0.1 → Planned for v0.3

---

#### 2. The `diri` Keyword (self/this)
```indonesiaku
diri.nama = nama_baru  # Lines 11, 16 - ERROR HERE
cetak(diri.nama)       # Line 17
```

**Issues**:
- ✅ Scanner RECOGNIZES the `diri` keyword  
- ❌ Compiler DOES NOT SUPPORT property access syntax (`diri.nama`)
- ❌ Compiler DOES NOT SUPPORT property assignment (`diri.nama = value`)
- Result: Compiler tries to parse `diri` as an expression, not as object reference

**Status**: Not implemented in v0.1 → Planned for v0.3

---

#### 3. Object Property System
```indonesiaku
kelas Manusia {
    nama = kosong      # Property declaration
    umur = kosong      # Property declaration
}
```

**Issue**: 
- v0.1 has NO object property system
- Classes don't exist, so properties can't be declared
- Global variables are the only "state" mechanism

**Status**: Not implemented in v0.1 → Planned for v0.3

---

## Comparison: FAIL vs PASS

### TEST 1 - FAILS ❌ (oop_test_01_basic.idk)

Uses **native OOP syntax** (v0.3 feature):
```indonesiaku
kelas Manusia {           # ❌ kelas keyword
    nama = kosong
    
    fungsi __init__(nama_baru) {
        diri.nama = nama_baru  # ❌ diri.property syntax
    }
    
    fungsi sapa() {
        cetak("Halo", diri.nama)  # ❌ diri.property access
    }
}

manusia1 = Manusia("Budi")    # ❌ Constructor call
manusia1.sapa()               # ❌ Method call on object
```

**Why it fails**: All of this syntax is v0.3-only

---

### TEST 2 - PASSES ✅ (oop_test_02_simulasi.idk)

Uses **functional OOP patterns** (v0.1 compatible):
```indonesiaku
# Global variables as properties
manusia_nama = kosong
manusia_umur = kosong

# Init function as constructor
fungsi manusia_init(nama, umur) {    # ✅ Regular function
    manusia_nama = nama              # ✅ Global variable
    manusia_umur = umur              # ✅ Global variable
}

# Method function
fungsi manusia_sapa() {               # ✅ Regular function
    cetak("Halo", manusia_nama)       # ✅ Access global var
    cetak("Umur:", manusia_umur)
}

# Usage
manusia_init("Budi", 25)              # ✅ Function call
manusia_sapa()                         # ✅ Function call
```

**Why it passes**: All features used are available in v0.1

---

## What's Missing for Native OOP

### Language Features Required (v0.3)

| Feature | Status | Example |
|---------|--------|---------|
| `kelas` keyword | ❌ Not impl | `kelas MyClass { }` |
| Class methods | ❌ Not impl | `fungsi method() { }` inside class |
| Class properties | ❌ Not impl | `nama = kosong` inside class |
| `diri` keyword | ❌ Not impl | Reference to instance |
| Property access | ❌ Not impl | `obj.property` syntax |
| Instance creation | ❌ Not impl | `obj = ClassName(args)` |
| Method calls | ❌ Not impl | `obj.method()` syntax |
| Constructor | ❌ Not impl | `__init__` or similar |

---

## Design Decision: Intentional Failure

### Why We Keep test_01_basic.idk Failing

```
Purpose: Show the DIFFERENCE between
  ✅ What CAN be done in v0.1 (tests 2-5)
  ❌ What CANNOT be done in v0.1 (test 1)
```

This serves as:
1. **Documentation** - Shows planned syntax for v0.3
2. **Roadmap** - Clear indication what's next
3. **Validation** - Proves our gap analysis is accurate
4. **Education** - Shows developers the language evolution

---

## Timeline

### v0.1 (Current)
- ✅ Functions
- ✅ Global variables
- ✅ Control flow (if, while)
- ✅ Arithmetic/logic operators
- ❌ Native OOP (test 1 fails)

### v0.2 (Planned)
- ⏳ For...in loops
- ⏳ Arrays
- ⏳ Break/continue
- ⏳ String operations

### v0.3 (Planned)
- ⏳ Classes (`kelas`)
- ⏳ Instances
- ⏳ Methods
- ⏳ Properties
- ⏳ Inheritance
- ⏳ `diri` (self) keyword
- ⏳ Access modifiers

---

## Summary

**Q: Why does oop_test_01_basic.idk fail?**

**A**: Because it uses v0.3 syntax features that don't exist yet in v0.1:
- `kelas` - Class definition (not implemented)
- `diri` - Object reference (not implemented)
- `obj.property` - Property access (not implemented)

**This is EXPECTED and INTENTIONAL.**

The test proves that:
1. ✅ OOP patterns CAN be simulated with functions
2. ❌ Native OOP CANNOT be used in v0.1
3. 🎯 Native OOP is clearly needed for v0.3

---

**Test Result: FAIL (Expected)**
- **Category**: OOP Test Suite
- **Status**: Documentation Valid ✅
- **Impact**: No impact on v0.1 functionality
- **Next Steps**: Implement OOP for v0.3
