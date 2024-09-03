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

	printf("Start\n");
	anE = FakeMalloc(22, &testArray);
	if(anE != NO_ERROR) { return anE; }
	printf("tstA:%lx\n", testArray);
	anE = WriteToArray(testArray, anInd, anNum + 0x10, Byte);
	if(anE != NO_ERROR) { return anE; }
	anE = ReadFromArray(testArray, anInd, &anNum, Byte);
	if(anE != NO_ERROR) { return anE; }
	printf("aNum:%lx\n", anNum);
	printf("End\n");

	return 0;
}
