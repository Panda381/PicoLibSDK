
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************
// We're going to use PIO to print "Hello, world!" on the same GPIO which we
// normally attach UART0 to.

#include "../include.h"

int main()
{
	const uint PIN_TX = 0;
	// This is the same as the default UART baud rate on Pico
	const uint SERIAL_BAUD = 115200;

	PIO pio = pio0;
	uint sm = 0;
	uint offset = pio_add_program(pio, &uart_tx_program);
	uart_tx_program_init(pio, sm, offset, PIN_TX, SERIAL_BAUD);

	while (true)
	{
		uart_tx_program_puts(pio, sm, "Hello, world! (from PIO!)\n");
		sleep_ms(1000);
	}
}
