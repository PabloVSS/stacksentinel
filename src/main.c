#include <stdio.h>
#include "stack.h"
#include "memory_map.h"

void trigger_trace() {
    walk_stack();
}

int main() {
    char input[64];

    load_memory_map();

    printf("Enter input: ");
    fgets(input, sizeof(input), stdin);

    trigger_trace();

    return 0;
}