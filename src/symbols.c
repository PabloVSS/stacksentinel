#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *resolve_symbol(void *addr) {
    static char buffer[256];
    char command[512];
    snprintf(command, sizeof(command),
         "addr2line -e ./build/bin/sentinel -f -C %p",
         addr);

    FILE *fp = popen(command, "r");
    if(!fp) {
        perror("popen");
        return "unknown";
    }

    if(fgets(buffer, sizeof(buffer), fp) == NULL) {
        pclose(fp);
        return "unknown";
    }
    
    buffer[strcspn(buffer, "\n")] = 0; 

    pclose(fp);
    return buffer;
}