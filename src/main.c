#include <stdio.h>
#include "stack.h"
#include "analyzer.h"

void vulnerable(){
	char buffer[16];
	printf("Enter input: ");
	scanf("%s", buffer);
}

void trigger_trace(){
	walk_stack();
}

int main(){
	vulnerable();
	trigger_trace();
	return 0;

}
