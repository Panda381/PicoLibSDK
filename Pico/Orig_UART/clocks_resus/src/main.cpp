
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************
// Resurection interrupt on system clock fail.

/*
Debug output
------------
Hello resus
 pll_sys = 125000kHz, clk_sys = 125000kHz
Resus event fired
 pll_sys = 0kHz, clk_sys = 12000kHz
Bye resus
 pll_sys = 125000kHz, clk_sys = 125000kHz
*/

#include "../include.h"

volatile bool seen_resus;

void resus_callback(void)
{
	uint f_pll_sys = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_PLL_SYS_CLKSRC_PRIMARY);
	uint f_clk_sys = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_SYS);

	// Reconfigure PLL sys back to the default state of 1500 / 6 / 2 = 125MHz
	pll_init(pll_sys, 1, 1500 * MHZ, 6, 2);

	// CLK SYS = PLL SYS (125MHz) / 1 = 125MHz
	clock_configure(clk_sys,
		CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
		CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS,
		125 * MHZ,
		125 * MHZ);

	// Reconfigure uart as clocks have changed
	stdio_init_all();
	printf("Resus event fired\n");
	printf(" pll_sys = %dkHz, clk_sys = %dkHz\n", f_pll_sys, f_clk_sys);

	// Wait for uart output to finish
	uart_default_tx_wait_blocking();

	seen_resus = true;
}

int main()
{
	stdio_init_all();
	printf("Hello resus\n");
	uint f_pll_sys = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_PLL_SYS_CLKSRC_PRIMARY);
	uint f_clk_sys = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_SYS);
	printf(" pll_sys = %dkHz, clk_sys = %dkHz\n", f_pll_sys, f_clk_sys);

	// Wait for uart output to finish
	uart_default_tx_wait_blocking();

	seen_resus = false;

	clocks_enable_resus(&resus_callback);

	// Break PLL sys
	pll_deinit(pll_sys);

	while(!seen_resus);

	printf("Bye resus\n");
	f_pll_sys = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_PLL_SYS_CLKSRC_PRIMARY);
	f_clk_sys = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_SYS);
	printf(" pll_sys = %dkHz, clk_sys = %dkHz\n", f_pll_sys, f_clk_sys);

	return 0;
}
