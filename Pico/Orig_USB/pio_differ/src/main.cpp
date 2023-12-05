
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************
// Differential serial transmit/receive example
// Need to connect a wire from GPIO2 -> GPIO3

/*
Debug output
------------
Connect GPIO2 with GPIO3 and press spacebar to start...
Transmit program loaded at 24
Receive program loaded at 14
00000000
0ff0a55a
12345678
*/

#include "../include.h"

const uint pin_tx = 2;
const uint pin_rx = 3;

int main()
{
	stdio_init_all();

	// wait to connect terminal
	int c;
	do {
		printf("Connect GPIO2 with GPIO3 and press spacebar to start...\n");
		c = getchar();
	} while (c != ' ');

	PIO pio = pio0;
	uint sm_tx = 0;
	uint sm_rx = 1;

	uint offset_tx = pio_add_program(pio, &differential_manchester_tx_program);
	uint offset_rx = pio_add_program(pio, &differential_manchester_rx_program);
	printf("Transmit program loaded at %d\n", offset_tx);
	printf("Receive program loaded at %d\n", offset_rx);

	// Configure state machines, set bit rate at 5 Mbps
	differential_manchester_tx_program_init(pio, sm_tx, offset_tx, pin_tx, 125.f / (16 * 5));
	differential_manchester_rx_program_init(pio, sm_rx, offset_rx, pin_rx, 125.f / (16 * 5));

	pio_sm_set_enabled(pio, sm_tx, false);
	pio_sm_put_blocking(pio, sm_tx, 0);
	pio_sm_put_blocking(pio, sm_tx, 0x0ff0a55a);
	pio_sm_put_blocking(pio, sm_tx, 0x12345678);
	pio_sm_set_enabled(pio, sm_tx, true);

	for (int i = 0; i < 3; ++i)
		printf("%08x\n", pio_sm_get_blocking(pio, sm_rx));

	// Wait for uart output to finish
	sleep_ms(100);

	return 0;
}
