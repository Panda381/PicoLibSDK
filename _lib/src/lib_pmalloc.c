
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

#include "../../global.h"	// globals

#if USE_PMALLOC		// use PSRAM Memory Allocator (lib_pmalloc.c, lib_pmalloc.h)

#include "../../_sdk/sdk_addressmap.h"		// Register address offsets
#include "../../_sdk/inc/sdk_spinlock.h"	// spinlock
#include "../../_sdk/inc/sdk_psram.h"
#include "../../_sdk/inc/sdk_cpu.h"
#include "../inc/lib_pmalloc.h"
#include "../inc/lib_rand.h"
#include "../inc/lib_print.h"

#if USE_MEMLOCK		// lock of Memory Allocator 0=no, 1=multicore, 2=multicore+IRQ (lib_malloc.c, lib_malloc.h)
#if USE_MEMLOCK >= 2
#define MEM_LOCK() SPINLOCK_LOCK(SYS_SPIN)
#define MEM_UNLOCK() SPINLOCK_UNLOCK(SYS_SPIN)
#else
#define MEM_LOCK() SpinLock(SYS_SPIN)
#define MEM_UNLOCK() SpinUnlock(SYS_SPIN)
#endif
#else
#define MEM_LOCK()
#define MEM_UNLOCK()
#endif

int PmemPageNum = 0;		// total number of pages
int PmemPageRes = 0;		// number of reserved pages, containing page descriptors
int PmemFreePageMem = 0;	// free memory - only whole pages
int PmemFreeMem = 0;		// total free memory
int PmemBlockNum = 0;		// number of allocated blocks

// arrary of free memory block descriptors (PMEM_TYPE_FREE: list of free pages)
sPmemBlock PmemBlock[PMEM_TYPE_NUM];

// convert size to type of memory block PMEM_TYPE_*
int PmemSizeToType(size_t size)
{
	// minimal size
	if (size <= 8) return PMEM_TYPE_8;

	// maximal size
	if (size > PMEM_PAGE_SIZE) return PMEM_TYPE_LONG;

	// get size type
	return 32 - clz(size - 1);
}

// initialize PSRAM Memory Allocator
void PmemInit(void)
{
	// initialize array of free memory block descriptors
	int i, j;
	for (i = 0; i < PMEM_TYPE_NUM; i++)
	{
		ListInit(&PmemBlock[i].freelist);
	}

	// check if PSRAM memory is valid
	if (!PmemValid()) return;

	// total number of pages
	int pagenum = PSRAM_Size >> PMEM_PAGE_BITS;
	PmemPageNum = pagenum;

	// number of reserved pages
	i = pagenum * sizeof(sPmemPage); // size of array of page descriptors
	i = PmemSizeToPage(i); // align to whole pages
	PmemFreePageMem = PSRAM_Size - i; // free memory - only whole pages
	PmemFreeMem = PSRAM_Size - i; // total free memory
	i = i >> PMEM_PAGE_BITS; // number of reserved pages
	PmemPageRes = i;
	PmemBlockNum = 0;	// number of allocated blocks

	// clear page descriptors (set type to PMEM_TYPE_FREE)
	memset((void*)PMEM_START, PMEM_TYPE_FREE, pagenum*sizeof(sPmemPage));

	// mark reserved pages
	u32 pg = PMEM_START;
	for (j = 0; j < i; j++)
	{
		((sPmemPage*)pg)->type = PMEM_TYPE_CONT;
		pg += sizeof(sPmemPage);
	}

	// add pages to list of free pages
	pg = PMEM_START + i*PMEM_PAGE_SIZE;
	for (; i < pagenum; i++)
	{
		// add page to end of list of free pages
		ListAddLast(&PmemBlock[PMEM_TYPE_FREE].freelist, (sListEntry*)pg);
		pg += PMEM_PAGE_SIZE;
	}
}

