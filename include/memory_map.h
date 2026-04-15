#ifndef MEMORY_MAP_H
#define MEMORY_MAP_H

#include <stdint.h>

typedef enum {
    REGION_STACK,
    REGION_HEAP,
    REGION_EXEC,
    REGION_LIB,
    REGION_ANON,
    REGION_UNKNOWN
} region_type_t;

typedef struct {
    uintptr_t start;
    uintptr_t end;
    int is_executable;
    region_type_t type;
    char name[256];
} memory_region_t;

// API
void load_memory_map();
memory_region_t* find_region(uintptr_t addr);

// limites da stack
extern uintptr_t stack_start;
extern uintptr_t stack_end;

#endif