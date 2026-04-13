#include <stdio.h>
#include <string.h>

void safe_function(){
    char buffer[16];
    printf("[Safe] Enter input: ");
    fgets(buffer, sizeof(buffer), stdin); 
    buffer[strcspn(buffer, "\n")] = 0; 
    printf(" [Safe] You entered: %s\n", buffer);
}

void intermediate() {
    safe_function();
}

int main() {
    printf("[Safe] Starting program...\n");
    intermediate();
    return 0;
}