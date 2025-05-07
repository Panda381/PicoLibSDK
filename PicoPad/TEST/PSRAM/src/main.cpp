
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "../include.h"

// RAM test buffer
#define RAMTEST_SIZE 0x10000
u32 RAMTestBuf[RAMTEST_SIZE];

int main()
{
	int i, j, k, n;

	// flash size and address
	printf("Compile Flash size %d MB\n", FLASHSIZE/(1024*1024));
	printf("Real Flash size %d MB\n", FlashSize/(1024*1024));
	printf("Flash addr. 0x%08X-0x%08X\n", FLASH_BASE, FLASH_BASE + FlashSize - 1);

	// SRAM size and address
	printf("SRAM size %d KB\n", RAMSIZE/1024);
	printf("SRAM addr. 0x%08X-0x%08X\n", SRAM_BASE, SRAM_BASE + RAMSIZE - 1);

	// PSRAM size and address
	printf("PSRAM size %d MB\n", PSRAM_Size/(1024*1024));
	printf("PSRAM addr. 0x%08X-0x%08X\n", PSRAM_BASE, PSRAM_BASE + PSRAM_Size - 1);

	// no PSRAM
	if (PSRAM_Size == 0)
	{
		printf("No PSRAM, stop\n");
	}
	else
	{
		// write PSRAM (182us/KB)
		u32* d = (u32*)PSRAM_BASE;
		u64 rand_seed = RandU64();
		RandSet(rand_seed);
		printf("Write PSRAM...");
		u32 t1 = Time();
		for (i = PSRAM_Size/4; i > 0; i--) *d++ = RandU32();
		u32 t2 = Time();

		// verify PSRAM (132us/KB)
		printf("OK, %dus/KB\nVerify PSRAM...", (u32)((u64)(t2-t1)*1024/PSRAM_Size));
		u32 d1, d2;
		d = (u32*)PSRAM_BASE;
		RandSet(rand_seed);
		t1 = Time();
		for (i = PSRAM_Size/4; i > 0; i--)
		{
			d1 = RandU32();
			d2 = *d;
			if (d1 != d2)
			{
				printf("ERROR\n Found 0x%08X instead of 0x%08X\n", d2, d1);
				printf(" at address 0x%08X\n", (u32)d);
				break;
			}
			d++;
		}
		t2 = Time();

		// verify OK
		if (i == 0)
		{
			printf("OK, %dus/KB\n", (u32)((u64)(t2-t1)*1024/PSRAM_Size));
		}

/*
		// DMA read speed (43us/KB)
		t1 = Time();
		Sum8DMA((void*)PSRAM_BASE, PSRAM_Size);
		t2 = Time();
		printf("DMA read speed %dus/KB\n", (u32)((u64)(t2-t1)*1024/PSRAM_Size));
*/
	}

	// initialize PSRAM Memory Allocator
	PmemInit();
	printf("PmemPageNum=%d PmemPageRes=%d\n", PmemPageNum, PmemPageRes);

	// check PmemSizeToType()
	printf("Check PmemSizeToType...");
	for (i = 1000000; i > 0; i--)
	{
		j = RandU16();
		k = PmemSizeToType(j);
		n = BIT(k);
		if (((j <= 8) && (k != PMEM_TYPE_8)) ||
			((j > 16384) && (k != PMEM_TYPE_LONG)) ||
			((j >= 8) && (j <= 16384) && ((j > n) || (j <= (n/2)))))
		{
			printf("ERROR %d->%d\n", j, k);
			break;
		}
	}
	if (i == 0) printf("OK\n");

	// initialize long check memory manager
	PmemLongCheckInit();

	printf("Long check (press Y to quit):\n");
	printf("Fr%d Pg%d Nm%d Mx%d\n", PmemFreeMem, PmemFreePageMem, PmemBlockNum, pgetmaxfree());
	while (KeyGet() != KEY_Y)
	{
		printf("\rFr%d Pg%d Nm%d Mx%d ", PmemFreeMem, PmemFreePageMem, PmemBlockNum, pgetmaxfree());

		// Long check memory manager (returns 0 if OK)
		if (PmemLongCheck(5000) != 0)
		{
			printf("\nERROR!");
			while (True) {}
		}
	}

	// terminate long check memory manager - release blocks
	PmemLongCheckTerm();

#if USE_SCREENSHOT		// use screen shots
	ScreenShot();
#endif

	// print end state
	printf("\nFr%d Pg%d Nm%d Mx%d\n", PmemFreeMem, PmemFreePageMem, PmemBlockNum, pgetmaxfree());

	// end - wait for Y to exit
	while (True)
	{
		LedOn(LED1);
		WaitMs(250);
		if (KeyGet() == KEY_Y) ResetToBootLoader();
		LedOn(LED2);
		WaitMs(250);
		if (KeyGet() == KEY_Y) ResetToBootLoader();

		LedOff(LED1);
		WaitMs(250);
		if (KeyGet() == KEY_Y) ResetToBootLoader();
		LedOff(LED2);
		WaitMs(250);
		if (KeyGet() == KEY_Y) ResetToBootLoader();
	}
}
