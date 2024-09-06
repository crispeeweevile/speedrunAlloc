#include "FakeMalloc.h"
#include <stdio.h>
#include <string.h>

#define _ms (1 << 16) - 1
const Number MEM_SIZE = _ms;
FPtr ReservedMemoryListEnd = ~0;
AllocSlot MEM_RANGE = {0x0, _ms};

// friendly reminder these are basically depricated
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

FPtr GetModifiedBase(FPtr baseAddr, DataSize length) {
	FPtr ModifiedBase = (baseAddr / length) - (baseAddr % length);
	return ModifiedBase;
}

FPtr GetAllocSlotBase(Number offset) {
	FPtr ModifiedBase = GetModifiedBase(MEM_SIZE, SysTypeSize);
	Number ProperOffset = ((offset + 1) * (SysTypeSize * 2)) - 1;
	return ModifiedBase - ProperOffset;
}

Error GetAllocSlot(Number index, AllocSlot *holder) {
	holder->AllocBase = readNumberFromMem(GetAllocSlotBase(index), SysTypeSize);
	holder->AllocSize = readNumberFromMem(GetAllocSlotBase(index) + SysTypeSize, SysTypeSize);

	return NO_ERROR;
}

Error SetAllocSlot(Number index, AllocSlot alloc) {
	writeNumberToMem(GetAllocSlotBase(index), SysTypeSize, alloc.AllocBase);
	writeNumberToMem(GetAllocSlotBase(index) + SysTypeSize, SysTypeSize, alloc.AllocSize);

	return NO_ERROR;
}

Error GetSortedReserveList(AllocSlot **SortedReservedList, Number *LengthHolder) {
	// just to be clear, I'm happy to get feedback on this, but i am aware it's a seriously bad sorting function
	Number MemListLength = GetReserveListSize();
	AllocSlot CurrAlloc;
	AllocSlot PrevAlloc;
	AllocSlot SmalAlloc;
	bool SmalChanged = false;
	Number Blanks = 0;
	Number sorted_index = 0;

	(*SortedReservedList) = NULL;
	(*LengthHolder) = 0;

	for(Number indexy = 0; indexy < MemListLength; indexy++) {
		GetAllocSlot(indexy, &CurrAlloc);
		if(CurrAlloc.AllocSize == 0) {
			Blanks++;
		}
	}

	(*SortedReservedList) = malloc((MemListLength - Blanks) * sizeof(AllocSlot));
	if((*SortedReservedList) == NULL) { return FAILED_MALLOC; }
	memset((*SortedReservedList), 0, (MemListLength - Blanks) * sizeof(AllocSlot));
	(*LengthHolder) = (MemListLength - Blanks);

	for(Number AIndex = 0; AIndex < MemListLength; AIndex++) {
		SmalChanged = false;
		SmalAlloc.AllocBase = ~(Number)0;
		SmalAlloc.AllocSize = ~(Number)0;
		if(sorted_index > 0) {
			PrevAlloc.AllocBase = (*SortedReservedList)[sorted_index - 1].AllocBase;
			PrevAlloc.AllocSize = (*SortedReservedList)[sorted_index - 1].AllocSize;
		}
		for(Number BIndex = 0; BIndex < MemListLength; BIndex++) {
			GetAllocSlot(BIndex, &CurrAlloc);
			if(CurrAlloc.AllocSize == 0) {
				// invalid alloc
				continue;
			} else {
				if(sorted_index == 0 && BIndex == 0) {
					SmalAlloc = CurrAlloc;
					SmalChanged = true;
					continue;
				}
				if((sorted_index == 0 || CurrAlloc.AllocBase > PrevAlloc.AllocBase)) {
					if(CurrAlloc.AllocBase < SmalAlloc.AllocBase) {
						SmalAlloc = CurrAlloc;
						SmalChanged = true;
					}
				}
			}
		}

		if(SmalChanged == true) {
			(*SortedReservedList)[sorted_index].AllocBase = SmalAlloc.AllocBase;
			(*SortedReservedList)[sorted_index].AllocSize = SmalAlloc.AllocSize;
			sorted_index++;
		}
	}

	return NO_ERROR;
}

