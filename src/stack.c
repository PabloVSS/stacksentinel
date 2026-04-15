#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "stack.h"
#include "analyzer.h"
#include "symbols.h"
#include "memory_map.h"

#define MAX_FRAMES 15
#define COLOR_RESET   "\x1b[0m"
#define COLOR_RED     "\x1b[31m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_BOLD    "\x1b[1m"

static int is_pattern(uintptr_t addr) {
    return (addr == 0x4141414141414141ULL ||
            addr == 0x4242424242424242ULL ||
            addr == 0x4343434343434343ULL);
}

static const char* region_type_str(region_type_t type) {
    switch (type) {
        case REGION_STACK: return "STACK";
        case REGION_HEAP:  return "HEAP";
        case REGION_EXEC:  return "EXEC";
        case REGION_LIB:   return "LIB";
        case REGION_ANON:  return "ANON";
        default:           return "UNKNOWN";
    }
}

void walk_stack() {
    void **rbp;
    __asm__("movq %%rbp, %0" : "=r"(rbp));

    printf("\n" COLOR_BOLD "[STACK INTROSPECTION]" COLOR_RESET "\n");
    printf("%-4s %-18s %-20s %-15s %-10s\n", "ID", "ADDRESS", "SYMBOL", "MODULE", "REGION");
    printf("--------------------------------------------------------------------------------\n");

    for (int i = 0; i < MAX_FRAMES; i++) {
        // 1. Validação de Limites de RBP
        if ((uintptr_t)rbp < stack_start || (uintptr_t)rbp > stack_end) {
            printf(COLOR_YELLOW "#%-3d [STOP] RBP out of bounds (%p)" COLOR_RESET "\n", i, rbp);
            break;
        }

        void *ret_addr = *(rbp + 1);
        void *next_rbp = *rbp;

        // 2. Validação de Endereço de Retorno
        if (!ret_addr) {
            printf(COLOR_CYAN "#%-3d [STOP] NULL return address reached" COLOR_RESET "\n", i);
            break;
        }

        uintptr_t addr = (uintptr_t)ret_addr;
        char *symbol = resolve_symbol(ret_addr);
        const char *module = get_module_name(ret_addr);
        memory_region_t *region = find_region(addr);

        // 3. Print da Linha Principal (Formato Tabela)
        printf("#%-3d %-18p %-20.20s %-15.15s [%s]\n", 
               i, ret_addr, symbol, module, region ? region_type_str(region->type) : "UNMAPPED");

        // 4. 🔥 ANÁLISE DE SEGURANÇA (Deteção de Anomalias)
        
        // Padrão de Overflow (Heurística de Exploit)
        if (is_pattern(addr)) {
            printf(COLOR_RED "  [!] ALERT: Pattern 0x%lx detected (Possible Overflow)" COLOR_RESET "\n", addr);
        }

        // Integridade de Memória e Execução
        if (!region) {
            printf(COLOR_RED "  [!] WARNING: Return address is NOT MAPPED" COLOR_RESET "\n");
        } else {
            if (!region->is_executable) {
                printf(COLOR_YELLOW "  [!] WARNING: Non-executable region (%s)" COLOR_RESET "\n", 
                       region->name[0] ? region->name : "anonymous");
            }
            if (region->type == REGION_HEAP || region->type == REGION_ANON) {
                printf(COLOR_RED "  [!] CRITICAL: Execution outside code segments" COLOR_RESET "\n");
            }
        }

        // Check de Alinhamento (ABI Compliance)
        if (addr % 8 != 0) {
            printf(COLOR_YELLOW "  [!] WARNING: Unaligned address (ABI violation)" COLOR_RESET "\n");
        }

        // Análise Forense Adicional
        analyze_frame(ret_addr, rbp, next_rbp);

        // 5. Validação da Corrente (Chain Integrity)
        if ((uintptr_t)next_rbp <= (uintptr_t)rbp) {
            printf(COLOR_RED "#%-3d [STOP] Corrupted frame chain (Loop or Smashing)" COLOR_RESET "\n", i);
            break;
        }

        rbp = (void **)next_rbp;
    }
    printf("--------------------------------------------------------------------------------\n\n");
}