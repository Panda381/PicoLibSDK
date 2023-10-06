
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "include.h"

// main function
int main()
{
	// set maximum voltage
	// - enables overlocking up to 310 MHz
	VregSetVoltage(VREG_VOLTAGE_1_30);
	printf("Vreg Voltage = 1.30V\n");

#if NO_FLASH
	// initialize Flash interface (clkdiv = clock divider, must be even number, 4 is default)
	// - enables overlocking up to 420 MHz
	SSI_InitFlash(6);
	printf("Flash clkdiv=6\n");
#endif

	// initial system clock frequency 125 MHz
	u32 f = PLL_KHZ;
	ClockPllSysFreq(f);

	// CRC of the flash memory
	u32 crc = Crc32ADMA((void*)XIP_BASE, 2*1024*1024);
	printf("Flash memory CRC = 0x%08X\n", crc);
	WaitMs(1000);

	// check calculations
	double s, c, s2, c2;
	sincos(0.9, &s, &c);

	while (True)
	{
#if USE_PICOPADVGA
		// retune VGA
		VgaRetune(f*1000);
		WaitMs(2500);
#endif

		// set system clock frequency
		ClockPllSysFreq(f);
		printf("freq %'d Hz\n", ClockGetHz(CLK_PLL_SYS));

		// check calculations
		sincos(0.9, &s2, &c2);
		if ((s != s2) || (c != c2))
		{
			printf("SinCos ERROR!\n");
			while (True) {}
		}

		// check flash memory
		if (crc != Crc32ADMA((void*)XIP_BASE, 2*1024*1024))
		{
			printf("Flash CRC ERROR!\n");
			while (True) {}
		}

		// shift frequency
#if USE_PICOPADVGA
		f += 1000;
#else
		f += 200;
#endif

		// quit program
		if (KeyGet() == KEY_Y)
		{
			ClockPllSysFreq(PLL_KHZ);
			VregSetVoltage(VREG_VOLTAGE_1_10);

#if USE_SCREENSHOT		// use screen shots

			ScreenShot();
#endif
			ResetToBootLoader();
		}
	}
}
