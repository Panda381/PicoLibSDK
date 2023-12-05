
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************
// Pulse width modulation on PIO. Use it to gradually fade the brightness of an LED.

/*
Debug output
------------
Loaded program at 25
Level = 0
Level = 1
Level = 2
Level = 3
Level = 4
Level = 5
Level = 6
Level = 7
Level = 8
Level = 9
Level = 10
Level = 11
Level = 12
Level = 13
..
*/

#include "../include.h"

// Write 'period' to the input shift register
void pio_pwm_set_period(PIO pio, uint sm, uint32_t period)
{
	pio_sm_set_enabled(pio, sm, false);
	pio_sm_put_blocking(pio, sm, period);
	pio_sm_exec(pio, sm, pio_encode_pull(false, false));
	pio_sm_exec(pio, sm, pio_encode_out(pio_isr, 32));
	pio_sm_set_enabled(pio, sm, true);
}

// Write 'level' to TX FIFO. State machine will copy this into X.
void pio_pwm_set_level(PIO pio, uint sm, uint32_t level)
{
	pio_sm_put_blocking(pio, sm, level);
}

int main()
{
	stdio_init_all();

	PIO pio = pio0;
	int sm = 0;
	uint offset = pio_add_program(pio, &pwm_program);
	printf("Loaded program at %d\n", offset);

	pwm_program_init(pio, sm, offset, 25);
	pio_pwm_set_period(pio, sm, (1u << 16) - 1);

	int level = 0;
	while (true)
	{
		printf("Level = %d\n", level);
		pio_pwm_set_level(pio, sm, level * level);
		level = (level + 1) % 256;
		sleep_ms(10);
	}
}
