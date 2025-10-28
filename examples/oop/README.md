# OOP Tests - Indonesiaku Programming Language

## Quick Start

Run all OOP tests:
```bash
powershell -ExecutionPolicy Bypass -File ../../tests/test_all.ps1
```

Run individual test:
```bash
../../indk.exe oop_test_02_simulasi.idk
```

---

## Files in This Folder

| File | Status | Purpose |
|------|--------|---------|
| `oop_test_01_basic.idk` | ❌ FAIL (Expected) | Shows what native OOP syntax would look like - not yet implemented |
| `oop_test_02_simulasi.idk` | ✅ PASS | Object simulation using functions and global namespacing |
| `oop_test_03_closure.idk` | ✅ PASS | Closure-like patterns and counter/state management |
| `oop_test_04_inheritance.idk` | ✅ PASS | Inheritance simulation with base class method calls |
| `oop_test_05_polymorphism.idk` | ✅ PASS | Duck typing and polymorphic behavior |
| `OOP_TESTING_GUIDE.md` | 📖 Guide | Comprehensive documentation of all tests and patterns |
| `README.md` | 📖 This file | Quick reference |

---

## Status Summary

```
✅ Passing:  4/5 (80%)
❌ Failing:  1/5 (expected - native classes not implemented)
━━━━━━━━━━━━━━━━━━━━━━━━
TOTAL:      19/20 (95%)
```

---

## What Works ✅

- Function-based OOP patterns
- Global state management  
- Method-like function calls
- Inheritance simulation
- Polymorphism (duck typing)
- Object composition

---

## What Doesn't Work Yet ❌

- Native `kelas` keyword (v0.3 feature)
- `diri` (self/this) keyword (v0.3 feature)
- True encapsulation
- Method chaining
- Access modifiers

---

## Key Learning

These tests prove that OOP-like behavior CAN be achieved in functional languages through:

1. **Namespacing** - Using function name prefixes (`circle_`, `rect_`, etc.)
2. **Closures** - Global variables acting as state
3. **First-class functions** - Functions treated as methods
4. **Duck typing** - Same interface, different implementations

---

**Documentation**: See `OOP_TESTING_GUIDE.md` for detailed analysis  
**Language Version**: v0.1  
**Last Updated**: October 28, 2025
