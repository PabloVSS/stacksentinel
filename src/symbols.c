#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <limits.h> // Para PATH_MAX

#include "symbols.h"

// Aumentamos para 256 para evitar truncamento de símbolos longos (mangled names)
#define MAX_SYMBOL_LEN 256

char* resolve_symbol(void *addr) {
    // Usamos static, mas com tamanho suficiente para a maioria dos nomes de funções
    static char sym_buffer[MAX_SYMBOL_LEN];

    if (!addr) {
        return "NULL";
    }

    Dl_info info;

    // dladdr tenta encontrar o símbolo mais próximo do endereço fornecido
    if (dladdr(addr, &info) && info.dli_sname) {
        // snprintf garante que não haverá overflow se o nome for maior que 256
        snprintf(sym_buffer, sizeof(sym_buffer), "%s", info.dli_sname);
    } else {
        snprintf(sym_buffer, sizeof(sym_buffer), "??");
    }

    return sym_buffer;
}

const char* get_module_name(void *addr) {
    // No Linux, o tamanho máximo de um caminho de arquivo é 4096 (PATH_MAX)
    static char path_buffer[PATH_MAX];

    Dl_info info;

    if (dladdr(addr, &info) && info.dli_fname) {
        // Copiamos o caminho completo do módulo (ex: /usr/lib/libc.so.6)
        snprintf(path_buffer, sizeof(path_buffer), "%s", info.dli_fname);
    } else {
        snprintf(path_buffer, sizeof(path_buffer), "unknown");
    }

    return path_buffer;
}