#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "memory_map.h"

#define MAX_REGIONS 256

static memory_region_t regions[MAX_REGIONS];
static int region_count = 0;

uintptr_t stack_start = 0;
uintptr_t stack_end   = 0;

void load_memory_map() {
    region_count = 0;
    stack_start = 0;
    stack_end   = 0;

    FILE *fp = fopen("/proc/self/maps", "r");
    if (!fp) return;

    char line[512];

    while (fgets(line, sizeof(line), fp)) {
        if (region_count >= MAX_REGIONS) break;

        uintptr_t start, end;
        char perms[5];
        char path[256] = "";

        int fields = sscanf(line, "%lx-%lx %4s %*s %*s %*s %[^\n]",
                            &start, &end, perms, path);

        if (fields < 3) continue;
        if (fields < 4) path[0] = '\0';

        memory_region_t *r = &regions[region_count++];
        r->start = start;
        r->end   = end;
        r->is_executable = (perms[2] == 'x');

        strncpy(r->name, path, sizeof(r->name) - 1);
        r->name[sizeof(r->name) - 1] = '\0';

        // classificação da região
        if (strstr(path, "[stack]")) {
            r->type = REGION_STACK;
            stack_start = start;
            stack_end   = end;
        }
        else if (strstr(path, "[heap]")) {
            r->type = REGION_HEAP;
        }
        else if (strstr(path, ".so")) {
            r->type = REGION_LIB;
        }
        else if (r->is_executable) {
            r->type = REGION_EXEC;
        }
        else if (path[0] == '\0') {
            r->type = REGION_ANON;
        }
        else {
            r->type = REGION_UNKNOWN;
        }
    }

    fclose(fp);

    // fallback caso stack não seja encontrada
    if (stack_start == 0 || stack_end == 0) {
        uintptr_t local;
        stack_start = (uintptr_t)&local - 0x100000;
        stack_end   = (uintptr_t)&local + 0x100000;
    }
}

memory_region_t* find_region(uintptr_t addr) {
    for (int i = 0; i < region_count; i++) {
        if (addr >= regions[i].start && addr < regions[i].end) {
            return &regions[i];
        }
    }
    return NULL;
}