
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************
// Repeated measure ADC voltage on pin GPIO26.

/*
Debug output
------------
ADC Example, measuring GPIO26
Raw value: 0x082, voltage: 0.104736 V
Raw value: 0x2d5, voltage: 0.584106 V
Raw value: 0x2d6, voltage: 0.584912 V
Raw value: 0x2d4, voltage: 0.583301 V
*/

#include "../include.h"

int main()
{
	stdio_init_all();
	printf("ADC Example, measuring GPIO26\n");

	adc_init();

	// Make sure GPIO is high-impedance, no pullups etc
	adc_gpio_init(26);
	// Select ADC input 0 (GPIO26)
	adc_select_input(0);

	while (1)
	{
		// 12-bit conversion, assume max value == ADC_VREF == 3.3 V
		const float conversion_factor = 3.3f / (1 << 12);
		uint16_t result = adc_read();
		printf("Raw value: 0x%03x, voltage: %f V\n", result, result * conversion_factor);
		sleep_ms(500);
	}
}
