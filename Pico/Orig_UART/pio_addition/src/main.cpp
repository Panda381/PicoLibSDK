
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************
// Do some PIO additions

/*
Debug output
------------
Doing some random additions:
33 + 43 = 76
62 + 29 = 91
0 + 8 = 8
52 + 56 = 108
56 + 19 = 75
11 + 51 = 62
43 + 5 = 48
8 + 93 = 101
30 + 66 = 96
69 + 32 = 101
*/

#include "../include.h"

// Pop quiz: how many additions does the processor do when calling this function
uint32_t do_addition(PIO pio, uint sm, uint32_t a, uint32_t b)
{
	pio_sm_put_blocking(pio, sm, a);
	pio_sm_put_blocking(pio, sm, b);
	return pio_sm_get_blocking(pio, sm);
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

	PIO pio = pio0;
	uint sm = 0;
	uint offset = pio_add_program(pio, &addition_program);
	addition_program_init(pio, sm, offset);

	printf("Doing some random additions:\n");
	for (int i = 0; i < 10; ++i)
	{
		uint a = rand() % 100;
		uint b = rand() % 100;
		printf("%u + %u = %u\n", a, b, do_addition(pio, sm, a, b));
	}

	// Wait for uart output to finish
	sleep_ms(100);

	return 0;
}
