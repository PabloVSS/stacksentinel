#include <stdio.h>
#include <stdint.h>
#include "analyzer.h"

#define LOW_ADDR_BOUND 0X400000
#define HIGH_ADDR_BOUND 0x7fffffffffff

void analyze_frame(void *return_addr) {
	uintptr_t addr = (uintptr_t)return_addr;

	if(addr < LOW_ADDR_BOUND || addr > HIGH_ADDR_BOUND) {
		printf(" [WARNING] Suspicious return address: %p\n", return_addr);

	}

	if(addr % 8 != 0) {
		printf(" [WARNING] Unaligned return address: %p\n", return_addr);
	}
}
