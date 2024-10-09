
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************
// Perform a hard reset on some peripherals, then bring them back up.

#include "../include.h"

int main()
{
	stdio_init_all();

	// wait to connect terminal
	int c;
	do {
		printf("Press spacebar to start...\n");
		c = getchar();
	} while (c != ' ');

	printf("Hello, reset!\n");

	// Wait for uart output to finish
	sleep_ms(100);

	printf("PWM reset...");

	// Put the PWM block into reset
	reset_block(RESETS_RESET_PWM_BITS);

	// And bring it out
	unreset_block_wait(RESETS_RESET_PWM_BITS);

	printf("done\n");

#if RP2040
	printf("PWM and RTC reset...");

	// Put the PWM and RTC block into reset
	reset_block(RESETS_RESET_PWM_BITS | RESETS_RESET_RTC_BITS);

	// Wait for both to come out of reset
	unreset_block_wait(RESETS_RESET_PWM_BITS | RESETS_RESET_RTC_BITS);

	printf("done\n");
#endif

	return 0;
}