bool IsPointerInMemRange(FPtr ptr, AllocSlot alloc) {
	if(ptr >= alloc.AllocBase && ptr <= (alloc.AllocBase + alloc.AllocSize)) {
		return true;
	}
	return false;
}

Error FindMemoryHoleOfSize(Number ASize, AllocSlot *holder, AllocSlot *SortedReservedList, Number SortedLen) {
	AllocSlot *CurAlloc; // current
	AllocSlot fillerNex = {0,0};
	AllocSlot *NexAlloc = &fillerNex; // next
	Number BytesBetween;
	for(Number index = 0; index < SortedLen - 1; index++) {
		CurAlloc = &(SortedReservedList[index]);
		NexAlloc = &(SortedReservedList[index + 1]);
		BytesBetween = (NexAlloc->AllocBase - (CurAlloc->AllocBase + (CurAlloc->AllocSize - 1))) - 1;
		if((ASize - 1) <= BytesBetween) {
			holder->AllocBase = CurAlloc->AllocBase + CurAlloc->AllocSize;
			holder->AllocSize = ASize;
			return NO_ERROR;
		}
	}

	holder->AllocBase = NexAlloc->AllocBase + NexAlloc->AllocSize;
	holder->AllocSize = ASize;
	if(IsPointerInMemRange(holder->AllocBase, MEM_RANGE) != true) {
		holder->AllocBase = 0;
		holder->AllocSize = 0;
		return NO_SPACE_IN_MEMORY;
	} else if(IsPointerInMemRange(holder->AllocBase + (holder->AllocSize - 1), MEM_RANGE) != true) {
		holder->AllocBase = 0;
		holder->AllocSize = 0;
		return NO_SPACE_IN_MEMORY;
	}

	return NO_HOLE_IN_MEMORY;
}

Number GetReserveListSize() {
	return ((GetModifiedBase(MEM_SIZE, SysTypeSize) - ReservedMemoryListEnd) + 1) / (SysTypeSize * 2);
}

Error ReserveSlot(AllocSlot alloc) {
	AllocSlot CurrAlloc = {0};
	FPtr ASlotBase = 0;
	Error anError = NO_ERROR;
	for(Number offset = 0; ReservedMemoryListEnd < (MEM_SIZE - offset); offset++) {
		ASlotBase = GetAllocSlotBase(offset);
		anError = GetAllocSlot(offset, &CurrAlloc);
		if(anError != NO_ERROR) { return anError; }
		if((CurrAlloc.AllocBase == 0) && (CurrAlloc.AllocSize == 0)) {
			// invalid alloc, must be fine to save here.... (hopefully LOL)
			anError = SetAllocSlot(offset, alloc);
			if(anError != NO_ERROR) { return anError; }
			if(ASlotBase < ReservedMemoryListEnd) { ReservedMemoryListEnd = ASlotBase; }
			break;
		}
	}

	return NO_ERROR;
}

Error CanUseMemory(FPtr ptr) {
	AllocSlot ptrSlot = {0,0};
	AllocSlot curSlot;
	for(Number Index = 0; Index < GetReserveListSize(); Index++) {
		GetAllocSlot(Index, &curSlot);
		if(IsPointerInMemRange(ptr, curSlot)) {
			return NO_ERROR;
		}
	}
	if(ptrSlot.AllocSize == 0) { return BAD_MEMORY_ACCESS; }
	return NO_ERROR;
}

Error GetAllocOfPtr(FPtr ptr, AllocSlot *holder) {
	AllocSlot ptrSlot = {0,0};
	AllocSlot curSlot;
	for(Number Index = 0; Index < GetReserveListSize(); Index++) {
		GetAllocSlot(Index, &curSlot);
		if(IsPointerInMemRange(ptr, curSlot)) {
			(*holder) = curSlot;
			return NO_ERROR;
		}
	}
	if(ptrSlot.AllocSize == 0) { return BAD_MEMORY_ACCESS; }
	return NO_ERROR;
}

