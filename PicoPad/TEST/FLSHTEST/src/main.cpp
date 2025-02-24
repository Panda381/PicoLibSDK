
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "include.h"

// test of one flash
void Test(int clkdiv)
{
	SSI_SetFlashClkDiv(clkdiv);
	u32 t1, t2;
	t1 = Time();
	u32 crc = Sum8DMA((void*)XIP_BASE, 2*1024*1024);
	t2 = Time();
	printf("\nCLKDIV=%d, SUM=0x%08X, %dns/Byte", SSI_FlashClkDiv(), crc, (t2 - t1)*1000/(2*1024*1024));
}

// test of one speed
void TestSpeed(u32 f)
{
	int i, j;
	u32 t1, t2;

	// initialize Flash
	SSI_InitFlash(6);

	// system clock frequency
	ClockPllSysFreq(f);

	// disable interrupts
	di();

	// check RAM speed
	t1 = Time();
	for (i = 100; i > 0; i--) Sum8DMA((void*)SRAM_BASE, 264*1024);
	t2 = Time();
	printf("RAM speed at %dMHz: %.2fns/Byte\n", ClockGetHz(CLK_SYS)/1000000, (double)(t2 - t1)*10/(264*1024));

	// enable interrupts
	ei();

	// check Flash speed
	printf("\nFlash speed at %dMHz:", ClockGetHz(CLK_SYS)/1000000);
	Test(2);
	Test(4);
	Test(6);
	Test(8);
}

// main function
int main()
{
	// set maximum voltage
	// - enables overclocking up to 310 MHz
	VregSetVoltage(VREG_VOLTAGE_1_30);

	// test 125 MHz
	TestSpeed(PLL_KHZ);
	printf("\n\n");

	// test 252 MHz
	TestSpeed(252000);

	// wait for a key
	while (True)
	{
		// quit program
		if (KeyGet() == KEY_Y)
		{
			ClockPllSysFreq(PLL_KHZ);
			SSI_SetFlashClkDiv(4);
			VregSetVoltage(VREG_VOLTAGE_1_10);

#if USE_SCREENSHOT		// use screen shots

			ScreenShot();
#endif
			ResetToBootLoader();
		}
	}
}
