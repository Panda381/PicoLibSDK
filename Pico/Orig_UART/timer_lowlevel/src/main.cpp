
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************
// Simplest form of getting 64 bit time from the timer.
// It isn't safe when called from 2 cores because of the latching
// so isn't implemented this way in the sdk

/*
Debug output
------------
Timer lowlevel!
Alarm IRQ fired
Alarm IRQ fired
Alarm IRQ fired
*/

#include "../include.h"

static uint64_t get_time(void)
{
	// Reading low latches the high value
	uint32_t lo = timer_hw->timelr;
	uint32_t hi = timer_hw->timehr;
	return ((uint64_t) hi << 32u) | lo;
}

// Use alarm 0
#define ALARM_NUM 0
#define ALARM_INT TIMER_IRQ_0

// Alarm interrupt handler
static volatile bool alarm_fired;

static void alarm_irq(void)
{
	// Clear the alarm irq
	hw_clear_bits(&timer_hw->intr, 1u << ALARM_NUM);

	// Assume alarm 0 has fired
	printf("Alarm IRQ fired\n");
	alarm_fired = true;
}

static void alarm_in_us(uint32_t delay_us)
{
	// Enable the interrupt for our alarm (the timer outputs 4 alarm irqs)
	hw_set_bits(&timer_hw->inte, 1u << ALARM_NUM);
	// Set irq handler for alarm irq
	irq_set_exclusive_handler(ALARM_INT, alarm_irq);
	// Enable the alarm irq
	irq_set_enabled(ALARM_INT, true);
	// Enable interrupt in block and at processor

	// Alarm is only 32 bits so if trying to delay more
	// than that need to be careful and keep track of the upper
	// bits
	uint64_t target = timer_hw->timerawl + delay_us;

	// Write the lower 32 bits of the target time to the alarm which
	// will arm it
	timer_hw->alarm[ALARM_NUM] = (uint32_t) target;
}

int main()
{
	stdio_init_all();

	printf("Timer lowlevel!\n");

	// Set alarm every 2 seconds
	while (1)
	{
		alarm_fired = false;
		alarm_in_us(1000000 * 2);
		// Wait for alarm to fire
		while (!alarm_fired) {};
	}
}
