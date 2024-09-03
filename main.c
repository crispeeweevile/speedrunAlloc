#include "FakeMalloc.h"
#include <stdio.h>

uint8_t *SystemMemory = NULL;

int main(void) {
	SetupSysMem(MEM_SIZE);
	if(SystemMemory == NULL) { return 1; }
	Error anE;
	FPtr testArray;
	Number anNum = 0x10;
	Number anInd = 0;
	Number testASize = 22;
	
	printf("Start\n");
	anE = FakeMalloc(testASize, &testArray);
	if(anE != NO_ERROR) { return anE; }
	printf("tstA:%lx\n", testArray);
	for(Number index = 0; index < testASize; index++) {
		anE = WriteToArray(testArray, index, index, Byte);
		if(anE != NO_ERROR) { return anE; }
		if(index % 2 == 0) {
			anE = ReadFromArray(testArray, index / 2, &anNum, Word);
			if(anE != NO_ERROR) { return anE; }
			printf("index:%lu, aNum:%lx\n", index, anNum);
		}
	}
	printf("End\n");

	return 0;
}
