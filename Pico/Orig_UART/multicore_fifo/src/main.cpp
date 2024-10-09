
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************
// Multicore fifo iqrs.

/*
Debug output
------------
Hello, multicore_fifo_irqs!
Irq handlers should have rx'd some stuff - core 0 got 123, core 1 got 321!
*/

#include "../include.h"

#define FLAG_VALUE1 123
#define FLAG_VALUE2 321

static int core0_rx_val = 0, core1_rx_val = 0;

void core0_sio_irq()
{
	// Just record the latest entry
	while (multicore_fifo_rvalid())
		core0_rx_val = multicore_fifo_pop_blocking();

	multicore_fifo_clear_irq();
}

void core1_sio_irq()
{
	// Just record the latest entry
	while (multicore_fifo_rvalid())
		core1_rx_val = multicore_fifo_pop_blocking();

	multicore_fifo_clear_irq();
}

void core1_entry()
{
	multicore_fifo_clear_irq();

#if RP2040
	irq_set_exclusive_handler(SIO_IRQ_PROC1, core1_sio_irq);
	irq_set_enabled(SIO_IRQ_PROC1, true);
#else
	irq_set_exclusive_handler(IRQ_SIO_FIFO, core1_sio_irq);
	irq_set_enabled(IRQ_SIO_FIFO, true);
#endif

	// Send something to Core0, this should fire the interrupt.
	multicore_fifo_push_blocking(FLAG_VALUE1);

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

	printf("Hello, multicore_fifo_irqs!\n");

	// We MUST start the other core before we enabled FIFO interrupts.
	// This is because the launch uses the FIFO's, enabling interrupts before
	// they are used for the launch will result in unexpected behaviour.
	multicore_launch_core1(core1_entry);

#if RP2040
	irq_set_exclusive_handler(SIO_IRQ_PROC0, core0_sio_irq);
	irq_set_enabled(SIO_IRQ_PROC0, true);
#else
	irq_set_exclusive_handler(IRQ_SIO_FIFO, core0_sio_irq);
	irq_set_enabled(IRQ_SIO_FIFO, true);
#endif

	// Wait for a bit for things to happen
	sleep_ms(10);

	// Send something back to the other core
	multicore_fifo_push_blocking(FLAG_VALUE2);

	// Wait for a bit for things to happen
	sleep_ms(10);

	printf("Irq handlers should have rx'd some stuff - core 0 got %d, core 1 got %d!\n",
		core0_rx_val, core1_rx_val);

	while (1)
		tight_loop_contents();
}
