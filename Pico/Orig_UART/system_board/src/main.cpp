
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************
// RP2040 does not have a unique on-board ID, but this is a standard feature
// on the NOR flash it boots from. There is a 1:1 association between RP2040
// and the flash, so this can be used to get a 64 bit globally unique board ID
// for an RP2040-based board, including Pico.
//
// The pico_unique_id library retrieves this unique identifier during boot and
// stores it in memory, where it can be accessed at any time without
// disturbing the flash XIP hardware.

/*
Debug output
------------
Unique identifier: e6 60 38 b7 13 6c 89 39
*/

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

	pico_unique_board_id_t board_id;
	pico_get_unique_board_id(&board_id);

	printf("Unique identifier:");
	for (int i = 0; i < PICO_UNIQUE_BOARD_ID_SIZE_BYTES; ++i)
	{
		printf(" %02x", board_id.id[i]);
	}
	printf("\n");

	// Wait for uart output to finish
	sleep_ms(100);
}
