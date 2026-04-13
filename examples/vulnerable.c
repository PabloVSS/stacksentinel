#include <stdio.h>

void vulnerable_function() {
    char buffer[16];
    printf("[Vunerable] Enter unput: ");
    fgets(buffer, sizeof(buffer), stdin);
    printf(" [Vunerable] You entered: %s\n", buffer);
}

void intermediate() {
    vulnerable_function();

}

int main() {
    printf("[Vulnerable] Starting program...\n");
    intermediate();
    return 0;
}

