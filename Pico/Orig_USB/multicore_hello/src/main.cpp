
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************
// Multicore hello

/*
Debug output
------------
Hello, multicore!
It's all gone well on core 0!
It's all gone well on core 1!
*/

#include "../include.h"

#define FLAG_VALUE 123

void core1_entry()
{
	multicore_fifo_push_blocking(FLAG_VALUE);

	uint32_t g = multicore_fifo_pop_blocking();

	if (g != FLAG_VALUE)
		printf("Hmm, that's not right on core 1!\n");
	else
		printf("It's all gone well on core 1!\n");

	while (1)
		tight_loop_contents();
}

int main()
{
	stdio_init_all();

	// wait to connect terminal
	int ch;
	do {
		printf("Press spacebar to start...\n");
		ch = getchar();
	} while (ch != ' ');

	printf("Hello, multicore!\n");

	/// \tag::setup_multicore[]

	multicore_launch_core1(core1_entry);

	// Wait for it to start up

	uint32_t g = multicore_fifo_pop_blocking();

	if (g != FLAG_VALUE)
		printf("Hmm, that's not right on core 0!\n");
	else
	{
		multicore_fifo_push_blocking(FLAG_VALUE);
		printf("It's all gone well on core 0!\n");
	}

	// Wait for uart output to finish
	sleep_ms(100);

	return 0;
}
