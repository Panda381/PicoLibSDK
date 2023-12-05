
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************
// ADC input with console.

/*
Debug output
------------
===========================
RP2040 ADC and Test Console
===========================

Commands:
a       : Select ADC channel 0 (GPIO26)
b       : Select ADC channel 1 (GPIO27)
c       : Select ADC channel 2 (GPIO28)
d       : Select ADC channel 3 (GPIO29, Vsys/3)
e       : Select temperature sensor
s       : Sample once
m       : Sample many
w       : Wiggle pins
d
Switched to channel 3
s
0x811 -> 1.663696 V
m
Starting capture...Done
0x811 0x811 0x80f 0x80e 0x80b 0x80a 0x809 0x807
0x806 0x804 0x803 0x801 0x800 0x7fd 0x7fc 0x7fb
0x7f9 0x7fa 0x7f8 0x7f6 0x7f5 0x7f4 0x7f3 0x7f2
0x7f2 0x7f1 0x7f1 0x7ef 0x7ee 0x7ee 0x7ed 0x7ed
0x7eb 0x7eb 0x7ea 0x7e9 0x7e9 0x7e8 0x7e8 0x7e7
0x7e6 0x7e5 0x7e4 0x7e4 0x7e4 0x7e3 0x7e2 0x7e3
0x7e2 0x7e2 0x7e1 0x7e0 0x7e0 0x7e0 0x7e0 0x7e0
0x7df 0x7de 0x7de 0x7de 0x7dd 0x7de 0x7dd 0x7dc
0x7dc 0x7dc 0x7dc 0x7dc 0x7db 0x7db 0x7da 0x7da
0x7da 0x7db 0x7d9 0x7da 0x7da 0x7da 0x7d9 0x7d9
0x7d9 0x7d9 0x7d9 0x7d8 0x7d8 0x7d8 0x7d8 0x7d8
0x7d8 0x7d8 0x7d6 0x7d6 0x7d7 0x7d8 0x7d6 0x7d6
0x7d6 0x7d5 0x7d6 0x7d6 0x7d6 0x7d6 0x7d5 0x7d6
0x7d5 0x7d6 0x7d5 0x7d4 0x7d5 0x7d5 0x7d5 0x7d5
0x7d6 0x7d4 0x7d4 0x7d4 0x7d4 0x7d5 0x7d4 0x7d5
0x7d5 0x7d4 0x7d4 0x7d4 0x7d4 0x7d3 0x7d4 0x7d4
*/

#include "../include.h"

#define N_SAMPLES 128
uint16_t sample_buf[N_SAMPLES];

void printhelp()
{
	puts("\nCommands:");
	puts("a\t: Select ADC channel 0 (GPIO26)");
	puts("b\t: Select ADC channel 1 (GPIO27)");
	puts("c\t: Select ADC channel 2 (GPIO28)");
	puts("d\t: Select ADC channel 3 (GPIO29, Vsys/3)");
	puts("e\t: Select temperature sensor");
	puts("s\t: Sample once");
	puts("m\t: Sample many");
	puts("w\t: Wiggle pins");
}

void __not_in_flash_func(adc_capture)(uint16_t *buf, size_t count)
{
	int i;
	adc_fifo_setup(true, false, 0, false, false);
	adc_run(true);
	for (i = 0; i < count; i++)
		buf[i] = adc_fifo_get_blocking();
	adc_run(false);
	adc_fifo_drain();
}

int main()
{
	int i;
	char c;
	uint32_t result;
	const float conversion_factor = 3.3f / (1 << 12);

	stdio_init_all();
	adc_init();
	adc_set_temp_sensor_enabled(true);

	// Set all pins to input (as far as SIO is concerned)
	gpio_set_dir_all_bits(0);
	for (i = 2; i < 30; ++i)
	{
		gpio_set_function(i, GPIO_FUNC_SIO);
		if (i >= 26)
		{
			gpio_disable_pulls(i);
			gpio_set_input_enabled(i, false);
		}
	}

	printf("\n===========================\n");
	printf(  "RP2040 ADC and Test Console\n");
	printf(  "===========================\n");
	printhelp();

	while (1)
	{
		c = getchar();
		printf("%c\n", c);
		switch (c)
		{
		case 'a':
		case 'A':
		case 'b':
		case 'B':
		case 'c':
		case 'C':
		case 'd':
		case 'D':
		case 'e':
		case 'E':
			c = (c - 'A') & 7;
			adc_select_input(c);
			printf("Switched to channel %d\n", c);
			break;

		case 's':
		case 'S':
			result = adc_read();
			printf("0x%03x -> %f V\n", result, result * conversion_factor);
			break;

		case 'm':
		case 'M':
			printf("Starting capture...");
			adc_capture(sample_buf, N_SAMPLES);
			printf("Done\n");
			for (i = 0; i < N_SAMPLES; i++)
			{
				printf("0x%03x ", sample_buf[i]);
				if ((i & 7) == 7) printf("\n");
			}
			break;

		case 'w':
			printf("Press any key to stop wiggling\n");
			i = 1;
			gpio_set_dir_all_bits(-1);
			while (getchar_timeout_us(0) == PICO_ERROR_TIMEOUT)
			{
				// Pattern: Flash all pins for a cycle,
				// Then scan along pins for one cycle each
				i = i ? i << 1 : 1;
				gpio_put_all(i ? i : ~0);
			}
			gpio_set_dir_all_bits(0);
			printf("Wiggling halted.\n");
			break;

		case '\n':
		case '\r':
			break;

		case 'h':
			printhelp();
			break;

		default:
			printf("Unrecognised command: %c\n", c);
			printhelp();
			break;
		}
	}
}
