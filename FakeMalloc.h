#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#define u16 16
#define u32 32
#define u64 64
// depending on what SYS is, the system will assume 16b, 32b, or 64b
#define SYS u16


#if SYS == u32
#define NUMBER_AT_LEAST_16B
#define NUMBER_AT_LEAST_32B
#define SysType uint32_t
#elif SYS == u64
#define NUMBER_AT_LEAST_16B
#define NUMBER_AT_LEAST_32B
#define NUMBER_AT_LEAST_64B
#define SysType uint64_t
#else
#define NUMBER_AT_LEAST_16B
#define SysType uint16_t
#endif // SysType == uint16_t

typedef SysType Number;
typedef SysType FPtr;
typedef SysType Regg;

#define SysTypeSize sizeof(SysType)

enum _ErrorTypes {
	NO_ERROR, // yay :)
	ATTEMPT_TO_ACCESS_ZERO_BYTES, // do you not know how to sanitize or smthn?
	INVALID_DATA_SIZE,
	FUNCTION_NOT_IMPLEMENTED,
	NO_SPACE_IN_MEMORY,
	FAILED_MALLOC,
	BAD_MEMORY_ACCESS,
	NO_HOLE_IN_MEMORY, // not really a fatal error, just a "warning" telling you that there was no good hole (either none at all, or just wrong size)
};
typedef enum _ErrorTypes Error;

enum _DataSize {
	Byte = 1,

	Word = 2,

	DoubleWord = 4,

	QuadWord = 8,
};

typedef enum _DataSize  DataSize;

struct _AllocSlot {
	FPtr   AllocBase;
	Number AllocSize;
};
typedef struct _AllocSlot AllocSlot;

extern uint8_t  *SystemMemory;
extern FPtr ReservedMemoryListEnd;
extern const Number MEM_SIZE;
extern Regg AX;
extern Regg BX;
extern Regg CX;
extern Regg DX;

extern Regg DI;
extern Regg SI;
extern Regg SP;
extern Regg BP;


Number readNumberFromMem(FPtr base, DataSize length);

Error writeNumberToMem(FPtr base, DataSize length, Number value);

Error FakeMalloc(Number bytes, FPtr *holder);

Error SetupSysMem(Number size);

Error ReserveSlot(AllocSlot alloc);

FPtr GetModifiedBase(FPtr baseAddr, DataSize length);

FPtr GetAllocSlotBase(Number offset);

Number GetReserveListSize();

Error GetAllocSlot(Number index, AllocSlot *holder);

Error SetAllocSlot(Number index, AllocSlot alloc);

Error GetSortedReserveList(AllocSlot **SortedReservedList, Number *LengthHolder);

bool IsPointerInMemRange(FPtr ptr, AllocSlot alloc);

Error FakeFree(FPtr ptr);

Error FindMemoryHoleOfSize(Number ASize, AllocSlot *holder, AllocSlot *SortedReservedList, Number SortedLen);

Error ReadFromArray(FPtr array, Number index, Number *value, DataSize size);

Error WriteToArray(FPtr array, Number index, Number value, DataSize size);
