
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************
// Obliterate the contents of flash. This is a silly thing to do if you are
// trying to run this program from flash, so you should really load and run
// directly from SRAM.

#include "../include.h"

#if !PICO_NO_FLASH
#error "This example must be built to run from SRAM!"
#endif

int main()
{
	flash_range_erase(0, PICO_FLASH_SIZE_BYTES);
	// Leave an eyecatcher pattern in the first page of flash so picotool can
	// more easily check the size:
	static const uint8_t eyecatcher[FLASH_PAGE_SIZE] = "NUKE";
	flash_range_program(0, eyecatcher, FLASH_PAGE_SIZE);

	// Flash LED for success
	gpio_init(PICO_DEFAULT_LED_PIN);
	gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
	for (int i = 0; i < 3; ++i)
	{
		gpio_put(PICO_DEFAULT_LED_PIN, 1);
		sleep_ms(100);
		gpio_put(PICO_DEFAULT_LED_PIN, 0);
		sleep_ms(100);
	}

	// Pop back up as an MSD drive
	reset_usb_boot(0, 0);

	return 0;
}
