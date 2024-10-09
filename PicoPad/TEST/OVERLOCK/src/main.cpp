
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "include.h"

// main function
int main()
{
	int volt = -1;
	int clk = -1;
	int temp = 1;

	// enable temperature sensor
	ADC_TempEnable();

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
		// max. frequency without increasing voltage: 270 MHz

		// setup voltage (it increases frequency to 310 MHz)
		int newvolt;
		if (f >= 250000)
			newvolt = VREG_VOLTAGE_1_30;
		else if (f >= 220000)
			newvolt = VREG_VOLTAGE_1_25;
		else if (f >= 190000)
			newvolt = VREG_VOLTAGE_1_20;
		else if (f >= 150000)
			newvolt = VREG_VOLTAGE_1_15;
		else
			newvolt = VREG_VOLTAGE_1_10;

		if (newvolt != volt)
		{
			volt = newvolt;
			VregSetVoltage(volt);
		}

		// setup flash clkdiv (it increases frequency to 400 MHz)
		int newclk;
		if (f >= 350000)
			newclk = 8;
		else if (f >= 280000)
			newclk = 6;
		else
			newclk = 4;

		if (newclk != clk)
		{
			clk = newclk;
			SSI_InitFlash(clk);
		}

#if USE_PICOPADVGA
		// retune VGA
		VgaRetune(f*1000);
		WaitMs(2500);
#endif

		// set system clock frequency
		ClockPllSysFreq(f);
		printf("freq %'d Hz vreg=%.2f clk=%d\n", ClockGetHz(CLK_PLL_SYS), VregVoltageFloat(), clk);

		// temperature
		temp--;
		if (temp <= 0)
		{
			temp = 10;
			printf("temp=%.1f C\n", ADC_Temp());
		}

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
		f += 500;
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
