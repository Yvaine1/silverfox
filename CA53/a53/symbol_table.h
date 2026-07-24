#pragma once
#include <stdint.h>

typedef struct {
    uintptr_t addr;
    uintptr_t size;
    const char *obj;
    const char *name;
} symbol_t;

/* symbol table provided by generated/symbol_table.c (non-weak)
   or by symbol_table_stub.c (weak stub) on first build. */
extern const symbol_t g_symbol_table[];
extern const int g_symbol_count;
