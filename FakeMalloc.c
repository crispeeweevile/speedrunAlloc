#include "FakeMalloc.h"
#include <stdio.h>
#include <string.h>

const Number MEM_SIZE = (1 << 16) - 1;
FPtr ReservedMemoryListEnd = !0;
Regg AX = 0;
Regg BX = 0;
Regg CX = 0;
Regg DX = 0;

Regg DI = 0;
Regg SI = 0;
Regg SP = 0;
Regg BP = 0;

Number readNumberFromMem(FPtr base, DataSize length) {
	if(length == 0) { return ATTEMPT_TO_ACCESS_ZERO_BYTES; } // once again, you CANNOT READ 0 BYTES
	Number nVal = 0;
	FPtr ModifiedBase = (base / length) - (base % length);
	switch(length) {
		case Byte: {
			nVal = ((uint8_t *)SystemMemory)[ModifiedBase];
			break;
		}
#if defined NUMBER_AT_LEAST_16B
		case Word: {
			nVal = ((uint16_t *)SystemMemory)[ModifiedBase];
			break;
		}
#endif
#if defined NUMBER_AT_LEAST_32B
		case DoubleWord: {
			nVal = ((uint32_t *)_SystemMemory)[ModifiedBase];
			break;
		}
#endif
#if defined NUMBER_AT_LEAST_64B
		case QuadWord: {
			nVal = ((uint64_t *)_SystemMemory)[ModifiedBase];
			break;
		}
#endif
		default: {
			return INVALID_DATA_SIZE;
		}
	}

	return nVal;
}

Error writeNumberToMem(FPtr base, DataSize length, Number value) {
	if(length == 0) { return ATTEMPT_TO_ACCESS_ZERO_BYTES; } // can't write 0 bytes
	FPtr ModifiedBase = (base / length) - (base % length);
	switch(length) {
		case Byte: {
			((uint8_t *)SystemMemory)[ModifiedBase]  = value & ((1 << (8 * length)) - 1);
			break;
		}
#if Number == uint16_t || Number == uint32_t || Number == uint64_t
		case Word: {
			((uint16_t *)SystemMemory)[ModifiedBase] = value & ((1 << (8 * length)) - 1);
			break;
		}
#elif Number == uint32_t || Number == uint64_t
		case DoubleWord: {
			((uint32_t *)SystemMemory)[ModifiedBase] = value & ((1 << (8 * length)) - 1);
			break;
		}
#elif Number == uint64_t
		case QuadWord: {
			((uint64_t *)SystemMemory)[ModifiedBase] = value & ((1 << (8 * length)) - 1);
			break;
		}
#endif
		default: {
			return INVALID_DATA_SIZE;
		}
	}

	return NO_ERROR;
}

Error ReserverSlot(AllocSlot alloc) {
	FPtr CurrAddr = 0;
	AllocSlot CurrAlloc = {0};
	for(Number offset = 0; ReservedMemoryListEnd < (MEM_SIZE - offset); offset++) {
		CurrAddr = MEM_SIZE - (((offset + 1) * SysTypeSize * 2) - 1);
		CurrAlloc.AllocBase = readNumberFromMem(CurrAddr, SysTypeSize);
		CurrAlloc.AllocSize = readNumberFromMem(CurrAddr + SysTypeSize, SysTypeSize);
	}

	return NO_ERROR;
}

Error FindUnreservedSlot(AllocSlot *holder, Number size) {

	return FUNCTION_NOT_IMPLEMENTED;
}

Error FakeMalloc(Number bytes, FPtr *holder) {
	if(bytes == 0) { return ATTEMPT_TO_ACCESS_ZERO_BYTES; } // you can't malloc 0 bytes of data...
	AllocSlot PrevAlloc = {0};
	AllocSlot CurrAlloc = {0};
	Number DistanceBetweenAllocs = 0;
	for(Number i = MEM_SIZE; i - (SysTypeSize * 2) > 0;) { // loop through memory starting at the end (where the malloc list is)
		PrevAlloc.AllocBase = readNumberFromMem(i - (SysTypeSize * 2 - 1), SysTypeSize);
		PrevAlloc.AllocSize = readNumberFromMem(i - (SysTypeSize - 1), SysTypeSize);
		i = i - (SysTypeSize * 2);
		CurrAlloc.AllocBase = readNumberFromMem(i - (SysTypeSize * 2 - 1), SysTypeSize);
		CurrAlloc.AllocSize = readNumberFromMem(i - (SysTypeSize - 1), SysTypeSize);
		DistanceBetweenAllocs = (CurrAlloc.AllocBase) - (PrevAlloc.AllocBase + PrevAlloc.AllocSize);
	}

	return NO_ERROR;
}

Error SetupSysMem(Number size) {
	SystemMemory = malloc(size * sizeof(uint8_t));
	if(SystemMemory == NULL) { return 1; }
	memset(SystemMemory, 0, size * sizeof(uint8_t));

	return NO_ERROR;
}