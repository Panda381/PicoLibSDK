
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************
// Print some text from one of the UART serial ports, without going through stdio.
// We are using pins 0 and 1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.

/*
Debug output
------------
AB Hello, UART!
*/

#include "../include.h"

#define UART_ID uart0
#define BAUD_RATE 115200

#define UART_TX_PIN 0
#define UART_RX_PIN 1

int main()
{
	// Set up our UART with the required speed.
	uart_init(UART_ID, BAUD_RATE);

	// Set the TX and RX pins by using the function select on the GPIO
	// Set datasheet for more information on function select
	gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
	gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

	// Use some the various UART functions to send out data
	// In a default system, printf will also output via the default UART

	// Send out a character without any conversions
	uart_putc_raw(UART_ID, 'A');

	// Send out a character but do CR/LF conversions
	uart_putc(UART_ID, 'B');

	// Send out a string, with CR/LF conversions
	uart_puts(UART_ID, " Hello, UART!\n");

	// Wait for uart output to finish
	sleep_ms(100);

	return 0;
}
