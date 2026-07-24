/* weak stub for symbol table.
   This file is compiled in every build and provides a weak placeholder.
   When generated/symbol_table.c (non-weak) is present, linker will use that instead.
*/

#include "symbol_table.h"

/* IAR supports __weak; GCC/clang use __attribute__((weak)).
   Use both to be portable. */
#if defined(__ICCARM__) || defined(__IAR_SYSTEMS_ICC__)
  #define WEAK __weak
#else
  #define WEAK __attribute__((weak))
#endif

WEAK const symbol_t g_symbol_table[] = { };
WEAK const int g_symbol_count = 0;
