
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************
// Set the watchdog timer, and let it expire. Detect the reboot, and halt.

/*
Debug output
------------
Clean boot
Updating watchdog 0
Updating watchdog 1
Updating watchdog 2
Updating watchdog 3
Updating watchdog 4
Waiting to be rebooted by watchdog
Rebooted by Watchdog!
*/

#include "../include.h"

int main()
{
	stdio_init_all();

	if (watchdog_caused_reboot())
	{
		printf("Rebooted by Watchdog!\n");
		return 0;
	}
	else
	{
		// wait to connect terminal
		int c;
		do {
			printf("Press spacebar to start...\n");
			c = getchar();
		} while (c != ' ');

		printf("Clean boot\n");
	}

	// Enable the watchdog, requiring the watchdog to be updated every 100ms or the chip will reboot
	// second arg is pause on debug which means the watchdog will pause when stepping through code
	watchdog_enable(100, 1);

	for (uint i = 0; i < 5; i++)
	{
		printf("Updating watchdog %d\n", i);
		watchdog_update();
	}

	// Wait in an infinite loop and don't update the watchdog so it reboots us
	printf("Waiting to be rebooted by watchdog\n");

	while(1);
}
