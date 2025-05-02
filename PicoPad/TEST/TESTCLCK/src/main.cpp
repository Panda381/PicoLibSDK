
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "include.h"

// precise measure frequency of clock CLK_REF..CLK_GPIN1 (CLK_PLL_OPCG) with result in Hz
//  clk ... clock line or clock generator CLK_*
//  - measure interval: 128 ms, resolution +-15 Hz
//u32 FreqCount(int clk);

// main function
int main()
{
	int i, j, s, f;
	double m;

#if RP2040
	printf("CPU: RP2040-B%d\n", RomGetVersion() - 1);
#elif RISCV
	printf("CPU: RP2350-RISCV-A%d\n", RomGetVersion());
#else
	printf("CPU: RP2350-ARM-A%d\n", RomGetVersion());
#endif

	// waste first measurement
	FreqCount(CLK_REF);

	while (True)
	{
		DrawPrintPos = 0;  // console print character position
		DrawPrintRow = 1;  // console print character row

#define TESTN 2
		for (i = CLK_REF; i < CLK_SRC_NUM; i++)
		{
			// skip GPIN
			if ((i == CLK_GPIN0) || (i == CLK_GPIN1)) continue;

			// check frequency
			f = 0;
			for (j = TESTN; j > 0; j--) f += FreqCount(i);
			m = (double)f/(MHZ*TESTN);

			// source
			s = ClockGetSrc(i);
			if ((i == CLK_PLL_SYS) || (i == CLK_PLL_USB)) s = CLK_XOSC;

			if (s == 0)
				printf("%s: f=%.6fMHz  \n", ClockGetName(i), m);
			else
				printf("%s: src=%s f=%.6fMHz  \n", ClockGetName(i), ClockGetName(s), m);
		}

		// quit program
		if (KeyGet() == KEY_Y)
		{
#if USE_SCREENSHOT		// use screen shots

			ScreenShot();
#endif
			ResetToBootLoader();
		}
	}
}
