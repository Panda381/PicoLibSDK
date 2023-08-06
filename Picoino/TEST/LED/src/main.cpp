
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "../include.h"

int main()
{
	while (True)
	{
#if USE_PICOINOMINI
		LedOn(LED1);
		WaitMs(250);
		if (KeyGet() == KEY_Y) ResetToBootLoader();
		LedOn(LED2);
		WaitMs(250);
		if (KeyGet() == KEY_Y) ResetToBootLoader();

		LedOff(LED1);
		WaitMs(250);
		if (KeyGet() == KEY_Y) ResetToBootLoader();
		LedOff(LED2);
		WaitMs(250);
		if (KeyGet() == KEY_Y) ResetToBootLoader();

#else // USE_PICOINOMINI

		LedOn(LED1);
		WaitMs(250);
		if (KeyGet() == KEY_Y) ResetToBootLoader();

		LedOff(LED1);
		WaitMs(250);
		if (KeyGet() == KEY_Y) ResetToBootLoader();

#endif // USE_PICOINOMINI
	}
}
