
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************
// Output clock signal to GPIO21, check input on GPIO20 (connect GPIO21 and GPIO20)

/*
Debug output
------------
Hello gpout
clk_sys = 125000kHz, output on GPIO21 = 1250kHz, check on GPIO20 = 1250kHz
clk_usb = 48000kHz, output on GPIO21 = 480kHz, check on GPIO20 = 480kHz
clk_adc = 48000kHz, output on GPIO21 = 480kHz, check on GPIO20 = 480kHz
clk_rtc = 47kHz, output on GPIO21 = 4705Hz, check on GPIO20 = 4kHz
*/

#include "../include.h"

int main()
{
	stdio_init_all();

	// test input from GPIO20
	gpio_set_function(20, GPIO_FUNC_GPCK);
	uint f_in;

	while (True)
	{
		printf("Hello gpout\n");

		// Output clk_sys / 100 to gpio 21, etc...
		uint f_clk_sys = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_SYS);
		clock_gpio_init(21, CLOCKS_CLK_GPOUT0_CTRL_AUXSRC_VALUE_CLK_SYS, 100);
		f_in = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLKSRC_GPIN0);
		printf("clk_sys = %dkHz, output on GPIO21 = %dkHz, check on GPIO20 = %dkHz\n",
			f_clk_sys, (f_clk_sys+50)/100, f_in);
		sleep_ms(5000);

		uint f_clk_usb = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_USB);
		clock_gpio_init(21, CLOCKS_CLK_GPOUT0_CTRL_AUXSRC_VALUE_CLK_USB, 100);
		f_in = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLKSRC_GPIN0);
		printf("clk_usb = %dkHz, output on GPIO21 = %dkHz, check on GPIO20 = %dkHz\n",
			f_clk_usb, (f_clk_usb+50)/100, f_in);
		sleep_ms(5000);

		uint f_clk_adc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_ADC);
		clock_gpio_init(21, CLOCKS_CLK_GPOUT0_CTRL_AUXSRC_VALUE_CLK_ADC, 100);
		f_in = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLKSRC_GPIN0);
		printf("clk_adc = %dkHz, output on GPIO21 = %dkHz, check on GPIO20 = %dkHz\n",
			f_clk_adc, (f_clk_adc+50)/100, f_in);
		sleep_ms(5000);

#if RP2040
		uint f_clk_rtc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_RTC);
		clock_gpio_init(21, CLOCKS_CLK_GPOUT0_CTRL_AUXSRC_VALUE_CLK_RTC, 10);
		f_in = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLKSRC_GPIN0);
		printf("clk_rtc = %dkHz, output on GPIO21 = %dHz, check on GPIO20 = %dkHz\n",
			f_clk_rtc, (f_clk_rtc*1000+50)/10, f_in);
		sleep_ms(5000);
#endif
	}

	return 0;
}
