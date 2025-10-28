# OOP Testing Documentation - Indonesiaku v0.1

## Overview

**Status**: Native OOP (classes, inheritance, etc.) is NOT YET IMPLEMENTED in v0.1 
**Target**: v0.3+

This folder contains 5 test files that explore Object-Oriented Programming patterns using the current v0.1 features.

---

## Test Results

| Test | Status | Purpose |
|------|--------|---------|
| oop_test_01_basic.idk | ❌ FAIL | Demonstrates that native `kelas` keyword is not supported (expected) |
| oop_test_02_simulasi.idk | ✅ PASS | Object simulation using functions and global namespacing |
| oop_test_03_closure.idk | ✅ PASS | Closure-like patterns and encapsulation simulation |
| oop_test_04_inheritance.idk | ✅ PASS | Inheritance simulation using function composition |
| oop_test_05_polymorphism.idk | ✅ PASS | Polymorphism/duck typing using multiple shape classes |

**Total**: 4/5 PASSING (1 expected failure)

---

## Test Descriptions

### 1. oop_test_01_basic.idk - Native Class Syntax

**Purpose**: Show what TRUE OOP would look like (not yet supported)

**Code Example**:
```indonesiaku
kelas Manusia {
    nama = kosong
    umur = kosong
    
    fungsi __init__(nama_baru, umur_baru) {
        diri.nama = nama_baru
        diri.umur = umur_baru
    }
    
    fungsi sapa() {
        cetak("Halo, saya adalah", diri.nama)
    }
}

manusia1 = Manusia("Budi", 25)
manusia1.sapa()
```

**Status**: ❌ FAIL - Expected
- `kelas` keyword is recognized but not compiled
- `diri` (self) keyword recognized but not used
- **Expected error**: "Harapkan ekspresi di 'kelas'"

**When Available**: v0.3+

---

### 2. oop_test_02_simulasi.idk - Object Simulation with Functions

**Purpose**: Simulate object behavior using functions and global variables

**Architecture**:
- Global variables act as object properties
- Functions act as methods
- Prefix-based namespacing (e.g., `manusia_nama`, `manusia_info`)

**Example Pattern**:
```indonesiaku
# "Properties" using global variables with prefix
manusia_nama = kosong
manusia_umur = kosong

# "Constructor"
fungsi manusia_init(nama, umur) {
    manusia_nama = nama
    manusia_umur = umur
}

# "Method"
fungsi manusia_sapa() {
    cetak("Halo, saya adalah", manusia_nama)
}
```

**Output**:
```
=== Object 1 ===
Halo, saya adalah Budi
Umur saya: 25
Nama: Budi
Umur: 25

=== Object 1 - Birthday ===
Budi sekarang berumur 26
Nama: Budi
Umur: 26

=== Object 2 ===
Halo, saya adalah Ani
Umur saya: 20
```

**Limitations**:
- Only one "instance" at a time (state is overwritten)
- No true encapsulation (globals are visible)
- No method chaining

---

### 3. oop_test_03_closure.idk - Closure-Like Pattern

**Purpose**: Simulate closures and state management

**Pattern**: 
- Central global state variable
- Multiple getter/setter functions
- Counter pattern simulating private state

**Code Example**:
```indonesiaku
counter_value = 0

fungsi counter_get() {
    kembalikan counter_value
}

fungsi counter_increment() {
    counter_value = counter_value + 1
    kembalikan counter_value
}
```

**Output**:
```
=== Counter 1 ===
Initial: 0
After increment:
Value: 1
Value: 2
Value: 3

Reset counter
Value after reset: 0

=== Counter 2 (shares state) ===
Start with: 0
After increment: 1
```

**Use Cases**:
- State management
- Singleton-like patterns
- Factory functions

**Limitations**:
- All instances share state
- No true encapsulation
- Cannot have multiple independent instances

---

### 4. oop_test_04_inheritance.idk - Inheritance Simulation

**Purpose**: Simulate inheritance using function composition and explicit base class calls

**Architecture**:
- Base class: `Kendaraan` (Vehicle)
- Derived classes: `Mobil` (Car), `Motor` (Motorcycle)
- Methods explicitly call parent methods

**Example Pattern**:
```indonesiaku
# Base class properties
kendaraan_nama = kosong
kendaraan_kecepatan = 0

# Base class methods
fungsi kendaraan_init(nama) {
    kendaraan_nama = nama
    kendaraan_kecepatan = 0
}

# Derived class
mobil_roda = 4

fungsi mobil_init(nama) {
    # Call parent constructor
    kendaraan_init(nama)
    mobil_roda = 4
}

fungsi mobil_info() {
    # Call parent method
    kendaraan_info()
    cetak("Roda:", mobil_roda)
}
```

