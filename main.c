
#include "FakeMalloc.h"
#include <stdio.h>

uint8_t *SystemMemory = NULL;

int main(void) {
	SystemMemory = malloc(MEM_SIZE);
	if(SystemMemory == NULL) { return 1; }
	Number someNum = 69;
	FPtr addr  = 0;

	printf("Start\n");
	printf("Begining Write.\n");
	writeNumberToMem(addr, Word, someNum);
	printf("Finished Write. Reading\n");
	printf("%lu\n", readNumberFromMem(addr, Word));
	printf("Finished Reading.'n");
	printf("End\n");

	return 0;
}
