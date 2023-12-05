
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************
// Output text to USB-UART port.

#include "../include.h"

int main()
{
//	stdio_init_all();
	while (true)
	{
		printf("USB: Hello, world!\n");
		sleep_ms(1000);
	}
}
