
// ****************************************************************************
//
//                            PSRAM Memory Allocator
//
// ****************************************************************************
// PicoLibSDK - Alternative SDK library for Raspberry Pico and RP2040
// Copyright (c) 2023 Miroslav Nemecek, Panda38@seznam.cz, hardyplotter2@gmail.com
// 	https://github.com/Panda381/PicoLibSDK
//	https://www.breatharian.eu/hw/picolibsdk/index_en.html
//	https://github.com/pajenicko/picopad
//	https://picopad.eu/en/
// License:
//	This source code is freely available for any purpose, including commercial.
//	It is possible to take and modify the code or parts of it, without restriction.

/*
How this allocator works
------------------------
The memory is divided into 16 KB pages. Memory blocks of 1 Byte to 16 KB are allocated
with a normalized size of power of 2, starting with a block of 8 bytes. In this case,
each page is divided into small blocks of the same size. Memory blocks larger than
16 KB are allocated in 16 KB page increments.

At the beginning of the memory is the page descriptor array. Each descriptor refers to
a single 16 KB page and contains information about whether the page is free, whether
it contains small blocks of size 8 B to 16 KB, or whether the page is allocated for
a large block (larger than 16 KB). There is an array in RAM containing lists of free
blocks and pages.

At initialization, page descriptors are marked as free pages. All memory pages are
appended to the free page list (block descriptor with index 0).

When allocating a block size up to 16KB, the requested size is rounded to the power
of 2 (="block type") and the list of free blocks corresponding to the size type is
used. If the list contains a free block of the appropriate size, the block is detached
from the list and returned as an allocated block. If no free block is available, a new
page is extracted from the list of free pages, divided into blocks of the required size
and all blocks are appended to the list of free blocks of the appropriate size. Then
the first block is detached and returned as an allocated block.

When allocating a block of size greater than 16 KB, the page descriptor list is
traversed to find a sequence of free pages of the desired size. When found, the pages
are detached from the free page list and returned as an allocated block.

When releasing a block, the block size is first determined. From the block address, the
address of the block descriptor is derived and the block type (representing the order
of magnitude of the block size to the power of 2) is determined. For a long block over
16 KB, the page list is traversed to determine the block size and the pages marked as
"continuation of long block" are counted. If a short block up to 16KB is being released,
it is included in the list of free blocks of the appropriate type (size). If this
releases all blocks of the page (according to the free block counter), the blocks are
removed from the free block list and the entire page is added to the free page list.

When a large block over 16 KB is released, the list of pages belonging to the block is
scanned, and the pages are added to the list of free pages.
*/

#if USE_PMALLOC		// use PSRAM Memory Allocator (lib_pmalloc.c, lib_pmalloc.h)

#ifndef _LIB_PMALLOC_H
#define _LIB_PMALLOC_H

#include "lib_list.h"			// Doubly Linked List