// allocate PSRAM memory block (returns NULL on error)
void* pmalloc(size_t size)
{
	int i, j, n, t, pgnum, bsize, bnum;
	u32 pg;
	sPmemPage *m, *m2;
	sPmemBlock* p;
	void* b;
	sList* f;

	// check if PSRAM memory is valid
	if (!PmemValid()) return NULL;

	// base check size
	if ((size > PMEM_SIZE) || (size == 0)) return NULL;

	// convert size to type of memory block PMEM_TYPE_*
	t = PmemSizeToType(size);

	// lock IRQ and spinlock
	MEM_LOCK();

	// allocate multiply pages
	if (t == PMEM_TYPE_LONG)
	{
		// required number of pages
		pgnum = (size + PMEM_PAGE_MASK) >> PMEM_PAGE_BITS;

		// search chain of pages
		n = PmemPageNum - pgnum; // max. page to check
		m = (sPmemPage*)PMEM_START;
		for (i = 1; i <= n; i++)
		{
			// next page
			m++;

			// check chain
			m2 = m;
			for (j = pgnum; j > 0; j--)
			{
				if (m2->type != PMEM_TYPE_FREE) break;
				m2++;
			}

			// found
			if (j == 0)
			{
				// decrease free memory
				j = pgnum * PMEM_PAGE_SIZE;
				PmemFreePageMem -= j;
				PmemFreeMem -= j;
				PmemBlockNum++;	// number of allocated blocks

				// start address of the block
				b = PmemPgDescToPage(m);

				// allocate chain of the pages
				pg = (u32)b;
				m2 = m;
				for (j = pgnum; j > 0; j--)
				{
					m2->type = PMEM_TYPE_CONT;
					ListEntryRemove((sListEntry*)pg);
					m2++;
					pg += PMEM_PAGE_SIZE;
				}
				m->type = PMEM_TYPE_LONG;

				MEM_UNLOCK();
				return b;
			}
		}

		// memory error
		MEM_UNLOCK();
		return NULL;
	}

	// prepare pointer to free memory block descriptor
	p = &PmemBlock[t];

	// allocate new page if there is no free memory block left
	if (ListIsEmpty(&p->freelist))
	{
		// pointer to page list
		f = &PmemBlock[PMEM_TYPE_FREE].freelist;

		// memory error if no free page left
		if (ListIsEmpty(f))
		{
			MEM_UNLOCK();
			return NULL;
		}

		// allocate page
		pg = (int)ListGetFirst(f);
		ListEntryRemove((sListEntry*)pg);

		// decrease free memory
		PmemFreePageMem -= PMEM_PAGE_SIZE;

		// prepare size of the block
		bsize = BIT(t);

		// prepare number of blocks per page
		bnum = PMEM_PAGE_SIZE >> t;

		// prepare page descriptor
		m = PmemPtrToPgDesc((void*)pg);
		m->freenum = bnum;
		m->type = t;

		// add blocks of the page to the list
		for (; bnum > 0; bnum--)
		{
			ListAddLast(&p->freelist, (sListEntry*)pg);
			pg += bsize;
		}
	}

	// remove block from the list
	b = (void*)ListGetFirst(&p->freelist);
	ListEntryRemove((sListEntry*)b);

	// decrease free memory
	PmemFreeMem -= BIT(t);
	PmemBlockNum++;	// number of allocated blocks

	// update page descriptor
	m = PmemPtrToPgDesc(b);
	m->freenum--;

	// unlock spinlock and IRQ
	MEM_UNLOCK();
	return b;
}

// allocate PSRAM 'nitems' number of elements of size 'size' and clears memory
void* pcalloc(size_t nitems, size_t size)
{
	size_t n = nitems*size;
	void* addr = pmalloc(n);
	if (addr != NULL) memset(addr, 0, n);
	return addr;
}

