#ifndef indonesiaku_common_h
#define indonesiaku_common_h

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Uncomment untuk debugging
// #define DEBUG_PRINT_CODE
// #define DEBUG_TRACE_EXECUTION

// Uncomment untuk debugging garbage collector
// #define DEBUG_STRESS_GC   // Jalankan GC pada setiap alokasi
// #define DEBUG_LOG_GC      // Cetak aktivitas GC

#define UINT8_COUNT (UINT8_MAX + 1)

// Application version
#define INDK_VERSION "0.3.0"

#endif
