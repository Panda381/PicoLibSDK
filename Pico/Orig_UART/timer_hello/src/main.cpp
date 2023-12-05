
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************
// Set callbacks on the system timer, which repeat at regular intervals.
// Cancel the timer when we're done.

/*
Debug output
------------
Hello Timer!
Timer 1 fired!
Repeat at 11742807
Repeat at 12242881
Repeat at 12742915
Repeat at 13242949
Repeat at 13742983
cancelled... 1
Repeat at 16742896
Repeat at 17242896
Repeat at 17742896
Repeat at 18242895
Repeat at 18742896
Repeat at 19242895
cancelled... 1
Done
*/

#include "../include.h"

volatile bool timer_fired = false;

int64_t alarm_callback(alarm_id_t id, void *user_data)
{
	printf("Timer %d fired!\n", (int) id);
	timer_fired = true;
	// Can return a value here in us to fire in the future
	return 0;
}

bool repeating_timer_callback(struct repeating_timer *t)
{
	printf("Repeat at %lld\n", time_us_64());
	return true;
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

	printf("Hello Timer!\n");

	// Call alarm_callback in 2 seconds
	add_alarm_in_ms(2000, alarm_callback, NULL, false);

	// Wait for alarm callback to set timer_fired
	while (!timer_fired)
	{
		tight_loop_contents();
	}

	// Create a repeating timer that calls repeating_timer_callback.
	// If the delay is > 0 then this is the delay between the previous callback ending and the next starting.
	// If the delay is negative (see below) then the next call to the callback will be exactly 500ms after the
	// start of the call to the last callback
	struct repeating_timer timer;
	add_repeating_timer_ms(500, repeating_timer_callback, NULL, &timer);
	sleep_ms(3000);
	bool cancelled = cancel_repeating_timer(&timer);
	printf("cancelled... %d\n", cancelled);
	sleep_ms(2000);

	// Negative delay so means we will call repeating_timer_callback, and call it again
	// 500ms later regardless of how long the callback took to execute
	add_repeating_timer_ms(-500, repeating_timer_callback, NULL, &timer);
	sleep_ms(3000);
	cancelled = cancel_repeating_timer(&timer);
	printf("cancelled... %d\n", cancelled);
	sleep_ms(2000);
	printf("Done\n");

	// Wait for uart output to finish
	sleep_ms(100);

	return 0;
}