#ifdef __cplusplus
extern "C" {
#endif

#define PMEM_START		PSRAM_BASE		// start address of the PSRAM memory
#define PMEM_SIZE		PSRAM_Size		// size of PSRAM memory
#define PMEM_END		(PSRAM_BASE+PSRAM_Size) // end address after the PSRAM memory

#define PMEM_PAGE_BITS		14			// page size in number of bits
#define PMEM_PAGE_SIZE		BIT(PMEM_PAGE_BITS)	// page size (= 16384 = 0x4000)
#define PMEM_PAGE_MASK		(PMEM_PAGE_SIZE-1)	// page mask (= 0x3FFF)
#define PMEM_PAGE_INVMASK	(~PMEM_PAGE_MASK)	// page inverse mask (= 0xFFFFC000)

// type of memory blocks (= order, size will be in interval (1<<(type-1)) < size <= (1<<type) )
// - Type must be equal to order of memory block.
#define PMEM_TYPE_FREE		0		// 0: free page
#define PMEM_TYPE_LONG		1		// 1: start of long block
#define PMEM_TYPE_CONT		2		// 2: continue of long block, or page is reserved (descriptors)
#define PMEM_TYPE_8		3		// 3: 8 bytes (size 1 to 8; minimum block size must hold the sListEntry item)
#define PMEM_TYPE_16		4		// 4: 16 bytes (size 9 to 16)
#define PMEM_TYPE_32		5		// 5: 32 bytes (size 17 to 32)
#define PMEM_TYPE_64		6		// 6: 64 bytes (size 33 to 64)
#define PMEM_TYPE_128		7		// 7: 128 bytes (size 65 to 128)
#define PMEM_TYPE_256		8		// 8: 256 bytes (size 129 to 256)
#define PMEM_TYPE_512		9		// 9: 512 bytes (size 257 to 512)
#define PMEM_TYPE_1K		10		// 10: 1 Kbytes (size 513 to 1024)
#define PMEM_TYPE_2K		11		// 11: 2 Kbytes (size 1025 to 2048)
#define PMEM_TYPE_4K		12		// 12: 4 Kbytes (size 2049 to 4096)
#define PMEM_TYPE_8K		13		// 13: 8 Kbytes (size 4097 to 8192)
#define PMEM_TYPE_16K		14		// 14: 16 Kbytes (size 8193 to 16384)

#define PMEM_TYPE_NUM		15		// number of type entries

extern int PmemPageNum;		// total number of pages
extern int PmemPageRes;		// number of reserved pages with page descriptors
extern int PmemFreePageMem;	// free memory - only whole pages
extern int PmemFreeMem;		// total free memory
extern int PmemBlockNum;	// number of allocated blocks

// page descriptor (size 4 bytes) ... Array of memory block descriptors is located at beginning of PSRAM.
typedef struct {
	u16	freenum;	// number of free memory blocks
	u16	type;		// type of memory block
} sPmemPage;
STATIC_ASSERT(sizeof(sPmemPage) == 4, "Incorrect sPmemPage!");

// free memory block descriptor ... Array of free memory block descriptors is in global SRAM.
typedef struct {
	sList	freelist;	// list of free memory blocks (PMEM_TYPE_FREE: list of free pages)
} sPmemBlock;
STATIC_ASSERT(sizeof(sListEntry) == 8, "Incorrect sListEntry!");
STATIC_ASSERT(sizeof(sList) == 8, "Incorrect sList!");

// arrary of free memory block descriptors (PMEM_TYPE_FREE: list of free pages)
extern sPmemBlock PmemBlock[PMEM_TYPE_NUM];

// check if PSRAM memory is valid
INLINE Bool PmemValid(void) { return (PSRAM_Size != 0); }

// check if pointer is valid PSRAM address
INLINE Bool PmemPtrValid(void* ptr)
	{ return ((u32)ptr >= PMEM_START) && ((u32)ptr < PMEM_END); }

// round pointer down to page address
INLINE void* PmemPtrToPage(void* ptr)
	{ return (void*)((u32)ptr & PMEM_PAGE_INVMASK); }

// round size up to pages
INLINE int PmemSizeToPage(int size)
	{ return (size + PMEM_PAGE_MASK) & PMEM_PAGE_INVMASK; }

// convert pointer to page index
INLINE int PmemPtrToPageInx(void* ptr)
	{ return ((u32)ptr - PMEM_START) >> PMEM_PAGE_BITS; }

// convert pointer to page descriptor
INLINE sPmemPage* PmemPtrToPgDesc(void* ptr)
	{ return (sPmemPage*)(PmemPtrToPageInx(ptr)*sizeof(sPmemPage) + PMEM_START); }

// convert page descriptor to the page
INLINE void* PmemPgDescToPage(sPmemPage* m)
	{ return (void*)(((((u32)m - PMEM_START)/sizeof(sPmemPage)) << PMEM_PAGE_BITS) + PMEM_START); }

// convert size to type of memory block PMEM_TYPE_*
int PmemSizeToType(size_t size);

// initialize PSRAM Memory Allocator
void PmemInit(void);

// allocate PSRAM memory block (returns NULL on error)
void* pmalloc(size_t size);

// allocate PSRAM 'nitems' number of elements of size 'size' and clears memory
void* pcalloc(size_t nitems, size_t size);

// release PSRAM memory block (addr can be NULL)
void pfree(void* addr);

// resize PSRAM memory block (returns new block or NULL on error)
void* prealloc(void* addr, size_t size);

// get size of the block (returns 0 if block is not valid; size is rounded up to power of 2 or to page size)
// - Can be used to partially check if the address is a valid pointer to a PSRAM memory block.
int pgetsize(void* addr);

// get max. free block
int pgetmaxfree(void);

#if USE_RAND		// use Random number generator (lib_rand.c, lib_rand.h)
// initialize long check memory manager
void PmemLongCheckInit();

// Long check memory manager (returns 0 if OK)
int PmemLongCheck(int loop);

// terminate long check memory manager - release blocks
void PmemLongCheckTerm();
#endif // USE_RAND		// use Random number generator (lib_rand.c, lib_rand.h)

#ifdef __cplusplus
}
#endif

#endif // _LIB_PMALLOC_H

#endif // USE_PMALLOC
