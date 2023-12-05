
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************
// Measure frequency of on-board clocks.

/*
Debug output
------------
Hello, world!
pll_sys  = 125000kHz
pll_usb  = 48000kHz
rosc     = 5099kHz
clk_sys  = 125000kHz
clk_peri = 125000kHz
clk_usb  = 48000kHz
clk_adc  = 48000kHz
clk_rtc  = 47kHz
...clk_sys->48MHz
pll_sys  = 0kHz
pll_usb  = 48000kHz
rosc     = 5110kHz
clk_sys  = 48000kHz
clk_peri = 48000kHz
clk_usb  = 48000kHz
clk_adc  = 48000kHz
clk_rtc  = 47kHz
Hello, 48MHz
*/

#include "../include.h"

void measure_freqs(void)
{
	uint f_pll_sys = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_PLL_SYS_CLKSRC_PRIMARY);
	uint f_pll_usb = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_PLL_USB_CLKSRC_PRIMARY);
	uint f_rosc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_ROSC_CLKSRC);
	uint f_clk_sys = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_SYS);
	uint f_clk_peri = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_PERI);
	uint f_clk_usb = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_USB);
	uint f_clk_adc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_ADC);
	uint f_clk_rtc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_RTC);

	printf("pll_sys  = %dkHz\n", f_pll_sys);
	printf("pll_usb  = %dkHz\n", f_pll_usb);
	printf("rosc     = %dkHz\n", f_rosc);
	printf("clk_sys  = %dkHz\n", f_clk_sys);
	printf("clk_peri = %dkHz\n", f_clk_peri);
	printf("clk_usb  = %dkHz\n", f_clk_usb);
	printf("clk_adc  = %dkHz\n", f_clk_adc);
	printf("clk_rtc  = %dkHz\n", f_clk_rtc);

    // Can't measure clk_ref / xosc as it is the ref
}

int main()
{
	stdio_init_all();

	// wait to connect terminal
	int c;
	do {
		printf("Press spacebar to start...\n");
		c = getchar();
	} while (c != ' ');
	
	printf("\nHello, world!\n");

	measure_freqs();

	printf("...clk_sys->48MHz\n");

	// Wait for uart output to finish ... do not call if using USB port
	// uart_default_tx_wait_blocking();

	// Change clk_sys to be 48MHz. The simplest way is to take this from PLL_USB
	// which has a source frequency of 48MHz
	clock_configure(clk_sys,
		CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
		CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB,
		48 * MHZ,
		48 * MHZ);

	// Turn off PLL sys for good measure
	pll_deinit(pll_sys);

	// CLK peri is clocked from clk_sys so need to change clk_peri's freq
	clock_configure(clk_peri,
		0,
		CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS,
		48 * MHZ,
		48 * MHZ);

	// Re init uart now that clk_peri has changed ... do not call if using USB port
	// stdio_init_all();

	measure_freqs();
	printf("Hello, 48MHz\n");

	return 0;
}
