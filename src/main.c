#define _GNU_SOURCE
#include <stdio.h>
#include "stack.h"
#include "memory_map.h"
char *gets(char *s); // força uso mesmo sendo deprecated

void vuln() {
    char buffer[16];

    printf("[Vulnerable] Enter input: ");
    gets(buffer); // use isso para exploração real

    printf("[Vunerable] You entered: %s\n", buffer);

    trigger_trace(); // mantém o processo vivo
}

int main() {
    printf("[Vulnerable] Starting program...\n");
    vuln();
    return 0;
}