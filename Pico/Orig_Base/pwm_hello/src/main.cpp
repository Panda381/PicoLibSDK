
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************
// Output PWM signals on pins 0 and 1

#include "../include.h"

int main()
{
	// Tell GPIO 0 and 1 they are allocated to the PWM
	gpio_set_function(0, GPIO_FUNC_PWM);
	gpio_set_function(1, GPIO_FUNC_PWM);

	// Find out which PWM slice is connected to GPIO 0 (it's slice 0)
	uint slice_num = pwm_gpio_to_slice_num(0);

	// Set period of 4 cycles (0 to 3 inclusive)
	pwm_set_wrap(slice_num, 3);
	// Set channel A output high for one cycle before dropping
	pwm_set_chan_level(slice_num, PWM_CHAN_A, 1);
	// Set initial B output high for three cycles before dropping
	pwm_set_chan_level(slice_num, PWM_CHAN_B, 3);
	// Set the PWM running
	pwm_set_enabled(slice_num, true);

	// Note we could also use pwm_set_gpio_level(gpio, x) which looks up the
	// correct slice and channel for a given GPIO.
}
