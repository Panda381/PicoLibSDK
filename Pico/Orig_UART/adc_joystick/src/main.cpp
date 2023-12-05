
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************
// Repeated measure joystick positin using ADC, X axis on GPIO26, Y axis on GPIO27.

/*
Debug output
------------
X: [      o                                 ]  Y: [       o                                ]
*/

#include "../include.h"

int main()
{
	stdio_init_all();
	adc_init();

	// Make sure GPIO is high-impedance, no pullups etc
	adc_gpio_init(26);
	adc_gpio_init(27);

	while (1)
	{
		// measure X/Y inputs
		adc_select_input(0);
		uint adc_x_raw = adc_read();
		adc_select_input(1);
		uint adc_y_raw = adc_read();

		// Display the joystick position something like this:
		// X: [            o             ]  Y: [              o         ]
		const uint bar_width = 40;
		const uint adc_max = (1 << 12) - 1;
		uint bar_x_pos = adc_x_raw * bar_width / adc_max;
		uint bar_y_pos = adc_y_raw * bar_width / adc_max;

		// display X
		printf("\rX: [");
		for (int i = 0; i < bar_width; ++i)
			putchar( i == bar_x_pos ? 'o' : ' ');

		// display Y
		printf("]  Y: [");
		for (int i = 0; i < bar_width; ++i)
			putchar( i == bar_y_pos ? 'o' : ' ');
		printf("]");

		// wait
		sleep_ms(50);
	}
}
