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
	INVALID_DATA_SIZE, // DataSize provided wasn't Byte, Word, DoubleWord, or QuadWord
	FUNCTION_NOT_IMPLEMENTED, // WIP
	NO_SPACE_IN_MEMORY, // SystemMemory is full
	FAILED_MALLOC, // a fatal error, meaning that a call to the actual malloc() failed
	BAD_MEMORY_ACCESS, // you weren't allowed to access the memory
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

// I was planning to use these, but ultimately decided to pretend they didn't exist. I just never removed em lol
extern Regg AX;
extern Regg BX;
extern Regg CX;
extern Regg DX;

extern Regg DI;
extern Regg SI;
extern Regg SP;
extern Regg BP;

// many of these aren't actually intended for use by the user of this allocator, but I left them here, mostly due to being lazy
Number readNumberFromMem(FPtr base, DataSize length);

Error writeNumberToMem(FPtr base, DataSize length, Number value);

Error ReserveSlot(AllocSlot alloc);

FPtr GetModifiedBase(FPtr baseAddr, DataSize length);

FPtr GetAllocSlotBase(Number offset);

Number GetReserveListSize();

Error GetAllocSlot(Number index, AllocSlot *holder);

Error SetAllocSlot(Number index, AllocSlot alloc);

Error GetSortedReserveList(AllocSlot **SortedReservedList, Number *LengthHolder);

bool IsPointerInMemRange(FPtr ptr, AllocSlot alloc);

Error FindMemoryHoleOfSize(Number ASize, AllocSlot *holder, AllocSlot *SortedReservedList, Number SortedLen);

// The following are intended for users
 // fills value with the number pointed to by array[index] with size
Error ReadFromArray(FPtr array, Number index, Number *value, DataSize size);

// writes a number of size to the array at index
Error WriteToArray(FPtr array, Number index, Number value, DataSize size);

// frees fake memory via fake pointer
Error FakeFree(FPtr ptr);

// allocates memory in the fake system, and fills holder with a pointer to the fake array
Error FakeMalloc(Number bytes, FPtr *holder);

// For when you are done using the library (deinitialize)
Error FreeWholeSystem();

// initializes the system for usage.
Error SetupSysMem(Number size);