// free PSRAM memory block
void pfree(void* addr)
{
// No need to lock at this place, only one process is allowed to change the structure of this block.

	// check address, get block size
	int bsize = pgetsize(addr);
	if (bsize == 0) return;

	// lock IRQ and spinlock
	MEM_LOCK();

	// get page descriptor and block type
	sPmemPage* m = PmemPtrToPgDesc(addr);
	int t = m->type;

	// shift free memory
	PmemFreeMem += bsize;
	PmemBlockNum--;	// number of allocated blocks

	// long block
	if (t == PMEM_TYPE_LONG)
	{
		// shift free memory
		PmemFreePageMem += bsize;

		// release pages
		int i = bsize >> PMEM_PAGE_BITS; // number of pages
		u32 pg = (u32)addr; // page address
		sList* f = &PmemBlock[PMEM_TYPE_FREE].freelist;
		for (; i > 0; i--)
		{
			// add page to free page list
			ListAddLast(f, (sListEntry*)pg);
			pg += PMEM_PAGE_SIZE;
			m->type = PMEM_TYPE_FREE;
			m++;
		}

		// unlock spinlock and IRQ
		MEM_UNLOCK();
		return;
	}

	// number of blocks per page
	int bnum = PMEM_PAGE_SIZE >> t;

	// prepare free memory block descriptor
	sPmemBlock* p = &PmemBlock[t];

	// add block to free list
	ListAddLast(&p->freelist, (sListEntry*)addr);

	// update page descriptor
	m->freenum++;

	// release the page if all blocks are free
	if (m->freenum >= bnum)
	{
		// mark this page free
		m->type = PMEM_TYPE_FREE;

		// get page address
		u32 pg = (int)PmemPgDescToPage(m);

		// remove blocks from the list
		int pg2 = pg;
		for (; bnum > 0; bnum--)
		{
			ListEntryRemove((sListEntry*)pg2);
			pg2 += bsize;
		}

		// add page to free pages
		ListAddLast(&PmemBlock[PMEM_TYPE_FREE].freelist, (sListEntry*)pg);

		// shift free memory
		PmemFreePageMem += PMEM_PAGE_SIZE;
	}

	// unlock spinlock and IRQ
	MEM_UNLOCK();
}

// resize PSRAM memory block (returns new block or NULL on error)
void* prealloc(void* addr, size_t size)
{
	// release memory block
	if (size == 0)
	{
		pfree(addr);
		return NULL;
	}

	// create new block
	if (addr == NULL) return pmalloc(size);

	// get old size
	int oldsize = pgetsize(addr);
	if (oldsize == 0) return NULL; // invalid block

	// prepare new size
	int t = PmemSizeToType(size);
	int newsize = (t == PMEM_TYPE_LONG) ? PmemSizeToPage(size) : BIT(t);

	// size not changed
	if (oldsize == newsize) return addr;

	// create new block
	void* newaddr = pmalloc(size);
	if (newaddr == NULL) return newaddr; // memory error

	// prepare size to copy
	int n = (oldsize < newsize) ? oldsize : newsize;

	// copy data
	memcpy(newaddr, addr, n);

	// delete old block
	pfree(addr);
	return newaddr;
}

// get size of the block (returns 0 if block is not valid; size is rounded up to power of 2 or to page size)
// - Can be used to partially check if the address is a valid pointer to a PSRAM memory block.
int pgetsize(void* addr)
{
	// check address
	if (!PmemPtrValid(addr)) return 0;

// No need to lock, only one process is allowed to change the structure of this block.

	// get page descriptor and block type
	sPmemPage* m = PmemPtrToPgDesc(addr);
	int t = m->type;

	// check block type
	if ((t == PMEM_TYPE_FREE) || (t == PMEM_TYPE_CONT) || (t >= PMEM_TYPE_NUM)) return 0;

	// long block
	if (t == PMEM_TYPE_LONG)
	{
		// address must be page aligned
		if ((u32)addr != (u32)PmemPgDescToPage(m)) return 0;

		// count pages
		int size = PMEM_PAGE_SIZE;
		int i = PmemPageNum - PmemPtrToPageInx(addr) - 1;
		for (; i > 0; i--)
		{
			m++;
			if (m->type != PMEM_TYPE_CONT) break;
			size += PMEM_PAGE_SIZE;
		}

		return size;
	}

	// prepare number of blocks per page
	int bnum = PMEM_PAGE_SIZE >> t;

	// check number of free entries
	if (m->freenum >= bnum) return 0;

	// prepare block size
	int bsize = BIT(t);

	// check align of the block
	if (((u32)addr & (bsize - 1)) != 0) bsize = 0;

	// block size
	return bsize;
}

