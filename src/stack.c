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

// Heurística simples para detectar padrões clássicos de overflow
static int is_pattern(uintptr_t addr) {
    return (addr == 0x4141414141414141ULL || // 'A'
            addr == 0x4242424242424242ULL || // 'B'
            addr == 0x4343434343434343ULL);  // 'C'
}

// Conversão de enum para string (legível no output)
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

// 🔬 Função principal de introspecção da stack
void walk_stack() {
    void **rbp;

    // Captura o frame pointer atual (RBP)
    __asm__("movq %%rbp, %0" : "=r"(rbp));

    printf("\n" COLOR_BOLD "[STACK INTROSPECTION]" COLOR_RESET "\n");
    printf("%-4s %-18s %-20s %-15s %-10s\n", "ID", "ADDRESS", "SYMBOL", "MODULE", "REGION");
    printf("--------------------------------------------------------------------------------\n");

    for (int i = 0; i < MAX_FRAMES; i++) {

        // 1. Validação de limites da stack
        if ((uintptr_t)rbp < stack_start || (uintptr_t)rbp > stack_end) {
            printf(COLOR_YELLOW "#%-3d [STOP] RBP out of bounds (%p)" COLOR_RESET "\n", i, rbp);
            break;
        }

        void *ret_addr = *(rbp + 1);
        void *next_rbp = *rbp;

        // 2. Validação de retorno nulo
        if (!ret_addr) {
            printf(COLOR_CYAN "#%-3d [STOP] NULL return address reached" COLOR_RESET "\n", i);
            break;
        }

        uintptr_t addr = (uintptr_t)ret_addr;

        char *symbol = resolve_symbol(ret_addr);
        const char *module = get_module_name(ret_addr);
        memory_region_t *region = find_region(addr);

        // 3. Output principal
        printf("#%-3d %-18p %-20.20s %-15.15s [%s]\n",
               i,
               ret_addr,
               symbol ? symbol : "??",
               module ? module : "unknown",
               region ? region_type_str(region->type) : "UNMAPPED");

        // 4. 🔥 Análise de segurança

        // Padrão típico de overflow
        if (is_pattern(addr)) {
            printf(COLOR_RED "  [!] ALERT: Pattern 0x%lx detected (Possible Overflow)" COLOR_RESET "\n", addr);
        }

        // Região inválida
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

        // Violação de alinhamento (ABI x86-64)
        if (addr % 8 != 0) {
            printf(COLOR_YELLOW "  [!] WARNING: Unaligned address (ABI violation)" COLOR_RESET "\n");
        }

        // Análise adicional por frame
        analyze_frame(ret_addr, rbp, next_rbp);

        // 5. Validação da cadeia de frames
        if ((uintptr_t)next_rbp <= (uintptr_t)rbp) {
            printf(COLOR_RED "#%-3d [STOP] Corrupted frame chain (Loop or Smashing)" COLOR_RESET "\n", i);
            break;
        }

        rbp = (void **)next_rbp;
    }

    printf("--------------------------------------------------------------------------------\n\n");
}

// 🚀 Entry point da análise (wrapper)
void trigger_trace() {

    printf(COLOR_CYAN "\n>> DISPARANDO ANÁLISE FORENSE (Sentinel):\n" COLOR_RESET);

    // ⚠️ Importante: garantir que o mapa de memória está carregado
    load_memory_map();  // <- assume que você tem isso implementado

    walk_stack();
}