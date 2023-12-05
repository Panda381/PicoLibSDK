
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************
// Set up some PIO state machines to blink LEDs at different frequencies,
// according to delay counts pushed into their FIFOs.

#include "../include.h"

void blink_pin_forever(PIO pio, uint sm, uint offset, uint pin, uint freq)
{
	blink_program_init(pio, sm, offset, pin);
	pio_sm_set_enabled(pio, sm, true);

	printf("Blinking pin %d at freq %d\n", pin, freq);
	pio->txf[sm] = 24000000 / freq;
}

int main()
{
	// wait to connect terminal
	int c;
	do {
		printf("Press spacebar to start...\n");
		c = getchar();
	} while (c != ' ');

	// todo get free sm
	PIO pio = pio0;
	uint offset = pio_add_program(pio, &blink_program);
	printf("Loaded program at %d\n", offset);

	blink_pin_forever(pio, 0, offset, 25, 3);
	blink_pin_forever(pio, 1, offset, 6, 4);

	// Wait for uart output to finish
	sleep_ms(100);
}