Error WriteToArray(FPtr array, Number index, Number value, DataSize size) {
	Error anError = NO_ERROR;
	FPtr arraySum = array + (index * size);
	AllocSlot ArrayRange;
	anError = GetAllocOfPtr(array, &ArrayRange);
	if(anError != NO_ERROR) { return anError; }
	if(IsPointerInMemRange(arraySum, ArrayRange)) {
		if(IsPointerInMemRange(arraySum + size - 1, ArrayRange)) {
			writeNumberToMem(arraySum, size, value);
		} else {
			return BAD_MEMORY_ACCESS;
		}
	} else {
		return BAD_MEMORY_ACCESS;
	}

	return NO_ERROR;
}

Error ReadFromArray(FPtr array, Number index, Number *value, DataSize size) {
	Error anError = NO_ERROR;
	FPtr arraySum = array + (index * size);
	AllocSlot ArrayRange;
	anError = GetAllocOfPtr(array, &ArrayRange);
	if(anError != NO_ERROR) { return anError; }
	if(IsPointerInMemRange(arraySum, ArrayRange)) {
		if(IsPointerInMemRange(arraySum + size - 1, ArrayRange)) {
			(*value) = readNumberFromMem(arraySum, size);
		} else {
			return BAD_MEMORY_ACCESS;
		}
	} else {
		return BAD_MEMORY_ACCESS;
	}

	return NO_ERROR;
}

Error FakeFree(FPtr ptr) {
	AllocSlot ptrSlot = {0,0};
	Number ptrSlotInd;
	AllocSlot curSlot;
	for(Number Index = 0; Index < GetReserveListSize(); Index++) {
		GetAllocSlot(Index, &curSlot);
		if(IsPointerInMemRange(ptr, curSlot)) {
			ptrSlotInd = Index;
			ptrSlot = curSlot;
			break;
		}
	}
	if(ptrSlot.AllocSize == 0) { return BAD_MEMORY_ACCESS; }
	Error anError = SetAllocSlot(ptrSlotInd, ptrSlot);
	if(anError != NO_ERROR) { return anError; }
	ptrSlot.AllocBase = 0;
	ptrSlot.AllocSize = 0;
	return NO_ERROR;
}



Error FakeMalloc(Number bytes, FPtr *holder) {
	if(bytes == 0) { return ATTEMPT_TO_ACCESS_ZERO_BYTES; } // you can't malloc 0 bytes of data...
	(*holder) = 0;
	Error anError;
	AllocSlot freeSpace = {0,0};
	AllocSlot *sortedRes;
	Number sortedLen;
	anError = GetSortedReserveList(&sortedRes, &sortedLen);
	if(anError != NO_ERROR) { return anError; }
	anError = FindMemoryHoleOfSize(bytes, &freeSpace, sortedRes, sortedLen);
	free(sortedRes); // get rid of it since no more use
	if(anError != NO_ERROR && anError != NO_HOLE_IN_MEMORY) { return anError; }
	anError = ReserveSlot(freeSpace);
	if(anError != NO_ERROR) { return anError; }
	(*holder) = freeSpace.AllocBase;

	return NO_ERROR;
}

Error SetupSysMem(Number size) {
	SystemMemory = malloc(size * sizeof(uint8_t));
	if(SystemMemory == NULL) { return FAILED_MALLOC; }
	memset(SystemMemory, 0, size * sizeof(uint8_t));
	ReservedMemoryListEnd = ((MEM_SIZE / SysTypeSize) - (MEM_SIZE % SysTypeSize));

	return NO_ERROR;
}

Error FreeWholeSystem() {
	free(SystemMemory);
	ReservedMemoryListEnd = 0;
	return NO_ERROR;
}