// get max. free block
int pgetmaxfree(void)
{
	int maxsize = 0;
	int i = PmemPageRes;
	int n = PmemPageNum;
	sPmemPage* m = (sPmemPage*)(PMEM_START + i*sizeof(sPmemPage));
	int size = 0;
	for (; i < n; i++)
	{
		if (m->type == PMEM_TYPE_FREE)
		{
			size += PMEM_PAGE_SIZE;
		}
		else
		{
			if (size > maxsize) maxsize = size;
			size = 0;
		}
		m++;
	}
	if (size > maxsize) maxsize = size;
	return maxsize;
}

#if USE_RAND		// use Random number generator (lib_rand.c, lib_rand.h)

#define PMEMCHECK_NUM 20000
void* PmemCheckBuf[PMEMCHECK_NUM]; // buffer of allocated blocks (NULL = not allocated)
int PmemCheckSize[PMEMCHECK_NUM]; // required size of allocated blocks
u64 PmemCheckSeed[PMEMCHECK_NUM]; // seed of allocated blocks

// initialize long check memory manager
void PmemLongCheckInit()
{
	int i;
	for (i = 0; i < PMEMCHECK_NUM; i++) PmemCheckBuf[i] = NULL;
}

// Long check memory manager (returns 0 if OK)
int PmemLongCheck(int loop)
{
	int i;
	u64 oldseed;
	int level = 100;
	u8 *adr, *adr2;
	int siz;

	for (; loop > 0; loop--)
	{
		// random operation
		i = RandU32Max(PMEMCHECK_NUM-1);

		// shift block level (small block ... big block)
		level += RandS8MinMax(-1, +1);
		if (level < 2) level = 2;
		if (level > 999) level = 999;

		// create new block
		if (PmemCheckBuf[i] == NULL)
		{
			siz = pgetmaxfree()/level;
			siz = RandU32Max(siz); // required size
			PmemCheckSize[i] = siz;
			adr = (u8*)pmalloc(siz); // allocate block
			PmemCheckBuf[i] = adr;

			// fill pattern
			if (adr != NULL)
			{
				if (pgetsize(adr) == 0) return -1;
				PmemCheckSeed[i] = RandGet();
				for (; siz > 0; siz--) *adr++ = RandU8();
			}
		}
		else
		{
			// free block
			if ((RandU32() & 3) != 0)
			{
				adr = PmemCheckBuf[i];
				siz = PmemCheckSize[i];

				if (adr != NULL)
				{
					// check pattern
					if (pgetsize(adr) == 0) return -1;
					oldseed = RandGet();
					RandSet(PmemCheckSeed[i]);
					adr2 = adr;
					for (; siz > 0; siz--)
					{
						if (*adr2 != RandU8()) return -1;
						adr2++;
					}
					RandSet(oldseed);
				}

				pfree(adr);
				PmemCheckBuf[i] = NULL;
			}

			// reallocate
			else
			{
				adr = PmemCheckBuf[i];
				siz = PmemCheckSize[i];

				// check pattern
				if (adr != NULL)
				{
					if (pgetsize(adr) == 0) return -1;
					oldseed = RandGet();
					RandSet(PmemCheckSeed[i]);
					adr2 = adr;
					for (; siz > 0; siz--)
					{
						if (*adr2 != RandU8()) return -1;
						adr2++;
					}
					RandSet(oldseed);
				}

				siz = pgetmaxfree()/level;
				siz = RandU32Max(siz); // required size
				adr = (u8*)prealloc(adr, siz); // reallocate block

				if ((siz == 0) || (adr != NULL))
				{
					PmemCheckSize[i] = siz;
					PmemCheckBuf[i] = adr;

					// fill pattern
					if (adr != NULL)
					{
						if (pgetsize(adr) == 0) return -1;
						PmemCheckSeed[i] = RandGet();
						for (; siz > 0; siz--) *adr++ = RandU8();
					}
				}
			}
		}
	}
	return 0;
}

// terminate long check memory manager - release blocks
void PmemLongCheckTerm()
{
	int i;
	for (i = 0; i < PMEMCHECK_NUM; i++) pfree(PmemCheckBuf[i]);
}

#endif // USE_RAND		// use Random number generator (lib_rand.c, lib_rand.h)

#endif // USE_PMALLOC
