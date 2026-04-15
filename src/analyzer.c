#include <stdio.h>
#include <stdint.h>
#include "analyzer.h"
#include "memory_map.h"

void analyze_frame(void *return_addr, void *current_rbp, void *next_rbp) {
    uintptr_t addr = (uintptr_t)return_addr;
    uintptr_t rbp = (uintptr_t)current_rbp;
    uintptr_t next = (uintptr_t)next_rbp;

    memory_region_t *region = find_region(addr);

    // Classificação da região
    if (region) {
        printf(" [%s]", region->is_executable ? "EXEC" : "NON-EXEC");

        if (region->name[0]) {
            printf(" (%s)", region->name);
        }
    } else {
        printf(" [UNKNOWN]");
    }

    // Validação executável
    if (!region || !region->is_executable) {
        printf("\n  [WARNING] Return address outside executable region");
    }

    // Validação de encadeamento
    if (next <= rbp) {
        printf("\n  [WARNING] Broken stack frame chain");
    }

    // Validação de stack bounds
    if (rbp < stack_start || rbp > stack_end) {
        printf("\n  [WARNING] RBP outside stack bounds");
    }

    printf("\n");
}