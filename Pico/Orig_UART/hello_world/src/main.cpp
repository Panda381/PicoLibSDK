
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************
// Output text to UART0 port.

#include "../include.h"

int main()
{
//	stdio_init_all();
	while (true)
	{
		printf("UART: Hello, world!\n");
		sleep_ms(1000);
	}
}