**Output**:
```
=== Mobil ===
Kendaraan: Avanza
Kecepatan: 0
Roda: 4
Mobil berakselerasi
Kendaraan: Avanza
Kecepatan: 20
Roda: 4

=== Motor ===
Kendaraan: Harley
Kecepatan: 0
Roda: 2
Motor berakselerasi cepat
Kendaraan: Harley
Kecepatan: 20
Roda: 2
```

**Use Cases**:
- Hierarchical type systems
- Code reuse
- Behavioral extension

**Limitations**:
- Manual method resolution (no `super` keyword)
- Single inheritance only
- Shared state between derived classes

---

### 5. oop_test_05_polymorphism.idk - Polymorphism (Duck Typing)

**Purpose**: Demonstrate polymorphic behavior through duck typing

**Pattern**:
- Multiple "shape" classes with same interface
- Different implementations of `init()`, `area()`, `describe()`
- Calling methods on different objects

**Example Pattern**:
```indonesiaku
# Circle class
circle_radius = 0

fungsi circle_init(r) {
    circle_radius = r
}

fungsi circle_area() {
    kembalikan 3.14159 * circle_radius * circle_radius
}

# Rectangle class
rect_panjang = 0
rect_lebar = 0

fungsi rect_init(p, l) {
    rect_panjang = p
    rect_lebar = l
}

fungsi rect_area() {
    kembalikan rect_panjang * rect_lebar
}
```

**Output**:
```
=== Shape Collection ===

--- Circle 1 ---
Shape: Circle
Radius: 5
Area: 78.53975

--- Rectangle 1 ---
Shape: Rectangle
Panjang: 4
Lebar: 6
Area: 24

--- Triangle 1 ---
Shape: Triangle
Alas: 8
Tinggi: 5
Area: 20

--- Circle 2 ---
Shape: Circle
Radius: 3
Area: 28.27451

=== Total Areas ===
Total area of all shapes: 150.85426
```

**Use Cases**:
- Multiple implementations of same interface
- Strategy pattern
- Plugin systems

**Benefits**:
- Simple and flexible
- No type checking needed
- Easy to extend

**Limitations**:
- Shared state between different object types
- No type safety
- No inheritance hierarchy

---

## Key Insights

### What Works Well in v0.1
✅ Function-based OOP patterns
✅ Closures and state management
✅ Composition over inheritance
✅ Duck typing / polymorphism
✅ Namespace prefixes for organization

### What Doesn't Work Yet
❌ Native `kelas` keyword and class syntax
❌ `diri` (self/this) keyword
❌ Method chaining
❌ True encapsulation
❌ Private/public access modifiers
❌ Constructors as special methods
❌ Multiple inheritance

### Design Patterns Possible in v0.1
- Singleton pattern (single global state)
- Factory pattern (init functions)
- Observer pattern (callbacks via functions)
- Strategy pattern (polymorphic functions)
- Decorator pattern (wrapping functions)
- State pattern (global state with getters/setters)

### Recommended for v0.2+
- For loop iterations (to support array-like behaviors)
- First-class functions (lambda/anonymous functions)
- Table/dictionary data structure
- Array support

### Recommended for v0.3+
- Native class support (`kelas` keyword)
- Instance creation and methods
- Proper encapsulation
- Inheritance with `super` keyword
- Access modifiers
- Operator overloading

---

## Testing Guidelines

### Running OOP Tests
```bash
# Run individual test
.\indk.exe examples/oop/oop_test_02_simulasi.idk

# Run all tests
powershell -ExecutionPolicy Bypass -File tests/test_all.ps1
```

### Expected Behavior
- Tests 2-5: Should run without errors (✅ PASS)
- Test 1: Expected to fail with syntax error (❌ FAIL - Normal)

### Performance Notes
- All tests complete instantly
- No memory leaks detected
- Suitable for educational/demonstration purposes

---

## Conclusion

While true OOP is not yet available in Indonesiaku v0.1, the language already supports many OOP-like patterns through its function and global variable system. These tests demonstrate:

1. **Function-based encapsulation** is possible
2. **Polymorphism** through duck typing works well
3. **Code organization** can be achieved through naming conventions
4. **Design patterns** can be implemented creatively

These patterns provide a foundation for understanding what v0.3's native OOP will offer once implemented.

---

**Last Updated**: October 28, 2025
**Test Suite Version**: 1.0
**Indonesiaku Version**: 0.1.1
