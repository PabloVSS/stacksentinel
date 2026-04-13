#include <stdio.h>
#include <stdint.h>
#include "stack.h"
#include "analyzer.h"
#include "symbols.h"

#define MAX_FRAMES 20

void walk_stack(){
	void **rbp;

	__asm__("movq %%rbp, %0" : "=r"(rbp));

	for(int i=0; i < MAX_FRAMES && rbp; i++) {
		void *ret_addr = *(rbp+1);

		if(!ret_addr)
			break;

		 char *symbol = resolve_symbol(ret_addr);

		printf("#%d %s (%p)\n", i, symbol, ret_addr);

		analyze_frame(ret_addr);

		rbp = (void **)(*rbp);

	}
}
